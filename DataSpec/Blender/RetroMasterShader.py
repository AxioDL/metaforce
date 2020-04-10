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
    diffuse_mod_input = new_grp.inputs.new('NodeSocketColor', 'DiffuseMod')
    diffuse_mod_input.default_value = (1.0, 1.0, 1.0, 1.0)
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
    alpha_mod_input = new_grp.inputs.new('NodeSocketFloatFactor', 'AlphaMod')
    alpha_mod_input.default_value = 1.0
    alpha_mod_input.min_value = 0.0
    alpha_mod_input.max_value = 1.0
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

    # Multiply (Multiples diffuse with diffusemod)
    diffuse_mult = new_grp.nodes.new('ShaderNodeMixRGB')
    diffuse_mult.location = (-1094, 122)
    diffuse_mult.blend_type = 'MULTIPLY'
    diffuse_mult.inputs['Fac'].default_value = 1.0

    # Multiply (Multiples alpha with alphamod)
    alpha_mult = new_grp.nodes.new('ShaderNodeMath')
    alpha_mult.location = (-1094, -178)
    alpha_mult.operation = 'MULTIPLY'

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
    new_grp.links.new(grp_in.outputs['Diffuse'], diffuse_mult.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['DiffuseMod'], diffuse_mult.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Alpha'], alpha_mult.inputs[0])
    new_grp.links.new(grp_in.outputs['AlphaMod'], alpha_mult.inputs[1])
    new_grp.links.new(grp_in.outputs['Lightmap'], lightmap_mult.inputs['Color1'])
    new_grp.links.new(diffuse_mult.outputs['Color'], lightmap_mult.inputs['Color2'])
    new_grp.links.new(diffuse_mult.outputs['Color'], diffuse_bdsf.inputs['Color'])
    new_grp.links.new(diffuse_mult.outputs['Color'], principled_bsdf.inputs['Base Color'])
    new_grp.links.new(grp_in.outputs['Emissive'], emissive_add_shader.inputs[0])
    new_grp.links.new(grp_in.outputs['Specular'], specular_mult.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['Specular'], principled_bsdf.inputs['Specular'])
    new_grp.links.new(grp_in.outputs['ExtendedSpecular'], extended_specular_mult.inputs['Color1'])
    new_grp.links.new(grp_in.outputs['Reflection'], specular_mult.inputs['Color2'])
    new_grp.links.new(grp_in.outputs['Reflection'], extended_specular_mult.inputs['Color2'])
    new_grp.links.new(alpha_mult.outputs[0], alpha_mix.inputs['Fac'])
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

# MP3 / DKCR Material Passes:
# https://wiki.axiodl.com/w/Materials_(Metroid_Prime_3)

def make_retro_shader_mp3_color():
    new_grp = bpy.data.node_groups.new("__RetroShaderMP3Color", "ShaderNodeTree")
    new_grp.use_fake_user = True
    input = new_grp.inputs.new("NodeSocketColor", "DIFFC")
    input.default_value = (0.0, 0.0, 0.0, 1.0)
    input = new_grp.inputs.new("NodeSocketColor", "DIFBC")
    input.default_value = (1.0, 1.0, 1.0, 1.0)
    input = new_grp.inputs.new("NodeSocketColor", "CLRC")
    input.default_value = (0.5, 0.5, 0.5, 1.0)
    input = new_grp.inputs.new("NodeSocketFloatFactor", "CLRA")
    input.default_value = 1.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "TRAN")
    input.default_value = 1.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketColor", "RFLDC")
    input.default_value = (0.0, 0.0, 0.0, 1.0)
    input = new_grp.inputs.new("NodeSocketFloatFactor", "RFLDA")
    input.default_value = 1.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketColor", "RFLV")
    input.default_value = (0.0, 0.0, 0.0, 1.0)
    input = new_grp.inputs.new("NodeSocketColor", "LRLD")
    input.default_value = (0.0, 0.0, 0.0, 1.0)
    input = new_grp.inputs.new("NodeSocketColor", "LURDC")
    input.default_value = (0.0, 0.0, 0.0, 1.0)
    input = new_grp.inputs.new("NodeSocketFloatFactor", "LURDA")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketColor", "INCAC")
    input.default_value = (0.0, 0.0, 0.0, 1.0)
    input = new_grp.inputs.new("NodeSocketInt", "Add INCA")
    input.default_value = 0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "OPAC")
    input.default_value = 1.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    new_grp.outputs.new("NodeSocketShader", "Shader")
    nodes = {}
    node = new_grp.nodes.new("ShaderNodeBsdfDiffuse")
    node.name = "Diffuse BSDF.004"
    nodes["Diffuse BSDF.004"] = node
    node.label = ""
    node.location = (-196.910400390625, -503.60546875)
    node.inputs[0].default_value = (0.800000011920929, 0.800000011920929, 0.800000011920929, 1.0)
    node.inputs[1].default_value = 0.0
    node.inputs[2].default_value = (0.0, 0.0, 0.0)
    node = new_grp.nodes.new("ShaderNodeAddShader")
    node.name = "Add Shader.009"
    nodes["Add Shader.009"] = node
    node.label = ""
    node.location = (14.618888854980469, -571.516357421875)
    node = new_grp.nodes.new("ShaderNodeAddShader")
    node.name = "Add Shader.008"
    nodes["Add Shader.008"] = node
    node.label = ""
    node.location = (6.4276123046875, -926.3602905273438)
    node = new_grp.nodes.new("ShaderNodeBsdfDiffuse")
    node.name = "Diffuse BSDF.005"
    nodes["Diffuse BSDF.005"] = node
    node.label = ""
    node.location = (-189.85516357421875, -865.79345703125)
    node.inputs[0].default_value = (0.800000011920929, 0.800000011920929, 0.800000011920929, 1.0)
    node.inputs[1].default_value = 0.0
    node.inputs[2].default_value = (0.0, 0.0, 0.0)
    node = new_grp.nodes.new("ShaderNodeMixRGB")
    node.name = "Mix.005"
    nodes["Mix.005"] = node
    node.label = ""
    node.location = (-190.5804901123047, -1017.0886840820312)
    node.blend_type = "MULTIPLY"
    node.inputs[0].default_value = 1.0
    node.inputs[1].default_value = (0.5, 0.5, 0.5, 1.0)
    node.inputs[2].default_value = (0.5, 0.5, 0.5, 1.0)
    node = new_grp.nodes.new("ShaderNodeMixRGB")
    node.name = "Mix.004"
    nodes["Mix.004"] = node
    node.label = ""
    node.location = (-381.6676940917969, -870.815673828125)
    node.blend_type = "MULTIPLY"
    node.inputs[0].default_value = 1.0
    node.inputs[1].default_value = (0.5, 0.5, 0.5, 1.0)
    node.inputs[2].default_value = (0.5, 0.5, 0.5, 1.0)
    node = new_grp.nodes.new("ShaderNodeAddShader")
    node.name = "Add Shader.006"
    nodes["Add Shader.006"] = node
    node.label = ""
    node.location = (220.7507781982422, -724.6066284179688)
    node = new_grp.nodes.new("ShaderNodeAddShader")
    node.name = "Add Shader.005"
    nodes["Add Shader.005"] = node
    node.label = ""
    node.location = (218.0698699951172, -528.0934448242188)
    node = new_grp.nodes.new("ShaderNodeAddShader")
    node.name = "Add Shader.007"
    nodes["Add Shader.007"] = node
    node.label = ""
    node.location = (388.0714416503906, -600.8295288085938)
    node = new_grp.nodes.new("ShaderNodeMixRGB")
    node.name = "Mix.002"
    nodes["Mix.002"] = node
    node.label = ""
    node.location = (-192.1793212890625, -281.65264892578125)
    node.blend_type = "MULTIPLY"
    node.inputs[0].default_value = 1.0
    node.inputs[1].default_value = (0.5, 0.5, 0.5, 1.0)
    node.inputs[2].default_value = (0.5, 0.5, 0.5, 1.0)
    node = new_grp.nodes.new("ShaderNodeAddShader")
    node.name = "Add Shader.010"
    nodes["Add Shader.010"] = node
    node.label = ""
    node.location = (522.2215576171875, -284.7532653808594)
    node = new_grp.nodes.new("ShaderNodeMixRGB")
    node.name = "Mix.001"
    nodes["Mix.001"] = node
    node.label = ""
    node.location = (-198.2812957763672, -13.079503059387207)
    node.blend_type = "MULTIPLY"
    node.inputs[0].default_value = 1.0
    node.inputs[1].default_value = (0.5, 0.5, 0.5, 1.0)
    node.inputs[2].default_value = (0.5, 0.5, 0.5, 1.0)
    node = new_grp.nodes.new("ShaderNodeBsdfDiffuse")
    node.name = "Diffuse BSDF.001"
    nodes["Diffuse BSDF.001"] = node
    node.label = ""
    node.location = (-200.4605255126953, 138.9542694091797)
    node.inputs[0].default_value = (0.800000011920929, 0.800000011920929, 0.800000011920929, 1.0)
    node.inputs[1].default_value = 0.0
    node.inputs[2].default_value = (0.0, 0.0, 0.0)
    node = new_grp.nodes.new("ShaderNodeAddShader")
    node.name = "Add Shader.001"
    nodes["Add Shader.001"] = node
    node.label = ""
    node.location = (-14.161624908447266, 32.61324691772461)
    node = new_grp.nodes.new("NodeGroupOutput")
    node.name = "Group Output"
    nodes["Group Output"] = node
    node.label = ""
    node.location = (948.8831176757812, -299.1160583496094)
    node = new_grp.nodes.new("ShaderNodeBsdfTransparent")
    node.name = "Transparent BSDF.001"
    nodes["Transparent BSDF.001"] = node
    node.label = ""
    node.location = (604.5911254882812, -88.7776870727539)
    node.inputs[0].default_value = (1.0, 1.0, 1.0, 1.0)
    node = new_grp.nodes.new("ShaderNodeMixShader")
    node.name = "Mix Shader"
    nodes["Mix Shader"] = node
    node.label = ""
    node.location = (772.179443359375, -91.1546401977539)
    node.inputs[0].default_value = 0.5
    node = new_grp.nodes.new("ShaderNodeAddShader")
    node.name = "Add Shader.012"
    nodes["Add Shader.012"] = node
    node.label = ""
    node.location = (776.751953125, -432.8694152832031)
    node = new_grp.nodes.new("ShaderNodeAddShader")
    node.name = "Add Shader.011"
    nodes["Add Shader.011"] = node
    node.label = ""
    node.location = (779.857177734375, -294.9550476074219)
    node = new_grp.nodes.new("ShaderNodeMixRGB")
    node.name = "Mix.006"
    nodes["Mix.006"] = node
    node.label = ""
    node.location = (-192.534912109375, -643.984619140625)
    node.blend_type = "MULTIPLY"
    node.inputs[0].default_value = 1.0
    node.inputs[1].default_value = (0.5, 0.5, 0.5, 1.0)
    node.inputs[2].default_value = (0.5, 0.5, 0.5, 1.0)
    node = new_grp.nodes.new("ShaderNodeMixRGB")
    node.name = "Mix.003"
    nodes["Mix.003"] = node
    node.label = ""
    node.location = (-374.2341003417969, -515.1140747070312)
    node.blend_type = "MULTIPLY"
    node.inputs[0].default_value = 1.0
    node.inputs[1].default_value = (0.5, 0.5, 0.5, 1.0)
    node.inputs[2].default_value = (0.5, 0.5, 0.5, 1.0)
    node = new_grp.nodes.new("ShaderNodeMixRGB")
    node.name = "Mix"
    nodes["Mix"] = node
    node.label = ""
    node.location = (-500.3056640625, -114.82369995117188)
    node.blend_type = "MULTIPLY"
    node.inputs[0].default_value = 1.0
    node.inputs[1].default_value = (0.5, 0.5, 0.5, 1.0)
    node.inputs[2].default_value = (0.5, 0.5, 0.5, 1.0)
    node = new_grp.nodes.new("ShaderNodeMath")
    node.name = "Math"
    nodes["Math"] = node
    node.label = ""
    node.location = (454.39404296875, 96.02081298828125)
    node.operation = "MULTIPLY"
    node.inputs[0].default_value = 0.5
    node.inputs[1].default_value = 0.5
    node = new_grp.nodes.new("ShaderNodeMath")
    node.name = "Math.001"
    nodes["Math.001"] = node
    node.label = ""
    node.location = (619.3079223632812, 90.52423095703125)
    node.operation = "MULTIPLY"
    node.inputs[0].default_value = 0.5
    node.inputs[1].default_value = 0.5
    node = new_grp.nodes.new("ShaderNodeMath")
    node.name = "Math.002"
    nodes["Math.002"] = node
    node.label = ""
    node.location = (785.3211059570312, 81.7295913696289)
    node.operation = "MULTIPLY"
    node.inputs[0].default_value = 0.5
    node.inputs[1].default_value = 0.5
    node = new_grp.nodes.new("ShaderNodeBsdfTransparent")
    node.name = "Transparent BSDF"
    nodes["Transparent BSDF"] = node
    node.label = ""
    node.location = (597.9944458007812, -480.7802734375)
    node.inputs[0].default_value = (1.0, 1.0, 1.0, 1.0)
    node = new_grp.nodes.new("NodeGroupInput")
    node.name = "Group Input"
    nodes["Group Input"] = node
    node.label = ""
    node.location = (-669.6587524414062, -193.9534149169922)
    new_grp.links.new(nodes["Group Input"].outputs[0], nodes["Mix"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[1], nodes["Mix"].inputs[2])
    new_grp.links.new(nodes["Mix"].outputs[0], nodes["Mix.001"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[2], nodes["Mix.001"].inputs[2])
    new_grp.links.new(nodes["Mix.001"].outputs[0], nodes["Add Shader.001"].inputs[1])
    new_grp.links.new(nodes["Diffuse BSDF.001"].outputs[0], nodes["Add Shader.001"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[2], nodes["Diffuse BSDF.001"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[5], nodes["Mix.002"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[7], nodes["Mix.002"].inputs[2])
    new_grp.links.new(nodes["Mix.002"].outputs[0], nodes["Add Shader.005"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[5], nodes["Mix.003"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[8], nodes["Mix.003"].inputs[2])
    new_grp.links.new(nodes["Mix.003"].outputs[0], nodes["Diffuse BSDF.004"].inputs[0])
    new_grp.links.new(nodes["Diffuse BSDF.004"].outputs[0], nodes["Add Shader.009"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[5], nodes["Mix.004"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[10], nodes["Mix.004"].inputs[2])
    new_grp.links.new(nodes["Mix.004"].outputs[0], nodes["Diffuse BSDF.005"].inputs[0])
    new_grp.links.new(nodes["Diffuse BSDF.005"].outputs[0], nodes["Add Shader.008"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[9], nodes["Add Shader.006"].inputs[0])
    new_grp.links.new(nodes["Add Shader.005"].outputs[0], nodes["Add Shader.007"].inputs[0])
    new_grp.links.new(nodes["Add Shader.006"].outputs[0], nodes["Add Shader.007"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[10], nodes["Mix.005"].inputs[2])
    new_grp.links.new(nodes["Mix"].outputs[0], nodes["Mix.005"].inputs[1])
    new_grp.links.new(nodes["Add Shader.008"].outputs[0], nodes["Add Shader.006"].inputs[1])
    new_grp.links.new(nodes["Mix.005"].outputs[0], nodes["Add Shader.008"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[8], nodes["Mix.006"].inputs[2])
    new_grp.links.new(nodes["Mix"].outputs[0], nodes["Mix.006"].inputs[1])
    new_grp.links.new(nodes["Add Shader.009"].outputs[0], nodes["Add Shader.005"].inputs[1])
    new_grp.links.new(nodes["Mix.006"].outputs[0], nodes["Add Shader.009"].inputs[1])
    new_grp.links.new(nodes["Add Shader.007"].outputs[0], nodes["Add Shader.010"].inputs[1])
    new_grp.links.new(nodes["Add Shader.001"].outputs[0], nodes["Add Shader.010"].inputs[0])
    new_grp.links.new(nodes["Transparent BSDF"].outputs[0], nodes["Add Shader.012"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[11], nodes["Add Shader.012"].inputs[0])
    new_grp.links.new(nodes["Add Shader.012"].outputs[0], nodes["Add Shader.011"].inputs[1])
    new_grp.links.new(nodes["Add Shader.011"].outputs[0], nodes["Group Output"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[3], nodes["Math"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[13], nodes["Math"].inputs[1])
    new_grp.links.new(nodes["Math"].outputs[0], nodes["Math.001"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[4], nodes["Math.001"].inputs[1])
    new_grp.links.new(nodes["Math.001"].outputs[0], nodes["Math.002"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[6], nodes["Math.002"].inputs[0])
    new_grp.links.new(nodes["Math.002"].outputs[0], nodes["Mix Shader"].inputs[0])
    new_grp.links.new(nodes["Transparent BSDF.001"].outputs[0], nodes["Mix Shader"].inputs[1])
    new_grp.links.new(nodes["Add Shader.010"].outputs[0], nodes["Mix Shader"].inputs[2])
    new_grp.links.new(nodes["Mix Shader"].outputs[0], nodes["Add Shader.011"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[12], nodes["Transparent BSDF"].inputs[0])

def make_retro_shader_mp3_bloom():
    new_grp = bpy.data.node_groups.new("__RetroShaderMP3Bloom", "ShaderNodeTree")
    new_grp.use_fake_user = True
    input = new_grp.inputs.new("NodeSocketFloatFactor", "DIFFA")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "DIFBA")
    input.default_value = 1.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "BLOL")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "BLOD")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "BLODB")
    input.default_value = 0.5
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "TRAN")
    input.default_value = 1.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "INCAA")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "BNIF")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "BLOI")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "BLOIB")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "OPAC")
    input.default_value = 1.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketInt", "Add INCA")
    input.default_value = 0
    input.min_value = 0.000000
    input.max_value = 1.000000
    new_grp.outputs.new("NodeSocketShader", "Shader")
    nodes = {}
    node = new_grp.nodes.new("ShaderNodeMath")
    node.name = "Math.003"
    nodes["Math.003"] = node
    node.label = ""
    node.location = (-131.26889038085938, -228.6888885498047)
    node.operation = "MULTIPLY"
    node.inputs[0].default_value = 0.5
    node.inputs[1].default_value = 0.5
    node = new_grp.nodes.new("ShaderNodeMath")
    node.name = "Math"
    nodes["Math"] = node
    node.label = ""
    node.location = (-501.6487731933594, -144.7719268798828)
    node.operation = "MULTIPLY"
    node.inputs[0].default_value = 0.5
    node.inputs[1].default_value = 0.5
    node = new_grp.nodes.new("ShaderNodeMath")
    node.name = "Math.002"
    nodes["Math.002"] = node
    node.label = ""
    node.location = (-328.3370666503906, -209.53160095214844)
    node.operation = "MULTIPLY"
    node.inputs[0].default_value = 0.5
    node.inputs[1].default_value = 0.5
    node = new_grp.nodes.new("NodeGroupOutput")
    node.name = "Group Output"
    nodes["Group Output"] = node
    node.label = ""
    node.location = (1109.7938232421875, -257.2006530761719)
    node = new_grp.nodes.new("ShaderNodeMath")
    node.name = "Math.001"
    nodes["Math.001"] = node
    node.label = ""
    node.location = (129.59579467773438, -299.0679626464844)
    node.operation = "MULTIPLY"
    node.inputs[0].default_value = 0.5
    node.inputs[1].default_value = 0.5
    node = new_grp.nodes.new("ShaderNodeBsdfDiffuse")
    node.name = "Diffuse BSDF.002"
    nodes["Diffuse BSDF.002"] = node
    node.label = ""
    node.location = (122.80331420898438, -150.7427520751953)
    node.inputs[0].default_value = (0.800000011920929, 0.800000011920929, 0.800000011920929, 1.0)
    node.inputs[1].default_value = 0.0
    node.inputs[2].default_value = (0.0, 0.0, 0.0)
    node = new_grp.nodes.new("ShaderNodeAddShader")
    node.name = "Add Shader.002"
    nodes["Add Shader.002"] = node
    node.label = ""
    node.location = (312.7171325683594, -220.0266571044922)
    node = new_grp.nodes.new("ShaderNodeAddShader")
    node.name = "Add Shader.005"
    nodes["Add Shader.005"] = node
    node.label = ""
    node.location = (-165.06072998046875, -549.3956298828125)
    node = new_grp.nodes.new("ShaderNodeAddShader")
    node.name = "Add Shader.006"
    nodes["Add Shader.006"] = node
    node.label = ""
    node.location = (20.3157958984375, -545.8302612304688)
    node = new_grp.nodes.new("ShaderNodeBsdfTransparent")
    node.name = "Transparent BSDF.001"
    nodes["Transparent BSDF.001"] = node
    node.label = ""
    node.location = (205.5854034423828, -558.1273803710938)
    node.inputs[0].default_value = (1.0, 1.0, 1.0, 1.0)
    node = new_grp.nodes.new("ShaderNodeAddShader")
    node.name = "Add Shader.001"
    nodes["Add Shader.001"] = node
    node.label = ""
    node.location = (399.876708984375, -533.2184448242188)
    node = new_grp.nodes.new("ShaderNodeMath")
    node.name = "Math.004"
    nodes["Math.004"] = node
    node.label = ""
    node.location = (-354.23876953125, -508.8504943847656)
    node.operation = "MULTIPLY"
    node.inputs[0].default_value = 0.5
    node.inputs[1].default_value = 0.5
    node = new_grp.nodes.new("ShaderNodeAddShader")
    node.name = "Add Shader"
    nodes["Add Shader"] = node
    node.label = ""
    node.location = (875.3080444335938, -248.47450256347656)
    node = new_grp.nodes.new("ShaderNodeBsdfTransparent")
    node.name = "Transparent BSDF"
    nodes["Transparent BSDF"] = node
    node.label = ""
    node.location = (502.63671875, -341.6871032714844)
    node.inputs[0].default_value = (1.0, 1.0, 1.0, 1.0)
    node = new_grp.nodes.new("ShaderNodeMath")
    node.name = "Math.006"
    nodes["Math.006"] = node
    node.label = ""
    node.location = (505.8763122558594, -171.7743377685547)
    node.operation = "MULTIPLY"
    node.inputs[0].default_value = 0.5
    node.inputs[1].default_value = 0.5
    node = new_grp.nodes.new("ShaderNodeMixShader")
    node.name = "Mix Shader"
    nodes["Mix Shader"] = node
    node.label = ""
    node.location = (682.0885620117188, -169.31057739257812)
    node.inputs[0].default_value = 0.5
    node = new_grp.nodes.new("NodeGroupInput")
    node.name = "Group Input"
    nodes["Group Input"] = node
    node.label = ""
    node.location = (-669.6587524414062, -193.9534149169922)
    new_grp.links.new(nodes["Group Input"].outputs[0], nodes["Math"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[1], nodes["Math"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[3], nodes["Math.002"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[4], nodes["Math.003"].inputs[1])
    new_grp.links.new(nodes["Math.002"].outputs[0], nodes["Math.003"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[2], nodes["Math.002"].inputs[0])
    new_grp.links.new(nodes["Math"].outputs[0], nodes["Math.001"].inputs[0])
    new_grp.links.new(nodes["Diffuse BSDF.002"].outputs[0], nodes["Add Shader.002"].inputs[0])
    new_grp.links.new(nodes["Math.001"].outputs[0], nodes["Add Shader.002"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[10], nodes["Math.006"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[5], nodes["Math.006"].inputs[0])
    new_grp.links.new(nodes["Math.006"].outputs[0], nodes["Mix Shader"].inputs[0])
    new_grp.links.new(nodes["Transparent BSDF"].outputs[0], nodes["Mix Shader"].inputs[1])
    new_grp.links.new(nodes["Transparent BSDF.001"].outputs[0], nodes["Add Shader.001"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[6], nodes["Math.004"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[7], nodes["Math.004"].inputs[1])
    new_grp.links.new(nodes["Math.003"].outputs[0], nodes["Math.001"].inputs[1])
    new_grp.links.new(nodes["Math.003"].outputs[0], nodes["Diffuse BSDF.002"].inputs[0])
    new_grp.links.new(nodes["Math.004"].outputs[0], nodes["Add Shader.005"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[8], nodes["Add Shader.005"].inputs[1])
    new_grp.links.new(nodes["Add Shader.005"].outputs[0], nodes["Add Shader.006"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[9], nodes["Add Shader.006"].inputs[1])
    new_grp.links.new(nodes["Add Shader.006"].outputs[0], nodes["Add Shader.001"].inputs[1])
    new_grp.links.new(nodes["Add Shader"].outputs[0], nodes["Group Output"].inputs[0])
    new_grp.links.new(nodes["Mix Shader"].outputs[0], nodes["Add Shader"].inputs[0])
    new_grp.links.new(nodes["Add Shader.002"].outputs[0], nodes["Mix Shader"].inputs[2])
    new_grp.links.new(nodes["Add Shader.001"].outputs[0], nodes["Add Shader"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[11], nodes["Transparent BSDF.001"].inputs[0])

def make_retro_shader_mp3():
    new_grp = bpy.data.node_groups.new("RetroShaderMP3", "ShaderNodeTree")
    new_grp.use_fake_user = True
    input = new_grp.inputs.new("NodeSocketColor", "DIFFC")
    input.default_value = (0.0, 0.0, 0.0, 1.0)
    input = new_grp.inputs.new("NodeSocketFloatFactor", "DIFFA")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketColor", "DIFBC")
    input.default_value = (1.0, 1.0, 1.0, 1.0)
    input = new_grp.inputs.new("NodeSocketFloatFactor", "DIFBA")
    input.default_value = 1.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "BLOL")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "BLOD")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "BLODB")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketColor", "CLR")
    input.default_value = (0.0, 0.0, 0.0, 1.0)
    input = new_grp.inputs.new("NodeSocketFloatFactor", "CLRA")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "TRAN")
    input.default_value = 1.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketColor", "RFLD")
    input.default_value = (0.0, 0.0, 0.0, 1.0)
    input = new_grp.inputs.new("NodeSocketFloatFactor", "RFLDA")
    input.default_value = 1.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketColor", "RFLV")
    input.default_value = (0.0, 0.0, 0.0, 1.0)
    input = new_grp.inputs.new("NodeSocketColor", "LRLD")
    input.default_value = (0.0, 0.0, 0.0, 1.0)
    input = new_grp.inputs.new("NodeSocketColor", "LURDC")
    input.default_value = (0.0, 0.0, 0.0, 1.0)
    input = new_grp.inputs.new("NodeSocketFloatFactor", "LURDA")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketColor", "INCAC")
    input.default_value = (0.0, 0.0, 0.0, 1.0)
    input = new_grp.inputs.new("NodeSocketFloatFactor", "INCAA")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketInt", "Add INCA")
    input.default_value = 0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "BNIF")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "BLOI")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "BLOIB")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "OPAC")
    input.default_value = 1.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "XRAYC")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "XRAYA")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    input = new_grp.inputs.new("NodeSocketFloatFactor", "XRBR")
    input.default_value = 0.0
    input.min_value = 0.000000
    input.max_value = 1.000000
    nodes = {}
    node = new_grp.nodes.new("ShaderNodeMixShader")
    node.name = "Mix Shader"
    nodes["Mix Shader"] = node
    node.label = ""
    node.location = (-118.33348846435547, -291.9857482910156)
    node.inputs[0].default_value = 0.0
    node = new_grp.nodes.new("ShaderNodeOutputMaterial")
    node.name = "Material Output"
    nodes["Material Output"] = node
    node.label = ""
    node.location = (81.25957489013672, -265.6065368652344)
    node.inputs[2].default_value = (0.0, 0.0, 0.0)
    node = new_grp.nodes.new("ShaderNodeGroup")
    node.name = "Group.001"
    nodes["Group.001"] = node
    node.label = ""
    node.location = (-358.6896057128906, -60.17391586303711)
    node.node_tree = bpy.data.node_groups["__RetroShaderMP3Color"]
    node.inputs[0].default_value = (1.0, 1.0, 1.0, 1.0)
    node.inputs[1].default_value = (1.0, 1.0, 1.0, 1.0)
    node.inputs[2].default_value = (0.5, 0.5, 0.5, 1.0)
    node.inputs[3].default_value = 0.5
    node.inputs[4].default_value = 0.5
    node.inputs[5].default_value = (0.0, 0.0, 0.0, 1.0)
    node.inputs[6].default_value = 0.5
    node.inputs[7].default_value = (0.0, 0.0, 0.0, 1.0)
    node.inputs[8].default_value = (0.0, 0.0, 0.0, 1.0)
    node.inputs[9].default_value = (0.0, 0.0, 0.0, 1.0)
    node.inputs[10].default_value = 0.0
    node.inputs[11].default_value = (0.0, 0.0, 0.0, 1.0)
    node.inputs[12].default_value = 0
    node.inputs[13].default_value = 0.5
    node = new_grp.nodes.new("ShaderNodeGroup")
    node.name = "Group"
    nodes["Group"] = node
    node.label = ""
    node.location = (-356.9021301269531, -446.9474182128906)
    node.node_tree = bpy.data.node_groups["__RetroShaderMP3Bloom"]
    node.inputs[0].default_value = 1.0
    node.inputs[1].default_value = 1.0
    node.inputs[2].default_value = 0.0
    node.inputs[3].default_value = 0.0
    node.inputs[4].default_value = 0.5
    node.inputs[5].default_value = 0.5
    node.inputs[6].default_value = 0.0
    node.inputs[7].default_value = 0.0
    node.inputs[8].default_value = 0.0
    node.inputs[9].default_value = 0.0
    node.inputs[10].default_value = 0.5
    node.inputs[11].default_value = 0
    node = new_grp.nodes.new("NodeGroupInput")
    node.name = "Group Input"
    nodes["Group Input"] = node
    node.label = ""
    node.location = (-669.6587524414062, -193.9534149169922)
    new_grp.links.new(nodes["Group Input"].outputs[0], nodes["Group.001"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[1], nodes["Group"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[3], nodes["Group"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[4], nodes["Group"].inputs[2])
    new_grp.links.new(nodes["Group Input"].outputs[5], nodes["Group"].inputs[3])
    new_grp.links.new(nodes["Group Input"].outputs[6], nodes["Group"].inputs[4])
    new_grp.links.new(nodes["Group Input"].outputs[17], nodes["Group"].inputs[6])
    new_grp.links.new(nodes["Group Input"].outputs[19], nodes["Group"].inputs[7])
    new_grp.links.new(nodes["Group Input"].outputs[20], nodes["Group"].inputs[8])
    new_grp.links.new(nodes["Group Input"].outputs[2], nodes["Group.001"].inputs[1])
    new_grp.links.new(nodes["Group Input"].outputs[7], nodes["Group.001"].inputs[2])
    new_grp.links.new(nodes["Group Input"].outputs[10], nodes["Group.001"].inputs[5])
    new_grp.links.new(nodes["Group Input"].outputs[12], nodes["Group.001"].inputs[7])
    new_grp.links.new(nodes["Group Input"].outputs[13], nodes["Group.001"].inputs[8])
    new_grp.links.new(nodes["Group Input"].outputs[14], nodes["Group.001"].inputs[9])
    new_grp.links.new(nodes["Group Input"].outputs[15], nodes["Group.001"].inputs[10])
    new_grp.links.new(nodes["Group Input"].outputs[16], nodes["Group.001"].inputs[11])
    new_grp.links.new(nodes["Group.001"].outputs[0], nodes["Mix Shader"].inputs[1])
    new_grp.links.new(nodes["Group"].outputs[0], nodes["Mix Shader"].inputs[2])
    new_grp.links.new(nodes["Mix Shader"].outputs[0], nodes["Material Output"].inputs[0])
    new_grp.links.new(nodes["Group Input"].outputs[21], nodes["Group"].inputs[9])
    new_grp.links.new(nodes["Group Input"].outputs[8], nodes["Group.001"].inputs[3])
    new_grp.links.new(nodes["Group Input"].outputs[9], nodes["Group.001"].inputs[4])
    new_grp.links.new(nodes["Group Input"].outputs[11], nodes["Group.001"].inputs[6])
    new_grp.links.new(nodes["Group Input"].outputs[22], nodes["Group.001"].inputs[13])
    new_grp.links.new(nodes["Group Input"].outputs[9], nodes["Group"].inputs[5])
    new_grp.links.new(nodes["Group Input"].outputs[22], nodes["Group"].inputs[10])
    new_grp.links.new(nodes["Group Input"].outputs[18], nodes["Group.001"].inputs[12])
    new_grp.links.new(nodes["Group Input"].outputs[18], nodes["Group"].inputs[11])

ROOT_SHADER_GROUPS = (
    make_retro_shader,
    make_retro_dynamic_shader,
    make_retro_dynamic_alpha_shader,
    make_retro_dynamic_character_shader,
    make_retro_shader_mp3_color,
    make_retro_shader_mp3_bloom,
    make_retro_shader_mp3
)

# UV animation nodes:
# https://wiki.axiodl.com/w/Materials_(Metroid_Prime)#UV_Animations

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
    uv_scale.inputs['Scale'].default_value = (2.0, 2.0, 0.0)
    uv_scale.inputs['Location'].default_value = (1.0, 1.0, 0.0)

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
    for drv in mapping.inputs['Scale'].driver_add('default_value'):
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
    map1.inputs['Scale'].default_value = (1.0, 0.0, 0.0)
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
    map1.inputs['Scale'].default_value = (0.0, 1.0, 0.0)
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
    view_flip.inputs['Scale'].default_value = (-1.0, -1.0, 1.0)

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

def make_master_shader_library():
    make_additive_output()
    make_blend_opaque_output()
    for shad in ROOT_SHADER_GROUPS:
        shad()
    for uva in UV_ANIMATION_GROUPS:
        uva()

