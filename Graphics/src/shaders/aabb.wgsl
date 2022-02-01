struct Uniform {
    xf: mat4x4<f32>;
    color: vec4<f32>;
};
[[group(0), binding(0)]]
var<uniform> ubuf: Uniform;

[[stage(vertex)]]
fn vs_main([[location(0)]] in_pos: vec3<f32>) -> [[builtin(position)]] vec4<f32> {
    return ubuf.xf * vec4<f32>(in_pos, 1.0);
}

[[stage(fragment)]]
fn fs_main() -> [[location(0)]] vec4<f32> {
    return ubuf.color;
}
