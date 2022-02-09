use std::hash::Hash;

use crate::shaders::cxxbridge::ffi;

struct MaterialShaderData {
    shader_type: u32,      // 5 values (incl. Invalid)
    world_shadow: u32,     // bool
    alpha_discard: u32,    // bool
    samus_reflection: u32, // bool
}
struct MaterialPassData {
    constant_color: [f32; 4],
    has_constant_color: u32, // bool
    uv_source: i32,          // 8 values
    normalize: u32,          // bool
    sample_alpha: u32,       // bool
}

struct ModelRenderData {
    // position: [f32; 3], // for UV anims
    ambient_color: [f32; 4],
    lights: [f32; 8], // TODO
    post_type: u32,
    game_blend_mode: u32,
    // model_flags: u32, TODO only needed for render
}

#[derive(Debug, Hash)]
struct ModelPipelineConfig {
    material_blend_mode: u32,
    material_depth_write: bool,
    render_blend_mode: u32,
    render_model_flags: u32,
    render_cull_mode: u32,
    render_dst_alpha: bool,
    render_color_update: bool,
    render_alpha_update: bool,
}

pub(crate) fn add_material_set(materials: Vec<ffi::MaterialInfo>) -> u32 { return u32::MAX; }
pub(crate) fn add_model(verts: &[u8], indices: &[u8]) -> u32 { return u32::MAX; }
