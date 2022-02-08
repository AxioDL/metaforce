use std::{collections::HashMap, hash::Hash, ops::Range};

use bytemuck_derive::{Pod, Zeroable};
use wgpu::{include_wgsl, vertex_attr_array};

use crate::{
    get_app,
    gpu::GraphicsConfig,
    shaders::{
        bind_pipeline,
        ffi::{CameraFilterType, TextureRef, ZTest},
        get_combined_matrix, pipeline_ref, push_draw_command, push_uniform, push_verts,
        texture::create_sampler,
        BuiltBuffers, PipelineCreateCommand, PipelineHolder, PipelineRef, ShaderDrawCommand, STATE,
    },
    util::{align, Vec2, Vec3},
    zeus::{CColor, CMatrix4f, CRectangle, CVector2f, CVector3f, CVector4f},
};

#[derive(Debug, Clone)]
pub(crate) struct DrawData {
    pipeline: PipelineRef,
    vert_range: Range<u64>,
    uniform_range: Range<u64>,
    texture: TextureRef,
}

#[derive(Hash)]
pub(crate) struct PipelineConfig {
    filter_type: CameraFilterType,
    z_test: ZTest,
}
pub(crate) const INITIAL_PIPELINES: &[PipelineCreateCommand] = &[
    // PipelineCreateCommand::TexturedQuad(PipelineConfig { z_only: false }),
    // PipelineCreateCommand::TexturedQuad(PipelineConfig { z_only: true }),
];

pub(crate) struct State {
    shader: wgpu::ShaderModule,
    uniform_layout: wgpu::BindGroupLayout,
    uniform_bind_group: wgpu::BindGroup,
    texture_layout: wgpu::BindGroupLayout,
    sampler: wgpu::Sampler,
    pipeline_layout: wgpu::PipelineLayout,
    // Transient state
    texture_bind_groups: HashMap<u32, wgpu::BindGroup>,
    frame_used_textures: Vec<u32>, // TODO use to clear bind groups
}

pub(crate) fn construct_state(
    device: &wgpu::Device,
    _queue: &wgpu::Queue,
    buffers: &BuiltBuffers,
    graphics_config: &GraphicsConfig,
) -> State {
    let shader = device.create_shader_module(&include_wgsl!("shader.wgsl"));
    let uniform_alignment = device.limits().min_uniform_buffer_offset_alignment;
    let uniform_size = wgpu::BufferSize::new(align(
        std::mem::size_of::<Uniform>() as u64,
        uniform_alignment as u64,
    ));
    let uniform_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
        label: Some("Textured Quad Uniform Bind Group Layout"),
        entries: &[
            wgpu::BindGroupLayoutEntry {
                binding: 0,
                visibility: wgpu::ShaderStages::VERTEX_FRAGMENT,
                ty: wgpu::BindingType::Buffer {
                    ty: wgpu::BufferBindingType::Uniform,
                    has_dynamic_offset: true,
                    min_binding_size: uniform_size,
                },
                count: None,
            },
            wgpu::BindGroupLayoutEntry {
                binding: 1,
                visibility: wgpu::ShaderStages::FRAGMENT,
                ty: wgpu::BindingType::Sampler(wgpu::SamplerBindingType::Filtering),
                count: None,
            },
        ],
    });
    let sampler = create_sampler(device, wgpu::AddressMode::Repeat, None);
    let uniform_bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: Some("Textured Quad Uniform Bind Group"),
        layout: &uniform_layout,
        entries: &[
            wgpu::BindGroupEntry {
                binding: 0,
                resource: wgpu::BindingResource::Buffer(wgpu::BufferBinding {
                    buffer: &buffers.uniform_buffer,
                    offset: 0, // dynamic
                    size: uniform_size,
                }),
            },
            wgpu::BindGroupEntry { binding: 1, resource: wgpu::BindingResource::Sampler(&sampler) },
        ],
    });
    let texture_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
        label: Some("Textured Quad Texture Bind Group Layout"),
        entries: &[wgpu::BindGroupLayoutEntry {
            binding: 0,
            visibility: wgpu::ShaderStages::FRAGMENT,
            ty: wgpu::BindingType::Texture {
                sample_type: wgpu::TextureSampleType::Float { filterable: true },
                view_dimension: wgpu::TextureViewDimension::D2,
                multisampled: false,
            },
            count: None,
        }],
    });
    let pipeline_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
        label: Some("Textured Quad Pipeline Layout"),
        bind_group_layouts: &[&uniform_layout, &texture_layout],
        push_constant_ranges: &[],
    });
    State {
        shader,
        uniform_layout,
        uniform_bind_group,
        texture_layout,
        sampler,
        pipeline_layout,
        texture_bind_groups: Default::default(),
        frame_used_textures: vec![],
    }
}

pub(crate) fn construct_pipeline(
    device: &wgpu::Device,
    graphics: &GraphicsConfig,
    state: &State,
    config: &PipelineConfig,
) -> PipelineHolder {
    let (blend_component, alpha_write) = match config.filter_type {
        CameraFilterType::Multiply => (
            wgpu::BlendComponent {
                src_factor: wgpu::BlendFactor::Zero,
                dst_factor: wgpu::BlendFactor::Src,
                operation: wgpu::BlendOperation::Add,
            },
            true,
        ),
        CameraFilterType::Add => (
            wgpu::BlendComponent {
                src_factor: wgpu::BlendFactor::SrcAlpha,
                dst_factor: wgpu::BlendFactor::One,
                operation: wgpu::BlendOperation::Add,
            },
            false,
        ),
        CameraFilterType::Subtract => (
            wgpu::BlendComponent {
                src_factor: wgpu::BlendFactor::SrcAlpha,
                dst_factor: wgpu::BlendFactor::One,
                operation: wgpu::BlendOperation::Subtract,
            },
            false,
        ),
        CameraFilterType::Blend => (
            wgpu::BlendComponent {
                src_factor: wgpu::BlendFactor::SrcAlpha,
                dst_factor: wgpu::BlendFactor::OneMinusSrcAlpha,
                operation: wgpu::BlendOperation::Add,
            },
            false,
        ),
        CameraFilterType::InvDstMultiply => (
            wgpu::BlendComponent {
                src_factor: wgpu::BlendFactor::Zero,
                dst_factor: wgpu::BlendFactor::OneMinusSrc,
                operation: wgpu::BlendOperation::Add,
            },
            true,
        ),
        _ => todo!(),
    };
    PipelineHolder {
        pipeline: device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
            label: Some("Textured Quad Pipeline"),
            layout: Some(&state.pipeline_layout),
            vertex: wgpu::VertexState {
                module: &state.shader,
                entry_point: "vs_main",
                buffers: &[wgpu::VertexBufferLayout {
                    array_stride: std::mem::size_of::<Vert>() as u64,
                    step_mode: wgpu::VertexStepMode::Vertex,
                    attributes: &vertex_attr_array![0 => Float32x3, 1 => Float32x2],
                }],
            },
            primitive: wgpu::PrimitiveState {
                topology: wgpu::PrimitiveTopology::TriangleStrip,
                ..Default::default()
            },
            depth_stencil: Some(wgpu::DepthStencilState {
                format: graphics.depth_format,
                depth_write_enabled: config.z_test == ZTest::GEqualZWrite,
                depth_compare: match config.z_test {
                    ZTest::None => wgpu::CompareFunction::Always,
                    ZTest::LEqual => wgpu::CompareFunction::LessEqual,
                    ZTest::GEqual | ZTest::GEqualZWrite => wgpu::CompareFunction::GreaterEqual,
                    _ => todo!(),
                },
                stencil: Default::default(),
                bias: Default::default(),
            }),
            multisample: wgpu::MultisampleState {
                count: graphics.msaa_samples,
                ..Default::default()
            },
            fragment: Some(wgpu::FragmentState {
                module: &state.shader,
                entry_point: "fs_main",
                targets: &[wgpu::ColorTargetState {
                    format: graphics.color_format,
                    blend: Some(wgpu::BlendState {
                        color: blend_component,
                        alpha: blend_component,
                    }),
                    write_mask: if alpha_write {
                        wgpu::ColorWrites::ALL
                    } else {
                        wgpu::ColorWrites::COLOR
                    },
                }],
            }),
            multiview: None,
        }),
    }
}

#[derive(Pod, Zeroable, Copy, Clone, Default, Debug)]
#[repr(C)]
struct Uniform {
    xf: CMatrix4f,
    color: CColor,
    lod: f32,
}

#[derive(Pod, Zeroable, Copy, Clone, Default, Debug)]
#[repr(C)]
struct Vert {
    pos: Vec3<f32>,
    uv: Vec2<f32>,
}

pub(crate) fn queue_textured_quad(
    filter_type: CameraFilterType,
    texture: TextureRef,
    z_test: ZTest,
    color: CColor,
    uv_scale: f32,
    rect: CRectangle,
    z: f32,
) {
    let pipeline =
        pipeline_ref(&PipelineCreateCommand::TexturedQuad(PipelineConfig { filter_type, z_test }));
    let vert_range = push_verts(&[
        Vert { pos: Vec3::new(0.0, 0.0, z), uv: Vec2::new(0.0, 0.0) },
        Vert { pos: Vec3::new(0.0, 1.0, z), uv: Vec2::new(0.0, uv_scale) },
        Vert { pos: Vec3::new(1.0, 0.0, z), uv: Vec2::new(uv_scale, 0.0) },
        Vert { pos: Vec3::new(1.0, 1.0, z), uv: Vec2::new(uv_scale, uv_scale) },
    ]);
    let uniform_range = push_uniform(&Uniform {
        xf: CMatrix4f::new(
            CVector4f::new(rect.size.x * 2.0, 0.0, 0.0, 0.0),
            CVector4f::new(0.0, rect.size.y * 2.0, 0.0, 0.0),
            CVector4f::new(0.0, 0.0, 1.0, 0.0),
            CVector4f::new(rect.position.x * 2.0 - 1.0, rect.position.y * 2.0 - 1.0, 0.0, 1.0),
        ),
        color,
        lod: 0.0,
    });

    push_textured_quad(texture, pipeline, vert_range, uniform_range);
}

pub(crate) fn queue_textured_quad_verts(
    filter_type: CameraFilterType,
    texture: TextureRef,
    z_test: ZTest,
    color: CColor,
    pos: &[CVector3f],
    uvs: &[CVector2f],
    lod: f32,
) {
    if pos.len() != 4 || uvs.len() != 4 {
        panic!("Invalid pos/uv sizes: {}/{}", pos.len(), uvs.len());
    }

    let pipeline =
        pipeline_ref(&PipelineCreateCommand::TexturedQuad(PipelineConfig { filter_type, z_test }));
    let vert_range = push_verts(
        &pos.iter()
            .zip(uvs)
            .map(|(pos, uv)| Vert { pos: pos.into(), uv: uv.into() })
            .collect::<Vec<Vert>>(),
    );
    let uniform_range = push_uniform(&Uniform { xf: get_combined_matrix(), color, lod: 0.0 });

    push_textured_quad(texture, pipeline, vert_range, uniform_range);
}

fn push_textured_quad(
    texture: TextureRef,
    pipeline: PipelineRef,
    vert_range: Range<u64>,
    uniform_range: Range<u64>,
) {
    // TODO defer bind group creation to draw time or another thread?
    let state = unsafe { STATE.as_mut().unwrap_unchecked() };
    let groups = &mut state.textured_quad.texture_bind_groups;
    if !groups.contains_key(&texture.id) {
        let tex = state.textures.get(&texture.id).unwrap();
        let bind_group = get_app().gpu.device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: None,
            layout: &state.textured_quad.texture_layout,
            entries: &[wgpu::BindGroupEntry {
                binding: 0,
                resource: wgpu::BindingResource::TextureView(&tex.view),
            }],
        });
        groups.insert(texture.id, bind_group);
    }

    push_draw_command(ShaderDrawCommand::TexturedQuad(DrawData {
        pipeline,
        vert_range,
        uniform_range,
        texture,
    }));
}

pub(crate) fn draw_textured_quad<'a>(
    data: &DrawData,
    state: &'a State,
    pass: &mut wgpu::RenderPass<'a>,
    buffers: &'a BuiltBuffers,
) {
    if !bind_pipeline(data.pipeline, pass) {
        return;
    }

    // Uniform bind group
    pass.set_bind_group(0, &state.uniform_bind_group, &[
        data.uniform_range.start as wgpu::DynamicOffset
    ]);

    // Texture bind group
    let texture_bind_group =
        state.texture_bind_groups.get(&data.texture.id).expect("Failed to find texture bind group");
    pass.set_bind_group(1, texture_bind_group, &[]);

    // Vertex buffer
    pass.set_vertex_buffer(0, buffers.vertex_buffer.slice(data.vert_range.clone()));
    pass.draw(0..4, 0..1);
}
