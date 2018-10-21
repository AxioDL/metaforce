#include "shader_CFluidPlaneShader.hpp"

static const char* VS =
"struct VertData\n"
"{\n"
"    float4 posIn [[ attribute(0) ]];\n"
"    float4 normalIn [[ attribute(1) ]];\n"
"    float4 binormalIn [[ attribute(2) ]];\n"
"    float4 tangentIn [[ attribute(3) ]];\n"
"    float4 colorIn [[ attribute(4) ]];\n"
"};\n"
"\n"
"struct FluidPlaneUniform\n"
"{\n"
"    float4x4 mv;\n"
"    float4x4 mvNorm;\n"
"    float4x4 proj;\n"
"    float4x4 texMtxs[6];\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 mvPos;\n"
"    float4 mvNorm;\n"
"    float4 mvBinorm;\n"
"    float4 mvTangent;\n"
"    float4 color;\n"
"    float2 uv0;\n"
"    float2 uv1;\n"
"    float2 uv2;\n"
"    float2 uv3;\n"
"    float2 uv4;\n"
"    float2 uv5;\n"
"    float2 uv6;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]],\n"
"                        constant FluidPlaneUniform& fu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    float4 pos = float4(v.posIn.xyz, 1.0);\n"
"    float4 normalIn = v.normalIn;\n"
"    vtf.mvPos = fu.mv * pos;\n"
"    vtf.pos = fu.proj * vtf.mvPos;\n"
"    vtf.mvNorm = fu.mvNorm * v.normalIn;\n"
"    vtf.mvBinorm = fu.mvNorm * v.binormalIn;\n"
"    vtf.mvTangent = fu.mvNorm * v.tangentIn;\n"
"    vtf.color = v.colorIn;\n"
"    vtf.uv0 = (fu.texMtxs[0] * pos).xy;\n"
"    vtf.uv1 = (fu.texMtxs[1] * pos).xy;\n"
"    vtf.uv2 = (fu.texMtxs[2] * pos).xy;\n"
"%s" // Additional TCGs here
"    return vtf;\n"
"}\n";

static const char* TessCS =
"struct VertData\n"
"{\n"
"    float4 minMaxPos [[ attribute(0) ]];\n"
"    float4 outerLevelsIn [[ attribute(1) ]];\n"
"    float2 innerLevelsIn [[ attribute(2) ]];\n"
"};\n"
"\n"
"struct KernelPatchInfo {\n"
"    uint numPatches; // total number of patches to process.\n"
"                     // we need this because this value may\n"
"                     // not be a multiple of threadgroup size.\n"
"    ushort numPatchesInThreadGroup; // number of patches processed by a\n"
"                                    // thread-group\n"
"    ushort numControlPointsPerPatch;\n"
"};\n"
"\n"
"kernel void\n"
"cmain(VertData v [[ stage_in ]],\n"
"      constant KernelPatchInfo& patchInfo [[ buffer(2) ]],\n"
"      device MTLQuadTessellationFactorsHalf* tessellationFactorBuffer [[ buffer(3) ]],\n"
"      ushort lID [[ thread_position_in_threadgroup ]],\n"
"      ushort groupID [[ threadgroup_position_in_grid ]])\n"
"{\n"
"    uint patchGroupID = groupID * patchInfo.numPatchesInThreadGroup;\n"
"\n"
"    // execute the per-patch hull function\n"
"    if (lID < patchInfo.numPatchesInThreadGroup)\n"
"    {\n"
"        uint patchID = patchGroupID + lID;\n"
"        device MTLQuadTessellationFactorsHalf& patchOut = tessellationFactorBuffer[patchID];\n"
"        for (int i=0 ; i<4 ; ++i)\n"
"            patchOut.edgeTessellationFactor[i] = v.outerLevelsIn[i];\n"
"        for (int i=0 ; i<2 ; ++i)\n"
"            patchOut.insideTessellationFactor[i] = v.innerLevelsIn[i];\n"
"    }\n"
"}\n";

static const char* TessES =
"struct Ripple\n"
"{\n"
"    float4 center; // time, distFalloff\n"
"    float4 params; // amplitude, lookupPhase, lookupTime\n"
"};\n"
"\n"
"struct FluidPlaneUniform\n"
"{\n"
"    float4x4 mv;\n"
"    float4x4 mvNorm;\n"
"    float4x4 proj;\n"
"    float4x4 texMtxs[6];\n"
"    Ripple ripples[20];\n"
"    float4 colorMul;\n"
"    float rippleNormResolution;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 mvPos;\n"
"    float4 mvNorm;\n"
"    float4 mvBinorm;\n"
"    float4 mvTangent;\n"
"    float4 color;\n"
"    float2 uv0;\n"
"    float2 uv1;\n"
"    float2 uv2;\n"
"    float2 uv3;\n"
"    float2 uv4;\n"
"    float2 uv5;\n"
"    float2 uv6;\n"
"};\n"
"\n"
"struct VertData\n"
"{\n"
"    float4 minMaxPos [[ attribute(0) ]];\n"
"    float4 outerLevelsIn [[ attribute(1) ]];\n"
"    float2 innerLevelsIn [[ attribute(2) ]];\n"
"};\n"
"\n"
"#define PI_X2 6.283185307179586\n"
"\n"
"static void ApplyRipple(constant Ripple& ripple, float2 pos, thread float& height,\n"
"                        sampler samp, texture2d<float> RippleMap)\n"
"{\n"
"    float dist = length(ripple.center.xy - pos);\n"
"    float rippleV = RippleMap.sample(samp, float2(dist * ripple.center.w, ripple.center.z), level(0.0)).r;\n"
"    height += rippleV * ripple.params.x * sin((dist * ripple.params.y + ripple.params.z) * PI_X2);\n"
"}\n"
"\n"
"[[ patch(quad, 1) ]]\n"
"vertex VertToFrag emain(VertData v [[ stage_in ]], float2 TessCoord [[ position_in_patch ]],\n"
"                        constant FluidPlaneUniform& fu [[ buffer(2) ]],\n"
"                        sampler samp [[ sampler(2) ]],\n"
"                        texture2d<float> RippleMap [[ texture(%d) ]])\n"
"{\n"
"    float2 posIn = float2(mix(v.minMaxPos.x, v.minMaxPos.z, TessCoord.x),\n"
"                          mix(v.minMaxPos.y, v.minMaxPos.w, TessCoord.y));\n"
"    float height = 0.0;\n"
"    float upHeight = 0.0;\n"
"    float downHeight = 0.0;\n"
"    float rightHeight = 0.0;\n"
"    float leftHeight = 0.0;\n"
"    for (int i=0 ; i<20 ; ++i)\n"
"    {\n"
"        ApplyRipple(fu.ripples[i], posIn, height, samp, RippleMap);\n"
"        ApplyRipple(fu.ripples[i], posIn + float2(0.0, fu.rippleNormResolution), upHeight, samp, RippleMap);\n"
"        ApplyRipple(fu.ripples[i], posIn - float2(0.0, fu.rippleNormResolution), downHeight, samp, RippleMap);\n"
"        ApplyRipple(fu.ripples[i], posIn + float2(fu.rippleNormResolution, 0.0), rightHeight, samp, RippleMap);\n"
"        ApplyRipple(fu.ripples[i], posIn - float2(fu.rippleNormResolution, 0.0), leftHeight, samp, RippleMap);\n"
"    }\n"
"    float4 normalIn = float4(normalize(float3((leftHeight - rightHeight),\n"
"                                              (downHeight - upHeight),\n"
"                                              fu.rippleNormResolution)), 1.0);\n"
"    float4 binormalIn = float4(normalIn.x, normalIn.z, -normalIn.y, 1.0);\n"
"    float4 tangentIn = float4(normalIn.z, normalIn.y, -normalIn.x, 1.0);\n"
"    float4 pos = float4(posIn, height, 1.0);\n"
"    VertToFrag vtf;\n"
"    vtf.mvPos = (fu.mv * pos);\n"
"    vtf.pos = (fu.proj * vtf.mvPos);\n"
"    vtf.mvNorm = (fu.mvNorm * normalIn);\n"
"    vtf.mvBinorm = (fu.mvNorm * binormalIn);\n"
"    vtf.mvTangent = (fu.mvNorm * tangentIn);\n"
"    vtf.color = max(height, 0.0) * fu.colorMul;\n"
"    vtf.color.a = 1.0;\n"
"    vtf.uv0 = (fu.texMtxs[0] * pos).xy;\n"
"    vtf.uv1 = (fu.texMtxs[1] * pos).xy;\n"
"    vtf.uv2 = (fu.texMtxs[2] * pos).xy;\n"
"%s\n" // Additional TCGs here
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
"    int mode;\n"
"    float4 color;\n"
"    float rangeScale;\n"
"    float start;\n"
"    float indScale;\n"
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
"static float4 LightingFunc(constant LightingUniform& lu, float3 mvPosIn, float3 mvNormIn)\n"
"{\n"
"    float4 ret = lu.ambient;\n"
"    \n"
"    for (int i=0 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        float3 delta = mvPosIn - lu.lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        float angDot = clamp(dot(normalize(delta), lu.lights[i].dir.xyz), 0.0, 1.0);\n"
"        float att = 1.0 / (lu.lights[i].linAtt[2] * dist * dist +\n"
"                           lu.lights[i].linAtt[1] * dist +\n"
"                           lu.lights[i].linAtt[0]);\n"
"        float angAtt = lu.lights[i].angAtt[2] * angDot * angDot +\n"
"                       lu.lights[i].angAtt[1] * angDot +\n"
"                       lu.lights[i].angAtt[0];\n"
"        ret += lu.lights[i].color * clamp(angAtt, 0.0, 1.0) * att * clamp(dot(normalize(-delta), mvNormIn), 0.0, 1.0);\n"
"    }\n"
"    \n"
"    return ret;\n"
"}\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 mvPos;\n"
"    float4 mvNorm;\n"
"    float4 mvBinorm;\n"
"    float4 mvTangent;\n"
"    float4 color;\n"
"    float2 uv0;\n"
"    float2 uv1;\n"
"    float2 uv2;\n"
"    float2 uv3;\n"
"    float2 uv4;\n"
"    float2 uv5;\n"
"    float2 uv6;\n"
"};\n"
"\n"
"float4 MainPostFunc(thread VertToFrag& vtf, constant LightingUniform& lu, float4 colorIn)\n"
"{\n"
"    float fogZ, temp;\n"
"    switch (lu.fog.mode)\n"
"    {\n"
"    case 2:\n"
"        fogZ = (-vtf.mvPos.z - lu.fog.start) * lu.fog.rangeScale;\n"
"        break;\n"
"    case 4:\n"
"        fogZ = 1.0 - exp2(-8.0 * (-vtf.mvPos.z - lu.fog.start) * lu.fog.rangeScale);\n"
"        break;\n"
"    case 5:\n"
"        temp = (-vtf.mvPos.z - lu.fog.start) * lu.fog.rangeScale;\n"
"        fogZ = 1.0 - exp2(-8.0 * temp * temp);\n"
"        break;\n"
"    case 6:\n"
"        fogZ = exp2(-8.0 * (lu.fog.start + vtf.mvPos.z) * lu.fog.rangeScale);\n"
"        break;\n"
"    case 7:\n"
"        temp = (lu.fog.start + vtf.mvPos.z) * lu.fog.rangeScale;\n"
"        fogZ = exp2(-8.0 * temp * temp);\n"
"        break;\n"
"    default:\n"
"        fogZ = 0.0;\n"
"        break;\n"
"    }\n"
"#if %d\n"
"    return float4(mix(colorIn, float4(0.0), saturate(fogZ)).rgb, colorIn.a);\n"
"#else\n"
"    return float4(mix(colorIn, lu.fog.color, saturate(fogZ)).rgb, colorIn.a);\n"
"#endif\n"
"}\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      sampler samp [[ sampler(0) ]],\n"
"                      constant LightingUniform& lu [[ buffer(4) ]]%s)\n" // Textures here
"{\n"
"    float4 lighting = LightingFunc(lu, vtf.mvPos.xyz, normalize(vtf.mvNorm.xyz));\n"
"    float4 colorOut;\n"
"%s" // Combiner expression here
"    return MainPostFunc(vtf, lu, colorOut);\n"
"}\n";

static const char* FSDoor =
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
"    int mode;\n"
"    float4 color;\n"
"    float rangeScale;\n"
"    float start;\n"
"    float indScale;\n"
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
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 mvPos;\n"
"    float4 mvNorm;\n"
"    float4 mvBinorm;\n"
"    float4 mvTangent;\n"
"    float4 color;\n"
"    float2 uv0;\n"
"    float2 uv1;\n"
"    float2 uv2;\n"
"    float2 uv3;\n"
"    float2 uv4;\n"
"    float2 uv5;\n"
"    float2 uv6;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      sampler samp [[ sampler(0) ]],\n"
"                      constant LightingUniform& lu [[ buffer(4) ]]%s)\n" // Textures here
"{\n"
"    float4 colorOut;\n"
"%s" // Combiner expression here
"    return colorOut;\n"
"}\n";

static std::string _BuildFS(const SFluidPlaneShaderInfo& info)
{
    std::string textures;
    std::string combiner;
    int nextTex = 0;
    int nextTCG = 3;
    int nextMtx = 4;
    int bumpMapUv, envBumpMapUv, envMapUv, lightmapUv;

    if (info.m_hasPatternTex1)
        textures += hecl::Format(",\ntexture2d<float> patternTex1 [[ texture(%d) ]]", nextTex++);
    if (info.m_hasPatternTex2)
        textures += hecl::Format(",\ntexture2d<float> patternTex2 [[ texture(%d) ]]", nextTex++);
    if (info.m_hasColorTex)
        textures += hecl::Format(",\ntexture2d<float> colorTex [[ texture(%d) ]]", nextTex++);
    if (info.m_hasBumpMap)
        textures += hecl::Format(",\ntexture2d<float> bumpMap [[ texture(%d) ]]", nextTex++);
    if (info.m_hasEnvMap)
        textures += hecl::Format(",\ntexture2d<float> envMap [[ texture(%d) ]]", nextTex++);
    if (info.m_hasEnvBumpMap)
        textures += hecl::Format(",\ntexture2d<float> envBumpMap [[ texture(%d) ]]", nextTex++);
    if (info.m_hasLightmap)
        textures += hecl::Format(",\ntexture2d<float> lightMap [[ texture(%d) ]]", nextTex++);

    if (info.m_hasBumpMap)
    {
        bumpMapUv = nextTCG;
    }
    if (info.m_hasEnvBumpMap)
    {
        envBumpMapUv = nextTCG;
    }
    if (info.m_hasEnvMap)
    {
        envMapUv = nextTCG;
    }
    if (info.m_hasLightmap)
    {
        lightmapUv = nextTCG;
    }

    switch (info.m_type)
    {
    case EFluidType::NormalWater:
    case EFluidType::PhazonFluid:
    case EFluidType::Four:
        if (info.m_hasLightmap)
        {
            combiner += hecl::Format("    float4 lightMapTexel = lightMap.sample(samp, vtf.uv%d);\n", lightmapUv);
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
                combiner += "    lighting += mix(lightMapTexel * lu.kColor2, lightMapTexel, lu.kColor3);\n";
            }
            else
            {
                // mix(Tex * K2, Tex, K3) + Lighting
                combiner += "    lighting += lightMapTexel * lu.kColor2;\n";
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
                combiner += "    colorOut = (patternTex1.sample(samp, vtf.uv0) * lu.kColor0 + lighting) *\n"
                            "               patternTex2.sample(samp, vtf.uv1) + vtf.color;\n";
            else
                combiner += "    colorOut = lighting * patternTex2.sample(samp, vtf.uv1) + vtf.color;\n";
        }
        else
        {
            combiner += "    colorOut = vtf.color;\n";
        }


        if (info.m_hasColorTex && !info.m_hasEnvMap && info.m_hasEnvBumpMap)
        {
            // Make previous stage indirect, mtx0
            combiner += hecl::Format("    float2 indUvs = (envBumpMap.sample(samp, vtf.uv%d).ra - float2(0.5, 0.5)) *\n"
                                     "        float2(lu.fog.indScale, -lu.fog.indScale);\n", envBumpMapUv);
            combiner += "    colorOut += colorTex.sample(samp, indUvs + vtf.uv2) * lighting;\n";
        }
        else if (info.m_hasEnvMap)
        {
            // Next: envTCG, envTex, NULL
            // PREV, TEX, KONST, ZERO
            // Output reg prev
            // KColor 1

            // Make previous stage indirect, mtx0
            if (info.m_hasColorTex)
                combiner += "    colorOut += colorTex.sample(samp, vtf.uv2) * lighting;\n";
            combiner += hecl::Format("    float2 indUvs = (envBumpMap.sample(samp, vtf.uv%d).ra - float2(0.5, 0.5)) *\n"
                                     "        float2(lu.fog.indScale, -lu.fog.indScale);\n", envBumpMapUv);
            combiner += hecl::Format("    colorOut = mix(colorOut, envMap.sample(samp, indUvs + vtf.uv%d), lu.kColor1);\n",
                                     envMapUv);
        }
        else if (info.m_hasColorTex)
        {
            combiner += "    colorOut += colorTex.sample(samp, vtf.uv2) * lighting;\n";
        }

        break;

    case EFluidType::PoisonWater:
        if (info.m_hasLightmap)
        {
            combiner += hecl::Format("    float4 lightMapTexel = lightMap.sample(samp, vtf.uv%d);\n", lightmapUv);
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
                combiner += "    lighting += mix(lightMapTexel * lu.kColor2, lightMapTexel, lu.kColor3);\n";
            }
            else
            {
                // mix(Tex * K2, Tex, K3) + Lighting
                combiner += "    lighting += lightMapTexel * lu.kColor2;\n";
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
                combiner += "    colorOut = (patternTex1.sample(samp, vtf.uv0) * lu.kColor0 + lighting) *\n"
                            "               patternTex2.sample(samp, vtf.uv1) + vtf.color;\n";
            else
                combiner += "    colorOut = lighting * patternTex2.sample(samp, vtf.uv1) + vtf.color;\n";
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
                combiner += hecl::Format("    float2 indUvs = (envBumpMap.sample(samp, vtf.uv%d).ra - float2(0.5, 0.5)) *\n"
                                         "        float2(lu.fog.indScale, -lu.fog.indScale);\n", envBumpMapUv);
                combiner += "    colorOut += colorTex.sample(samp, indUvs + vtf.uv2) * lighting;\n";
            }
            else
            {
                combiner += "    colorOut += colorTex.sample(samp, vtf.uv2) * lighting;\n";
            }
        }

        break;

    case EFluidType::Lava:
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
                combiner += "    colorOut = (patternTex1.sample(samp, vtf.uv0) * lu.kColor0 + vtf.color) *\n"
                            "               patternTex2.sample(samp, vtf.uv1) + vtf.color;\n";
            else
                combiner += "    colorOut = vtf.color * patternTex2.sample(samp, vtf.uv1) + vtf.color;\n";
        }
        else
        {
            combiner += "    colorOut = vtf.color;\n";
        }

        if (info.m_hasColorTex)
            combiner += "    colorOut += colorTex.sample(samp, vtf.uv2);\n";

        if (info.m_hasBumpMap)
        {
            // 3: bumpMapTCG, bumpMap, NULL
            // ZERO, TEX, ONE, HALF
            // Output reg 0, no clamp, no bias

            // 4: bumpMapTCG2, bumpMap, NULL
            // ZERO, TEX, ONE, C0
            // Output reg 0, subtract, clamp, no bias

            combiner += "    float3 lightVec = lu.lights[3].pos.xyz - vtf.mvPos.xyz;\n"
                        "    float lx = dot(vtf.mvTangent.xyz, lightVec);\n"
                        "    float ly = dot(vtf.mvBinorm.xyz, lightVec);\n";
            combiner += hecl::Format("    float4 emboss1 = bumpMap.sample(samp, vtf.uv%d) + float4(0.5);\n"
                                     "    float4 emboss2 = bumpMap.sample(samp, vtf.uv%d + float2(lx, ly));\n",
                                     bumpMapUv, bumpMapUv);

            // 5: NULL, NULL, NULL
            // ZERO, PREV, C0, ZERO
            // Output reg prev, scale 2, clamp

            // colorOut * clamp(emboss1 + 0.5 - emboss2, 0.0, 1.0) * 2.0
            combiner += "colorOut *= clamp((emboss1 + float4(0.5) - emboss2) * float4(2.0), float4(0.0), float4(1.0));\n";
        }

        break;

    case EFluidType::ThickLava:
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
                combiner += "    colorOut = (patternTex1.sample(samp, vtf.uv0) * lu.kColor0 + vtf.color) *\n"
                            "               patternTex2.sample(samp, vtf.uv1) + vtf.color;\n";
            else
                combiner += "    colorOut = vtf.color * patternTex2.sample(samp, vtf.uv1) + vtf.color;\n";
        }
        else
        {
            combiner += "    colorOut = vtf.color;\n";
        }

        if (info.m_hasColorTex)
            combiner += "    colorOut += colorTex.sample(samp, vtf.uv2);\n";

        if (info.m_hasBumpMap)
        {
            // 3: bumpMapTCG, bumpMap, NULL
            // ZERO, TEX, PREV, ZERO
            // Output reg prev, scale 2
            combiner += hecl::Format("    float4 emboss1 = bumpMap.sample(samp, vtf.uv%d) + float4(0.5);\n", bumpMapUv);
            combiner += "colorOut *= emboss1 * float4(2.0);\n";
        }

        break;
    }

    combiner += "    colorOut.a = lu.kColor0.a;\n";

    char *finalFS;
    asprintf(&finalFS, FS, int(info.m_additive), textures.c_str(), combiner.c_str());
    std::string ret(finalFS);
    free(finalFS);
    return ret;
}

static std::string _BuildAdditionalTCGs(const SFluidPlaneShaderInfo& info)
{
    std::string additionalTCGs;
    int nextTCG = 3;
    int nextMtx = 4;

    if (info.m_hasBumpMap)
    {
        additionalTCGs += hecl::Format("    vtf.uv%d = (fu.texMtxs[0] * pos).xy;\n", nextTCG++);
    }
    if (info.m_hasEnvBumpMap)
    {
        additionalTCGs += hecl::Format("    vtf.uv%d = (fu.texMtxs[3] * float4(normalIn.xyz, 1.0)).xy;\n", nextTCG++);
    }
    if (info.m_hasEnvMap)
    {
        additionalTCGs += hecl::Format("    vtf.uv%d = (fu.texMtxs[%d] * pos).xy;\n", nextTCG++, nextMtx++);
    }
    if (info.m_hasLightmap)
    {
        additionalTCGs += hecl::Format("    vtf.uv%d = (fu.texMtxs[%d] * pos).xy;\n", nextTCG++, nextMtx++);
    }

    return additionalTCGs;
}

static std::string _BuildVS(const SFluidPlaneShaderInfo& info, bool tessellation)
{
    std::string additionalTCGs = _BuildAdditionalTCGs(info);

    char *finalVSs;
    asprintf(&finalVSs, VS, additionalTCGs.c_str());
    std::string ret(finalVSs);
    free(finalVSs);
    return ret;
}
template <>
std::string StageObject_CFluidPlaneShader<hecl::PlatformType::Metal, hecl::PipelineStage::Vertex>::BuildShader
    (const SFluidPlaneShaderInfo& in, bool tessellation)
{
    return _BuildVS(in, tessellation);
}

template <>
std::string StageObject_CFluidPlaneShader<hecl::PlatformType::Metal, hecl::PipelineStage::Fragment>::BuildShader
    (const SFluidPlaneShaderInfo& in, bool tessellation)
{
    return _BuildFS(in);
}

template <>
std::string StageObject_CFluidPlaneShader<hecl::PlatformType::Metal, hecl::PipelineStage::Control>::BuildShader
    (const SFluidPlaneShaderInfo& in, bool tessellation)
{
    return TessCS;
}

static std::string BuildES(const SFluidPlaneShaderInfo& info)
{
    int nextTex = 0;
    if (info.m_hasPatternTex1)
        nextTex++;
    if (info.m_hasPatternTex2)
        nextTex++;
    if (info.m_hasColorTex)
        nextTex++;
    if (info.m_hasBumpMap)
        nextTex++;
    if (info.m_hasEnvMap)
        nextTex++;
    if (info.m_hasEnvBumpMap)
        nextTex++;
    if (info.m_hasLightmap)
        nextTex++;

    std::string additionalTCGs = _BuildAdditionalTCGs(info);

    char *finalESs;
    asprintf(&finalESs, TessES, nextTex, additionalTCGs.c_str());
    std::string ret(finalESs);
    free(finalESs);

    return ret;
}

template <>
std::string StageObject_CFluidPlaneShader<hecl::PlatformType::Metal, hecl::PipelineStage::Evaluation>::BuildShader
    (const SFluidPlaneShaderInfo& in, bool tessellation)
{
    return BuildES(in);
}

static std::string _BuildVS(const SFluidPlaneDoorShaderInfo& info)
{
    char *finalVSs;
    asprintf(&finalVSs, VS, "");
    std::string ret(finalVSs);
    free(finalVSs);
    return ret;
}

static std::string _BuildFS(const SFluidPlaneDoorShaderInfo& info)
{
    int nextTex = 0;
    std::string textures;
    std::string combiner;

    if (info.m_hasPatternTex1)
        textures += hecl::Format(",\ntexture2d<float> patternTex1 [[ texture(%d) ]]", nextTex++);
    if (info.m_hasPatternTex2)
        textures += hecl::Format(",\ntexture2d<float> patternTex2 [[ texture(%d) ]]", nextTex++);
    if (info.m_hasColorTex)
        textures += hecl::Format(",\ntexture2d<float> colorTex [[ texture(%d) ]]", nextTex++);

    // Tex0 * kColor0 * Tex1 + Tex2
    if (info.m_hasPatternTex1 && info.m_hasPatternTex2)
    {
        combiner += "    colorOut = patternTex1.sample(samp, vtf.uv0) * lu.kColor0 *\n"
                    "               patternTex2.sample(samp, vtf.uv1);\n";
    }
    else
    {
        combiner += "    colorOut = float4(0.0);\n";
    }

    if (info.m_hasColorTex)
    {
        combiner += "    colorOut += colorTex.sample(samp, vtf.uv2);\n";
    }

    combiner += "    colorOut.a = lu.kColor0.a;\n";

    char *finalFSs;
    asprintf(&finalFSs, FSDoor, textures.c_str(), combiner.c_str());
    std::string ret(finalFSs);
    free(finalFSs);
    return ret;
}

template <>
std::string StageObject_CFluidPlaneDoorShader<hecl::PlatformType::Metal, hecl::PipelineStage::Vertex>::BuildShader
    (const SFluidPlaneDoorShaderInfo& in)
{
    return _BuildVS(in);
}

template <>
std::string StageObject_CFluidPlaneDoorShader<hecl::PlatformType::Metal, hecl::PipelineStage::Fragment>::BuildShader
    (const SFluidPlaneDoorShaderInfo& in)
{
    return _BuildFS(in);
}
