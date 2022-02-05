fn calculate_tcg(
    traits: PassTrait,
    uv_in: vec3<f32>,
    obj_pos: ptr<function, vec4<f32>>,
    obj_norm: ptr<function, vec4<f32>>,
    model_matrix: ptr<function, mat4x4<f32>>,
    mv_inv: ptr<function, mat4x4<f32>>,
) -> vec2<f32> {
    var uv: vec4<f32> = vec4<f32>(uv_in, 1.0);
    if (traits.tcg_mtx_idx >= 0) {
        let tcgm = process_uv_anim(model_matrix, mv_inv, traits);
        var tmp = (tcgm.mtx * uv).xyz;
        if (traits.normalize == 1u) {
            tmp = normalize(tmp);
        }
        let tmp_proj = tcgm.post_mtx * vec4<f32>(tmp, 1.0);
        return (tmp_proj / tmp_proj.w).xy;
    }
    return uv.xy;
}

@stage(vertex)
fn vs_main(
    @location(0) pos: vec3<f32>,
    @location(1) norm: vec3<f32>,
    @location(2) in_uv_lightmap: vec3<f32>,
    @location(3) in_uv_diffuse: vec2<f32>,
    @location(4) in_uv_diffuse_mod: vec2<f32>,
    @location(5) in_uv_emissive: vec2<f32>,
    @location(6) in_uv_specular: vec2<f32>,
    @location(7) in_uv_extended_specular: vec2<f32>,
    @location(8) in_uv_reflection: vec2<f32>,
    @location(9) in_uv_alpha: vec2<f32>,
    @location(10) in_uv_alpha_mod: vec2<f32>,
) -> VertexOutput {
    var out: VertexOutput;
    var obj_pos = vec4<f32>(pos, 1.0);
    var obj_norm = vec4<f32>(norm, 0.0);
    var model_matrix_no_trans = make_rotate(radians(u_model.orientation)) * make_scale(u_model.scale);
    var model_matrix = make_translate(u_model.position) * model_matrix_no_trans;
    var mv = u_global.view * model_matrix;
    var mv_inv = transpose(u_global.view * model_matrix_no_trans);
    var mv_pos = mv * obj_pos;
    var mv_norm = mv_inv * obj_norm;

    out.position = u_global.proj * mv_pos;
    out.uv_lightmap = calculate_tcg(c_traits.lightmap, in_uv_lightmap, &obj_pos, &obj_norm, &model_matrix, &mv_inv);
    out.uv_diffuse = calculate_tcg(c_traits.diffuse, vec3<f32>(in_uv_diffuse, 0.0), &obj_pos, &obj_norm, &model_matrix, &mv_inv);
    out.uv_diffuse_mod = calculate_tcg(c_traits.diffuse_mod, vec3<f32>(in_uv_diffuse_mod, 0.0), &obj_pos, &obj_norm, &model_matrix, &mv_inv);
    out.uv_emissive = calculate_tcg(c_traits.emissive, vec3<f32>(in_uv_emissive, 0.0), &obj_pos, &obj_norm, &model_matrix, &mv_inv);
    out.uv_specular = calculate_tcg(c_traits.specular, vec3<f32>(in_uv_specular, 0.0), &obj_pos, &obj_norm, &model_matrix, &mv_inv);
    out.uv_extended_specular = calculate_tcg(c_traits.extended_specular, vec3<f32>(in_uv_extended_specular, 0.0), &obj_pos, &obj_norm, &model_matrix, &mv_inv);
    out.uv_reflection = calculate_tcg(c_traits.reflection, vec3<f32>(in_uv_reflection, 0.0), &obj_pos, &obj_norm, &model_matrix, &mv_inv);
    out.uv_alpha = calculate_tcg(c_traits.alpha, vec3<f32>(in_uv_alpha, 0.0), &obj_pos, &obj_norm, &model_matrix, &mv_inv);
    out.uv_alpha_mod = calculate_tcg(c_traits.alpha_mod, vec3<f32>(in_uv_alpha_mod, 0.0), &obj_pos, &obj_norm, &model_matrix, &mv_inv);

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
