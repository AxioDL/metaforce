struct Uniform {
    xf: mat4x4<f32>;
    color: vec4<f32>;
    lod: f32;
};
@group(0) @binding(0)
var<uniform> ubuf: Uniform;
@group(0) @binding(1)
var texture_sampler: sampler;
@group(1) @binding(0)
var texture: texture_2d<f32>;

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
    return ubuf.color * textureSampleBias(texture, texture_sampler, in.uv, ubuf.lod);
}
