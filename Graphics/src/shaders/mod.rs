use std::{
    collections::{HashMap, VecDeque},
    hash::{Hash, Hasher},
    ops::Range,
    sync::Arc,
};

use aabb::queue_aabb;
use bytemuck::Pod;
use bytemuck_derive::{Pod, Zeroable};
use cxx::{type_id, ExternType};
use cxx::private::hash;
use fog_volume_filter::queue_fog_volume_filter;
use fog_volume_plane::queue_fog_volume_plane;
use model::{add_material_set, add_model};
use texture::{create_render_texture, create_static_texture_2d, drop_texture};
use textured_quad::queue_textured_quad;
use twox_hash::Xxh3Hash64;
use wgpu::RenderPipeline;

use crate::{
    gpu::GraphicsConfig,
    zeus::{CColor, CMatrix4f, CRectangle, CVector2f, CVector3f, IDENTITY_MATRIX4F},
};
use crate::shaders::ffi::{TextureFormat, TextureRef};
use crate::shaders::texture::{RenderTexture, TextureWithView};

mod aabb;
mod fog_volume_filter;
mod fog_volume_plane;
mod model;
mod textured_quad;
mod texture;

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("zeus/CVector2f.hpp");
        include!("zeus/CVector3f.hpp");
        include!("zeus/CVector4f.hpp");
        include!("zeus/CMatrix4f.hpp");
        include!("zeus/CColor.hpp");
        include!("zeus/CRectangle.hpp");
        include!("zeus/CAABox.hpp");
        #[namespace = "zeus"]
        type CVector2f = crate::zeus::CVector2f;
        #[namespace = "zeus"]
        type CVector3f = crate::zeus::CVector3f;
        #[namespace = "zeus"]
        type CVector4f = crate::zeus::CVector4f;
        #[namespace = "zeus"]
        type CMatrix4f = crate::zeus::CMatrix4f;
        #[namespace = "zeus"]
        type CColor = crate::zeus::CColor;
        #[namespace = "zeus"]
        type CRectangle = crate::zeus::CRectangle;
        #[namespace = "zeus"]
        type CAABox = crate::zeus::CAABox;
    }

    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone, Hash)]
    pub(crate) enum CameraFilterType {
        Passthru,
        Multiply,
        Invert,
        Add,
        Subtract,
        Blend,
        Widescreen,
        SceneAdd,
        NoColor,
        InvDstMultiply,
    }
    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone, Hash)]
    pub(crate) enum ZTest {
        None,
        LEqual,
        GEqual,
        GEqualZWrite,
    }
    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone)]
    pub(crate) enum TextureFormat {
        RGBA8,
        R8,
        R32Float,
        DXT1,
        DXT3,
        DXT5,
        BPTC,
    }
    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone)]
    pub(crate) enum TextureClampMode {
        Repeat,
        ClampToEdge,
        ClampToWhite,
        ClampToBlack,
    }
    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone, Hash)]
    pub(crate) struct TextureRef {
        pub(crate) id: u32,
        pub(crate) render: bool,
    }

    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone)]
    #[repr(u32)]
    pub(crate) enum FogMode {
        None = 0x00,

        PerspLin = 0x02,
        PerspExp = 0x04,
        PerspExp2 = 0x05,
        PerspRevExp = 0x06,
        PerspRevExp2 = 0x07,

        OrthoLin = 0x0A,
        OrthoExp = 0x0C,
        OrthoExp2 = 0x0D,
        OrthoRevExp = 0x0E,
        OrthoRevExp2 = 0x0F,
    }
    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone)]
    pub(crate) struct FogState {
        #[cxx_name = "m_color"]
        pub(crate) color: CColor,
        #[cxx_name = "m_A"]
        pub(crate) a: f32,
        #[cxx_name = "m_B"]
        pub(crate) b: f32,
        #[cxx_name = "m_C"]
        pub(crate) c: f32,
        #[cxx_name = "m_mode"]
        pub(crate) mode: FogMode,
    }
    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone)]
    pub(crate) struct ClipRect {
        x: i32,
        y: i32,
        width: i32,
        height: i32,
    }

    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone)]
    pub(crate) struct MaterialPassInfo {
        tex_idx: u32,
        constant_color: CColor,
        has_constant_color: bool, // would love to use Option...
        uv_source: u8,
        uv_anim_type: u8,
        uv_anim_params: [f32; 9],
        normalize: bool, // TODO might be static based on uv type?
        sample_alpha: bool,
    }
    #[namespace = "aurora::shaders"]
    #[derive(Debug, Clone)]
    pub(crate) struct MaterialInfo {
        pass_info: [MaterialPassInfo; 8],
        texture_ids: Vec<TextureRef>,
        shader_type: u8,
        blend_mode: u8,
    }

    #[namespace = "aurora::shaders"]
    extern "Rust" {
        fn update_model_view(mv: CMatrix4f, mv_inv: CMatrix4f);
        fn update_projection(proj: CMatrix4f);
        fn update_fog_state(state: FogState);
        fn set_viewport(rect: CRectangle, znear: f32, zfar: f32);
        fn set_scissor(x: u32, y: u32, w: u32, h: u32);

        fn resolve_color(rect: ClipRect, bind: u32, clear_depth: bool);
        fn resolve_depth(rect: ClipRect, bind: u32);

        fn add_material_set(materials: Vec<MaterialInfo>) -> u32;
        fn add_model(verts: &[u8], indices: &[u8]) -> u32;

        fn queue_aabb(aabb: CAABox, color: CColor, z_only: bool);
        fn queue_fog_volume_plane(verts: &CxxVector<CVector4f>, pass: u8);
        fn queue_fog_volume_filter(color: CColor, two_way: bool);
        fn queue_textured_quad(
            filter_type: CameraFilterType,
            texture: TextureRef,
            z_test: ZTest,
            color: CColor,
            uv_scale: f32,
            rect: CRectangle,
            z: f32,
        );

        fn create_static_texture_2d(
            width: u32,
            height: u32,
            mips: u32,
            format: TextureFormat,
            data: &[u8],
            label: &str,
        ) -> TextureRef;
        fn create_render_texture(
            width: u32,
            height: u32,
            // clamp_mode: TextureClampMode,
            color_bind_count: u32,
            depth_bind_count: u32,
            label: &str,
        ) -> TextureRef;
        fn drop_texture(handle: TextureRef);
    }
}

unsafe impl bytemuck::Zeroable for ffi::FogMode {}
unsafe impl bytemuck::Pod for ffi::FogMode {}
unsafe impl bytemuck::Zeroable for ffi::FogState {}
unsafe impl bytemuck::Pod for ffi::FogState {}
impl Default for ffi::FogState {
    fn default() -> Self {
        Self { color: Default::default(), a: 0.0, b: 0.5, c: 0.0, mode: ffi::FogMode::None }
    }
}
impl Into<u32> for ffi::TextureFormat {
    // noinspection RsUnreachablePatterns
    fn into(self) -> u32 {
        match self {
            TextureFormat::RGBA8 => 1,
            TextureFormat::R8 => 2,
            TextureFormat::R32Float => 3,
            TextureFormat::DXT1 => 4,
            TextureFormat::DXT3 => 5,
            TextureFormat::DXT5 => 6,
            TextureFormat::BPTC => 7,
            _ => panic!("Invalid texture format {:?}", self),
        }
    }
}

#[derive(Debug, Copy, Clone)]
enum ColoredStripMode {
    Alpha,
    Additive,
    FullAdditive,
    Subtractive,
}
#[derive(Debug, Copy, Clone)]
struct ColoredStripVert {
    position: CVector3f,
    color: CColor,
    uv: CVector2f,
}

#[derive(Debug, Clone)]
enum Command {
    SetViewport(CRectangle, f32, f32),
    SetScissor(u32, u32, u32, u32),
    Draw(ShaderDrawCommand),
}
#[derive(Debug, Clone)]
enum ShaderDrawCommand {
    Aabb(aabb::DrawData),
    CameraBlurFilter {
        amount: f32,
        clear_depth: bool,
    },
    ColoredQuadFilter {
        filter_type: ffi::CameraFilterType,
        color: CColor,
        rect: CRectangle,
    },
    ColoredStripFilter {
        mode: ColoredStripMode,
        verts: Vec<ColoredStripVert>,
        color: CColor,
    },
    Decal {/* TODO */},
    ElementGen {/* TODO */},
    EnergyBar {/* TODO */},
    EnvFx {/* TODO */},
    FluidPlane {/* TODO */},
    FogVolumeFilter {
        two_way: bool,
        color: CColor,
    },
    FogVolumePlane {
        pass: u8,
        verts: Vec<CVector3f>,
    },
    LineRenderer {/* TODO */},
    MapSurface {/* TODO */},
    Model {
        pipeline_id: u32,
        material_id: u32,
        ambient_color: CColor,
        lights: u32,
        post_type: u32,
        game_blend_mode: u32,
        model_flags: u32,
    },
    MoviePlayer {/* TODO */},
    NES {/* TODO */},
    ParticleSwoosh {/* TODO */},
    PhazonSuitFilter {/* TODO */},
    RadarPaint {/* TODO */},
    RandomStaticFilter {/* TODO */},
    ScanLinesFilter {/* TODO */},
    TextSupport {/* TODO */},
    TexturedQuad(textured_quad::DrawData),
    ThermalCold,
    ThermalHot,
    WorldShadow {
        width: u32,
        height: u32,
        extent: f32,
    },
    XRayBlur {
        palette_tex: u32, /* TODO */
        amount: f32,
    },
}

struct RenderState {
    device: Arc<wgpu::Device>,
    queue: Arc<wgpu::Queue>,
    graphics_config: GraphicsConfig,
    pipelines: HashMap<u64, PipelineHolder>,
    current_pipeline: u64,
    uniform_alignment: usize,
    storage_alignment: usize,
    buffers: BuiltBuffers,
    commands: VecDeque<Command>,
    textures: HashMap<u32, TextureWithView>,
    render_textures: HashMap<u32, RenderTexture>,
    // Shader states
    aabb: aabb::State,
    textured_quad: textured_quad::State,
}
pub(crate) fn construct_state(
    device: Arc<wgpu::Device>,
    queue: Arc<wgpu::Queue>,
    graphics_config: &GraphicsConfig,
) {
    let limits = device.limits();
    let buffers = BuiltBuffers {
        uniform_buffer: device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Shared Uniform Buffer"),
            size: 134_217_728, // 128mb
            usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
            mapped_at_creation: false,
        }),
        vertex_buffer: device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("Shared Vertex Buffer"),
            size: 16_777_216, // 16mb
            usage: wgpu::BufferUsages::VERTEX | wgpu::BufferUsages::COPY_DST,
            mapped_at_creation: false,
        }),
    };
    let aabb = aabb::construct_state(&device, &queue, &buffers);
    let textured_quad = textured_quad::construct_state(&device, &queue, &buffers, graphics_config);
    let mut state = RenderState {
        device: device.clone(),
        queue: queue.clone(),
        graphics_config: graphics_config.clone(),
        pipelines: Default::default(),
        current_pipeline: u64::MAX,
        uniform_alignment: limits.min_uniform_buffer_offset_alignment as usize,
        storage_alignment: limits.min_storage_buffer_offset_alignment as usize,
        buffers,
        commands: Default::default(),
        textures: Default::default(),
        render_textures: Default::default(),
        aabb,
        textured_quad,
    };
    for config in aabb::INITIAL_PIPELINES {
        construct_pipeline(&mut state, config);
    }
    for config in textured_quad::INITIAL_PIPELINES {
        construct_pipeline(&mut state, config);
    }
    unsafe {
        STATE = Some(state);
    }
}
static mut STATE: Option<RenderState> = None;

#[derive(Default, Pod, Zeroable, Copy, Clone)]
#[repr(C)]
struct GlobalUniform {
    mv: CMatrix4f,
    mv_inv: CMatrix4f,
    proj: CMatrix4f,
    ambient: CColor, // TODO can this be combined with model?
    lightmap_mul: CColor,
    fog: ffi::FogState,
}
#[derive(Default)]
struct GlobalBuffers {
    uniforms: Vec<u8>,
    verts: Vec<u8>,

    global_current: GlobalUniform,
    global_dirty: bool,
    last_global: Option<Range<u64>>,
}
static mut GLOBAL_BUFFERS: GlobalBuffers = GlobalBuffers {
    uniforms: vec![],
    verts: vec![],
    global_current: GlobalUniform {
        mv: IDENTITY_MATRIX4F,
        mv_inv: IDENTITY_MATRIX4F,
        proj: IDENTITY_MATRIX4F,
        ambient: CColor::new(0.0, 0.0, 0.0, 1.0),
        lightmap_mul: CColor::new(0.0, 0.0, 0.0, 1.0),
        fog: ffi::FogState {
            color: CColor::new(0.0, 0.0, 0.0, 0.0),
            a: 0.0,
            b: 0.5,
            c: 0.0,
            mode: ffi::FogMode::None,
        },
    },
    global_dirty: false,
    last_global: None,
};

pub(crate) struct PipelineHolder {
    pipeline: wgpu::RenderPipeline,
}
pub(crate) struct BuiltBuffers {
    uniform_buffer: wgpu::Buffer,
    vertex_buffer: wgpu::Buffer,
}

const EMPTY_SLICE: &[u8] = &[0u8; 256];

fn push_value<T: Pod>(target: &mut Vec<u8>, buf: &T, alignment: usize) -> Range<u64> {
    let size_of = std::mem::size_of::<T>();
    let padding = if alignment == 0 { 0 } else { alignment - size_of % alignment };
    let begin = target.len() as u64;
    target.extend_from_slice(bytemuck::bytes_of(buf));
    if padding > 0 {
        target.extend_from_slice(&EMPTY_SLICE[..padding]);
    }
    begin..begin + size_of as u64
}
fn push_slice<T: Pod>(target: &mut Vec<u8>, buf: &[T], alignment: usize) -> Range<u64> {
    let size_of = std::mem::size_of::<T>();
    let padding = if alignment == 0 { 0 } else { alignment - size_of % alignment };
    let begin = target.len() as u64;
    target.extend_from_slice(bytemuck::cast_slice(buf));
    if padding > 0 {
        target.extend_from_slice(&EMPTY_SLICE[..padding]);
    }
    begin..begin + size_of as u64 * buf.len() as u64
}
fn push_verts<T: Pod>(buf: &[T]) -> Range<u64> {
    let global_buffers = unsafe { &mut GLOBAL_BUFFERS };
    push_slice(&mut global_buffers.verts, buf, 0 /* TODO? */)
}
fn push_uniform<T: Pod>(buf: &T) -> Range<u64> {
    let global_buffers = unsafe { &mut GLOBAL_BUFFERS };
    push_value(&mut global_buffers.uniforms, buf, unsafe {
        STATE.as_ref().unwrap().uniform_alignment
    })
}

#[derive(Debug, Copy, Clone)]
struct PipelineRef {
    id: u64,
}

pub(crate) enum PipelineCreateCommand {
    Aabb(aabb::PipelineConfig),
    TexturedQuad(textured_quad::PipelineConfig),
}
#[inline(always)]
fn hash_with_seed<T: Hash>(value: &T, seed: u64) -> u64 {
    let mut state = Xxh3Hash64::with_seed(seed);
    value.hash(&mut state);
    state.finish()
}
fn construct_pipeline(state: &mut RenderState, cmd: &PipelineCreateCommand) -> u64 {
    let id = match cmd {
        PipelineCreateCommand::Aabb(ref config) => hash_with_seed(config, 0xAABB),
        PipelineCreateCommand::TexturedQuad(ref config) => hash_with_seed(config, 0xEEAD),
    };
    if !state.pipelines.contains_key(&id) {
        let pipeline = match cmd {
            PipelineCreateCommand::Aabb(ref config) => aabb::construct_pipeline(
                state.device.as_ref(),
                &state.graphics_config,
                &state.aabb,
                config,
            ),
            PipelineCreateCommand::TexturedQuad(ref config) => textured_quad::construct_pipeline(
                state.device.as_ref(),
                &state.graphics_config,
                &state.textured_quad,
                config,
            ),
        };
        state.pipelines.insert(id, pipeline);
    }
    id
}
fn pipeline_ref(cmd: &PipelineCreateCommand) -> PipelineRef {
    let state = unsafe { STATE.as_mut().unwrap_unchecked() };
    // TODO queue for creation if not found
    let id = construct_pipeline(state, cmd);
    PipelineRef { id }
}

fn bind_pipeline(pipeline_ref: PipelineRef, pass: &mut wgpu::RenderPass) -> bool {
    let state = unsafe { STATE.as_ref().unwrap_unchecked() };
    if pipeline_ref.id == state.current_pipeline {
        return true;
    }
    if let Some(PipelineHolder { pipeline }) = state.pipelines.get(&pipeline_ref.id) {
        pass.set_pipeline(pipeline);
        return true;
    }
    return false;
}

pub(crate) fn render_into_pass(pass: &mut wgpu::RenderPass) {
    let state = unsafe { STATE.as_mut().unwrap_unchecked() };
    {
        let global_buffers = unsafe { &mut GLOBAL_BUFFERS };
        state.queue.write_buffer(&state.buffers.vertex_buffer, 0, &global_buffers.verts);
        state.queue.write_buffer(&state.buffers.uniform_buffer, 0, &global_buffers.uniforms);
        global_buffers.verts.clear();
        global_buffers.uniforms.clear();
        global_buffers.global_dirty = true;
    }
    for cmd in &state.commands {
        match cmd {
            Command::SetViewport(rect, znear, zfar) => {
                pass.set_viewport(
                    rect.position.x,
                    rect.position.y,
                    rect.size.x,
                    rect.size.y,
                    *znear,
                    *zfar,
                );
            }
            Command::SetScissor(x, y, w, h) => {
                pass.set_scissor_rect(*x, *y, *w, *h);
            }
            Command::Draw(cmd) => match cmd {
                ShaderDrawCommand::Aabb(data) => {
                    aabb::draw_aabb(data, &state.aabb, pass, &state.buffers);
                }
                ShaderDrawCommand::TexturedQuad(data) => {
                    textured_quad::draw_textured_quad(data, &state.textured_quad, pass, &state.buffers);
                }
                _ => todo!(),
            },
        }
    }
    state.commands.clear();
}

fn update_model_view(mv: CMatrix4f, mv_inv: CMatrix4f) {
    let global_buffers = unsafe { &mut GLOBAL_BUFFERS };
    global_buffers.global_current.mv = mv;
    global_buffers.global_current.mv_inv = mv_inv;
    global_buffers.global_dirty = true;
}
fn update_projection(proj: CMatrix4f) {
    let global_buffers = unsafe { &mut GLOBAL_BUFFERS };
    global_buffers.global_current.proj = proj;
    global_buffers.global_dirty = true;
}
fn update_fog_state(state: ffi::FogState) {
    let global_buffers = unsafe { &mut GLOBAL_BUFFERS };
    global_buffers.global_current.fog = state;
    global_buffers.global_dirty = true;
}

fn get_combined_matrix() -> CMatrix4f {
    let global_buffers = unsafe { &GLOBAL_BUFFERS };
    CMatrix4f::from(
        cgmath::Matrix4::from(global_buffers.global_current.mv)
            * cgmath::Matrix4::from(global_buffers.global_current.proj),
    )
}

fn finalize_global_uniform() -> Range<u64> {
    let global_buffers = unsafe { &mut GLOBAL_BUFFERS };
    if global_buffers.global_dirty || global_buffers.last_global.is_none() {
        global_buffers.last_global = Some(push_uniform(&global_buffers.global_current));
        global_buffers.global_dirty = false;
    }
    global_buffers.last_global.as_ref().unwrap().clone()
}

fn push_draw_command(cmd: ShaderDrawCommand) {
    let state = unsafe { STATE.as_mut().unwrap_unchecked() };
    state.commands.push_back(Command::Draw(cmd));
}
fn set_viewport(rect: CRectangle, znear: f32, zfar: f32) {
    let state = unsafe { STATE.as_mut().unwrap_unchecked() };
    state.commands.push_back(Command::SetViewport(rect, znear, zfar));
}
fn set_scissor(x: u32, y: u32, w: u32, h: u32) {
    let state = unsafe { STATE.as_mut().unwrap_unchecked() };
    state.commands.push_back(Command::SetScissor(x, y, w, h));
}

fn resolve_color(rect: ffi::ClipRect, bind: u32, clear_depth: bool) {
    // TODO
}
fn resolve_depth(rect: ffi::ClipRect, bind: u32) {
    // TODO
}
