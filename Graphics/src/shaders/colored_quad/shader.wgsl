struct Uniform {
    xf: mat4x4<f32>;
    color: vec4<f32>;
};
@group(0) @binding(0)
var<uniform> ubuf: Uniform;

struct VertexOutput {
    @builtin(position) pos: vec4<f32>;
    //@builtin(normal) norm: vec4<f32>;
};

@stage(vertex)
fn vs_main(@location(0) in_pos: vec3<f32>) -> VertexOutput {//, @location(1) in_norm: vec3<f32>) -> VertexOutput {
    var out: VertexOutput;
    out.pos = ubuf.xf * vec4<f32>(in_pos, 1.0);
    //out.norm = in_norm;
    return out;
}

@stage(fragment)
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    return ubuf.color;
}
