#include "shader_CFluidPlaneShader.hpp"

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
    "    float4 pos = float4(v.posIn.xyz, 1.0);\n"
    "    float4 normalIn = v.normalIn;\n"
    "    vtf.mvPos = mul(mv, pos);\n"
    "    vtf.pos = mul(proj, vtf.mvPos);\n"
    "    vtf.mvNorm = mul(mvNorm, v.normalIn);\n"
    "    vtf.mvBinorm = mul(mvNorm, v.binormalIn);\n"
    "    vtf.mvTangent = mul(mvNorm, v.tangentIn);\n"
    "    vtf.color = float4(v.colorIn.xyz, 1.0);\n"
    "    vtf.uvs[0] = mul(texMtxs[0], pos).xy;\n"
    "    vtf.uvs[1] = mul(texMtxs[1], pos).xy;\n"
    "    vtf.uvs[2] = mul(texMtxs[2], pos).xy;\n"
    "%s" // Additional TCGs here
    "    return vtf;\n"
    "}\n";

static const char* TessVS =
    "struct VertData\n"
    "{\n"
    "    float4 posIn : POSITION;\n"
    "    float4 outerLevelsIn : UV0;\n"
    "    float2 innerLevelsIn : UV1;\n"
    "};\n"
    "\n"
    "struct VertToControl\n"
    "{\n"
    "    float4 minMaxPos : POSITION;\n"
    "    float4 outerLevels : OUTERLEVELS;\n"
    "    float2 innerLevels : INNERLEVELS;\n"
    "};\n"
    "\n"
    "VertToControl main(in VertData v)\n"
    "{\n"
    "    VertToControl vtc;\n"
    "    vtc.minMaxPos = v.posIn;\n"
    "    vtc.outerLevels = v.outerLevelsIn;\n"
    "    vtc.innerLevels = v.innerLevelsIn;\n"
    "    return vtc;\n"
    "}\n";

static const char* TessCS =
    "struct VertToControl\n"
    "{\n"
    "    float4 minMaxPos : POSITION;\n"
    "    float4 outerLevels : OUTERLEVELS;\n"
    "    float2 innerLevels : INNERLEVELS;\n"
    "};\n"
    "\n"
    "struct ControlToEvaluation\n"
    "{\n"
    "    float4 minMaxPos : POSITION;\n"
    "    float outerLevels[4] : SV_TessFactor;\n"
    "    float innerLevels[2] : SV_InsideTessFactor;\n"
    "};\n"
    "\n"
    "struct ControlPoint\n"
    "{};\n"
    "\n"
    "ControlToEvaluation patchmain(InputPatch<VertToControl, 1> vtc, uint id : SV_PrimitiveID)\n"
    "{\n"
    "    ControlToEvaluation cte;\n"
    "    cte.minMaxPos = vtc[id].minMaxPos;\n"
    "    for (int i=0 ; i<4 ; ++i)\n"
    "        cte.outerLevels[i] = vtc[id].outerLevels[i];\n"
    "    for (int i=0 ; i<2 ; ++i)\n"
    "        cte.innerLevels[i] = vtc[id].innerLevels[i];\n"
    "    return cte;\n"
    "}\n"
    "\n"
    "[domain(\"quad\")]\n"
    "[partitioning(\"integer\")]\n"
    "[outputtopology(\"triangle_cw\")]\n"
    "[outputcontrolpoints(1)]\n"
    "[patchconstantfunc(\"patchmain\")]\n"
    "ControlPoint main(InputPatch<VertToControl, 1> vtc, uint i : SV_OutputControlPointID, uint id : SV_PrimitiveID)\n"
    "{\n"
    "    ControlPoint pt;\n"
    "    return pt;\n"
    "}\n";

static const char* TessES =
    "struct Ripple\n"
    "{\n"
    "    float4 center; // time, distFalloff\n"
    "    float4 params; // amplitude, lookupPhase, lookupTime\n"
    "};\n"
    "\n"
    "cbuffer FluidPlaneUniform : register(b0)\n"
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
    "    float4 pos : SV_Position;\n"
    "    float4 mvPos : POSITION;\n"
    "    float4 mvNorm : NORMAL;\n"
    "    float4 mvBinorm : BINORMAL;\n"
    "    float4 mvTangent : TANGENT;\n"
    "    float4 color : COLOR;\n"
    "    float2 uvs[7] : UV;\n"
    "};\n"
    "\n"
    "struct ControlToEvaluation\n"
    "{\n"
    "    float4 minMaxPos : POSITION;\n"
    "    float outerLevels[4] : SV_TessFactor;\n"
    "    float innerLevels[2] : SV_InsideTessFactor;\n"
    "};\n"
    "\n"
    "struct ControlPoint\n"
    "{};\n"
    "\n"
    "Texture2D RippleMap : register(t%d);\n"
    "SamplerState samp : register(s2);\n"
    "\n"
    "static const float PI_X2 = 6.283185307179586;\n"
    "\n"
    "static void ApplyRipple(in Ripple ripple, in float2 pos, inout float height)\n"
    "{\n"
    "    float dist = length(ripple.center.xy - pos);\n"
    "    float rippleV = RippleMap.SampleLevel(samp, float2(dist * ripple.center.w, ripple.center.z), 0.0).r;\n"
    "    height += rippleV * ripple.params.x * sin((dist * ripple.params.y + ripple.params.z) * PI_X2);\n"
    "}\n"
    "\n"
    "[domain(\"quad\")]\n"
    "VertToFrag main(in ControlToEvaluation cte, in float2 TessCoord : SV_DomainLocation,\n"
    "                const OutputPatch<ControlPoint, 1> bezpatch)\n"
    "{\n"
    "    float2 posIn = float2(lerp(cte.minMaxPos.x, cte.minMaxPos.z, TessCoord.x),\n"
    "                          lerp(cte.minMaxPos.y, cte.minMaxPos.w, TessCoord.y));\n"
    "    float height = 0.0;\n"
    "    float upHeight = 0.0;\n"
    "    float downHeight = 0.0;\n"
    "    float rightHeight = 0.0;\n"
    "    float leftHeight = 0.0;\n"
    "    for (int i=0 ; i<20 ; ++i)\n"
    "    {\n"
    "        ApplyRipple(ripples[i], posIn, height);\n"
    "        ApplyRipple(ripples[i], posIn + float2(0.0, rippleNormResolution), upHeight);\n"
    "        ApplyRipple(ripples[i], posIn - float2(0.0, rippleNormResolution), downHeight);\n"
    "        ApplyRipple(ripples[i], posIn + float2(rippleNormResolution, 0.0), rightHeight);\n"
    "        ApplyRipple(ripples[i], posIn - float2(rippleNormResolution, 0.0), leftHeight);\n"
    "    }\n"
    "    float4 normalIn = float4(normalize(float3((leftHeight - rightHeight),\n"
    "                                              (downHeight - upHeight),\n"
    "                                              rippleNormResolution)), 1.0);\n"
    "    float4 binormalIn = float4(normalIn.x, normalIn.z, -normalIn.y, 1.0);\n"
    "    float4 tangentIn = float4(normalIn.z, normalIn.y, -normalIn.x, 1.0);\n"
    "    float4 pos = float4(posIn, height, 1.0);\n"
    "    VertToFrag vtf;\n"
    "    vtf.mvPos = mul(mv, pos);\n"
    "    vtf.pos = mul(proj, vtf.mvPos);\n"
    "    vtf.mvNorm = mul(mvNorm, normalIn);\n"
    "    vtf.mvBinorm = mul(mvNorm, binormalIn);\n"
    "    vtf.mvTangent = mul(mvNorm, tangentIn);\n"
    "    vtf.color = max(height, 0.0) * colorMul;\n"
    "    vtf.color.a = 1.0;\n"
    "    vtf.uvs[0] = mul(texMtxs[0], pos).xy;\n"
    "    vtf.uvs[1] = mul(texMtxs[1], pos).xy;\n"
    "    vtf.uvs[2] = mul(texMtxs[2], pos).xy;\n"
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
"cbuffer LightingUniform : register(b2)\n"
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
"static float4 LightingFunc(float3 mvPosIn, float3 mvNormIn)\n"
"{\n"
"    float4 ret = ambient;\n"
"    \n"
"    for (int i=0 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        float3 delta = mvPosIn - lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        float angDot = clamp(dot(normalize(delta), lights[i].dir.xyz), 0.0, 1.0);\n"
"        float att = 1.0 / (lights[i].linAtt[2] * dist * dist +\n"
"                           lights[i].linAtt[1] * dist +\n"
"                           lights[i].linAtt[0]);\n"
"        float angAtt = lights[i].angAtt[2] * angDot * angDot +\n"
"                       lights[i].angAtt[1] * angDot +\n"
"                       lights[i].angAtt[0];\n"
"        ret += lights[i].color * angAtt * att * saturate(dot(normalize(-delta), mvNormIn));\n"
"    }\n"
"    \n"
"    return ret;\n"
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
"static float4 MainPostFunc(in VertToFrag vtf, float4 colorIn)\n"
"{\n"
"    float fogZ, temp;\n"
"    switch (fog.mode)\n"
"    {\n"
"    case 2:\n"
"        fogZ = (-vtf.mvPos.z - fog.start) * fog.rangeScale;\n"
"        break;\n"
"    case 4:\n"
"        fogZ = 1.0 - exp2(-8.0 * (-vtf.mvPos.z - fog.start) * fog.rangeScale);\n"
"        break;\n"
"    case 5:\n"
"        temp = (-vtf.mvPos.z - fog.start) * fog.rangeScale;\n"
"        fogZ = 1.0 - exp2(-8.0 * temp * temp);\n"
"        break;\n"
"    case 6:\n"
"        fogZ = exp2(-8.0 * (fog.start + vtf.mvPos.z) * fog.rangeScale);\n"
"        break;\n"
"    case 7:\n"
"        temp = (fog.start + vtf.mvPos.z) * fog.rangeScale;\n"
"        fogZ = exp2(-8.0 * temp * temp);\n"
"        break;\n"
"    default:\n"
"        fogZ = 0.0;\n"
"        break;\n"
"    }\n"
"#if %d\n"
"    return float4(lerp(colorIn, float4(0.0, 0.0, 0.0, 0.0), saturate(fogZ)).rgb, colorIn.a);\n"
"#else\n"
"    return float4(lerp(colorIn, fog.color, saturate(fogZ)).rgb, colorIn.a);\n"
"#endif\n"
"}\n"
"\n"
"SamplerState samp : register(s0);\n"
"%s" // Textures here
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float4 lighting = LightingFunc(vtf.mvPos.xyz, normalize(vtf.mvNorm.xyz));\n"
"    float4 colorOut;\n"
"%s" // Combiner expression here
"    return MainPostFunc(vtf, colorOut);\n"
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
"cbuffer LightingUniform : register(b2)\n"
"{\n"
"    Light lights[" _XSTR(URDE_MAX_LIGHTS) "];\n"
"    float4 ambient;\n"
"    float4 kColor0;\n"
"    float4 kColor1;\n"
"    float4 kColor2;\n"
"    float4 kColor3;\n"
"    Fog fog;\n"
"};\n"
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
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float4 colorOut;\n"
"%s" // Combiner expression here
"    return colorOut;\n"
"}\n";

static std::string _BuildFS(const SFluidPlaneShaderInfo& info) {
  std::string additionalTCGs;
  std::string textures;
  std::string combiner;
  int nextTex = 0;
  int nextTCG = 3;
  int nextMtx = 4;
  int bumpMapUv, envBumpMapUv, envMapUv, lightmapUv;

  if (info.m_hasPatternTex1)
    textures += hecl::Format("Texture2D patternTex1 : register(t%d);\n", nextTex++);
  if (info.m_hasPatternTex2)
    textures += hecl::Format("Texture2D patternTex2 : register(t%d);\n", nextTex++);
  if (info.m_hasColorTex)
    textures += hecl::Format("Texture2D colorTex : register(t%d);\n", nextTex++);
  if (info.m_hasBumpMap)
    textures += hecl::Format("Texture2D bumpMap : register(t%d);\n", nextTex++);
  if (info.m_hasEnvMap)
    textures += hecl::Format("Texture2D envMap : register(t%d);\n", nextTex++);
  if (info.m_hasEnvBumpMap)
    textures += hecl::Format("Texture2D envBumpMap : register(t%d);\n", nextTex++);
  if (info.m_hasLightmap)
    textures += hecl::Format("Texture2D lightMap : register(t%d);\n", nextTex++);

  if (info.m_hasBumpMap) {
    bumpMapUv = nextTCG;
    additionalTCGs += hecl::Format("    vtf.uvs[%d] = mul(texMtxs[0], pos).xy;\n", nextTCG++);
  }
  if (info.m_hasEnvBumpMap) {
    envBumpMapUv = nextTCG;
    additionalTCGs += hecl::Format("    vtf.uvs[%d] = mul(texMtxs[3], float4(normalIn.xyz, 1.0)).xy;\n", nextTCG++);
  }
  if (info.m_hasEnvMap) {
    envMapUv = nextTCG;
    additionalTCGs += hecl::Format("    vtf.uvs[%d] = mul(texMtxs[%d], pos).xy;\n", nextTCG++, nextMtx++);
  }
  if (info.m_hasLightmap) {
    lightmapUv = nextTCG;
    additionalTCGs += hecl::Format("    vtf.uvs[%d] = mul(texMtxs[%d], pos).xy;\n", nextTCG++, nextMtx++);
  }

  switch (info.m_type) {
  case EFluidType::NormalWater:
  case EFluidType::PhazonFluid:
  case EFluidType::Four:
    if (info.m_hasLightmap) {
      combiner += hecl::Format("    float4 lightMapTexel = lightMap.Sample(samp, vtf.uvs[%d]);\n", lightmapUv);
      // 0: Tex4TCG, Tex4, doubleLightmapBlend ? NULL : GX_COLOR1A1
      // ZERO, TEX, KONST, doubleLightmapBlend ? ZERO : RAS
      // Output reg 2
      // KColor 2
      if (info.m_doubleLightmapBlend) {
        // 1: Tex4TCG2, Tex4, GX_COLOR1A1
        // C2, TEX, KONST, RAS
        // Output reg 2
        // KColor 3
        // Tex * K2 + Lighting
        combiner += "    lighting += lerp(lightMapTexel * kColor2, lightMapTexel, kColor3);\n";
      } else {
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
    if (info.m_hasPatternTex2) {
      if (info.m_hasPatternTex1)
        combiner +=
            "    colorOut = (patternTex1.Sample(samp, vtf.uvs[0]) * kColor0 + lighting) *\n"
            "               patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
      else
        combiner += "    colorOut = lighting * patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
    } else {
      combiner += "    colorOut = vtf.color;\n";
    }

    if (info.m_hasColorTex && !info.m_hasEnvMap && info.m_hasEnvBumpMap) {
      // Make previous stage indirect, mtx0
      combiner += hecl::Format(
          "    float2 indUvs = (envBumpMap.Sample(samp, vtf.uvs[%d]).ra - float2(0.5, 0.5)) *\n"
          "        float2(fog.indScale, -fog.indScale);\n",
          envBumpMapUv);
      combiner += "    colorOut += colorTex.Sample(samp, indUvs + vtf.uvs[2]) * lighting;\n";
    } else if (info.m_hasEnvMap) {
      // Next: envTCG, envTex, NULL
      // PREV, TEX, KONST, ZERO
      // Output reg prev
      // KColor 1

      // Make previous stage indirect, mtx0
      if (info.m_hasColorTex)
        combiner += "    colorOut += colorTex.Sample(samp, vtf.uvs[2]) * lighting;\n";
      combiner += hecl::Format(
          "    float2 indUvs = (envBumpMap.Sample(samp, vtf.uvs[%d]).ra - float2(0.5, 0.5)) *\n"
          "        float2(fog.indScale, -fog.indScale);\n",
          envBumpMapUv);
      combiner += hecl::Format("    colorOut = lerp(colorOut, envMap.Sample(samp, indUvs + vtf.uvs[%d]), kColor1);\n",
                               envMapUv);
    } else if (info.m_hasColorTex) {
      combiner += "    colorOut += colorTex.Sample(samp, vtf.uvs[2]) * lighting;\n";
    }

    break;

  case EFluidType::PoisonWater:
    if (info.m_hasLightmap) {
      combiner += hecl::Format("    float4 lightMapTexel = lightMap.Sample(samp, vtf.uvs[%d]);\n", lightmapUv);
      // 0: Tex4TCG, Tex4, doubleLightmapBlend ? NULL : GX_COLOR1A1
      // ZERO, TEX, KONST, doubleLightmapBlend ? ZERO : RAS
      // Output reg 2
      // KColor 2
      if (info.m_doubleLightmapBlend) {
        // 1: Tex4TCG2, Tex4, GX_COLOR1A1
        // C2, TEX, KONST, RAS
        // Output reg 2
        // KColor 3
        // Tex * K2 + Lighting
        combiner += "    lighting += lerp(lightMapTexel * kColor2, lightMapTexel, kColor3);\n";
      } else {
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
    if (info.m_hasPatternTex2) {
      if (info.m_hasPatternTex1)
        combiner +=
            "    colorOut = (patternTex1.Sample(samp, vtf.uvs[0]) * kColor0 + lighting) *\n"
            "               patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
      else
        combiner += "    colorOut = lighting * patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
    } else {
      combiner += "    colorOut = vtf.color;\n";
    }

    if (info.m_hasColorTex) {
      if (info.m_hasEnvBumpMap) {
        // Make previous stage indirect, mtx0
        combiner += hecl::Format(
            "    float2 indUvs = (envBumpMap.Sample(samp, vtf.uvs[%d]).ra - float2(0.5, 0.5)) *\n"
            "        float2(fog.indScale, -fog.indScale);\n",
            envBumpMapUv);
        combiner += "    colorOut += colorTex.Sample(samp, indUvs + vtf.uvs[2]) * lighting;\n";
      } else {
        combiner += "    colorOut += colorTex.Sample(samp, vtf.uvs[2]) * lighting;\n";
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
    if (info.m_hasPatternTex2) {
      if (info.m_hasPatternTex1)
        combiner +=
            "    colorOut = (patternTex1.Sample(samp, vtf.uvs[0]) * kColor0 + vtf.color) *\n"
            "               patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
      else
        combiner += "    colorOut = vtf.color * patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
    } else {
      combiner += "    colorOut = vtf.color;\n";
    }

    if (info.m_hasColorTex)
      combiner += "    colorOut += colorTex.Sample(samp, vtf.uvs[2]);\n";

    if (info.m_hasBumpMap) {
      // 3: bumpMapTCG, bumpMap, NULL
      // ZERO, TEX, ONE, HALF
      // Output reg 0, no clamp, no bias

      // 4: bumpMapTCG2, bumpMap, NULL
      // ZERO, TEX, ONE, C0
      // Output reg 0, subtract, clamp, no bias

      combiner +=
          "    float3 lightVec = lights[3].pos.xyz - vtf.mvPos.xyz;\n"
          "    float lx = dot(vtf.mvTangent.xyz, lightVec);\n"
          "    float ly = dot(vtf.mvBinorm.xyz, lightVec);\n";
      combiner += hecl::Format(
          "    float4 emboss1 = bumpMap.Sample(samp, vtf.uvs[%d]) + float4(0.5);\n"
          "    float4 emboss2 = bumpMap.Sample(samp, vtf.uvs[%d] + float2(lx, ly));\n",
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
    if (info.m_hasPatternTex2) {
      if (info.m_hasPatternTex1)
        combiner +=
            "    colorOut = (patternTex1.Sample(samp, vtf.uvs[0]) * kColor0 + vtf.color) *\n"
            "               patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
      else
        combiner += "    colorOut = vtf.color * patternTex2.Sample(samp, vtf.uvs[1]) + vtf.color;\n";
    } else {
      combiner += "    colorOut = vtf.color;\n";
    }

    if (info.m_hasColorTex)
      combiner += "    colorOut += colorTex.Sample(samp, vtf.uvs[2]);\n";

    if (info.m_hasBumpMap) {
      // 3: bumpMapTCG, bumpMap, NULL
      // ZERO, TEX, PREV, ZERO
      // Output reg prev, scale 2
      combiner += hecl::Format("    float4 emboss1 = bumpMap.Sample(samp, vtf.uvs[%d]) + float4(0.5);\n", bumpMapUv);
      combiner += "colorOut *= emboss1 * float4(2.0);\n";
    }

    break;
  }

  combiner += "    colorOut.a = kColor0.a;\n";

  char* finalFS;
  asprintf(&finalFS, FS, int(info.m_additive), textures.c_str(), combiner.c_str());
  std::string ret(finalFS);
  free(finalFS);
  return ret;
}

static std::string _BuildAdditionalTCGs(const SFluidPlaneShaderInfo& info) {
  std::string additionalTCGs;
  int nextTCG = 3;
  int nextMtx = 4;

  if (info.m_hasBumpMap) {
    additionalTCGs += hecl::Format("    vtf.uvs[%d] = mul(texMtxs[0], pos).xy;\n", nextTCG++);
  }
  if (info.m_hasEnvBumpMap) {
    additionalTCGs += hecl::Format("    vtf.uvs[%d] = mul(texMtxs[3], float4(normalIn.xyz, 1.0)).xy;\n", nextTCG++);
  }
  if (info.m_hasEnvMap) {
    additionalTCGs += hecl::Format("    vtf.uvs[%d] = mul(texMtxs[%d], pos).xy;\n", nextTCG++, nextMtx++);
  }
  if (info.m_hasLightmap) {
    additionalTCGs += hecl::Format("    vtf.uvs[%d] = mul(texMtxs[%d], pos).xy;\n", nextTCG, nextMtx);
  }

  return additionalTCGs;
}

static std::string _BuildVS(const SFluidPlaneShaderInfo& info, bool tessellation) {
  if (tessellation)
    return TessVS;

  std::string additionalTCGs = _BuildAdditionalTCGs(info);

  char* finalVSs;
  asprintf(&finalVSs, VS, additionalTCGs.c_str());
  std::string ret(finalVSs);
  free(finalVSs);
  return ret;
}
template <>
std::string StageObject_CFluidPlaneShader<hecl::PlatformType::D3D11, hecl::PipelineStage::Vertex>::BuildShader(
    const SFluidPlaneShaderInfo& in, bool tessellation) {
  return _BuildVS(in, tessellation);
}

template <>
std::string StageObject_CFluidPlaneShader<hecl::PlatformType::D3D11, hecl::PipelineStage::Fragment>::BuildShader(
    const SFluidPlaneShaderInfo& in, bool tessellation) {
  return _BuildFS(in);
}

template <>
std::string StageObject_CFluidPlaneShader<hecl::PlatformType::D3D11, hecl::PipelineStage::Control>::BuildShader(
    const SFluidPlaneShaderInfo& in, bool tessellation) {
  return TessCS;
}

static std::string BuildES(const SFluidPlaneShaderInfo& info) {
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

  char* finalESs;
  asprintf(&finalESs, TessES, nextTex, additionalTCGs.c_str());
  std::string ret(finalESs);
  free(finalESs);

  return ret;
}
template <>
std::string StageObject_CFluidPlaneShader<hecl::PlatformType::D3D11, hecl::PipelineStage::Evaluation>::BuildShader(
    const SFluidPlaneShaderInfo& in, bool tessellation) {
  return BuildES(in);
}

static std::string _BuildVS(const SFluidPlaneDoorShaderInfo& info) {
  char* finalVSs;
  asprintf(&finalVSs, VS, "");
  std::string ret(finalVSs);
  free(finalVSs);
  return ret;
}

static std::string _BuildFS(const SFluidPlaneDoorShaderInfo& info) {
  int nextTex = 0;
  std::string textures;
  std::string combiner;

  if (info.m_hasPatternTex1) {
    textures += hecl::Format("Texture2D patternTex1 : register(t%d);\n", nextTex++);
  }
  if (info.m_hasPatternTex2) {
    textures += hecl::Format("Texture2D patternTex2 : register(t%d);\n", nextTex++);
  }
  if (info.m_hasColorTex) {
    textures += hecl::Format("Texture2D colorTex : register(t%d);\n", nextTex++);
  }

  // Tex0 * kColor0 * Tex1 + Tex2
  if (info.m_hasPatternTex1 && info.m_hasPatternTex2) {
    combiner +=
        "    colorOut = patternTex1.Sample(samp, vtf.uvs[0]) * kColor0 *\n"
        "               patternTex2.Sample(samp, vtf.uvs[1]);\n";
  } else {
    combiner += "    colorOut = float4(0.0);\n";
  }

  if (info.m_hasColorTex) {
    combiner += "    colorOut += colorTex.Sample(samp, vtf.uvs[2]);\n";
  }

  combiner += "    colorOut.a = kColor0.a;\n";

  char* finalFSs;
  asprintf(&finalFSs, FSDoor, textures.c_str(), combiner.c_str());
  std::string ret(finalFSs);
  free(finalFSs);
  return ret;
}

template <>
std::string StageObject_CFluidPlaneDoorShader<hecl::PlatformType::D3D11, hecl::PipelineStage::Vertex>::BuildShader(
    const SFluidPlaneDoorShaderInfo& in) {
  return _BuildVS(in);
}

template <>
std::string StageObject_CFluidPlaneDoorShader<hecl::PlatformType::D3D11, hecl::PipelineStage::Fragment>::BuildShader(
    const SFluidPlaneDoorShaderInfo& in) {
  return _BuildFS(in);
}
