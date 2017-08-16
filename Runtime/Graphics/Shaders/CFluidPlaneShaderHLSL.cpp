#include "CFluidPlaneShader.hpp"

namespace urde
{

static boo::IVertexFormat* s_vtxFmt = nullptr;

static const char* VS =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"    float4 normalIn : NORMAL0;\n"
"    float4 binormalIn : NORMAL1;\n"
"    float4 tangentIn : NORMAL2;\n"
"    float4 colorIn : COLOR;\n"
"};\n"
"\n"
"cbuffer FluidPlaneUniform : register(b0)\n"
"{\n"
"    float4x4 mv;\n"
"    float4x4 mvNorm;\n"
"    float4x4 proj;\n"
"    float4x4 texMtxs[6];\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos : SV_Position;\n"
"    float4 mvPos : POSITION;\n"
"    float4 mvNorm : NORMAL;\n"
"    float4 mvBinorm : BINORMAL;\n"
"    float4 mvTangent : TANGENT;\n"
"    float4 color : COLOR;\n"
"    float2 uvs[7] : UV;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.mvPos = mul(mv, float4(v.posIn.xyz, 1.0));\n"
"    vtf.pos = mul(proj, vtf.mvPos);\n"
"    vtf.mvNorm = mul(mvNorm, v.normalIn);\n"
"    vtf.mvBinorm = mul(mvNorm, v.binormalIn);\n"
"    vtf.mvTangent = mul(mvNorm, v.tangentIn);\n"
"    vtf.color = v.colorIn;\n"
"    vtf.uvs[0] = mul(texMtxs[0], v.posIn).xy;\n"
"    vtf.uvs[1] = mul(texMtxs[1], v.posIn).xy;\n"
"    vtf.uvs[2] = mul(texMtxs[2], v.posIn).xy;\n"
"%s" // Additional TCGs here
"    return vtf;\n"
"}\n";

static const char* FS =
"struct Light\n"
"{\n"
"    float4 pos;\n"
"    float4 dir;\n"
"    float4 color;\n"
"    float4 linAtt;\n"
"    float4 angAtt;\n"
"};\n"
"struct Fog\n" // Reappropriated for indirect texture scaling
"{\n"
"    float4 color;\n"
"    float indScale;\n"
"    float start;\n"
"};\n"
"\n"
"struct LightingUniform\n"
"{\n"
"    Light lights[" _XSTR(URDE_MAX_LIGHTS) "];\n"
"    float4 ambient;\n"
"    float4 kColor0;\n"
"    float4 kColor1;\n"
"    float4 kColor2;\n"
"    float4 kColor3;\n"
"    Fog fog;\n"
"};\n"
"\n"
"static float4 LightingFunc(float4 mvPosIn, float4 mvNormIn)\n"
"{\n"
"    float4 ret = ambient;\n"
"    \n"
"    for (int i=0 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        float3 delta = mvPosIn.xyz - lights[i].pos.xyz;\n"
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
"    return clamp(ret, float4(0.0,0.0,0.0,0.0), float4(1.0,1.0,1.0,1.0));\n"
"}\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos : SV_Position;\n"
"    float4 mvPos : POSITION;\n"
"    float4 mvNorm : NORMAL;\n"
"    float4 mvBinorm : BINORMAL;\n"
"    float4 mvTangent : TANGENT;\n"
"    float4 color : COLOR;\n"
"    float2 uvs[7] : UV;\n"
"};\n"
"\n"
"SamplerState samp : register(s0);\n"
"%s" // Textures here
"float4 main(in VertToFrag vtf)\n"
"{\n"
"    float4 lighting = LightingFunc(vtf.mvPos, normalize(vtf.mvNorm));\n"
"    float4 colorOut;\n"
"%s" // Combiner expression here
"    return colorOut;\n"
"}\n";

static const char* FSDoor =
"struct VertToFrag\n"
"{\n"
"    float4 pos : SV_Position;\n"
"    float4 mvPos : POSITION;\n"
"    float4 mvNorm : NORMAL;\n"
"    float4 mvBinorm : BINORMAL;\n"
"    float4 mvTangent : TANGENT;\n"
"    float4 color : COLOR;\n"
"    float2 uvs[7] : UV;\n"
"};\n"
"\n"
"SamplerState samp : register(s0);\n"
"%s" // Textures here
"float4 main(in VertToFrag vtf)\n"
"{\n"
"    float4 colorOut;\n"
"%s" // Combiner expression here
"    return colorOut;\n"
"}\n";

boo::IShaderPipeline*
CFluidPlaneShader::BuildShader(boo::ID3DDataFactory::Context& ctx, const SFluidPlaneShaderInfo& info)
{
    if (s_vtxFmt == nullptr)
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

    std::string additionalTCGs;
    std::string textures;
    std::string combiner;
    int nextTex = 0;
    int nextTCG = 3;
    int nextMtx = 4;
    int bumpMapUv, envBumpMapUv, envMapUv, lightmapUv;

    if (info.m_hasPatternTex1)
        textures += hecl::Format("Texture2D patternTex1 : register(t%d)\n", nextTex++);
    if (info.m_hasPatternTex2)
        textures += hecl::Format("Texture2D patternTex2 : register(t%d)\n", nextTex++);
    if (info.m_hasColorTex)
        textures += hecl::Format("Texture2D colorTex : register(t%d)\n", nextTex++);
    if (info.m_hasBumpMap)
        textures += hecl::Format("Texture2D bumpMap : register(t%d)\n", nextTex++);
    if (info.m_hasEnvMap)
        textures += hecl::Format("Texture2D envMap : register(t%d)\n", nextTex++);
    if (info.m_hasEnvBumpMap)
        textures += hecl::Format("Texture2D envBumpMap : register(t%d)\n", nextTex++);
    if (info.m_hasLightmap)
        textures += hecl::Format("Texture2D lightMap : register(t%d)\n", nextTex++);

    if (info.m_hasBumpMap)
    {
        bumpMapUv = nextTCG;
        additionalTCGs += hecl::Format("    vtf.uvs[%d] = mul(texMtxs[0], v.posIn).xy;\n", nextTCG++);
    }
    if (info.m_hasEnvBumpMap)
    {
        envBumpMapUv = nextTCG;
        additionalTCGs += hecl::Format("    vtf.uvs[%d] = mul(texMtxs[3], v.posIn).xy;\n", nextTCG++);
    }
    if (info.m_hasEnvMap)
    {
        envMapUv = nextTCG;
        additionalTCGs += hecl::Format("    vtf.uvs[%d] = mul(texMtxs[%d], v.posIn).xy;\n", nextTCG++, nextMtx++);
    }
    if (info.m_hasLightmap)
    {
        lightmapUv = nextTCG;
        additionalTCGs += hecl::Format("    vtf.uvs[%d] = mul(texMtxs[%d], v.posIn).xy;\n", nextTCG++, nextMtx++);
    }

    switch (info.m_type)
    {
    case CFluidPlane::EFluidType::NormalWater:
    case CFluidPlane::EFluidType::PhazonFluid:
    case CFluidPlane::EFluidType::Four:
        if (info.m_hasLightmap)
        {
            combiner += hecl::Format("    float4 lightMapTexel = lightMap.Sample(samp, vtf.uvs[%d]);\n", lightmapUv);
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
                combiner += "    colorOut = (patternTex1.Sample(samp, vtf.uvs[0]) * kColor0 + lighting) *\n"
                            "               patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
            else
                combiner += "    colorOut = lighting * patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
        }
        else
        {
            combiner += "    colorOut = vtf.color;\n";
        }


        if (info.m_hasColorTex && !info.m_hasEnvMap && info.m_hasEnvBumpMap)
        {
            // Make previous stage indirect, mtx0
            combiner += hecl::Format("    float2 indUvs = (envBumpMap.Sample(samp, vtf.uvs[%d]).ra - float2(0.5, 0.5)) *\n"
                                     "        float2(fog.indScale, -fog.indScale);", envBumpMapUv);
            combiner += "    colorOut += colorTex.Sample(samp, indUvs + vtf.uvs[2]) * lighting;\n";
        }
        else if (info.m_hasEnvMap)
        {
            // Next: envTCG, envTex, NULL
            // PREV, TEX, KONST, ZERO
            // Output reg prev
            // KColor 1

            // Make previous stage indirect, mtx0
            if (info.m_hasColorTex)
                combiner += "    colorOut += colorTex.Sample(samp, vtf.uvs[2]) * lighting;\n";
            combiner += hecl::Format("    float2 indUvs = (envBumpMap.Sample(samp, vtf.uvs[%d]).ra - float2(0.5, 0.5)) *\n"
                                     "        float2(fog.indScale, -fog.indScale);", envBumpMapUv);
            combiner += hecl::Format("    colorOut = mix(colorOut, envMap.Sample(samp, indUvs + vtf.uvs[%d]), kColor1);\n",
                                     envMapUv);
        }
        else if (info.m_hasColorTex)
        {
            combiner += "    colorOut += colorTex.Sample(samp, vtf.uvs[2]) * lighting;\n";
        }

        break;

    case CFluidPlane::EFluidType::PoisonWater:
        if (info.m_hasLightmap)
        {
            combiner += hecl::Format("    float4 lightMapTexel = lightMap.Sample(samp, vtf.uvs[%d]);\n", lightmapUv);
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
                combiner += "    colorOut = (patternTex1.Sample(samp, vtf.uvs[0]) * kColor0 + lighting) *\n"
                            "               patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
            else
                combiner += "    colorOut = lighting * patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
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
                combiner += hecl::Format("    float2 indUvs = (envBumpMap.Sample(samp, vtf.uvs[%d]).ra - float2(0.5, 0.5)) *\n"
                                         "        float2(fog.indScale, -fog.indScale);", envBumpMapUv);
                combiner += "    colorOut += colorTex.Sample(samp, indUvs + vtf.uvs[2]) * lighting;\n";
            }
            else
            {
                combiner += "    colorOut += colorTex.Sample(samp, vtf.uvs[2]) * lighting;\n";
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
                combiner += "    colorOut = (patternTex1.Sample(samp, vtf.uvs[0]) * kColor0 + vtf.color) *\n"
                            "               patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
            else
                combiner += "    colorOut = vtf.color * patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
        }
        else
        {
            combiner += "    colorOut = vtf.color;\n";
        }

        if (info.m_hasColorTex)
            combiner += "    colorOut += colorTex.Sample(samp, vtf.uvs[2]);\n";

        if (info.m_hasBumpMap)
        {
            // 3: bumpMapTCG, bumpMap, NULL
            // ZERO, TEX, ONE, HALF
            // Output reg 0, no clamp, no bias

            // 4: bumpMapTCG2, bumpMap, NULL
            // ZERO, TEX, ONE, C0
            // Output reg 0, subtract, clamp, no bias

            combiner += "    float3 lightVec = lights[3].pos.xyz - vtf.mvPos.xyz;\n"
                        "    float lx = dot(vtf.mvTangent.xyz, lightVec);\n"
                        "    float ly = dot(vtf.mvBinorm.xyz, lightVec);\n";
            combiner += hecl::Format("    float4 emboss1 = bumpMap.Sample(samp, vtf.uvs[%d]) + float4(0.5);\n"
                                     "    float4 emboss2 = bumpMap.Sample(samp, vtf.uvs[%d] + float2(lx, ly));\n",
                                     bumpMapUv, bumpMapUv);

            // 5: NULL, NULL, NULL
            // ZERO, PREV, C0, ZERO
            // Output reg prev, scale 2, clamp

            // colorOut * clamp(emboss1 + 0.5 - emboss2, 0.0, 1.0) * 2.0
            combiner += "colorOut *= clamp((emboss1 + float4(0.5) - emboss2) * float4(2.0), float4(0.0), float4(1.0));\n";
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
                combiner += "    colorOut = (patternTex1.Sample(samp, vtf.uvs[0]) * kColor0 + vtf.color) *\n"
                            "               patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
            else
                combiner += "    colorOut = vtf.color * patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
        }
        else
        {
            combiner += "    colorOut = vtf.color;\n";
        }

        if (info.m_hasColorTex)
            combiner += "    colorOut += colorTex.Sample(samp, vtf.uvs[2]);\n";

        if (info.m_hasBumpMap)
        {
            // 3: bumpMapTCG, bumpMap, NULL
            // ZERO, TEX, PREV, ZERO
            // Output reg prev, scale 2
            combiner += hecl::Format("    float4 emboss1 = bumpMap.Sample(samp, vtf.uvs[%d]) + float4(0.5);\n", bumpMapUv);
            combiner += "colorOut *= emboss1 * float4(2.0);\n";
        }

        break;
    }

    combiner += "    colorOut.a = kColor0.a;\n";

    std::string finalVS = hecl::Format(VS, additionalTCGs.c_str());
    std::string finalFS = hecl::Format(FS, textures.c_str(), combiner.c_str());

    return ctx.newShaderPipeline(finalVS.c_str(), finalFS.c_str(), nullptr, nullptr, nullptr, s_vtxFmt,
                                 info.m_additive ? boo::BlendFactor::One : boo::BlendFactor::SrcAlpha,
                                 info.m_additive ? boo::BlendFactor::One : boo::BlendFactor::InvSrcAlpha,
                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false, true, false,
                                 boo::CullMode::None);
}

boo::IShaderPipeline*
CFluidPlaneShader::BuildShader(boo::ID3DDataFactory::Context& ctx, const SFluidPlaneDoorShaderInfo& info)
{
    if (s_vtxFmt == nullptr)
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

    std::string additionalTCGs;
    std::string textures;
    std::string combiner;
    int nextTex = 0;

    if (info.m_hasPatternTex1)
        textures += hecl::Format("Texture2D patternTex1 : register(t%d)\n", nextTex++);
    if (info.m_hasPatternTex2)
        textures += hecl::Format("Texture2D patternTex2 : register(t%d)\n", nextTex++);
    if (info.m_hasColorTex)
        textures += hecl::Format("Texture2D colorTex : register(t%d)\n", nextTex++);

    // Tex0 * kColor0 * Tex1 + Tex2
    if (info.m_hasPatternTex1 && info.m_hasPatternTex2)
    {
        combiner += "    colorOut = patternTex1.Sample(samp, vtf.uvs[0]) * kColor0 *\n"
                    "               patternTex2.Sample(samp, vtf.uvs[1]);\n";
    }
    else
    {
        combiner += "    colorOut = float4(0.0);\n";
    }

    if (info.m_hasColorTex)
    {
        combiner += "    colorOut += colorTex.Sample(samp, vtf.uvs[2]);\n";
    }

    combiner += "    colorOut.a = kColor0.a;\n";

    std::string finalVS = hecl::Format(VS, additionalTCGs.c_str());
    std::string finalFS = hecl::Format(FSDoor, textures.c_str(), combiner.c_str());

    return ctx.newShaderPipeline(finalVS.c_str(), finalFS.c_str(), nullptr, nullptr, nullptr, s_vtxFmt,
                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false, true, false,
                                 boo::CullMode::None);
}

boo::IShaderDataBinding* CFluidPlaneShader::BuildBinding(boo::ID3DDataFactory::Context& ctx,
                                                         boo::IShaderPipeline* pipeline, bool door)
{
    boo::IGraphicsBuffer* ubufs[] = { m_uniBuf, m_uniBuf, m_uniBuf };
    boo::PipelineStage ubufStages[] = { boo::PipelineStage::Vertex, boo::PipelineStage::Vertex,
                                        boo::PipelineStage::Fragment };
    size_t ubufOffs[] = {0, 0, 768};
    size_t ubufSizes[] = {768, 768, 256};
    size_t texCount = 0;
    boo::ITexture* texs[7] = {};
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
    return ctx.newShaderDataBinding(pipeline, s_vtxFmt, m_vbo, nullptr, nullptr, door ? 1 : 3,
                                    ubufs, ubufStages, ubufOffs, ubufSizes, texCount, texs, nullptr, nullptr);
}

}
