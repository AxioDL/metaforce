struct PassTrait {
    constant_color: vec4<f32>;
    tex_idx: i32;
    uv_source: i32;
    tcg_mtx_idx: i32;
    normalize: u32;
    sample_alpha: u32;
    has_constant_color: u32;
};
struct TCGUniform {
    mode: i32;
    f0: f32;
    f1: f32;
    f2: f32;
    f3: f32;
    f4: f32;
    f5: f32;
    f6: f32;
    f7: f32;
    f8: f32;
    pad1: u32;
    pad2: u32;
};
struct ConstShaderTraits {
    shader_type: u32;
    world_shadow: u32;
    short_uvs: u32;
    alpha_discard: u32;
    samus_reflection: u32;
    @align(16) tcgs: @stride(48) array<TCGUniform, 4>;
};
struct MutShaderTraits {
    post_type: u32;
    game_blend_mode: u32;
    model_flags: u32;
};
struct ConstTraits {
    shader: ConstShaderTraits;
    lightmap: PassTrait;
    diffuse: PassTrait;
    diffuse_mod: PassTrait;
    emissive: PassTrait;
    specular: PassTrait;
    extended_specular: PassTrait;
    reflection: PassTrait;
    alpha: PassTrait;
    alpha_mod: PassTrait;
};
@group(2) @binding(10)
var<uniform> c_traits: ConstTraits;

struct Light {
    pos: vec3<f32>;
    dir: vec3<f32>;
    color: vec4<f32>;
    lin_att: vec3<f32>;
    ang_att: vec3<f32>;
};
struct FogState {
    color: vec4<f32>;
    a: f32;
    b: f32;
    c: f32;
    mode: u32;
};
struct ModelUniform {
    position: vec3<f32>;
    orientation: vec3<f32>;
    scale: vec3<f32>;
    ambient_color: vec4<f32>;
    lights: array<Light, 8>;
};
struct GlobalUniform {
    view: mat4x4<f32>;
    proj: mat4x4<f32>;
    ambient: vec4<f32>;
    lightmap_mul: vec4<f32>;
    fog: FogState;
    seconds: f32;
};
@group(1) @binding(0)
var<uniform> u_model: ModelUniform;
@group(1) @binding(1)
var<uniform> u_global: GlobalUniform;
@group(1) @binding(2)
var<uniform> u_traits: MutShaderTraits;

@group(2) @binding(0)
var t_lightmap: texture_2d<f32>;
@group(2) @binding(1)
var t_diffuse: texture_2d<f32>;
@group(2) @binding(2)
var t_diffuse_mod: texture_2d<f32>;
@group(2) @binding(3)
var t_emissive: texture_2d<f32>;
@group(2) @binding(4)
var t_specular: texture_2d<f32>;
@group(2) @binding(5)
var t_extended_specular: texture_2d<f32>;
@group(2) @binding(6)
var t_reflection: texture_2d<f32>;
@group(2) @binding(7)
var t_alpha: texture_2d<f32>;
@group(2) @binding(8)
var t_alpha_mod: texture_2d<f32>;
@group(2) @binding(9)
var t_material: texture_2d<f32>;

@group(3) @binding(0)
var s_repeat: sampler;
@group(3) @binding(1)
var s_clamp: sampler;
@group(3) @binding(2)
var s_reflect: sampler;
@group(3) @binding(3)
var s_clamp_edge: sampler;

struct VertexOutput {
    @builtin(position) position: vec4<f32>;
    @location(0) lighting: vec3<f32>;
    @location(1) uv_lightmap: vec2<f32>;
    @location(2) uv_diffuse: vec2<f32>;
    @location(3) uv_diffuse_mod: vec2<f32>;
    @location(4) uv_emissive: vec2<f32>;
    @location(5) uv_specular: vec2<f32>;
    @location(6) uv_extended_specular: vec2<f32>;
    @location(7) uv_reflection: vec2<f32>;
    @location(8) uv_alpha: vec2<f32>;
    @location(9) uv_alpha_mod: vec2<f32>;
};

fn make_rotate_x(r: f32) -> mat4x4<f32> {
    var s = sin(r);
    var c = cos(r);
    return mat4x4<f32>(
        vec4<f32>(1.0, 0.0, 0.0, 0.0),
        vec4<f32>(0.0, c,   s,   0.0),
        vec4<f32>(0.0, -s,  c,   0.0),
        vec4<f32>(0.0, 0.0, 0.0, 1.0)
    );
}

fn make_rotate_y(r: f32) -> mat4x4<f32> {
    var s = sin(r);
    var c = cos(r);
    return mat4x4<f32>(
        vec4<f32>(c,   0.0, -s,   0.0),
        vec4<f32>(0.0, 1.0,  0.0, 0.0),
        vec4<f32>(s,   0.0,  c,   0.0),
        vec4<f32>(0.0, 0.0,  0.0, 1.0)
    );
}

fn make_rotate_z(r: f32) -> mat4x4<f32> {
    var s = sin(r);
    var c = cos(r);
    return mat4x4<f32>(
        vec4<f32>(c,   s,   0.0, 0.0),
        vec4<f32>(-s,  c,   0.0, 0.0),
        vec4<f32>(0.0, 0.0, 1.0, 0.0),
        vec4<f32>(0.0, 0.0, 0.0, 1.0)
    );
}

fn make_rotate(r: vec3<f32>) -> mat4x4<f32> {
    return make_rotate_z(r.z)
         * make_rotate_y(r.y)
         * make_rotate_x(r.x);
}

fn make_scale(s: vec3<f32>) -> mat4x4<f32> {
    return mat4x4<f32>(
        vec4<f32>(s.x, 0.0, 0.0, 0.0),
        vec4<f32>(0.0, s.y, 0.0, 0.0),
        vec4<f32>(0.0, 0.0, s.z, 0.0),
        vec4<f32>(0.0, 0.0, 0.0, 1.0)
    );
}

fn make_translate(translate: vec3<f32>) -> mat4x4<f32> {
    return mat4x4<f32>(
        vec4<f32>(1.0, 0.0, 0.0, 0.0),
        vec4<f32>(0.0, 1.0, 0.0, 0.0),
        vec4<f32>(0.0, 0.0, 1.0, 0.0),
        vec4<f32>(translate,     1.0)
    );
}

struct TCGMatrix {
    mtx: mat4x4<f32>;
    post_mtx: mat4x4<f32>;
};

let identity_mtx: mat4x4<f32> = mat4x4<f32>(
    vec4<f32>(1.0, 0.0, 0.0, 0.0),
    vec4<f32>(0.0, 1.0, 0.0, 0.0),
    vec4<f32>(0.0, 0.0, 1.0, 0.0),
    vec4<f32>(0.0, 0.0, 0.0, 1.0),
);

fn process_uv_anim(
    model_matrix: ptr<function, mat4x4<f32>>,
    mv_inv: ptr<function, mat4x4<f32>>,
    traits: PassTrait,
) -> TCGMatrix {
    var out = TCGMatrix(identity_mtx, identity_mtx);
    var tu = c_traits.shader.tcgs[traits.tcg_mtx_idx];
    switch (tu.mode) {
    case 0: {
        out.mtx = *mv_inv;
        out.mtx[3][3] = 1.0;
        out.post_mtx[0][0] = 0.5;
        out.post_mtx[1][1] = 0.5;
        out.post_mtx[3][0] = 0.5;
        out.post_mtx[3][1] = 0.5;
    }
    case 1: {
        out.mtx = *mv_inv;
        out.mtx[3] = u_global.view * vec4<f32>(u_model.position, 1.0);
        out.mtx[3][3] = 1.0;
        out.post_mtx[0][0] = 0.5;
        out.post_mtx[1][1] = 0.5;
        out.post_mtx[3][0] = 0.5;
        out.post_mtx[3][1] = 0.5;
    }
    case 2: {
        out.mtx[3][0] = u_global.seconds * tu.f2 + tu.f0;
        out.mtx[3][1] = u_global.seconds * tu.f3 + tu.f1;
    }
    case 3: {
        var angle = u_global.seconds * tu.f1 + tu.f0;
        var acos = cos(angle);
        var asin = sin(angle);
        out.mtx[0][0] = acos;
        out.mtx[0][1] = asin;
        out.mtx[1][0] = -asin;
        out.mtx[1][1] = acos;
        out.mtx[3][0] = (1.0 - (acos - asin)) * 0.5;
        out.mtx[3][1] = (1.0 - (asin + acos)) * 0.5;
    }
    case 4: {
        var n: f32 = tu.f2 * tu.f0 * (tu.f3 + u_global.seconds);
        out.mtx[3][0] = trunc(tu.f1 * fract(n)) * tu.f2;
    }
    case 5: {
        var n: f32 = tu.f2 * tu.f0 * (tu.f3 + u_global.seconds);
        out.mtx[3][1] = trunc(tu.f1 * fract(n)) * tu.f2;
    }
    case 6: {
        let pos = u_model.position;
        out.mtx = *model_matrix;
        out.mtx[3] = vec4<f32>(0.0, 0.0, 0.0, 1.0);
        out.post_mtx[0][0] = 0.5;
        out.post_mtx[1][1] = 0.0;
        out.post_mtx[2][1] = 0.5;
        out.post_mtx[3][0] = pos.x * 0.05;
        out.post_mtx[3][1] = pos.y * 0.05;
    }
    default: {}
    }
    return out;
}

fn t_sample(
    traits: PassTrait,
    tex: texture_2d<f32>,
    uv: vec2<f32>,
) -> vec3<f32> {
    // For control flow uniformity, we always have to sample
    var color: vec4<f32> = textureSample(tex, s_repeat, uv);
    if (traits.has_constant_color == 1u) {
        color = traits.constant_color;
    }
    if (traits.sample_alpha == 1u) {
        return vec3<f32>(color.w);
    }
    return color.xyz;
}

let kRGBToYPrime: vec3<f32> = vec3<f32>(0.299, 0.587, 0.114);

fn t_sample_alpha(
    traits: PassTrait,
    tex: texture_2d<f32>,
    uv: vec2<f32>,
) -> f32 {
    // For control flow uniformity, we always have to sample
    var color: vec4<f32> = textureSample(tex, s_repeat, uv);
    if (traits.has_constant_color == 1u) {
        color = traits.constant_color;
    }
    if (traits.sample_alpha == 1u) {
        return color.w;
    }
    return dot(color.xyz, kRGBToYPrime);
}

@stage(fragment)
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    var color: vec4<f32>;

    let mat_sample = textureSample(t_material, s_repeat, in.uv_diffuse);

    if (c_traits.shader.shader_type == 0u) {
        // Invalid
        color = vec4<f32>(
            t_sample(c_traits.diffuse, t_diffuse, in.uv_diffuse),
            t_sample_alpha(c_traits.alpha, t_alpha, in.uv_alpha)
        );
    } else if (c_traits.shader.shader_type == 1u) {
        // RetroShader
        var rgb = t_sample(c_traits.lightmap, t_lightmap, in.uv_lightmap)
            * u_global.lightmap_mul.xyz + in.lighting;
        rgb = rgb * t_sample(c_traits.diffuse, t_diffuse, in.uv_diffuse)
            + t_sample(c_traits.emissive, t_emissive, in.uv_emissive);
        let specular = t_sample(c_traits.specular, t_specular, in.uv_specular)
            + t_sample(c_traits.extended_specular, t_extended_specular, in.uv_extended_specular)
            * in.lighting;
        rgb = rgb + specular * t_sample(c_traits.reflection, t_reflection, in.uv_reflection);
        // TODO DynReflectionSample
        color = vec4<f32>(rgb, t_sample_alpha(c_traits.alpha, t_alpha, in.uv_alpha)
            * t_sample_alpha(c_traits.alpha_mod, t_alpha_mod, in.uv_alpha_mod));
    } else if (c_traits.shader.shader_type == 2u) {
        // RetroDynamicShader
        var rgb = t_sample(c_traits.lightmap, t_lightmap, in.uv_lightmap)
            * u_global.lightmap_mul.xyz + in.lighting;
        rgb = rgb * t_sample(c_traits.diffuse, t_diffuse, in.uv_diffuse)
            + t_sample(c_traits.emissive, t_emissive, in.uv_emissive);
        let specular = t_sample(c_traits.specular, t_specular, in.uv_specular)
            + t_sample(c_traits.extended_specular, t_extended_specular, in.uv_extended_specular)
            * in.lighting;
        rgb = rgb + specular * t_sample(c_traits.reflection, t_reflection, in.uv_reflection);
        // TODO DynReflectionSample
        color = vec4<f32>(rgb, t_sample_alpha(c_traits.alpha, t_alpha, in.uv_alpha)
            * t_sample_alpha(c_traits.alpha_mod, t_alpha_mod, in.uv_alpha_mod));
    } else {
        // TODO RetroDynamicAlphaShader + RetroDynamicCharacterShader
    }

    // TODO fog shader

    // TODO post type

    if (c_traits.shader.alpha_discard == 1u && color.w < 0.25) {
        discard;
    }

    return color;
}
