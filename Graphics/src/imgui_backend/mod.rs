use std::{borrow::BorrowMut, error::Error, fmt, mem::size_of, num::NonZeroU32};

use imgui::{
    Context, DrawCmd::Elements, DrawData, DrawIdx, DrawList, DrawVert, TextureId, Textures,
};
use smallvec::SmallVec;
use wgpu::{
    util::{BufferInitDescriptor, DeviceExt},
    *,
};

static VS_ENTRY_POINT: &str = "vs_main";
static FS_ENTRY_POINT_LINEAR: &str = "fs_main_linear";
static FS_ENTRY_POINT_SRGB: &str = "fs_main_srgb";

pub type RendererResult<T> = Result<T, RendererError>;

#[repr(transparent)]
#[derive(Debug, Copy, Clone)]
struct DrawVertPod(DrawVert);

unsafe impl bytemuck::Zeroable for DrawVertPod {}

unsafe impl bytemuck::Pod for DrawVertPod {}

#[derive(Clone, Debug)]
pub enum RendererError {
    BadTexture(TextureId),
}

impl fmt::Display for RendererError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match *self {
            RendererError::BadTexture(id) => {
                write!(f, "imgui render error: bad texture id '{}'", id.id())
            }
        }
    }
}

impl Error for RendererError {}

#[allow(dead_code)]
enum ShaderStage {
    Vertex,
    Fragment,
    Compute,
}

/// Config for creating a texture.
///
/// Uses the builder pattern.
#[derive(Clone)]
pub struct TextureConfig<'a> {
    /// The size of the texture.
    pub size: Extent3d,
    /// An optional label for the texture used for debugging.
    pub label: Option<&'a str>,
    /// The format of the texture, if not set uses the format from the renderer.
    pub format: Option<TextureFormat>,
    /// The usage of the texture.
    pub usage: TextureUsages,
    /// The mip level of the texture.
    pub mip_level_count: u32,
    /// The sample count of the texture.
    pub sample_count: u32,
    /// The dimension of the texture.
    pub dimension: TextureDimension,
}

impl<'a> Default for TextureConfig<'a> {
    /// Create a new texture config.
    fn default() -> Self {
        Self {
            size: Extent3d { width: 0, height: 0, depth_or_array_layers: 1 },
            label: None,
            format: None,
            usage: TextureUsages::TEXTURE_BINDING | TextureUsages::COPY_DST,
            mip_level_count: 1,
            sample_count: 1,
            dimension: TextureDimension::D2,
        }
    }
}

/// A container for a bindable texture.
pub struct Texture {
    texture: wgpu::Texture,
    view: wgpu::TextureView,
    bind_group: BindGroup,
    size: Extent3d,
}

impl Texture {
    /// Create a `Texture` from its raw parts.
    pub fn from_raw_parts(
        texture: wgpu::Texture,
        view: wgpu::TextureView,
        bind_group: BindGroup,
        size: Extent3d,
    ) -> Self {
        Self { texture, view, bind_group, size }
    }

    /// Create a new GPU texture width the specified `config`.
    pub fn new(device: &Device, renderer: &Renderer, config: TextureConfig) -> Self {
        // Create the wgpu texture.
        let texture = device.create_texture(&TextureDescriptor {
            label: config.label,
            size: config.size,
            mip_level_count: config.mip_level_count,
            sample_count: config.sample_count,
            dimension: config.dimension,
            format: config.format.unwrap_or(renderer.config.texture_format),
            usage: config.usage,
        });

        // Extract the texture view.
        let view = texture.create_view(&TextureViewDescriptor::default());

        // Create the texture sampler.
        let sampler = device.create_sampler(&SamplerDescriptor {
            label: Some("imgui-wgpu sampler"),
            address_mode_u: AddressMode::ClampToEdge,
            address_mode_v: AddressMode::ClampToEdge,
            address_mode_w: AddressMode::ClampToEdge,
            mag_filter: FilterMode::Linear,
            min_filter: FilterMode::Linear,
            mipmap_filter: FilterMode::Linear,
            lod_min_clamp: 0.0,
            lod_max_clamp: f32::MAX,
            compare: None,
            anisotropy_clamp: None,
            border_color: None,
        });

        // Create the texture bind group from the layout.
        let bind_group = device.create_bind_group(&BindGroupDescriptor {
            label: config.label,
            layout: &renderer.texture_layout,
            entries: &[
                BindGroupEntry { binding: 0, resource: BindingResource::TextureView(&view) },
                BindGroupEntry { binding: 1, resource: BindingResource::Sampler(&sampler) },
            ],
        });

        Self { texture, view, bind_group, size: config.size }
    }

    /// Write `data` to the texture.
    ///
    /// - `data`: 32-bit RGBA bitmap data.
    /// - `width`: The width of the source bitmap (`data`) in pixels.
    /// - `height`: The height of the source bitmap (`data`) in pixels.
    pub fn write(&self, queue: &Queue, data: &[u8], width: u32, height: u32) {
        queue.write_texture(
            // destination (sub)texture
            ImageCopyTexture {
                texture: &self.texture,
                mip_level: 0,
                origin: Origin3d { x: 0, y: 0, z: 0 },
                aspect: TextureAspect::All,
            },
            // source bitmap data
            data,
            // layout of the source bitmap
            ImageDataLayout {
                offset: 0,
                bytes_per_row: NonZeroU32::new(width * 4),
                rows_per_image: NonZeroU32::new(height),
            },
            // size of the source bitmap
            Extent3d { width, height, depth_or_array_layers: 1 },
        );
    }

    /// The width of the texture in pixels.
    pub fn width(&self) -> u32 { self.size.width }

    /// The height of the texture in pixels.
    pub fn height(&self) -> u32 { self.size.height }

    /// The depth of the texture.
    pub fn depth(&self) -> u32 { self.size.depth_or_array_layers }

    /// The size of the texture in pixels.
    pub fn size(&self) -> Extent3d { self.size }

    /// The underlying `wgpu::Texture`.
    pub fn texture(&self) -> &wgpu::Texture { &self.texture }

    /// The `wgpu::TextureView` of the underlying texture.
    pub fn view(&self) -> &wgpu::TextureView { &self.view }
}

pub struct CombinedShader<'s> {
    pub shader: ShaderModuleDescriptor<'s>,
}

pub struct SeparateShaders<'s> {
    pub vertex_shader: ShaderModuleDescriptorSpirV<'s>,
    pub fragment_shader: ShaderModuleDescriptorSpirV<'s>,
}

pub enum ShaderType<'s> {
    None,
    Combined(CombinedShader<'s>),
    Separate(SeparateShaders<'s>),
}

/// Configuration for the renderer.
pub struct RendererConfig<'s> {
    pub texture_format: TextureFormat,
    pub depth_format: Option<TextureFormat>,
    pub sample_count: u32,
    pub shader: ShaderType<'s>,
    pub vertex_shader_entry_point: String,
    pub fragment_shader_entry_point: String,
}

impl RendererConfig<'_> {
    /// Create a new renderer config with custom shaders.
    pub fn with_combined_shaders<'s>(
        shader: ShaderModuleDescriptor<'s>,
        fragment_shader_entry_point: &str,
    ) -> RendererConfig<'s> {
        RendererConfig {
            texture_format: TextureFormat::Rgba8Unorm,
            depth_format: None,
            sample_count: 1,
            shader: ShaderType::Combined(CombinedShader { shader }),
            vertex_shader_entry_point: VS_ENTRY_POINT.to_string(),
            fragment_shader_entry_point: fragment_shader_entry_point.to_string(),
        }
    }

    /// Create a new renderer config with custom shaders.
    pub fn with_separate_shaders<'s>(
        vertex_shader: ShaderModuleDescriptorSpirV<'s>,
        fragment_shader: ShaderModuleDescriptorSpirV<'s>,
        fragment_shader_entry_point: &str,
    ) -> RendererConfig<'s> {
        RendererConfig {
            texture_format: TextureFormat::Rgba8Unorm,
            depth_format: None,
            sample_count: 1,
            shader: ShaderType::Separate(SeparateShaders { vertex_shader, fragment_shader }),
            vertex_shader_entry_point: VS_ENTRY_POINT.to_string(),
            fragment_shader_entry_point: fragment_shader_entry_point.to_string(),
        }
    }
}

impl Default for RendererConfig<'_> {
    /// Create a new renderer config with precompiled default shaders outputting linear color.
    ///
    /// If you write to a Bgra8UnormSrgb framebuffer, this is what you want.
    fn default() -> Self { Self::new() }
}

impl RendererConfig<'_> {
    /// Create a new renderer config with precompiled default shaders outputting linear color.
    ///
    /// If you write to a Bgra8UnormSrgb framebuffer, this is what you want.
    pub fn new() -> Self {
        Self::with_combined_shaders(include_wgsl!("imgui.wgsl"), FS_ENTRY_POINT_LINEAR)
    }

    /// Create a new renderer config with precompiled default shaders outputting srgb color.
    ///
    /// If you write to a Bgra8Unorm framebuffer, this is what you want.
    pub fn new_srgb() -> Self {
        Self::with_combined_shaders(include_wgsl!("imgui.wgsl"), FS_ENTRY_POINT_SRGB)
    }
}

pub struct Renderer {
    pipeline: RenderPipeline,
    uniform_buffer: Buffer,
    uniform_bind_group: BindGroup,
    /// Textures of the font atlas and all images.
    pub textures: Textures<Texture>,
    texture_layout: BindGroupLayout,
    index_buffers: SmallVec<[Buffer; 4]>,
    vertex_buffers: SmallVec<[Buffer; 4]>,
    config: RendererConfig<'static>,
}

impl Renderer {
    /// Create an entirely new imgui wgpu renderer.
    pub fn new(
        imgui: &mut Context,
        device: &Device,
        queue: &Queue,
        config: RendererConfig,
    ) -> Self {
        let RendererConfig {
            texture_format,
            depth_format,
            sample_count,
            shader,
            vertex_shader_entry_point,
            fragment_shader_entry_point,
        } = config;

        // Load shaders.
        let shader1: Option<wgpu::ShaderModule>;
        let shader2: Option<wgpu::ShaderModule>;
        let (vs, fs) = match shader {
            ShaderType::Combined(d) => {
                shader1 = Some(device.create_shader_module(&d.shader));
                let r = shader1.as_ref().unwrap();
                (r, r)
            }
            ShaderType::Separate(d) => unsafe {
                shader1 = Some(device.create_shader_module_spirv(&d.vertex_shader));
                shader2 = Some(device.create_shader_module_spirv(&d.fragment_shader));
                (shader1.as_ref().unwrap(), shader2.as_ref().unwrap())
            },
            ShaderType::None => panic!(),
        };

        // Create the uniform matrix buffer.
        let size = 64;
        let uniform_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("imgui-wgpu uniform buffer"),
            size,
            usage: BufferUsages::UNIFORM | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        // Create the uniform matrix buffer bind group layout.
        let uniform_layout = device.create_bind_group_layout(&BindGroupLayoutDescriptor {
            label: None,
            entries: &[BindGroupLayoutEntry {
                binding: 0,
                visibility: wgpu::ShaderStages::VERTEX,
                ty: BindingType::Buffer {
                    ty: BufferBindingType::Uniform,
                    has_dynamic_offset: false,
                    min_binding_size: None,
                },
                count: None,
            }],
        });

        // Create the uniform matrix buffer bind group.
        let uniform_bind_group = device.create_bind_group(&BindGroupDescriptor {
            label: Some("imgui-wgpu bind group"),
            layout: &uniform_layout,
            entries: &[BindGroupEntry { binding: 0, resource: uniform_buffer.as_entire_binding() }],
        });

        // Create the texture layout for further usage.
        let texture_layout = device.create_bind_group_layout(&BindGroupLayoutDescriptor {
            label: Some("imgui-wgpu bind group layout"),
            entries: &[
                BindGroupLayoutEntry {
                    binding: 0,
                    visibility: wgpu::ShaderStages::FRAGMENT,
                    ty: BindingType::Texture {
                        multisampled: false,
                        sample_type: TextureSampleType::Float { filterable: true },
                        view_dimension: TextureViewDimension::D2,
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 1,
                    visibility: wgpu::ShaderStages::FRAGMENT,
                    ty: BindingType::Sampler(wgpu::SamplerBindingType::Filtering),
                    count: None,
                },
            ],
        });

        // Create the render pipeline layout.
        let pipeline_layout = device.create_pipeline_layout(&PipelineLayoutDescriptor {
            label: Some("imgui-wgpu pipeline layout"),
            bind_group_layouts: &[&uniform_layout, &texture_layout],
            push_constant_ranges: &[],
        });

        // Create the render pipeline.
        // Create the render pipeline.
        let pipeline = device.create_render_pipeline(&RenderPipelineDescriptor {
            label: Some("imgui-wgpu pipeline"),
            layout: Some(&pipeline_layout),
            vertex: VertexState {
                module: vs,
                entry_point: &vertex_shader_entry_point,
                buffers: &[VertexBufferLayout {
                    array_stride: size_of::<DrawVert>() as BufferAddress,
                    step_mode: VertexStepMode::Vertex,
                    attributes: &vertex_attr_array![0 => Float32x2, 1 => Float32x2, 2 => Unorm8x4],
                }],
            },
            primitive: PrimitiveState {
                topology: PrimitiveTopology::TriangleList,
                strip_index_format: None,
                front_face: FrontFace::Cw,
                cull_mode: None,
                polygon_mode: PolygonMode::Fill,
                unclipped_depth: false,
                conservative: false,
            },
            depth_stencil: depth_format.map(|format| wgpu::DepthStencilState {
                format,
                depth_write_enabled: false,
                depth_compare: wgpu::CompareFunction::Always,
                stencil: wgpu::StencilState::default(),
                bias: DepthBiasState::default(),
            }),
            multisample: MultisampleState { count: sample_count, ..Default::default() },
            fragment: Some(FragmentState {
                module: fs,
                entry_point: &fragment_shader_entry_point,
                targets: &[ColorTargetState {
                    format: texture_format,
                    blend: Some(BlendState {
                        color: BlendComponent {
                            src_factor: BlendFactor::SrcAlpha,
                            dst_factor: BlendFactor::OneMinusSrcAlpha,
                            operation: BlendOperation::Add,
                        },
                        alpha: BlendComponent {
                            src_factor: BlendFactor::OneMinusDstAlpha,
                            dst_factor: BlendFactor::One,
                            operation: BlendOperation::Add,
                        },
                    }),
                    write_mask: ColorWrites::ALL,
                }],
            }),
            multiview: None,
        });

        let mut renderer = Self {
            pipeline,
            uniform_buffer,
            uniform_bind_group,
            textures: Textures::new(),
            texture_layout,
            vertex_buffers: SmallVec::new(),
            index_buffers: SmallVec::new(),
            config: RendererConfig {
                texture_format,
                depth_format,
                sample_count,
                shader: ShaderType::None,
                vertex_shader_entry_point,
                fragment_shader_entry_point,
            },
        };

        // Immediately load the font texture to the GPU.
        renderer.reload_font_texture(imgui, device, queue);

        renderer
    }

    /// Render the current imgui frame.
    pub fn render<'r>(
        &'r mut self,
        draw_data: &DrawData,
        queue: &Queue,
        device: &Device,
        rpass: &mut RenderPass<'r>,
    ) -> RendererResult<()> {
        rpass.push_debug_group("imgui-wgpu");
        let mut rpass = scopeguard::guard(rpass, |rpass| {
            rpass.pop_debug_group();
        });

        let fb_width = draw_data.display_size[0] * draw_data.framebuffer_scale[0];
        let fb_height = draw_data.display_size[1] * draw_data.framebuffer_scale[1];

        // If the render area is <= 0, exit here and now.
        if !(fb_width > 0.0 && fb_height > 0.0) {
            return Ok(());
        }

        let width = draw_data.display_size[0];
        let height = draw_data.display_size[1];

        let offset_x = draw_data.display_pos[0] / width;
        let offset_y = draw_data.display_pos[1] / height;

        // Create and update the transform matrix for the current frame.
        // This is required to adapt to vulkan coordinates.
        // let matrix = [
        //     [2.0 / width, 0.0, 0.0, 0.0],
        //     [0.0, 2.0 / height as f32, 0.0, 0.0],
        //     [0.0, 0.0, -1.0, 0.0],
        //     [-1.0, -1.0, 0.0, 1.0],
        // ];
        let matrix = [
            [2.0 / width, 0.0, 0.0, 0.0],
            [0.0, 2.0 / -height as f32, 0.0, 0.0],
            [0.0, 0.0, 1.0, 0.0],
            [-1.0 - offset_x * 2.0, 1.0 + offset_y * 2.0, 0.0, 1.0],
        ];
        self.update_uniform_buffer(queue, &matrix);

        rpass.set_pipeline(&self.pipeline);
        rpass.set_bind_group(0, &self.uniform_bind_group, &[]);

        self.vertex_buffers.clear();
        self.index_buffers.clear();

        for draw_list in draw_data.draw_lists() {
            self.vertex_buffers.push(self.upload_vertex_buffer(device, draw_list.vtx_buffer()));
            self.index_buffers.push(self.upload_index_buffer(device, draw_list.idx_buffer()));
        }

        // Execute all the imgui render work.
        for (draw_list_buffers_index, draw_list) in draw_data.draw_lists().enumerate() {
            self.render_draw_list(
                rpass.borrow_mut(),
                draw_list,
                draw_data.display_pos,
                draw_data.framebuffer_scale,
                draw_list_buffers_index,
            )?;
        }

        Ok(())
    }

    /// Render a given `DrawList` from imgui onto a wgpu frame.
    fn render_draw_list<'render>(
        &'render self,
        rpass: &mut RenderPass<'render>,
        draw_list: &DrawList,
        clip_off: [f32; 2],
        clip_scale: [f32; 2],
        draw_list_buffers_index: usize,
    ) -> RendererResult<()> {
        let mut start = 0;

        let index_buffer = &self.index_buffers[draw_list_buffers_index];
        let vertex_buffer = &self.vertex_buffers[draw_list_buffers_index];

        // Make sure the current buffers are attached to the render pass.
        rpass.set_index_buffer(index_buffer.slice(..), IndexFormat::Uint16);
        rpass.set_vertex_buffer(0, vertex_buffer.slice(..));

        for cmd in draw_list.commands() {
            if let Elements { count, cmd_params } = cmd {
                let clip_rect = [
                    (cmd_params.clip_rect[0] - clip_off[0]) * clip_scale[0],
                    (cmd_params.clip_rect[1] - clip_off[1]) * clip_scale[1],
                    (cmd_params.clip_rect[2] - clip_off[0]) * clip_scale[0],
                    (cmd_params.clip_rect[3] - clip_off[1]) * clip_scale[1],
                ];

                // Set the current texture bind group on the renderpass.
                let texture_id = cmd_params.texture_id;
                let tex =
                    self.textures.get(texture_id).ok_or(RendererError::BadTexture(texture_id))?;
                rpass.set_bind_group(1, &tex.bind_group, &[]);

                // Set scissors on the renderpass.
                let scissors = (
                    clip_rect[0].max(0.0).floor() as u32,
                    clip_rect[1].max(0.0).floor() as u32,
                    (clip_rect[2] - clip_rect[0]).abs().ceil() as u32,
                    (clip_rect[3] - clip_rect[1]).abs().ceil() as u32,
                );
                rpass.set_scissor_rect(scissors.0, scissors.1, scissors.2, scissors.3);

                // Draw the current batch of vertices with the renderpass.
                let end = start + count as u32;
                rpass.draw_indexed(start..end, 0, 0..1);
                start = end;
            }
        }
        Ok(())
    }

    /// Updates the current uniform buffer containing the transform matrix.
    fn update_uniform_buffer(&mut self, queue: &Queue, matrix: &[[f32; 4]; 4]) {
        let data = bytemuck::bytes_of(matrix);

        queue.write_buffer(&self.uniform_buffer, 0, data);
    }

    /// Upload the vertex buffer to the GPU.
    fn upload_vertex_buffer(&self, device: &Device, vertices: &[DrawVert]) -> Buffer {
        // Safety: DrawVertPod is #[repr(transparent)] over DrawVert and DrawVert _should_ be Pod.
        let vertices = unsafe {
            std::slice::from_raw_parts(vertices.as_ptr() as *mut DrawVertPod, vertices.len())
        };

        let data = bytemuck::cast_slice(vertices);
        device.create_buffer_init(&BufferInitDescriptor {
            label: Some("imgui-wgpu vertex buffer"),
            contents: data,
            usage: BufferUsages::VERTEX,
        })
    }

    /// Upload the index buffer to the GPU.
    fn upload_index_buffer(&self, device: &Device, indices: &[DrawIdx]) -> Buffer {
        let data = bytemuck::cast_slice(indices);

        device.create_buffer_init(&BufferInitDescriptor {
            label: Some("imgui-wgpu index buffer"),
            contents: data,
            usage: BufferUsages::INDEX,
        })
    }

    /// Updates the texture on the GPU corresponding to the current imgui font atlas.
    ///
    /// This has to be called after loading a font.
    pub fn reload_font_texture(&mut self, imgui: &mut Context, device: &Device, queue: &Queue) {
        let mut fonts = imgui.fonts();
        // Remove possible font atlas texture.
        self.textures.remove(fonts.tex_id);

        // Create font texture and upload it.
        let handle = fonts.build_rgba32_texture();
        let font_texture_cnfig = TextureConfig {
            label: Some("imgui-wgpu font atlas"),
            size: Extent3d { width: handle.width, height: handle.height, ..Default::default() },
            format: Some(wgpu::TextureFormat::Rgba8Unorm),
            ..Default::default()
        };

        let font_texture = Texture::new(device, self, font_texture_cnfig);
        font_texture.write(queue, handle.data, handle.width, handle.height);
        fonts.tex_id = self.textures.insert(font_texture);
        // Clear imgui texture data to save memory.
        fonts.clear_tex_data();
    }
}
