#include "CFluidPlaneShader.hpp"

namespace urde
{

static const char* VS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 normalIn;\n"
"layout(location=2) in vec4 binormalIn;\n"
"layout(location=3) in vec4 tangentIn;\n"
"layout(location=4) in vec4 colorIn;\n"
"\n"
"UBINDING0 uniform FluidPlaneUniform\n"
"{\n"
"    mat4 mv;\n"
"    mat4 mvNorm;\n"
"    mat4 proj;\n"
"    mat4 texMtxs[6];\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 mvPos;\n"
"    vec4 mvNorm;\n"
"    vec4 mvBinorm;\n"
"    vec4 mvTangent;\n"
"    vec4 color;\n"
"    vec2 uvs[7];\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.mvPos = mv * vec4(posIn.xyz, 1.0);\n"
"    gl_Position = proj * vtf.mvPos;\n"
"    vtf.mvNorm = mvNorm * normalIn;\n"
"    vtf.mvBinorm = mvNorm * binormalIn;\n"
"    vtf.mvTangent = mvNorm * tangentIn;\n"
"    vtf.color = colorIn;\n"
"    vtf.uvs[0] = (texMtxs[0] * posIn).xy;\n"
"    vtf.uvs[1] = (texMtxs[1] * posIn).xy;\n"
"    vtf.uvs[2] = (texMtxs[2] * posIn).xy;\n"
"%s" // Additional TCGs here
"}\n";

static const char* FS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"\n"
"struct Light\n"
"{\n"
"    vec4 pos;\n"
"    vec4 dir;\n"
"    vec4 color;\n"
"    vec4 linAtt;\n"
"    vec4 angAtt;\n"
"};\n"
"struct Fog\n" // Reappropriated for indirect texture scaling
"{\n"
"    vec4 color;\n"
"    float indScale;\n"
"    float start;\n"
"};\n"
"\n"
"UBINDING2 uniform LightingUniform\n"
"{\n"
"    Light lights[" _XSTR(URDE_MAX_LIGHTS) "];\n"
"    vec4 ambient;\n"
"    vec4 kColor0;\n"
"    vec4 kColor1;\n"
"    vec4 kColor2;\n"
"    vec4 kColor3;\n"
"    Fog fog;\n"
"};\n"
"\n"
"vec4 LightingFunc(vec4 mvPosIn, vec4 mvNormIn)\n"
"{\n"
"    vec4 ret = ambient;\n"
"    \n"
"    for (int i=0 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        vec3 delta = mvPosIn.xyz - lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        float angDot = clamp(dot(normalize(delta), lights[i].dir.xyz), 0.0, 1.0);\n"
"        float att = 1.0 / (lights[i].linAtt[2] * dist * dist +\n"
"                           lights[i].linAtt[1] * dist +\n"
"                           lights[i].linAtt[0]);\n"
"        float angAtt = lights[i].angAtt[2] * angDot * angDot +\n"
"                       lights[i].angAtt[1] * angDot +\n"
"                       lights[i].angAtt[0];\n"
"        ret += lights[i].color * clamp(angAtt, 0.0, 1.0) * att * clamp(dot(normalize(-delta), mvNormIn.xyz), 0.0, 1.0);\n"
"    }\n"
"    \n"
"    return clamp(ret, vec4(0.0,0.0,0.0,0.0), vec4(1.0,1.0,1.0,1.0));\n"
"}\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 mvPos;\n"
"    vec4 mvNorm;\n"
"    vec4 mvBinorm;\n"
"    vec4 mvTangent;\n"
"    vec4 color;\n"
"    vec2 uvs[7];\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"%s" // Textures here
"void main()\n"
"{\n"
"    vec4 lighting = LightingFunc(vtf.mvPos, normalize(vtf.mvNorm));\n"
"%s" // Combiner expression here
"}\n";

static const char* FSDoor =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 mvPos;\n"
"    vec4 mvNorm;\n"
"    vec4 mvBinorm;\n"
"    vec4 mvTangent;\n"
"    vec4 color;\n"
"    vec2 uvs[7];\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"%s" // Textures here
"void main()\n"
"{\n"
"%s" // Combiner expression here
"}\n";

static void _BuildShader(std::string& finalVS, std::string& finalFS, int& nextTex, const char* texNames[7],
                         const SFluidPlaneShaderInfo& info)
{
    std::string additionalTCGs;
    std::string textures;
    std::string combiner;
    int nextTCG = 3;
    int nextMtx = 4;
    int bumpMapUv, envBumpMapUv, envMapUv, lightmapUv;

    if (info.m_hasPatternTex1)
    {
        texNames[nextTex] = "patternTex1";
        textures += hecl::Format("TBINDING%d uniform sampler2D patternTex1;\n", nextTex++);
    }
    if (info.m_hasPatternTex2)
    {
        texNames[nextTex] = "patternTex2";
        textures += hecl::Format("TBINDING%d uniform sampler2D patternTex2;\n", nextTex++);
    }
    if (info.m_hasColorTex)
    {
        texNames[nextTex] = "colorTex";
        textures += hecl::Format("TBINDING%d uniform sampler2D colorTex;\n", nextTex++);
    }
    if (info.m_hasBumpMap)
    {
        texNames[nextTex] = "bumpMap";
        textures += hecl::Format("TBINDING%d uniform sampler2D bumpMap;\n", nextTex++);
    }
    if (info.m_hasEnvMap)
    {
        texNames[nextTex] = "envMap";
        textures += hecl::Format("TBINDING%d uniform sampler2D envMap;\n", nextTex++);
    }
    if (info.m_hasEnvBumpMap)
    {
        texNames[nextTex] = "envBumpMap";
        textures += hecl::Format("TBINDING%d uniform sampler2D envBumpMap;\n", nextTex++);
    }
    if (info.m_hasLightmap)
    {
        texNames[nextTex] = "lightMap";
        textures += hecl::Format("TBINDING%d uniform sampler2D lightMap;\n", nextTex++);
    }

    if (info.m_hasBumpMap)
    {
        bumpMapUv = nextTCG;
        additionalTCGs += hecl::Format("    vtf.uvs[%d] = (texMtxs[0] * posIn).xy;\n", nextTCG++);
    }
    if (info.m_hasEnvBumpMap)
    {
        envBumpMapUv = nextTCG;
        additionalTCGs += hecl::Format("    vtf.uvs[%d] = (texMtxs[3] * posIn).xy;\n", nextTCG++);
    }
    if (info.m_hasEnvMap)
    {
        envMapUv = nextTCG;
        additionalTCGs += hecl::Format("    vtf.uvs[%d] = (texMtxs[%d] * posIn).xy;\n", nextTCG++, nextMtx++);
    }
    if (info.m_hasLightmap)
    {
        lightmapUv = nextTCG;
        additionalTCGs += hecl::Format("    vtf.uvs[%d] = (texMtxs[%d] * posIn).xy;\n", nextTCG++, nextMtx++);
    }

    switch (info.m_type)
    {
    case CFluidPlane::EFluidType::NormalWater:
    case CFluidPlane::EFluidType::PhazonFluid:
    case CFluidPlane::EFluidType::Four:
        if (info.m_hasLightmap)
        {
            combiner += hecl::Format("    vec4 lightMapTexel = texture(lightMap, vtf.uvs[%d]);\n", lightmapUv);
            // 0: Tex4TCG, Tex4, doubleLightmapBlend ? NULL : GX_COLOR1A1
            // ZERO, TEX, KONST, doubleLightmapBlend ? ZERO : RAS
            // Output reg 2
            // KColor 2
            if (info.m_doubleLightmapBlend)
            {
                // 1: Tex4TCG2, Tex4, GX_COLOR1A1
                // C2, TEX, KONST, RAS
                // Output reg 2
                // KColor 3
                // Tex * K2 + Lighting
                combiner += "    lighting += mix(lightMapTexel * kColor2, lightMapTexel, kColor3);\n";
            }
            else
            {
                // mix(Tex * K2, Tex, K3) + Lighting
                combiner += "    lighting += lightMapTexel * kColor2;\n";
            }
        }

        // Next: Tex0TCG, Tex0, GX_COLOR1A1
        // ZERO, TEX, KONST, RAS
        // Output reg prev
        // KColor 0

        // Next: Tex1TCG, Tex1, GX_COLOR0A0
        // ZERO, TEX, PREV, RAS
        // Output reg prev

        // Next: Tex2TCG, Tex2, GX_COLOR1A1
        // ZERO, TEX, hasTex4 ? C2 : RAS, PREV
        // Output reg prev

        // (Tex0 * kColor0 + Lighting) * Tex1 + VertColor + Tex2 * Lighting
        if (info.m_hasPatternTex2)
        {
            if (info.m_hasPatternTex1)
                combiner += "    colorOut = (texture(patternTex1, vtf.uvs[0]) * kColor0 + lighting) *\n"
                            "               texture(patternTex2, vtf.uvs[1]) + vtf.color;\n";
            else
                combiner += "    colorOut = lighting * texture(patternTex2, vtf.uvs[1]) + vtf.color;\n";
        }
        else
        {
            combiner += "    colorOut = vtf.color;\n";
        }


        if (info.m_hasColorTex && !info.m_hasEnvMap && info.m_hasEnvBumpMap)
        {
            // Make previous stage indirect, mtx0
            combiner += hecl::Format("    vec2 indUvs = (texture(envBumpMap, vtf.uvs[%d]).ra - vec2(0.5, 0.5)) *\n"
                                     "        vec2(fog.indScale, -fog.indScale);", envBumpMapUv);
            combiner += "    colorOut += texture(colorTex, indUvs + vtf.uvs[2]) * lighting;\n";
        }
        else if (info.m_hasEnvMap)
        {
            // Next: envTCG, envTex, NULL
            // PREV, TEX, KONST, ZERO
            // Output reg prev
            // KColor 1

            // Make previous stage indirect, mtx0
            if (info.m_hasColorTex)
                combiner += "    colorOut += texture(colorTex, vtf.uvs[2]) * lighting;\n";
            combiner += hecl::Format("    vec2 indUvs = (texture(envBumpMap, vtf.uvs[%d]).ra - vec2(0.5, 0.5)) *\n"
                                     "        vec2(fog.indScale, -fog.indScale);", envBumpMapUv);
            combiner += hecl::Format("    colorOut = mix(colorOut, texture(envMap, indUvs + vtf.uvs[%d]), kColor1);\n", 
                                     envMapUv);
        }
        else if (info.m_hasColorTex)
        {
            combiner += "    colorOut += texture(colorTex, vtf.uvs[2]) * lighting;\n";
        }

        break;

    case CFluidPlane::EFluidType::PoisonWater:
        if (info.m_hasLightmap)
        {
            combiner += hecl::Format("    vec4 lightMapTexel = texture(lightMap, vtf.uvs[%d]);\n", lightmapUv);
            // 0: Tex4TCG, Tex4, doubleLightmapBlend ? NULL : GX_COLOR1A1
            // ZERO, TEX, KONST, doubleLightmapBlend ? ZERO : RAS
            // Output reg 2
            // KColor 2
            if (info.m_doubleLightmapBlend)
            {
                // 1: Tex4TCG2, Tex4, GX_COLOR1A1
                // C2, TEX, KONST, RAS
                // Output reg 2
                // KColor 3
                // Tex * K2 + Lighting
                combiner += "    lighting += mix(lightMapTexel * kColor2, lightMapTexel, kColor3);\n";
            }
            else
            {
                // mix(Tex * K2, Tex, K3) + Lighting
                combiner += "    lighting += lightMapTexel * kColor2;\n";
            }
        }

        // Next: Tex0TCG, Tex0, GX_COLOR1A1
        // ZERO, TEX, KONST, RAS
        // Output reg prev
        // KColor 0

        // Next: Tex1TCG, Tex1, GX_COLOR0A0
        // ZERO, TEX, PREV, RAS
        // Output reg prev

        // Next: Tex2TCG, Tex2, GX_COLOR1A1
        // ZERO, TEX, hasTex4 ? C2 : RAS, PREV
        // Output reg prev

        // (Tex0 * kColor0 + Lighting) * Tex1 + VertColor + Tex2 * Lighting
        if (info.m_hasPatternTex2)
        {
            if (info.m_hasPatternTex1)
                combiner += "    colorOut = (texture(patternTex1, vtf.uvs[0]) * kColor0 + lighting) *\n"
                            "               texture(patternTex2, vtf.uvs[1]) + vtf.color;\n";
            else
                combiner += "    colorOut = lighting * texture(patternTex2, vtf.uvs[1]) + vtf.color;\n";
        }
        else
        {
            combiner += "    colorOut = vtf.color;\n";
        }

        if (info.m_hasColorTex)
        {
            if (info.m_hasEnvBumpMap)
            {
                // Make previous stage indirect, mtx0
                combiner += hecl::Format("    vec2 indUvs = (texture(envBumpMap, vtf.uvs[%d]).ra - vec2(0.5, 0.5)) *\n"
                                             "        vec2(fog.indScale, -fog.indScale);", envBumpMapUv);
                combiner += "    colorOut += texture(colorTex, indUvs + vtf.uvs[2]) * lighting;\n";
            }
            else
            {
                combiner += "    colorOut += texture(colorTex, vtf.uvs[2]) * lighting;\n";
            }
        }

        break;

    case CFluidPlane::EFluidType::Lava:
        // 0: Tex0TCG, Tex0, GX_COLOR0A0
        // ZERO, TEX, KONST, RAS
        // Output reg prev
        // KColor 0

        // 1: Tex1TCG, Tex1, GX_COLOR0A0
        // ZERO, TEX, PREV, RAS
        // Output reg prev

        // 2: Tex2TCG, Tex2, NULL
        // ZERO, TEX, ONE, PREV
        // Output reg prev

        // (Tex0 * kColor0 + VertColor) * Tex1 + VertColor + Tex2
        if (info.m_hasPatternTex2)
        {
            if (info.m_hasPatternTex1)
                combiner += "    colorOut = (texture(patternTex1, vtf.uvs[0]) * kColor0 + vtf.color) *\n"
                            "               texture(patternTex2, vtf.uvs[1]) + vtf.color;\n";
            else
                combiner += "    colorOut = vtf.color * texture(patternTex2, vtf.uvs[1]) + vtf.color;\n";
        }
        else
        {
            combiner += "    colorOut = vtf.color;\n";
        }

        if (info.m_hasColorTex)
            combiner += "    colorOut += texture(colorTex, vtf.uvs[2]);\n";

        if (info.m_hasBumpMap)
        {
            // 3: bumpMapTCG, bumpMap, NULL
            // ZERO, TEX, ONE, HALF
            // Output reg 0, no clamp, no bias

            // 4: bumpMapTCG2, bumpMap, NULL
            // ZERO, TEX, ONE, C0
            // Output reg 0, subtract, clamp, no bias

            combiner += "    vec3 lightVec = lights[3].pos.xyz - vtf.mvPos.xyz;\n"
                        "    float lx = dot(vtf.mvTangent, lightVec);\n"
                        "    float ly = dot(vtf.mvBinorm, lightVec);\n";
            combiner += hecl::Format("    vec4 emboss1 = texture(bumpMap, vtf.uvs[%d]) + vec4(0.5);\n"
                                     "    vec4 emboss2 = texture(bumpMap, vtf.uvs[%d] + vec2(lx, ly));\n",
                                     bumpMapUv, bumpMapUv);

            // 5: NULL, NULL, NULL
            // ZERO, PREV, C0, ZERO
            // Output reg prev, scale 2, clamp

            // colorOut * clamp(emboss1 + 0.5 - emboss2, 0.0, 1.0) * 2.0
            combiner += "colorOut *= clamp((emboss1 + vec4(0.5) - emboss2) * vec4(2.0), vec4(0.0), vec4(1.0));\n";
        }

        break;

    case CFluidPlane::EFluidType::ThickLava:
        // 0: Tex0TCG, Tex0, GX_COLOR0A0
        // ZERO, TEX, KONST, RAS
        // Output reg prev
        // KColor 0

        // 1: Tex1TCG, Tex1, GX_COLOR0A0
        // ZERO, TEX, PREV, RAS
        // Output reg prev

        // 2: Tex2TCG, Tex2, NULL
        // ZERO, TEX, ONE, PREV
        // Output reg prev

        // (Tex0 * kColor0 + VertColor) * Tex1 + VertColor + Tex2
        if (info.m_hasPatternTex2)
        {
            if (info.m_hasPatternTex1)
                combiner += "    colorOut = (texture(patternTex1, vtf.uvs[0]) * kColor0 + vtf.color) *\n"
                            "               texture(patternTex2, vtf.uvs[1]) + vtf.color;\n";
            else
                combiner += "    colorOut = vtf.color * texture(patternTex2, vtf.uvs[1]) + vtf.color;\n";
        }
        else
        {
            combiner += "    colorOut = vtf.color;\n";
        }

        if (info.m_hasColorTex)
            combiner += "    colorOut += texture(colorTex, vtf.uvs[2]);\n";

        if (info.m_hasBumpMap)
        {
            // 3: bumpMapTCG, bumpMap, NULL
            // ZERO, TEX, PREV, ZERO
            // Output reg prev, scale 2
            combiner += hecl::Format("    vec4 emboss1 = texture(bumpMap, vtf.uvs[%d]) + vec4(0.5);\n", bumpMapUv);
            combiner += "colorOut *= emboss1 * vec4(2.0);\n";
        }

        break;
    }

    combiner += "    colorOut.a = kColor0.a;\n";

    char *finalVSs, *finalFSs;
    asprintf(&finalVSs, VS, additionalTCGs.c_str());
    asprintf(&finalFSs, FS, textures.c_str(), combiner.c_str());

    finalVS = finalVSs;
    finalFS = finalFSs;

    free(finalVSs);
    free(finalFSs);
}

static void _BuildShader(std::string& finalVS, std::string& finalFS, int& nextTex, const char* texNames[3],
                         const SFluidPlaneDoorShaderInfo& info)
{
    std::string additionalTCGs;
    std::string textures;
    std::string combiner;

    if (info.m_hasPatternTex1)
    {
        texNames[nextTex] = "patternTex1";
        textures += hecl::Format("TBINDING%d uniform sampler2D patternTex1;\n", nextTex++);
    }
    if (info.m_hasPatternTex2)
    {
        texNames[nextTex] = "patternTex2";
        textures += hecl::Format("TBINDING%d uniform sampler2D patternTex2;\n", nextTex++);
    }
    if (info.m_hasColorTex)
    {
        texNames[nextTex] = "colorTex";
        textures += hecl::Format("TBINDING%d uniform sampler2D colorTex;\n", nextTex++);
    }

    // Tex0 * kColor0 * Tex1 + Tex2
    if (info.m_hasPatternTex1 && info.m_hasPatternTex2)
    {
        combiner += "    colorOut = texture(patternTex1, vtf.uvs[0]) * kColor0 *\n"
                    "               texture(patternTex2, vtf.uvs[1]);\n";
    }
    else
    {
        combiner += "    colorOut = vec4(0.0);\n";
    }

    if (info.m_hasColorTex)
    {
        combiner += "    colorOut += texture(colorTex, vtf.uvs[2]);\n";
    }

    combiner += "    colorOut.a = kColor0.a;\n";

    char *finalVSs, *finalFSs;
    asprintf(&finalVSs, VS, additionalTCGs.c_str());
    asprintf(&finalFSs, FSDoor, textures.c_str(), combiner.c_str());

    finalVS = finalVSs;
    finalFS = finalFSs;

    free(finalVSs);
    free(finalFSs);
}

boo::ObjToken<boo::IShaderPipeline>
CFluidPlaneShader::BuildShader(boo::GLDataFactory::Context& ctx, const SFluidPlaneShaderInfo& info)
{
    int nextTex = 0;
    const char* texNames[7] = {};
    std::string finalVS, finalFS;
    _BuildShader(finalVS, finalFS, nextTex, texNames, info);
    const char* uniNames[] = {"FluidPlaneUniform", "FluidPlaneUniform", "LightingUniform"};
    return ctx.newShaderPipeline(finalVS.c_str(), finalFS.c_str(), size_t(nextTex), texNames, 3, uniNames,
                                 info.m_additive ? boo::BlendFactor::One : boo::BlendFactor::SrcAlpha,
                                 info.m_additive ? boo::BlendFactor::One : boo::BlendFactor::InvSrcAlpha,
                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false, true, false,
                                 boo::CullMode::None);
}

boo::ObjToken<boo::IShaderPipeline>
CFluidPlaneShader::BuildShader(boo::GLDataFactory::Context& ctx, const SFluidPlaneDoorShaderInfo& info)
{
    int nextTex = 0;
    const char* texNames[3] = {};
    std::string finalVS, finalFS;
    _BuildShader(finalVS, finalFS, nextTex, texNames, info);
    const char* uniNames[] = {"FluidPlaneUniform"};
    return ctx.newShaderPipeline(finalVS.c_str(), finalFS.c_str(), size_t(nextTex), texNames, 1, uniNames,
                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false, true, false,
                                 boo::CullMode::None);
}

#if BOO_HAS_VULKAN
static boo::ObjToken<boo::IVertexFormat> s_vtxFmt;

boo::ObjToken<boo::IShaderPipeline>
CFluidPlaneShader::BuildShader(boo::VulkanDataFactory::Context& ctx, const SFluidPlaneShaderInfo& info)
{
    if (!s_vtxFmt)
    {
        boo::VertexElementDescriptor elements[] =
        {
            {nullptr, nullptr, boo::VertexSemantic::Position4},
            {nullptr, nullptr, boo::VertexSemantic::Normal4, 0},
            {nullptr, nullptr, boo::VertexSemantic::Normal4, 1},
            {nullptr, nullptr, boo::VertexSemantic::Normal4, 2},
            {nullptr, nullptr, boo::VertexSemantic::Color}
        };
        s_vtxFmt = ctx.newVertexFormat(5, elements);
    }

    int nextTex = 0;
    const char* texNames[7] = {};
    std::string finalVS, finalFS;
    _BuildShader(finalVS, finalFS, nextTex, texNames, info);
    return ctx.newShaderPipeline(finalVS.c_str(), finalFS.c_str(), s_vtxFmt,
                                 info.m_additive ? boo::BlendFactor::One : boo::BlendFactor::SrcAlpha,
                                 info.m_additive ? boo::BlendFactor::One : boo::BlendFactor::InvSrcAlpha,
                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false, true, false,
                                 boo::CullMode::None);
}

boo::ObjToken<boo::IShaderPipeline>
CFluidPlaneShader::BuildShader(boo::VulkanDataFactory::Context& ctx, const SFluidPlaneDoorShaderInfo& info)
{
    if (!s_vtxFmt)
    {
        boo::VertexElementDescriptor elements[] =
        {
            {nullptr, nullptr, boo::VertexSemantic::Position4},
            {nullptr, nullptr, boo::VertexSemantic::Normal4, 0},
            {nullptr, nullptr, boo::VertexSemantic::Normal4, 1},
            {nullptr, nullptr, boo::VertexSemantic::Normal4, 2},
            {nullptr, nullptr, boo::VertexSemantic::Color}
        };
        s_vtxFmt = ctx.newVertexFormat(5, elements);
    }

    int nextTex = 0;
    const char* texNames[3] = {};
    std::string finalVS, finalFS;
    _BuildShader(finalVS, finalFS, nextTex, texNames, info);
    return ctx.newShaderPipeline(finalVS.c_str(), finalFS.c_str(), s_vtxFmt,
                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false, true, false,
                                 boo::CullMode::None);
}
#endif

boo::ObjToken<boo::IShaderDataBinding>
CFluidPlaneShader::BuildBinding(boo::GLDataFactory::Context& ctx,
                                const boo::ObjToken<boo::IShaderPipeline>& pipeline, bool door)
{
    boo::VertexElementDescriptor elements[] =
    {
        {m_vbo.get(), nullptr, boo::VertexSemantic::Position4},
        {m_vbo.get(), nullptr, boo::VertexSemantic::Normal4, 0},
        {m_vbo.get(), nullptr, boo::VertexSemantic::Normal4, 1},
        {m_vbo.get(), nullptr, boo::VertexSemantic::Normal4, 2},
        {m_vbo.get(), nullptr, boo::VertexSemantic::Color}
    };
    boo::ObjToken<boo::IVertexFormat> vtxFmt = ctx.newVertexFormat(5, elements);
    boo::ObjToken<boo::IGraphicsBuffer> ubufs[] = { m_uniBuf.get(), m_uniBuf.get(), m_uniBuf.get() };
    boo::PipelineStage ubufStages[] = { boo::PipelineStage::Vertex, boo::PipelineStage::Vertex,
                                        boo::PipelineStage::Fragment };
    size_t ubufOffs[] = {0, 0, 768};
    size_t ubufSizes[] = {768, 768, 256};
    size_t texCount = 0;
    boo::ObjToken<boo::ITexture> texs[7];
    if (m_patternTex1)
        texs[texCount++] = (*m_patternTex1)->GetBooTexture();
    if (m_patternTex2)
        texs[texCount++] = (*m_patternTex2)->GetBooTexture();
    if (m_colorTex)
        texs[texCount++] = (*m_colorTex)->GetBooTexture();
    if (m_bumpMap)
        texs[texCount++] = (*m_bumpMap)->GetBooTexture();
    if (m_envMap)
        texs[texCount++] = (*m_envMap)->GetBooTexture();
    if (m_envBumpMap)
        texs[texCount++] = (*m_envBumpMap)->GetBooTexture();
    if (m_lightmap)
        texs[texCount++] = (*m_lightmap)->GetBooTexture();
    return ctx.newShaderDataBinding(pipeline, vtxFmt, m_vbo.get(), nullptr, nullptr, door ? 1 : 3,
                                    ubufs, ubufStages, ubufOffs, ubufSizes, texCount, texs, nullptr, nullptr);
}

#if BOO_HAS_VULKAN
boo::ObjToken<boo::IShaderDataBinding>
CFluidPlaneShader::BuildBinding(boo::VulkanDataFactory::Context& ctx,
                                const boo::ObjToken<boo::IShaderPipeline>& pipeline, bool door)
{
    boo::ObjToken<boo::IGraphicsBuffer> ubufs[] = { m_uniBuf.get(), m_uniBuf.get(), m_uniBuf.get() };
    boo::PipelineStage ubufStages[] = { boo::PipelineStage::Vertex, boo::PipelineStage::Vertex,
                                        boo::PipelineStage::Fragment };
    size_t ubufOffs[] = {0, 0, 768};
    size_t ubufSizes[] = {768, 768, 256};
    size_t texCount = 0;
    boo::ObjToken<boo::ITexture> texs[7] = {};
    if (m_patternTex1)
        texs[texCount++] = (*m_patternTex1)->GetBooTexture();
    if (m_patternTex2)
        texs[texCount++] = (*m_patternTex2)->GetBooTexture();
    if (m_colorTex)
        texs[texCount++] = (*m_colorTex)->GetBooTexture();
    if (m_bumpMap)
        texs[texCount++] = (*m_bumpMap)->GetBooTexture();
    if (m_envMap)
        texs[texCount++] = (*m_envMap)->GetBooTexture();
    if (m_envBumpMap)
        texs[texCount++] = (*m_envBumpMap)->GetBooTexture();
    if (m_lightmap)
        texs[texCount++] = (*m_lightmap)->GetBooTexture();
    return ctx.newShaderDataBinding(pipeline, s_vtxFmt, m_vbo.get(), nullptr, nullptr, door ? 1 : 3,
                                    ubufs, ubufStages, ubufOffs, ubufSizes, texCount, texs, nullptr, nullptr);
}
#endif

}
