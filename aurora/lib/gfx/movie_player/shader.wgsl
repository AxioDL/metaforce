struct Uniform {
    xf: mat4x4<f32>;
    color: vec4<f32>;
};
@group(0) @binding(0)
var<uniform> ubuf: Uniform;
@group(0) @binding(1)
var tex_sampler: sampler;
@group(1) @binding(0)
var tex_y: texture_2d<f32>;
@group(1) @binding(1)
var tex_u: texture_2d<f32>;
@group(1) @binding(2)
var tex_v: texture_2d<f32>;

struct VertexOutput {
    @builtin(position) pos: vec4<f32>;
    @location(0) uv: vec2<f32>;
};

@stage(vertex)
fn vs_main(@location(0) in_pos: vec3<f32>, @location(1) in_uv: vec2<f32>) -> VertexOutput {
    var out: VertexOutput;
    out.pos = ubuf.xf * vec4<f32>(in_pos, 1.0);
    out.uv = in_uv;
    return out;
}

@stage(fragment)
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    var yuv = vec3<f32>(
        1.1643 * (textureSample(tex_y, tex_sampler, in.uv).x - 0.0625),
        textureSample(tex_u, tex_sampler, in.uv).x - 0.5,
        textureSample(tex_v, tex_sampler, in.uv).x - 0.5
    );
    return ubuf.color * vec4<f32>(
        yuv.x + 1.5958 * yuv.z,
        yuv.x - 0.39173 * yuv.y - 0.8129 * yuv.z,
        yuv.x + 2.017 * yuv.y,
        1.0
    );
}
