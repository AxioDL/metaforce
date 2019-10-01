"Defines node groups implementing shader components found in Retro games"

import bpy

# Root Eevee Nodes

# Additive output node
def make_additive_output():
    new_grp = bpy.data.node_groups.new('HECLAdditiveOutput', 'ShaderNodeTree')
    shader_input = new_grp.inputs.new('NodeSocketShader', 'Surface')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (0, 0)

    # Add Shader
    emissive_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    emissive_add_shader.location = (200, 0)

    # Transparent BDSF (Provides alpha)
    transparent_bdsf = new_grp.nodes.new('ShaderNodeBsdfTransparent')
    transparent_bdsf.location = (0, 100)
    transparent_bdsf.inputs['Color'].default_value = (1.0, 1.0, 1.0, 1.0)

    # Material Output (Final output)
    mat_out = new_grp.nodes.new('ShaderNodeOutputMaterial')
    mat_out.location = (400, 0)

    # Links
    new_grp.links.new(grp_in.outputs['Surface'], emissive_add_shader.inputs[1])
    new_grp.links.new(transparent_bdsf.outputs[0], emissive_add_shader.inputs[0])
    new_grp.links.new(emissive_add_shader.outputs[0], mat_out.inputs['Surface'])

# Blend output node
def make_blend_opaque_output():
    for tp in ('HECLBlendOutput', 'HECLOpaqueOutput'):
        new_grp = bpy.data.node_groups.new(tp, 'ShaderNodeTree')
        shader_input = new_grp.inputs.new('NodeSocketShader', 'Surface')
        new_grp.use_fake_user = True

        # Group inputs
        grp_in = new_grp.nodes.new('NodeGroupInput')
        grp_in.location = (0, 0)

        # Material Output (Final output)
        mat_out = new_grp.nodes.new('ShaderNodeOutputMaterial')
        mat_out.location = (200, 0)

        # Links
        new_grp.links.new(grp_in.outputs['Surface'], mat_out.inputs['Surface'])

#0 - RetroShader
def make_retro_shader():
    new_grp = bpy.data.node_groups.new('RetroShader', 'ShaderNodeTree')
    surface_output = new_grp.outputs.new('NodeSocketShader', 'Surface')
    lightmap_input = new_grp.inputs.new('NodeSocketColor', 'Lightmap')
    lightmap_input.default_value = (0.0, 0.0, 0.0, 0.0)
    diffuse_input = new_grp.inputs.new('NodeSocketColor', 'Diffuse')
    diffuse_input.default_value = (0.0, 0.0, 0.0, 0.0)
    emissive_input = new_grp.inputs.new('NodeSocketColor', 'Emissive')
    emissive_input.default_value = (0.0, 0.0, 0.0, 0.0)
    specular_input = new_grp.inputs.new('NodeSocketColor', 'Specular')
    specular_input.default_value = (0.0, 0.0, 0.0, 0.0)
    ext_spec_input = new_grp.inputs.new('NodeSocketColor', 'ExtendedSpecular')
    ext_spec_input.default_value = (0.0, 0.0, 0.0, 0.0)
    reflection_input = new_grp.inputs.new('NodeSocketColor', 'Reflection')
    reflection_input.default_value = (0.0, 0.0, 0.0, 0.0)
    indirect_tex = new_grp.inputs.new('NodeSocketColor', 'IndirectTex')
    indirect_tex.default_value = (0.0, 0.0, 0.0, 0.0)
    alpha_input = new_grp.inputs.new('NodeSocketFloatFactor', 'Alpha')
    alpha_input.default_value = 1.0
    alpha_input.min_value = 0.0
    alpha_input.max_value = 1.0
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-1280, 27)

    # New shader model
    new_shader_model = new_grp.nodes.new('ShaderNodeValue')
    new_shader_model.name = 'NewShaderModel'
    new_shader_model.label = 'NewShaderModel'
    new_shader_model.location = (-1280, 118)
    new_shader_model.outputs[0].default_value = 0.0

    # Principled BSDF (For new shader model)
    principled_bsdf = new_grp.nodes.new('ShaderNodeBsdfPrincipled')
    principled_bsdf.location = (-1038, 874)
    principled_bsdf.inputs['Metallic'].default_value = 0.5

    # Invert (for roughness)
    invert = new_grp.nodes.new('ShaderNodeInvert')
    invert.location = (-1256, 492)
    invert.inputs[0].default_value = 1.0

    # Gamma (for roughness)
    gamma = new_grp.nodes.new('ShaderNodeGamma')
    gamma.location = (-1256, 640)
    gamma.inputs[1].default_value = 10.0

    # Diffuse BSDF (Multiplies dynamic lighting with diffuse)
    diffuse_bdsf = new_grp.nodes.new('ShaderNodeBsdfDiffuse')
    diffuse_bdsf.location = (-945, 293)

    # Mix shader (interpolates Principled and Diffuse BSDF)
    new_shader_model_mix1 = new_grp.nodes.new('ShaderNodeMixShader')
    new_shader_model_mix1.location = (-760, 340)

    # Multiply (Multiplies static lightmap with diffuse)
    lightmap_mult = new_grp.nodes.new('ShaderNodeMixRGB')
    lightmap_mult.location = (-944, 122)
    lightmap_mult.blend_type = 'MULTIPLY'
    lightmap_mult.inputs['Fac'].default_value = 1.0

    # Multiply (Multiplies specular with reflection)
    specular_mult = new_grp.nodes.new('ShaderNodeMixRGB')
    specular_mult.location = (-940, -105)
    specular_mult.blend_type = 'MULTIPLY'
    specular_mult.inputs['Fac'].default_value = 1.0

    # Multiply (Multiplies extended specular with reflection)
    extended_specular_mult = new_grp.nodes.new('ShaderNodeMixRGB')
    extended_specular_mult.location = (-941, -304)
    extended_specular_mult.blend_type = 'MULTIPLY'
    extended_specular_mult.inputs['Fac'].default_value = 1.0

    # Add Shader (Adds dynamic diffuse with static diffuse)
    diffuse_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    diffuse_add_shader.location = (-587, 209)

    # Mix shader (interpolates resolved reflection with nothing)
    new_shader_model_mix2 = new_grp.nodes.new('ShaderNodeMixShader')
    new_shader_model_mix2.location = (-512, -38)

    # Add Shader (Adds emissive with resolved reflection)
    emissive_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    emissive_add_shader.location = (-320, 8)

    # Add Shader (Adds specular and extended specular reflections)
    specular_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    specular_add_shader.location = (-734, -81)

    # Diffuse BDSF (Multiplies extended specular with dynamic lighting)
    extended_specular_bdsf = new_grp.nodes.new('ShaderNodeBsdfDiffuse')
    extended_specular_bdsf.location = (-738, -280)

    # Add shader (Adds diffuse with all emissive sources)
    final_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    final_add_shader.location = (-184, 234)

    # Transparent BDSF (Provides alpha)
    transparent_bdsf = new_grp.nodes.new('ShaderNodeBsdfTransparent')
    transparent_bdsf.location = (-224, -160)
    transparent_bdsf.inputs['Color'].default_value = (1.0, 1.0, 1.0, 1.0)

    # Mix Shader (Applies alpha proportion)
    alpha_mix = new_grp.nodes.new('ShaderNodeMixShader')
    alpha_mix.location = (-40, -112)

    # Group outputs (Final output)
    mat_out = new_grp.nodes.new('NodeGroupOutput')
    mat_out.location = (150, -88)

    # Links
    new_grp.links.new(grp_in.outputs['Lightmap'], lightmap_mult.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['Diffuse'], lightmap_mult.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Diffuse'], diffuse_bdsf.inputs['Color'])
    new_grp.links.new(grp_in.outputs['Diffuse'], principled_bsdf.inputs['Base Color'])
    new_grp.links.new(grp_in.outputs['Emissive'], emissive_add_shader.inputs[0])
    new_grp.links.new(grp_in.outputs['Specular'], specular_mult.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['Specular'], principled_bsdf.inputs['Specular'])
    new_grp.links.new(grp_in.outputs['ExtendedSpecular'], extended_specular_mult.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['Reflection'], specular_mult.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Reflection'], extended_specular_mult.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Alpha'], alpha_mix.inputs['Fac'])
    new_grp.links.new(new_shader_model.outputs['Value'], new_shader_model_mix1.inputs[0])
    new_grp.links.new(diffuse_bdsf.outputs['BSDF'], new_shader_model_mix1.inputs[1])
    new_grp.links.new(grp_in.outputs['Specular'], invert.inputs['Color'])
    new_grp.links.new(invert.outputs['Color'], gamma.inputs['Color'])
    new_grp.links.new(gamma.outputs['Color'], principled_bsdf.inputs['Roughness'])
    new_grp.links.new(principled_bsdf.outputs['BSDF'], new_shader_model_mix1.inputs[2])
    new_grp.links.new(new_shader_model_mix1.outputs['Shader'], diffuse_add_shader.inputs[0])
    new_grp.links.new(lightmap_mult.outputs['Color'], diffuse_add_shader.inputs[1])
    new_grp.links.new(specular_mult.outputs['Color'], specular_add_shader.inputs[0])
    new_grp.links.new(extended_specular_mult.outputs['Color'], extended_specular_bdsf.inputs['Color'])
    new_grp.links.new(extended_specular_bdsf.outputs['BSDF'], specular_add_shader.inputs[1])
    new_grp.links.new(new_shader_model.outputs['Value'], new_shader_model_mix2.inputs[0])
    new_grp.links.new(specular_add_shader.outputs['Shader'], new_shader_model_mix2.inputs[1])
    new_grp.links.new(new_shader_model_mix2.outputs['Shader'], emissive_add_shader.inputs[1])
    new_grp.links.new(diffuse_add_shader.outputs['Shader'], final_add_shader.inputs[0])
    new_grp.links.new(emissive_add_shader.outputs['Shader'], final_add_shader.inputs[1])
    new_grp.links.new(transparent_bdsf.outputs['BSDF'], alpha_mix.inputs[1])
    new_grp.links.new(final_add_shader.outputs['Shader'], alpha_mix.inputs[2])
    new_grp.links.new(alpha_mix.outputs['Shader'], mat_out.inputs['Surface'])

def make_retro_dynamic_shader():
    new_grp = bpy.data.node_groups.new('RetroDynamicShader', 'ShaderNodeTree')
    surface_output = new_grp.outputs.new('NodeSocketShader', 'Surface')
    lightmap_input = new_grp.inputs.new('NodeSocketColor', 'Lightmap')
    lightmap_input.default_value = (0.0, 0.0, 0.0, 0.0)
    diffuse_input = new_grp.inputs.new('NodeSocketColor', 'Diffuse')
    diffuse_input.default_value = (0.0, 0.0, 0.0, 0.0)
    emissive_input = new_grp.inputs.new('NodeSocketColor', 'Emissive')
    emissive_input.default_value = (0.0, 0.0, 0.0, 0.0)
    specular_input = new_grp.inputs.new('NodeSocketColor', 'Specular')
    specular_input.default_value = (0.0, 0.0, 0.0, 0.0)
    ext_spec_input = new_grp.inputs.new('NodeSocketColor', 'ExtendedSpecular')
    ext_spec_input.default_value = (0.0, 0.0, 0.0, 0.0)
    reflection_input = new_grp.inputs.new('NodeSocketColor', 'Reflection')
    reflection_input.default_value = (0.0, 0.0, 0.0, 0.0)
    indirect_tex = new_grp.inputs.new('NodeSocketColor', 'IndirectTex')
    indirect_tex.default_value = (0.0, 0.0, 0.0, 0.0)
    alpha_input = new_grp.inputs.new('NodeSocketFloatFactor', 'Alpha')
    alpha_input.default_value = 1.0
    alpha_input.min_value = 0.0
    alpha_input.max_value = 1.0
    dynamic_input = new_grp.inputs.new('NodeSocketColor', 'DynamicTest')
    dynamic_input.default_value = (1.0, 1.0, 1.0, 1.0)
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-1460, 27)

    # Multiply (Lightmap dynamic)
    lightmap_dynamic = new_grp.nodes.new('ShaderNodeMixRGB')
    lightmap_dynamic.location = (-1174, 158)
    lightmap_dynamic.blend_type = 'MULTIPLY'
    lightmap_dynamic.inputs['Fac'].default_value = 1.0

    # Multiply (Diffuse dynamic)
    diffuse_dynamic = new_grp.nodes.new('ShaderNodeMixRGB')
    diffuse_dynamic.location = (-1174, -32)
    diffuse_dynamic.blend_type = 'MULTIPLY'
    diffuse_dynamic.inputs['Fac'].default_value = 1.0

    # Multiply (Emissive dynamic)
    emissive_dynamic = new_grp.nodes.new('ShaderNodeMixRGB')
    emissive_dynamic.location = (-1174, -222)
    emissive_dynamic.blend_type = 'MULTIPLY'
    emissive_dynamic.inputs['Fac'].default_value = 1.0

    # New shader model
    new_shader_model = new_grp.nodes.new('ShaderNodeValue')
    new_shader_model.name = 'NewShaderModel'
    new_shader_model.label = 'NewShaderModel'
    new_shader_model.location = (-1460, 118)
    new_shader_model.outputs[0].default_value = 0.0

    # Principled BSDF (For new shader model)
    principled_bsdf = new_grp.nodes.new('ShaderNodeBsdfPrincipled')
    principled_bsdf.location = (-1038, 874)
    principled_bsdf.inputs['Metallic'].default_value = 0.5

    # Invert (for roughness)
    invert = new_grp.nodes.new('ShaderNodeInvert')
    invert.location = (-1256, 492)
    invert.inputs[0].default_value = 1.0

    # Gamma (for roughness)
    gamma = new_grp.nodes.new('ShaderNodeGamma')
    gamma.location = (-1256, 640)
    gamma.inputs[1].default_value = 10.0

    # Diffuse BSDF (Multiplies dynamic lighting with diffuse)
    diffuse_bdsf = new_grp.nodes.new('ShaderNodeBsdfDiffuse')
    diffuse_bdsf.location = (-945, 293)

    # Mix shader (interpolates Principled and Diffuse BSDF)
    new_shader_model_mix1 = new_grp.nodes.new('ShaderNodeMixShader')
    new_shader_model_mix1.location = (-760, 340)

    # Multiply (Multiplies static lightmap with diffuse)
    lightmap_mult = new_grp.nodes.new('ShaderNodeMixRGB')
    lightmap_mult.location = (-944, 122)
    lightmap_mult.blend_type = 'MULTIPLY'
    lightmap_mult.inputs['Fac'].default_value = 1.0

    # Multiply (Multiplies specular with reflection)
    specular_mult = new_grp.nodes.new('ShaderNodeMixRGB')
    specular_mult.location = (-940, -105)
    specular_mult.blend_type = 'MULTIPLY'
    specular_mult.inputs['Fac'].default_value = 1.0

    # Multiply (Multiplies extended specular with reflection)
    extended_specular_mult = new_grp.nodes.new('ShaderNodeMixRGB')
    extended_specular_mult.location = (-941, -304)
    extended_specular_mult.blend_type = 'MULTIPLY'
    extended_specular_mult.inputs['Fac'].default_value = 1.0

    # Add Shader (Adds dynamic diffuse with static diffuse)
    diffuse_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    diffuse_add_shader.location = (-587, 209)

    # Mix shader (interpolates resolved reflection with nothing)
    new_shader_model_mix2 = new_grp.nodes.new('ShaderNodeMixShader')
    new_shader_model_mix2.location = (-512, -38)

    # Add Shader (Adds emissive with resolved reflection)
    emissive_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    emissive_add_shader.location = (-320, 8)

    # Add Shader (Adds specular and extended specular reflections)
    specular_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    specular_add_shader.location = (-734, -81)

    # Diffuse BDSF (Multiplies extended specular with dynamic lighting)
    extended_specular_bdsf = new_grp.nodes.new('ShaderNodeBsdfDiffuse')
    extended_specular_bdsf.location = (-738, -280)

    # Add shader (Adds diffuse with all emissive sources)
    final_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    final_add_shader.location = (-184, 234)

    # Transparent BDSF (Provides alpha)
    transparent_bdsf = new_grp.nodes.new('ShaderNodeBsdfTransparent')
    transparent_bdsf.location = (-224, -160)
    transparent_bdsf.inputs['Color'].default_value = (1.0, 1.0, 1.0, 1.0)

    # Mix Shader (Applies alpha proportion)
    alpha_mix = new_grp.nodes.new('ShaderNodeMixShader')
    alpha_mix.location = (-40, -112)

    # Group outputs (Final output)
    mat_out = new_grp.nodes.new('NodeGroupOutput')
    mat_out.location = (150, -88)

    # Links
    new_grp.links.new(grp_in.outputs['Lightmap'], lightmap_dynamic.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['DynamicTest'], lightmap_dynamic.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Diffuse'], diffuse_dynamic.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['DynamicTest'], diffuse_dynamic.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Emissive'], emissive_dynamic.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['DynamicTest'], emissive_dynamic.inputs['Color2'])
    new_grp.links.new(lightmap_dynamic.outputs['Color'], lightmap_mult.inputs['Color1'])
    new_grp.links.new(diffuse_dynamic.outputs['Color'], lightmap_mult.inputs['Color2'])
    new_grp.links.new(diffuse_dynamic.outputs['Color'], diffuse_bdsf.inputs['Color'])
    new_grp.links.new(diffuse_dynamic.outputs['Color'], principled_bsdf.inputs['Base Color'])
    new_grp.links.new(emissive_dynamic.outputs['Color'], emissive_add_shader.inputs[0])
    new_grp.links.new(grp_in.outputs['Specular'], specular_mult.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['Specular'], principled_bsdf.inputs['Specular'])
    new_grp.links.new(grp_in.outputs['ExtendedSpecular'], extended_specular_mult.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['Reflection'], specular_mult.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Reflection'], extended_specular_mult.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Alpha'], alpha_mix.inputs['Fac'])
    new_grp.links.new(new_shader_model.outputs['Value'], new_shader_model_mix1.inputs[0])
    new_grp.links.new(diffuse_bdsf.outputs['BSDF'], new_shader_model_mix1.inputs[1])
    new_grp.links.new(grp_in.outputs['Specular'], invert.inputs['Color'])
    new_grp.links.new(invert.outputs['Color'], gamma.inputs['Color'])
    new_grp.links.new(gamma.outputs['Color'], principled_bsdf.inputs['Roughness'])
    new_grp.links.new(principled_bsdf.outputs['BSDF'], new_shader_model_mix1.inputs[2])
    new_grp.links.new(new_shader_model_mix1.outputs['Shader'], diffuse_add_shader.inputs[0])
    new_grp.links.new(lightmap_mult.outputs['Color'], diffuse_add_shader.inputs[1])
    new_grp.links.new(specular_mult.outputs['Color'], specular_add_shader.inputs[0])
    new_grp.links.new(extended_specular_mult.outputs['Color'], extended_specular_bdsf.inputs['Color'])
    new_grp.links.new(extended_specular_bdsf.outputs['BSDF'], specular_add_shader.inputs[1])
    new_grp.links.new(new_shader_model.outputs['Value'], new_shader_model_mix2.inputs[0])
    new_grp.links.new(specular_add_shader.outputs['Shader'], new_shader_model_mix2.inputs[1])
    new_grp.links.new(new_shader_model_mix2.outputs['Shader'], emissive_add_shader.inputs[1])
    new_grp.links.new(diffuse_add_shader.outputs['Shader'], final_add_shader.inputs[0])
    new_grp.links.new(emissive_add_shader.outputs['Shader'], final_add_shader.inputs[1])
    new_grp.links.new(transparent_bdsf.outputs['BSDF'], alpha_mix.inputs[1])
    new_grp.links.new(final_add_shader.outputs['Shader'], alpha_mix.inputs[2])
    new_grp.links.new(alpha_mix.outputs['Shader'], mat_out.inputs['Surface'])

def make_retro_dynamic_alpha_shader():
    new_grp = bpy.data.node_groups.new('RetroDynamicAlphaShader', 'ShaderNodeTree')
    surface_output = new_grp.outputs.new('NodeSocketShader', 'Surface')
    lightmap_input = new_grp.inputs.new('NodeSocketColor', 'Lightmap')
    lightmap_input.default_value = (0.0, 0.0, 0.0, 0.0)
    diffuse_input = new_grp.inputs.new('NodeSocketColor', 'Diffuse')
    diffuse_input.default_value = (0.0, 0.0, 0.0, 0.0)
    emissive_input = new_grp.inputs.new('NodeSocketColor', 'Emissive')
    emissive_input.default_value = (0.0, 0.0, 0.0, 0.0)
    specular_input = new_grp.inputs.new('NodeSocketColor', 'Specular')
    specular_input.default_value = (0.0, 0.0, 0.0, 0.0)
    ext_spec_input = new_grp.inputs.new('NodeSocketColor', 'ExtendedSpecular')
    ext_spec_input.default_value = (0.0, 0.0, 0.0, 0.0)
    reflection_input = new_grp.inputs.new('NodeSocketColor', 'Reflection')
    reflection_input.default_value = (0.0, 0.0, 0.0, 0.0)
    indirect_tex = new_grp.inputs.new('NodeSocketColor', 'IndirectTex')
    indirect_tex.default_value = (0.0, 0.0, 0.0, 0.0)
    alpha_input = new_grp.inputs.new('NodeSocketFloatFactor', 'Alpha')
    alpha_input.default_value = 1.0
    alpha_input.min_value = 0.0
    alpha_input.max_value = 1.0
    dynamic_input = new_grp.inputs.new('NodeSocketColor', 'DynamicTest')
    dynamic_input.default_value = (1.0, 1.0, 1.0, 1.0)
    dynamic_alpha_input = new_grp.inputs.new('NodeSocketFloatFactor', 'DynamicAlphaTest')
    dynamic_alpha_input.default_value = 1.0
    dynamic_alpha_input.min_value = 0.0
    dynamic_alpha_input.max_value = 1.0
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-1460, 27)

    # Multiply (Lightmap dynamic)
    lightmap_dynamic = new_grp.nodes.new('ShaderNodeMixRGB')
    lightmap_dynamic.location = (-1174, 158)
    lightmap_dynamic.blend_type = 'MULTIPLY'
    lightmap_dynamic.inputs['Fac'].default_value = 1.0

    # Multiply (Diffuse dynamic)
    diffuse_dynamic = new_grp.nodes.new('ShaderNodeMixRGB')
    diffuse_dynamic.location = (-1174, -32)
    diffuse_dynamic.blend_type = 'MULTIPLY'
    diffuse_dynamic.inputs['Fac'].default_value = 1.0

    # Multiply (Emissive dynamic)
    emissive_dynamic = new_grp.nodes.new('ShaderNodeMixRGB')
    emissive_dynamic.location = (-1174, -222)
    emissive_dynamic.blend_type = 'MULTIPLY'
    emissive_dynamic.inputs['Fac'].default_value = 1.0

    # Multiply (Alpha dynamic)
    alpha_dynamic = new_grp.nodes.new('ShaderNodeMath')
    alpha_dynamic.location = (-1174, -410)
    alpha_dynamic.operation = 'MULTIPLY'

    # New shader model
    new_shader_model = new_grp.nodes.new('ShaderNodeValue')
    new_shader_model.name = 'NewShaderModel'
    new_shader_model.label = 'NewShaderModel'
    new_shader_model.location = (-1460, 118)
    new_shader_model.outputs[0].default_value = 0.0

    # Principled BSDF (For new shader model)
    principled_bsdf = new_grp.nodes.new('ShaderNodeBsdfPrincipled')
    principled_bsdf.location = (-1038, 874)
    principled_bsdf.inputs['Metallic'].default_value = 0.5

    # Invert (for roughness)
    invert = new_grp.nodes.new('ShaderNodeInvert')
    invert.location = (-1256, 492)
    invert.inputs[0].default_value = 1.0

    # Gamma (for roughness)
    gamma = new_grp.nodes.new('ShaderNodeGamma')
    gamma.location = (-1256, 640)
    gamma.inputs[1].default_value = 10.0

    # Diffuse BSDF (Multiplies dynamic lighting with diffuse)
    diffuse_bdsf = new_grp.nodes.new('ShaderNodeBsdfDiffuse')
    diffuse_bdsf.location = (-945, 293)

    # Mix shader (interpolates Principled and Diffuse BSDF)
    new_shader_model_mix1 = new_grp.nodes.new('ShaderNodeMixShader')
    new_shader_model_mix1.location = (-760, 340)

    # Multiply (Multiplies static lightmap with diffuse)
    lightmap_mult = new_grp.nodes.new('ShaderNodeMixRGB')
    lightmap_mult.location = (-944, 122)
    lightmap_mult.blend_type = 'MULTIPLY'
    lightmap_mult.inputs['Fac'].default_value = 1.0

    # Multiply (Multiplies specular with reflection)
    specular_mult = new_grp.nodes.new('ShaderNodeMixRGB')
    specular_mult.location = (-940, -105)
    specular_mult.blend_type = 'MULTIPLY'
    specular_mult.inputs['Fac'].default_value = 1.0

    # Multiply (Multiplies extended specular with reflection)
    extended_specular_mult = new_grp.nodes.new('ShaderNodeMixRGB')
    extended_specular_mult.location = (-941, -304)
    extended_specular_mult.blend_type = 'MULTIPLY'
    extended_specular_mult.inputs['Fac'].default_value = 1.0

    # Add Shader (Adds dynamic diffuse with static diffuse)
    diffuse_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    diffuse_add_shader.location = (-587, 209)

    # Mix shader (interpolates resolved reflection with nothing)
    new_shader_model_mix2 = new_grp.nodes.new('ShaderNodeMixShader')
    new_shader_model_mix2.location = (-512, -38)

    # Add Shader (Adds emissive with resolved reflection)
    emissive_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    emissive_add_shader.location = (-320, 8)

    # Add Shader (Adds specular and extended specular reflections)
    specular_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    specular_add_shader.location = (-734, -81)

    # Diffuse BDSF (Multiplies extended specular with dynamic lighting)
    extended_specular_bdsf = new_grp.nodes.new('ShaderNodeBsdfDiffuse')
    extended_specular_bdsf.location = (-738, -280)

    # Add shader (Adds diffuse with all emissive sources)
    final_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    final_add_shader.location = (-184, 234)

    # Transparent BDSF (Provides alpha)
    transparent_bdsf = new_grp.nodes.new('ShaderNodeBsdfTransparent')
    transparent_bdsf.location = (-224, -160)
    transparent_bdsf.inputs['Color'].default_value = (1.0, 1.0, 1.0, 1.0)

    # Mix Shader (Applies alpha proportion)
    alpha_mix = new_grp.nodes.new('ShaderNodeMixShader')
    alpha_mix.location = (-40, -112)

    # Material Output (Final output)
    mat_out = new_grp.nodes.new('NodeGroupOutput')
    mat_out.location = (150, -88)

    # Links
    new_grp.links.new(grp_in.outputs['Lightmap'], lightmap_dynamic.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['DynamicTest'], lightmap_dynamic.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Diffuse'], diffuse_dynamic.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['DynamicTest'], diffuse_dynamic.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Emissive'], emissive_dynamic.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['DynamicTest'], emissive_dynamic.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Alpha'], alpha_dynamic.inputs[0])
    new_grp.links.new(grp_in.outputs['DynamicAlphaTest'], alpha_dynamic.inputs[1])
    new_grp.links.new(lightmap_dynamic.outputs['Color'], lightmap_mult.inputs['Color1'])
    new_grp.links.new(diffuse_dynamic.outputs['Color'], lightmap_mult.inputs['Color2'])
    new_grp.links.new(diffuse_dynamic.outputs['Color'], diffuse_bdsf.inputs['Color'])
    new_grp.links.new(diffuse_dynamic.outputs['Color'], principled_bsdf.inputs['Base Color'])
    new_grp.links.new(emissive_dynamic.outputs['Color'], emissive_add_shader.inputs[0])
    new_grp.links.new(grp_in.outputs['Specular'], specular_mult.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['Specular'], principled_bsdf.inputs['Specular'])
    new_grp.links.new(grp_in.outputs['ExtendedSpecular'], extended_specular_mult.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['Reflection'], specular_mult.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Reflection'], extended_specular_mult.inputs['Color2'])
    new_grp.links.new(new_shader_model.outputs['Value'], new_shader_model_mix1.inputs[0])
    new_grp.links.new(diffuse_bdsf.outputs['BSDF'], new_shader_model_mix1.inputs[1])
    new_grp.links.new(grp_in.outputs['Specular'], invert.inputs['Color'])
    new_grp.links.new(invert.outputs['Color'], gamma.inputs['Color'])
    new_grp.links.new(gamma.outputs['Color'], principled_bsdf.inputs['Roughness'])
    new_grp.links.new(principled_bsdf.outputs['BSDF'], new_shader_model_mix1.inputs[2])
    new_grp.links.new(new_shader_model_mix1.outputs['Shader'], diffuse_add_shader.inputs[0])
    new_grp.links.new(alpha_dynamic.outputs['Value'], alpha_mix.inputs['Fac'])
    new_grp.links.new(lightmap_mult.outputs['Color'], diffuse_add_shader.inputs[1])
    new_grp.links.new(specular_mult.outputs['Color'], specular_add_shader.inputs[0])
    new_grp.links.new(extended_specular_mult.outputs['Color'], extended_specular_bdsf.inputs['Color'])
    new_grp.links.new(extended_specular_bdsf.outputs['BSDF'], specular_add_shader.inputs[1])
    new_grp.links.new(new_shader_model.outputs['Value'], new_shader_model_mix2.inputs[0])
    new_grp.links.new(specular_add_shader.outputs['Shader'], new_shader_model_mix2.inputs[1])
    new_grp.links.new(new_shader_model_mix2.outputs['Shader'], emissive_add_shader.inputs[1])
    new_grp.links.new(diffuse_add_shader.outputs['Shader'], final_add_shader.inputs[0])
    new_grp.links.new(emissive_add_shader.outputs['Shader'], final_add_shader.inputs[1])
    new_grp.links.new(transparent_bdsf.outputs['BSDF'], alpha_mix.inputs[1])
    new_grp.links.new(final_add_shader.outputs['Shader'], alpha_mix.inputs[2])
    new_grp.links.new(alpha_mix.outputs['Shader'], mat_out.inputs['Surface'])

def make_retro_dynamic_character_shader():
    new_grp = bpy.data.node_groups.new('RetroDynamicCharacterShader', 'ShaderNodeTree')
    surface_output = new_grp.outputs.new('NodeSocketShader', 'Surface')
    lightmap_input = new_grp.inputs.new('NodeSocketColor', 'Lightmap')
    lightmap_input.default_value = (0.0, 0.0, 0.0, 0.0)
    diffuse_input = new_grp.inputs.new('NodeSocketColor', 'Diffuse')
    diffuse_input.default_value = (0.0, 0.0, 0.0, 0.0)
    emissive_input = new_grp.inputs.new('NodeSocketColor', 'Emissive')
    emissive_input.default_value = (0.0, 0.0, 0.0, 0.0)
    specular_input = new_grp.inputs.new('NodeSocketColor', 'Specular')
    specular_input.default_value = (0.0, 0.0, 0.0, 0.0)
    ext_spec_input = new_grp.inputs.new('NodeSocketColor', 'ExtendedSpecular')
    ext_spec_input.default_value = (0.0, 0.0, 0.0, 0.0)
    reflection_input = new_grp.inputs.new('NodeSocketColor', 'Reflection')
    reflection_input.default_value = (0.0, 0.0, 0.0, 0.0)
    indirect_tex = new_grp.inputs.new('NodeSocketColor', 'IndirectTex')
    indirect_tex.default_value = (0.0, 0.0, 0.0, 0.0)
    alpha_input = new_grp.inputs.new('NodeSocketFloatFactor', 'Alpha')
    alpha_input.default_value = 1.0
    alpha_input.min_value = 0.0
    alpha_input.max_value = 1.0
    dynamic_input = new_grp.inputs.new('NodeSocketColor', 'DynamicTest')
    dynamic_input.default_value = (1.0, 1.0, 1.0, 1.0)
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-1460, 27)

    # Multiply (Emissive dynamic)
    emissive_dynamic = new_grp.nodes.new('ShaderNodeMixRGB')
    emissive_dynamic.location = (-1174, -32)
    emissive_dynamic.blend_type = 'MULTIPLY'
    emissive_dynamic.inputs['Fac'].default_value = 1.0

    # New shader model
    new_shader_model = new_grp.nodes.new('ShaderNodeValue')
    new_shader_model.name = 'NewShaderModel'
    new_shader_model.label = 'NewShaderModel'
    new_shader_model.location = (-1460, 118)
    new_shader_model.outputs[0].default_value = 0.0

    # Principled BSDF (For new shader model)
    principled_bsdf = new_grp.nodes.new('ShaderNodeBsdfPrincipled')
    principled_bsdf.location = (-1038, 874)
    principled_bsdf.inputs['Metallic'].default_value = 0.5

    # Invert (for roughness)
    invert = new_grp.nodes.new('ShaderNodeInvert')
    invert.location = (-1256, 492)
    invert.inputs[0].default_value = 1.0

    # Gamma (for roughness)
    gamma = new_grp.nodes.new('ShaderNodeGamma')
    gamma.location = (-1256, 640)
    gamma.inputs[1].default_value = 10.0

    # Diffuse BSDF (Multiplies dynamic lighting with diffuse)
    diffuse_bdsf = new_grp.nodes.new('ShaderNodeBsdfDiffuse')
    diffuse_bdsf.location = (-945, 293)

    # Mix shader (interpolates Principled and Diffuse BSDF)
    new_shader_model_mix1 = new_grp.nodes.new('ShaderNodeMixShader')
    new_shader_model_mix1.location = (-760, 340)

    # Multiply (Multiplies static lightmap with diffuse)
    lightmap_mult = new_grp.nodes.new('ShaderNodeMixRGB')
    lightmap_mult.location = (-944, 122)
    lightmap_mult.blend_type = 'MULTIPLY'
    lightmap_mult.inputs['Fac'].default_value = 1.0

    # Multiply (Multiplies specular with reflection)
    specular_mult = new_grp.nodes.new('ShaderNodeMixRGB')
    specular_mult.location = (-940, -105)
    specular_mult.blend_type = 'MULTIPLY'
    specular_mult.inputs['Fac'].default_value = 1.0

    # Multiply (Multiplies extended specular with reflection)
    extended_specular_mult = new_grp.nodes.new('ShaderNodeMixRGB')
    extended_specular_mult.location = (-941, -304)
    extended_specular_mult.blend_type = 'MULTIPLY'
    extended_specular_mult.inputs['Fac'].default_value = 1.0

    # Add Shader (Adds dynamic diffuse with static diffuse)
    diffuse_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    diffuse_add_shader.location = (-587, 209)

    # Mix shader (interpolates resolved reflection with nothing)
    new_shader_model_mix2 = new_grp.nodes.new('ShaderNodeMixShader')
    new_shader_model_mix2.location = (-512, -38)

    # Add Shader (Adds emissive with resolved reflection)
    emissive_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    emissive_add_shader.location = (-320, 8)

    # Add Shader (Adds specular and extended specular reflections)
    specular_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    specular_add_shader.location = (-734, -81)

    # Diffuse BDSF (Multiplies extended specular with dynamic lighting)
    extended_specular_bdsf = new_grp.nodes.new('ShaderNodeBsdfDiffuse')
    extended_specular_bdsf.location = (-738, -280)

    # Add shader (Adds diffuse with all emissive sources)
    final_add_shader = new_grp.nodes.new('ShaderNodeAddShader')
    final_add_shader.location = (-184, 234)

    # Transparent BDSF (Provides alpha)
    transparent_bdsf = new_grp.nodes.new('ShaderNodeBsdfTransparent')
    transparent_bdsf.location = (-224, -160)
    transparent_bdsf.inputs['Color'].default_value = (1.0, 1.0, 1.0, 1.0)

    # Mix Shader (Applies alpha proportion)
    alpha_mix = new_grp.nodes.new('ShaderNodeMixShader')
    alpha_mix.location = (-40, -112)

    # Material Output (Final output)
    mat_out = new_grp.nodes.new('NodeGroupOutput')
    mat_out.location = (150, -88)

    # Links
    new_grp.links.new(grp_in.outputs['Emissive'], emissive_dynamic.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['DynamicTest'], emissive_dynamic.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Lightmap'], lightmap_mult.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['Diffuse'], lightmap_mult.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Diffuse'], diffuse_bdsf.inputs['Color'])
    new_grp.links.new(grp_in.outputs['Diffuse'], principled_bsdf.inputs['Base Color'])
    new_grp.links.new(emissive_dynamic.outputs['Color'], emissive_add_shader.inputs[0])
    new_grp.links.new(grp_in.outputs['Specular'], specular_mult.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['Specular'], principled_bsdf.inputs['Specular'])
    new_grp.links.new(grp_in.outputs['ExtendedSpecular'], extended_specular_mult.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['Reflection'], specular_mult.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Reflection'], extended_specular_mult.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Alpha'], alpha_mix.inputs['Fac'])
    new_grp.links.new(new_shader_model.outputs['Value'], new_shader_model_mix1.inputs[0])
    new_grp.links.new(diffuse_bdsf.outputs['BSDF'], new_shader_model_mix1.inputs[1])
    new_grp.links.new(grp_in.outputs['Specular'], invert.inputs['Color'])
    new_grp.links.new(invert.outputs['Color'], gamma.inputs['Color'])
    new_grp.links.new(gamma.outputs['Color'], principled_bsdf.inputs['Roughness'])
    new_grp.links.new(principled_bsdf.outputs['BSDF'], new_shader_model_mix1.inputs[2])
    new_grp.links.new(new_shader_model_mix1.outputs['Shader'], diffuse_add_shader.inputs[0])
    new_grp.links.new(lightmap_mult.outputs['Color'], diffuse_add_shader.inputs[1])
    new_grp.links.new(specular_mult.outputs['Color'], specular_add_shader.inputs[0])
    new_grp.links.new(extended_specular_mult.outputs['Color'], extended_specular_bdsf.inputs['Color'])
    new_grp.links.new(extended_specular_bdsf.outputs['BSDF'], specular_add_shader.inputs[1])
    new_grp.links.new(new_shader_model.outputs['Value'], new_shader_model_mix2.inputs[0])
    new_grp.links.new(specular_add_shader.outputs['Shader'], new_shader_model_mix2.inputs[1])
    new_grp.links.new(new_shader_model_mix2.outputs['Shader'], emissive_add_shader.inputs[1])
    new_grp.links.new(diffuse_add_shader.outputs['Shader'], final_add_shader.inputs[0])
    new_grp.links.new(emissive_add_shader.outputs['Shader'], final_add_shader.inputs[1])
    new_grp.links.new(transparent_bdsf.outputs['BSDF'], alpha_mix.inputs[1])
    new_grp.links.new(final_add_shader.outputs['Shader'], alpha_mix.inputs[2])
    new_grp.links.new(alpha_mix.outputs['Shader'], mat_out.inputs['Surface'])

ROOT_SHADER_GROUPS = (
    make_retro_shader,
    make_retro_dynamic_shader,
    make_retro_dynamic_alpha_shader,
    make_retro_dynamic_character_shader
)

# UV animation nodes:
# http://www.metroid2002.com/retromodding/wiki/Materials_(Metroid_Prime)#UV_Animations

# 0 - Modelview Inverse (zero translation)
def make_uva0():
    new_grp = bpy.data.node_groups.new('RetroUVMode0NodeN', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-100, 0)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (1000, 0)

    # Vector Transform to bring world space into camera space
    vec_xf = new_grp.nodes.new('ShaderNodeVectorTransform')
    vec_xf.location = (100, 0)
    vec_xf.vector_type = 'NORMAL'
    vec_xf.convert_from = 'WORLD'
    vec_xf.convert_to = 'CAMERA'

    # UV scale (to match GameCube's UV-coordinate space)
    uv_scale = new_grp.nodes.new('ShaderNodeMapping')
    uv_scale.location = (400, -400)
    uv_scale.vector_type = 'TEXTURE'
    uv_scale.scale[0] = 2.0
    uv_scale.scale[1] = 2.0
    uv_scale.translation[0] = 1.0
    uv_scale.translation[1] = 1.0
    
    # Links
    new_grp.links.new(grp_in.outputs[0], vec_xf.inputs[0])
    new_grp.links.new(vec_xf.outputs[0], uv_scale.inputs[0])
    new_grp.links.new(uv_scale.outputs[0], grp_out.inputs[0])

# 1 - Modelview Inverse
def make_uva1():
    new_grp = bpy.data.node_groups.new('RetroUVMode1NodeN', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-300, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (500, 0)
    
    # Links
    new_grp.links.new(grp_in.outputs[0], grp_out.inputs[0])

# 2 - UV Scroll
def make_uva2():
    new_grp = bpy.data.node_groups.new('RetroUVMode2Node', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.inputs.new('NodeSocketVector', 'Offset')
    new_grp.inputs.new('NodeSocketVector', 'Scale')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-457, 22)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (500, 0)

    # Mapping
    mapping = new_grp.nodes.new('ShaderNodeMapping')
    mapping.location = (-235, 125)
    drvs = mapping.driver_add('scale')
    for drv in drvs:
        drv.driver.expression = 'frame/60'
    
    # Adder1
    adder1 = new_grp.nodes.new('ShaderNodeVectorMath')
    adder1.operation = 'ADD'
    adder1.location = (100, 0)
    
    # Adder2
    adder2 = new_grp.nodes.new('ShaderNodeVectorMath')
    adder2.operation = 'ADD'
    adder2.location = (100, 200)
    
    # Links
    new_grp.links.new(grp_in.outputs[0], adder2.inputs[0])
    new_grp.links.new(grp_in.outputs[1], adder1.inputs[0])
    new_grp.links.new(grp_in.outputs[2], mapping.inputs[0])
    new_grp.links.new(mapping.outputs[0], adder1.inputs[1])
    new_grp.links.new(adder1.outputs[0], adder2.inputs[1])
    new_grp.links.new(adder2.outputs[0], grp_out.inputs[0])

# 3 - Rotation
def make_uva3():
    new_grp = bpy.data.node_groups.new('RetroUVMode3Node', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.inputs.new('NodeSocketFloat', 'Offset')
    new_grp.inputs.new('NodeSocketFloat', 'Scale')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-100, 0)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (700, 0)
    
    # Adder1
    add1 = new_grp.nodes.new('ShaderNodeMath')
    add1.operation = 'ADD'
    add1.location = (500, 0)

    # Multiply
    mult = new_grp.nodes.new('ShaderNodeMath')
    mult.operation = 'MULTIPLY'
    mult.location = (230, -112)
    drv = mult.inputs[1].driver_add('default_value')
    drv.driver.expression = 'frame/60'
    
    # Links
    new_grp.links.new(grp_in.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], add1.inputs[0])
    new_grp.links.new(grp_in.outputs[2], mult.inputs[0])
    new_grp.links.new(mult.outputs[0], add1.inputs[1])

# 4 - Horizontal Filmstrip Animation
def make_uva4():
    new_grp = bpy.data.node_groups.new('RetroUVMode4Node', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.inputs.new('NodeSocketFloat', 'Scale')
    new_grp.inputs.new('NodeSocketFloat', 'NumFrames')
    new_grp.inputs.new('NodeSocketFloat', 'Step')
    new_grp.inputs.new('NodeSocketFloat', 'Offset')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-1000, 0)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (800, 0)
    
    # Multiply1
    mult1 = new_grp.nodes.new('ShaderNodeMath')
    mult1.operation = 'MULTIPLY'
    mult1.location = (-800, 0)
    
    # Multiply2
    mult2 = new_grp.nodes.new('ShaderNodeMath')
    mult2.operation = 'MULTIPLY'
    mult2.location = (-600, 0)
    
    # Modulo
    mod1 = new_grp.nodes.new('ShaderNodeMath')
    mod1.operation = 'MODULO'
    mod1.inputs[1].default_value = 1.0
    mod1.location = (-400, 0)
    
    # Multiply3
    mult3 = new_grp.nodes.new('ShaderNodeMath')
    mult3.operation = 'MULTIPLY'
    mult3.location = (-200, 0)
    
    # Multiply4
    mult4 = new_grp.nodes.new('ShaderNodeMath')
    mult4.operation = 'MULTIPLY'
    mult4.location = (0, 0)
    
    # Mapping
    map1 = new_grp.nodes.new('ShaderNodeMapping')
    map1.scale = (1.0, 0.0, 0.0)
    map1.location = (200, 0)
    
    # Add
    add1 = new_grp.nodes.new('ShaderNodeVectorMath')
    add1.operation = 'ADD'
    add1.location = (600, 0)

    # Timing Add
    time_add = new_grp.nodes.new('ShaderNodeMath')
    time_add.operation = 'ADD'
    time_add.location = (-802, -180)
    drv = time_add.inputs[1].driver_add('default_value')
    drv.driver.expression = 'frame/60'

    # Floor
    floor = new_grp.nodes.new('ShaderNodeMath')
    floor.operation = 'FLOOR'
    floor.location = (-204, -180)
    floor.inputs[1].default_value = 0.0
    
    # Links
    new_grp.links.new(grp_in.outputs[0], add1.inputs[1])
    new_grp.links.new(grp_in.outputs[1], mult1.inputs[1])
    new_grp.links.new(grp_in.outputs[2], mult3.inputs[1])
    new_grp.links.new(grp_in.outputs[3], mult4.inputs[1])
    new_grp.links.new(grp_in.outputs[3], mult1.inputs[0])
    new_grp.links.new(grp_in.outputs[4], time_add.inputs[0])
    new_grp.links.new(time_add.outputs[0], mult2.inputs[1])
    new_grp.links.new(mult1.outputs[0], mult2.inputs[0])
    new_grp.links.new(mult2.outputs[0], mod1.inputs[0])
    new_grp.links.new(mod1.outputs[0], mult3.inputs[0])
    new_grp.links.new(mult3.outputs[0], floor.inputs[0])
    new_grp.links.new(floor.outputs[0], mult4.inputs[0])
    new_grp.links.new(mult4.outputs[0], map1.inputs[0])
    new_grp.links.new(map1.outputs[0], add1.inputs[0])
    new_grp.links.new(add1.outputs[0], grp_out.inputs[0])

# 5 - Vertical Filmstrip Animation
def make_uva5():
    new_grp = bpy.data.node_groups.new('RetroUVMode5Node', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.inputs.new('NodeSocketFloat', 'Scale')
    new_grp.inputs.new('NodeSocketFloat', 'NumFrames')
    new_grp.inputs.new('NodeSocketFloat', 'Step')
    new_grp.inputs.new('NodeSocketFloat', 'Offset')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-1000, 0)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (800, 0)
    
    # Multiply1
    mult1 = new_grp.nodes.new('ShaderNodeMath')
    mult1.operation = 'MULTIPLY'
    mult1.location = (-800, 0)
    
    # Multiply2
    mult2 = new_grp.nodes.new('ShaderNodeMath')
    mult2.operation = 'MULTIPLY'
    mult2.location = (-600, 0)
    
    # Modulo
    mod1 = new_grp.nodes.new('ShaderNodeMath')
    mod1.operation = 'MODULO'
    mod1.inputs[1].default_value = 1.0
    mod1.location = (-400, 0)
    
    # Multiply3
    mult3 = new_grp.nodes.new('ShaderNodeMath')
    mult3.operation = 'MULTIPLY'
    mult3.location = (-200, 0)
    
    # Multiply4
    mult4 = new_grp.nodes.new('ShaderNodeMath')
    mult4.operation = 'MULTIPLY'
    mult4.location = (0, 0)
    
    # Mapping
    map1 = new_grp.nodes.new('ShaderNodeMapping')
    map1.scale = (0.0, 1.0, 0.0)
    map1.location = (200, 0)
    
    # Add
    add1 = new_grp.nodes.new('ShaderNodeVectorMath')
    add1.operation = 'ADD'
    add1.location = (600, 0)

    # Timing Add
    time_add = new_grp.nodes.new('ShaderNodeMath')
    time_add.operation = 'ADD'
    time_add.location = (-802, -180)
    drv = time_add.inputs[1].driver_add('default_value')
    drv.driver.expression = 'frame/60'

    # Floor
    floor = new_grp.nodes.new('ShaderNodeMath')
    floor.operation = 'FLOOR'
    floor.location = (-204, -180)
    floor.inputs[1].default_value = 0.0
    
    # Links
    new_grp.links.new(grp_in.outputs[0], add1.inputs[1])
    new_grp.links.new(grp_in.outputs[1], mult1.inputs[1])
    new_grp.links.new(grp_in.outputs[2], mult3.inputs[1])
    new_grp.links.new(grp_in.outputs[3], mult4.inputs[1])
    new_grp.links.new(grp_in.outputs[3], mult1.inputs[0])
    new_grp.links.new(grp_in.outputs[4], time_add.inputs[0])
    new_grp.links.new(time_add.outputs[0], mult2.inputs[1])
    new_grp.links.new(mult1.outputs[0], mult2.inputs[0])
    new_grp.links.new(mult2.outputs[0], mod1.inputs[0])
    new_grp.links.new(mod1.outputs[0], mult3.inputs[0])
    new_grp.links.new(mult3.outputs[0], floor.inputs[0])
    new_grp.links.new(floor.outputs[0], mult4.inputs[0])
    new_grp.links.new(mult4.outputs[0], map1.inputs[0])
    new_grp.links.new(map1.outputs[0], add1.inputs[0])
    new_grp.links.new(add1.outputs[0], grp_out.inputs[0])

# 6 - Model Matrix
def make_uva6():
    new_grp = bpy.data.node_groups.new('RetroUVMode6NodeN', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-100, 0)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (300, 0)
    
    # Geometry input
    geom_in = new_grp.nodes.new('ShaderNodeTexCoord')
    geom_in.location = (-300, 0)
    
    # Adder1
    adder1 = new_grp.nodes.new('ShaderNodeVectorMath')
    adder1.operation = 'ADD'
    adder1.location = (100, 0)
    
    # Links
    new_grp.links.new(grp_in.outputs[0], adder1.inputs[0])
    new_grp.links.new(geom_in.outputs['Object'], adder1.inputs[1])
    new_grp.links.new(adder1.outputs[0], grp_out.inputs[0])

# 7 - Mode Who Must Not Be Named
def make_uva7():
    new_grp = bpy.data.node_groups.new('RetroUVMode7NodeN', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.inputs.new('NodeSocketFloat', 'ParamA')
    new_grp.inputs.new('NodeSocketFloat', 'ParamB')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)
    
    # Geometry input
    geom_in = new_grp.nodes.new('ShaderNodeTexCoord')
    geom_in.location = (-1000, 0)
    
    # View flip
    view_flip = new_grp.nodes.new('ShaderNodeMapping')
    view_flip.location = (-800, -150)
    view_flip.vector_type = 'TEXTURE'
    view_flip.scale = (-1.0, -1.0, 1.0)

    # Separate
    sep1 = new_grp.nodes.new('ShaderNodeSeparateRGB')
    sep1.location = (-400, -200)

    # Add1
    add1 = new_grp.nodes.new('ShaderNodeMath')
    add1.operation = 'ADD'
    add1.location = (-200, -200)

    # Multiply1
    mult1 = new_grp.nodes.new('ShaderNodeMath')
    mult1.operation = 'MULTIPLY'
    mult1.inputs[1].default_value = 0.025
    mult1.location = (0, -200)

    # Multiply2
    mult2 = new_grp.nodes.new('ShaderNodeMath')
    mult2.operation = 'MULTIPLY'
    mult2.location = (200, -200)

    # Multiply3
    mult3 = new_grp.nodes.new('ShaderNodeMath')
    mult3.operation = 'MULTIPLY'
    mult3.inputs[1].default_value = 0.05
    mult3.location = (0, -400)

    # Multiply4
    mult4 = new_grp.nodes.new('ShaderNodeMath')
    mult4.operation = 'MULTIPLY'
    mult4.location = (200, -400)

    # Combine1
    comb1 = new_grp.nodes.new('ShaderNodeCombineRGB')
    comb1.location = (400, -300)

    # Combine2
    comb2 = new_grp.nodes.new('ShaderNodeCombineRGB')
    comb2.location = (-600, 0)

    # Multiply5
    mult5 = new_grp.nodes.new('ShaderNodeMixRGB')
    mult5.blend_type = 'MULTIPLY'
    mult5.inputs[0].default_value = 1.0
    mult5.location = (-400, 0)

    # Add2
    add2 = new_grp.nodes.new('ShaderNodeVectorMath')
    add2.operation = 'ADD'
    add2.location = (-200, 0)
    
    # Links
    new_grp.links.new(grp_in.outputs[0], add2.inputs[0])
    new_grp.links.new(geom_in.outputs['Window'], view_flip.inputs[0])
    new_grp.links.new(view_flip.outputs[0], sep1.inputs[0])
    new_grp.links.new(grp_in.outputs[1], comb2.inputs[0])
    new_grp.links.new(grp_in.outputs[1], comb2.inputs[1])
    new_grp.links.new(grp_in.outputs[1], comb2.inputs[2])
    new_grp.links.new(comb2.outputs[0], mult5.inputs[1])
    new_grp.links.new(grp_in.outputs[2], mult2.inputs[1])
    new_grp.links.new(grp_in.outputs[2], mult4.inputs[1])
    new_grp.links.new(sep1.outputs[0], add1.inputs[0])
    new_grp.links.new(sep1.outputs[1], add1.inputs[1])
    new_grp.links.new(sep1.outputs[2], mult3.inputs[0])
    new_grp.links.new(add1.outputs[0], mult1.inputs[0])
    new_grp.links.new(mult1.outputs[0], mult2.inputs[0])
    new_grp.links.new(mult2.outputs[0], comb1.inputs[0])
    new_grp.links.new(mult3.outputs[0], mult4.inputs[0])
    new_grp.links.new(mult4.outputs[0], comb1.inputs[1])
    new_grp.links.new(comb1.outputs[0], mult5.inputs[2])
    new_grp.links.new(mult5.outputs[0], add2.inputs[1])
    new_grp.links.new(add2.outputs[0], grp_out.inputs[0])

# 8 - Mode 8
def make_uva8():
    new_grp = bpy.data.node_groups.new('RetroUVMode8Node', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.inputs.new('NodeSocketFloat', 'Param1')
    new_grp.inputs.new('NodeSocketFloat', 'Param2')
    new_grp.inputs.new('NodeSocketFloat', 'Param3')
    new_grp.inputs.new('NodeSocketFloat', 'Param4')
    new_grp.inputs.new('NodeSocketFloat', 'Param5')
    new_grp.inputs.new('NodeSocketFloat', 'Param6')
    new_grp.inputs.new('NodeSocketFloat', 'Param7')
    new_grp.inputs.new('NodeSocketFloat', 'Param8')
    new_grp.inputs.new('NodeSocketFloat', 'Param9')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Links
    new_grp.links.new(grp_in.outputs[0], grp_out.inputs[0])

UV_ANIMATION_GROUPS = (
    make_uva0,
    make_uva1,
    make_uva2,
    make_uva3,
    make_uva4,
    make_uva5,
    make_uva6,
    make_uva7,
    make_uva8
)

# MP3 / DKCR Material Passes:
# http://www.metroid2002.com/retromodding/wiki/Materials_(Metroid_Prime_3)

# Lightmap
def make_pass_diff():
    new_grp = bpy.data.node_groups.new('RetroPassDIFF', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketColor', 'Prev Color')
    new_grp.inputs.new('NodeSocketFloat', 'Prev Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Tex Color')
    new_grp.inputs.new('NodeSocketFloat', 'Tex Alpha')
    new_grp.outputs.new('NodeSocketColor', 'Next Color')
    new_grp.outputs.new('NodeSocketFloat', 'Next Alpha')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Multiply1
    mult1 = new_grp.nodes.new('ShaderNodeMixRGB')
    mult1.blend_type = 'ADD'
    mult1.inputs[0].default_value = 1.0
    mult1.location = (-600, 0)

    # Links
    new_grp.links.new(grp_in.outputs[0], mult1.inputs[1])
    new_grp.links.new(grp_in.outputs[2], mult1.inputs[2])
    new_grp.links.new(mult1.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], grp_out.inputs[1])


# Rim Lighting Map
def make_pass_riml():
    new_grp = bpy.data.node_groups.new('RetroPassRIML', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketColor', 'Prev Color')
    new_grp.inputs.new('NodeSocketFloat', 'Prev Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Tex Color')
    new_grp.inputs.new('NodeSocketFloat', 'Tex Alpha')
    new_grp.outputs.new('NodeSocketColor', 'Next Color')
    new_grp.outputs.new('NodeSocketFloat', 'Next Alpha')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Links
    new_grp.links.new(grp_in.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], grp_out.inputs[1])

# Bloom Lightmap
def make_pass_blol():
    new_grp = bpy.data.node_groups.new('RetroPassBLOL', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketColor', 'Prev Color')
    new_grp.inputs.new('NodeSocketFloat', 'Prev Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Tex Color')
    new_grp.inputs.new('NodeSocketFloat', 'Tex Alpha')
    new_grp.outputs.new('NodeSocketColor', 'Next Color')
    new_grp.outputs.new('NodeSocketFloat', 'Next Alpha')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Links
    new_grp.links.new(grp_in.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], grp_out.inputs[1])

# Bloom Diffuse Map
def make_pass_blod():
    new_grp = bpy.data.node_groups.new('RetroPassBLOD', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketColor', 'Prev Color')
    new_grp.inputs.new('NodeSocketFloat', 'Prev Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Tex Color')
    new_grp.inputs.new('NodeSocketFloat', 'Tex Alpha')
    new_grp.outputs.new('NodeSocketColor', 'Next Color')
    new_grp.outputs.new('NodeSocketFloat', 'Next Alpha')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Links
    new_grp.links.new(grp_in.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], grp_out.inputs[1])

# Diffuse Map
def make_pass_clr():
    new_grp = bpy.data.node_groups.new('RetroPassCLR', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketColor', 'Prev Color')
    new_grp.inputs.new('NodeSocketFloat', 'Prev Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Tex Color')
    new_grp.inputs.new('NodeSocketFloat', 'Tex Alpha')
    new_grp.outputs.new('NodeSocketColor', 'Next Color')
    new_grp.outputs.new('NodeSocketFloat', 'Next Alpha')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Multiply
    mult1 = new_grp.nodes.new('ShaderNodeMixRGB')
    mult1.blend_type = 'MULTIPLY'
    mult1.inputs[0].default_value = 1.0
    grp_in.location = (-400, 0)

    # Links
    new_grp.links.new(grp_in.outputs[0], mult1.inputs[1])
    new_grp.links.new(grp_in.outputs[2], mult1.inputs[2])
    new_grp.links.new(mult1.outputs[0], grp_out.inputs[0])
    grp_out.inputs[1].default_value = 1.0

# Opacity Map
def make_pass_tran():
    new_grp = bpy.data.node_groups.new('RetroPassTRAN', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketColor', 'Prev Color')
    new_grp.inputs.new('NodeSocketFloat', 'Prev Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Tex Color')
    new_grp.inputs.new('NodeSocketFloat', 'Tex Alpha')
    new_grp.outputs.new('NodeSocketColor', 'Next Color')
    new_grp.outputs.new('NodeSocketFloat', 'Next Alpha')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Multiply
    mul1 = new_grp.nodes.new('ShaderNodeMath')
    mul1.operation = 'MULTIPLY'
    mul1.inputs[0].default_value = 1.0
    mul1.location = (-400, 0)

    # Links
    new_grp.links.new(grp_in.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], mul1.inputs[0])
    new_grp.links.new(grp_in.outputs[2], mul1.inputs[1])
    new_grp.links.new(mul1.outputs[0], grp_out.inputs[1])

# Opacity Map Inverted
def make_pass_tran_inv():
    new_grp = bpy.data.node_groups.new('RetroPassTRANInv', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketColor', 'Prev Color')
    new_grp.inputs.new('NodeSocketFloat', 'Prev Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Tex Color')
    new_grp.inputs.new('NodeSocketFloat', 'Tex Alpha')
    new_grp.outputs.new('NodeSocketColor', 'Next Color')
    new_grp.outputs.new('NodeSocketFloat', 'Next Alpha')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Multiply
    mul1 = new_grp.nodes.new('ShaderNodeMath')
    mul1.operation = 'MULTIPLY'
    mul1.inputs[0].default_value = 1.0
    mul1.location = (-400, 0)

    # Invert
    inv1 = new_grp.nodes.new('ShaderNodeInvert')
    inv1.inputs[0].default_value = 1.0
    inv1.location = (-600, 0)

    # Links
    new_grp.links.new(grp_in.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], mul1.inputs[1])
    new_grp.links.new(grp_in.outputs[2], inv1.inputs[1])
    new_grp.links.new(inv1.outputs[0], mul1.inputs[0])
    new_grp.links.new(mul1.outputs[0], grp_out.inputs[1])

# Incandescence Map
def make_pass_inca():
    new_grp = bpy.data.node_groups.new('RetroPassINCA', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketColor', 'Prev Color')
    new_grp.inputs.new('NodeSocketFloat', 'Prev Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Tex Color')
    new_grp.inputs.new('NodeSocketFloat', 'Tex Alpha')
    new_grp.outputs.new('NodeSocketColor', 'Next Color')
    new_grp.outputs.new('NodeSocketFloat', 'Next Alpha')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Multiply
    add1 = new_grp.nodes.new('ShaderNodeMixRGB')
    add1.blend_type = 'ADD'
    add1.inputs[0].default_value = 1.0
    grp_in.location = (-400, 0)

    # Links
    new_grp.links.new(grp_in.outputs[0], add1.inputs[1])
    new_grp.links.new(grp_in.outputs[2], add1.inputs[2])
    new_grp.links.new(add1.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], grp_out.inputs[1])
    grp_out.inputs[1].default_value = 1.0

# Reflection Map
def make_pass_rfld():
    new_grp = bpy.data.node_groups.new('RetroPassRFLD', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketColor', 'Prev Color')
    new_grp.inputs.new('NodeSocketFloat', 'Prev Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Mask Color')
    new_grp.inputs.new('NodeSocketFloat', 'Mask Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Tex Color')
    new_grp.inputs.new('NodeSocketFloat', 'Tex Alpha')
    new_grp.outputs.new('NodeSocketColor', 'Next Color')
    new_grp.outputs.new('NodeSocketFloat', 'Next Alpha')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Multiply
    mult1 = new_grp.nodes.new('ShaderNodeMixRGB')
    mult1.location = (-600, 0)
    mult1.blend_type = 'MULTIPLY'
    mult1.inputs[0].default_value = 1.0

    # Add
    add1 = new_grp.nodes.new('ShaderNodeMixRGB')
    add1.location = (-400, 0)
    add1.blend_type = 'ADD'
    add1.inputs[0].default_value = 1.0

    # Links
    new_grp.links.new(grp_in.outputs[0], add1.inputs[1])
    new_grp.links.new(grp_in.outputs[2], mult1.inputs[1])
    new_grp.links.new(grp_in.outputs[4], mult1.inputs[2])
    new_grp.links.new(mult1.outputs[0], add1.inputs[2])
    new_grp.links.new(add1.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], grp_out.inputs[1])

# Unk1
def make_pass_lrld():
    new_grp = bpy.data.node_groups.new('RetroPassLRLD', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketColor', 'Prev Color')
    new_grp.inputs.new('NodeSocketFloat', 'Prev Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Tex Color')
    new_grp.inputs.new('NodeSocketFloat', 'Tex Alpha')
    new_grp.outputs.new('NodeSocketColor', 'Next Color')
    new_grp.outputs.new('NodeSocketFloat', 'Next Alpha')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Links
    new_grp.links.new(grp_in.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], grp_out.inputs[1])

# Unk2
def make_pass_lurd():
    new_grp = bpy.data.node_groups.new('RetroPassLURD', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketColor', 'Prev Color')
    new_grp.inputs.new('NodeSocketFloat', 'Prev Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Tex Color')
    new_grp.inputs.new('NodeSocketFloat', 'Tex Alpha')
    new_grp.outputs.new('NodeSocketColor', 'Next Color')
    new_grp.outputs.new('NodeSocketFloat', 'Next Alpha')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Links
    new_grp.links.new(grp_in.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], grp_out.inputs[1])

# Bloom Incandescence Map
def make_pass_bloi():
    new_grp = bpy.data.node_groups.new('RetroPassBLOI', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketColor', 'Prev Color')
    new_grp.inputs.new('NodeSocketFloat', 'Prev Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Tex Color')
    new_grp.inputs.new('NodeSocketFloat', 'Tex Alpha')
    new_grp.outputs.new('NodeSocketColor', 'Next Color')
    new_grp.outputs.new('NodeSocketFloat', 'Next Alpha')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Links
    new_grp.links.new(grp_in.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], grp_out.inputs[1])

# X-ray Reflection Map
def make_pass_xray():
    new_grp = bpy.data.node_groups.new('RetroPassXRAY', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketColor', 'Prev Color')
    new_grp.inputs.new('NodeSocketFloat', 'Prev Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Tex Color')
    new_grp.inputs.new('NodeSocketFloat', 'Tex Alpha')
    new_grp.outputs.new('NodeSocketColor', 'Next Color')
    new_grp.outputs.new('NodeSocketFloat', 'Next Alpha')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Links
    new_grp.links.new(grp_in.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], grp_out.inputs[1])

# Unused
def make_pass_toon():
    new_grp = bpy.data.node_groups.new('RetroPassTOON', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketColor', 'Prev Color')
    new_grp.inputs.new('NodeSocketFloat', 'Prev Alpha')
    new_grp.inputs.new('NodeSocketColor', 'Tex Color')
    new_grp.inputs.new('NodeSocketFloat', 'Tex Alpha')
    new_grp.outputs.new('NodeSocketColor', 'Next Color')
    new_grp.outputs.new('NodeSocketFloat', 'Next Alpha')
    new_grp.use_fake_user = True

    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)

    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)

    # Links
    new_grp.links.new(grp_in.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], grp_out.inputs[1])

MP3_PASS_GROUPS = (
    make_pass_diff,
    make_pass_riml,
    make_pass_blol,
    make_pass_blod,
    make_pass_clr,
    make_pass_tran,
    make_pass_tran_inv,
    make_pass_inca,
    make_pass_rfld,
    make_pass_lrld,
    make_pass_lurd,
    make_pass_bloi,
    make_pass_xray,
    make_pass_toon
)

def make_master_shader_library():
    make_additive_output()
    make_blend_opaque_output()
    for shad in ROOT_SHADER_GROUPS:
        shad()
    for uva in UV_ANIMATION_GROUPS:
        uva()
    for aPass in MP3_PASS_GROUPS:
        aPass()

