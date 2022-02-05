struct Vec3Block {
    data: @stride(16) array<vec3<f32>>;
};
struct Vec2Block {
    data: @stride(8) array<vec2<f32>>;
};
struct TCGUniforms {
    data: array<TCGUniform>;
};

@group(0) @binding(0)
var<storage, read> v_verts: Vec3Block;
@group(0) @binding(1)
var<storage, read> v_norms: Vec3Block;
@group(0) @binding(2)
var<storage, read> v_uvs: Vec2Block;
@group(0) @binding(3)
var<storage, read> v_short_uvs: Vec2Block;
//@group(0) @binding(4)
//var<uniform> v_tcg: TCGUniforms;

fn calculate_tcg(
    traits: PassTrait,
    obj_pos: ptr<function, vec4<f32>>,
    obj_norm: ptr<function, vec4<f32>>,
    model_matrix: ptr<function, mat4x4<f32>>,
    mv_inv: ptr<function, mat4x4<f32>>,
    uv_0_4_idx: vec4<i32>,
    uv_4_7_idx: vec4<i32>,
) -> vec2<f32> {
    if (traits.uv_source == -1) {
        return vec2<f32>(0.0);
    }
    if (traits.tcg_mtx_idx >= 0) {
        var src: vec4<f32>;
        switch (traits.uv_source) {
            case 0: { src = vec4<f32>((*obj_pos).xyz, 1.0); }
            case 1: { src = vec4<f32>((*obj_norm).xyz, 1.0); }
            case 2: {
                if (c_traits.shader.short_uvs == 1u) {
                    src = vec4<f32>(v_short_uvs.data[uv_0_4_idx.x], 0.0, 1.0);
                } else {
                    src = vec4<f32>(v_uvs.data[uv_0_4_idx.x], 0.0, 1.0);
                }
            }
            case 3: { src = vec4<f32>(v_uvs.data[uv_0_4_idx.y], 0.0, 1.0); }
            case 4: { src = vec4<f32>(v_uvs.data[uv_0_4_idx.z], 0.0, 1.0); }
            case 5: { src = vec4<f32>(v_uvs.data[uv_0_4_idx.w], 0.0, 1.0); }
            case 6: { src = vec4<f32>(v_uvs.data[uv_4_7_idx.x], 0.0, 1.0); }
            case 7: { src = vec4<f32>(v_uvs.data[uv_4_7_idx.y], 0.0, 1.0); }
            case 8: { src = vec4<f32>(v_uvs.data[uv_4_7_idx.z], 0.0, 1.0); }
            default: {}
        }
        let tcgm = process_uv_anim(model_matrix, mv_inv, traits);
        var tmp = (tcgm.mtx * src).xyz;
        if (traits.normalize == 1u) {
            tmp = normalize(tmp);
        }
        let tmp_proj = tcgm.post_mtx * vec4<f32>(tmp, 1.0);
        return (tmp_proj / tmp_proj.w).xy;
    } else {
        switch (traits.uv_source) {
            case 0: { return (*obj_pos).xy; }
            case 1: { return (*obj_norm).xy; }
            case 2: {
                if (c_traits.shader.short_uvs == 1u) {
                    return v_short_uvs.data[uv_0_4_idx.x];
                } else {
                    return v_uvs.data[uv_0_4_idx.x];
                }
            }
            case 3: { return v_uvs.data[uv_0_4_idx.y]; }
            case 4: { return v_uvs.data[uv_0_4_idx.z]; }
            case 5: { return v_uvs.data[uv_0_4_idx.w]; }
            case 6: { return v_uvs.data[uv_4_7_idx.x]; }
            case 7: { return v_uvs.data[uv_4_7_idx.y]; }
            case 8: { return v_uvs.data[uv_4_7_idx.z]; }
            default: {}
        }
    }
    return vec2<f32>(0.0);
}

@stage(vertex)
fn vs_main(
    @location(0) pos_norm_idx: vec2<i32>,
    @location(1) uv_0_4_idx: vec4<i32>,
    @location(2) uv_4_7_idx: vec4<i32>,
) -> VertexOutput {
    var out: VertexOutput;
    var obj_pos = vec4<f32>(v_verts.data[pos_norm_idx.x], 1.0);
    var obj_norm = vec4<f32>(v_norms.data[pos_norm_idx.y], 0.0);
    var model_matrix_no_trans = make_rotate(radians(u_model.orientation)) * make_scale(u_model.scale);
    var model_matrix = make_translate(u_model.position) * model_matrix_no_trans;
    var mv = u_global.view * model_matrix;
    var mv_inv = transpose(u_global.view * model_matrix_no_trans);
    var mv_pos = mv * obj_pos;
    var mv_norm = mv_inv * obj_norm;

    out.position = u_global.proj * mv_pos;
    out.uv_lightmap = calculate_tcg(c_traits.lightmap, &obj_pos, &obj_norm, &model_matrix, &mv_inv, uv_0_4_idx, uv_4_7_idx);
    out.uv_diffuse = calculate_tcg(c_traits.diffuse, &obj_pos, &obj_norm, &model_matrix, &mv_inv, uv_0_4_idx, uv_4_7_idx);
    out.uv_diffuse_mod = calculate_tcg(c_traits.diffuse_mod, &obj_pos, &obj_norm, &model_matrix, &mv_inv, uv_0_4_idx, uv_4_7_idx);
    out.uv_emissive = calculate_tcg(c_traits.emissive, &obj_pos, &obj_norm, &model_matrix, &mv_inv, uv_0_4_idx, uv_4_7_idx);
    out.uv_specular = calculate_tcg(c_traits.specular, &obj_pos, &obj_norm, &model_matrix, &mv_inv, uv_0_4_idx, uv_4_7_idx);
    out.uv_extended_specular = calculate_tcg(c_traits.extended_specular, &obj_pos, &obj_norm, &model_matrix, &mv_inv, uv_0_4_idx, uv_4_7_idx);
    out.uv_reflection = calculate_tcg(c_traits.reflection, &obj_pos, &obj_norm, &model_matrix, &mv_inv, uv_0_4_idx, uv_4_7_idx);
    out.uv_alpha = calculate_tcg(c_traits.alpha, &obj_pos, &obj_norm, &model_matrix, &mv_inv, uv_0_4_idx, uv_4_7_idx);
    out.uv_alpha_mod = calculate_tcg(c_traits.alpha_mod, &obj_pos, &obj_norm, &model_matrix, &mv_inv, uv_0_4_idx, uv_4_7_idx);

    if (c_traits.shader.samus_reflection == 1u) {
        // TODO dyn reflection
    }

    if (u_traits.post_type > 0u && u_traits.post_type < 5u) {
        out.lighting = vec3<f32>(1.0);
    } else {
        var lighting = u_global.ambient.xyz + u_model.ambient_color.xyz;
        for (var i = 0; i < 8; i = i + 1) {
            var light = u_model.lights[i];
            var delta = mv_pos.xyz - light.pos;
            var dist = length(delta);
            var delta_norm = delta / dist;
            var ang_dot = max(dot(delta_norm, light.dir), 0.0);
            var lin_att = light.lin_att;
            var att = 1.0 / (lin_att.z * dist * dist * lin_att.y * dist + lin_att.x);
            var ang_att = light.ang_att;
            var ang_att_d = ang_att.z * ang_dot * ang_dot * ang_att.y * ang_dot + ang_att.x;
            var this_color = light.color.xyz * ang_att_d * att * max(dot(-delta_norm, mv_norm.xyz), 0.0);
            if (i == 0 && c_traits.shader.world_shadow == 1u) {
                // TODO ExtTex0 sample
            }
            lighting = lighting + this_color;
        }
        out.lighting = clamp(lighting, vec3<f32>(0.0), vec3<f32>(1.0));
    }

    // TODO dyn reflection sample

    return out;
}
