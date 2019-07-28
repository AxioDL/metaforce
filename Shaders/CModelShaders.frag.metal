constant float3 kRGBToYPrime = float3(0.257, 0.504, 0.098);

/*
#if defined(ALPHA_%s)
#define SampleTexture_%s() %s.sample(samp, vtf.%sUv).aaa
#define SampleTextureAlpha_%s() %s.sample(samp, vtf.%sUv).a
#else
#define SampleTexture_%s() %s.sample(samp, vtf.%sUv).rgb
#define SampleTextureAlpha_%s() dot(%s.sample(samp, vtf.%sUv).rgb, kRGBToYPrime)
#endif
 */

#if defined(ALPHA_lightmap)
#define SampleTexture_lightmap() lightmap.sample(samp, vtf.lightmapUv).aaa
#define SampleTextureAlpha_lightmap() lightmap.sample(samp, vtf.lightmapUv).a
#else
#define SampleTexture_lightmap() lightmap.sample(samp, vtf.lightmapUv).rgb
#define SampleTextureAlpha_lightmap() dot(lightmap.sample(samp, vtf.lightmapUv).rgb, kRGBToYPrime)
#endif

#if defined(ALPHA_diffuse)
#define SampleTexture_diffuse() diffuse.sample(samp, vtf.diffuseUv).aaa
#define SampleTextureAlpha_diffuse() diffuse.sample(samp, vtf.diffuseUv).a
#else
#define SampleTexture_diffuse() diffuse.sample(samp, vtf.diffuseUv).rgb
#define SampleTextureAlpha_diffuse() dot(diffuse.sample(samp, vtf.diffuseUv).rgb, kRGBToYPrime)
#endif

#if defined(ALPHA_emissive)
#define SampleTexture_emissive() emissive.sample(samp, vtf.emissiveUv).aaa
#define SampleTextureAlpha_emissive() emissive.sample(samp, vtf.emissiveUv).a
#else
#define SampleTexture_emissive() emissive.sample(samp, vtf.emissiveUv).rgb
#define SampleTextureAlpha_emissive() dot(emissive.sample(samp, vtf.emissiveUv).rgb, kRGBToYPrime)
#endif

#if defined(ALPHA_specular)
#define SampleTexture_specular() specular.sample(samp, vtf.specularUv).aaa
#define SampleTextureAlpha_specular() specular.sample(samp, vtf.specularUv).a
#else
#define SampleTexture_specular() specular.sample(samp, vtf.specularUv).rgb
#define SampleTextureAlpha_specular() dot(specular.sample(samp, vtf.specularUv).rgb, kRGBToYPrime)
#endif

#if defined(ALPHA_extendedSpecular)
#define SampleTexture_extendedSpecular() extendedSpecular.sample(samp, vtf.extendedSpecularUv).aaa
#define SampleTextureAlpha_extendedSpecular() extendedSpecular.sample(samp, vtf.extendedSpecularUv).a
#else
#define SampleTexture_extendedSpecular() extendedSpecular.sample(samp, vtf.extendedSpecularUv).rgb
#define SampleTextureAlpha_extendedSpecular() dot(extendedSpecular.sample(samp, vtf.extendedSpecularUv).rgb, kRGBToYPrime)
#endif

#if defined(ALPHA_reflection)
#define SampleTexture_reflection() reflection.sample(samp, vtf.reflectionUv).aaa
#define SampleTextureAlpha_reflection() reflection.sample(samp, vtf.reflectionUv).a
#else
#define SampleTexture_reflection() reflection.sample(samp, vtf.reflectionUv).rgb
#define SampleTextureAlpha_reflection() dot(reflection.sample(samp, vtf.reflectionUv).rgb, kRGBToYPrime)
#endif

#if defined(ALPHA_alpha)
#define SampleTexture_alpha() alpha.sample(samp, vtf.alphaUv).aaa
#define SampleTextureAlpha_alpha() alpha.sample(samp, vtf.alphaUv).a
#else
#define SampleTexture_alpha() alpha.sample(samp, vtf.alphaUv).rgb
#define SampleTextureAlpha_alpha() dot(alpha.sample(samp, vtf.alphaUv).rgb, kRGBToYPrime)
#endif

#if defined(URDE_LIGHTING) || defined(URDE_LIGHTING_SHADOW) || defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW) || defined(URDE_DISINTEGRATE)
struct Fog {
  float4 color;
  float A;
  float B;
  float C;
  int mode;
};
#endif

#if defined(URDE_LIGHTING) || defined(URDE_LIGHTING_SHADOW) || defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
struct Light {
  float4 pos;
  float4 dir;
  float4 color;
  float4 linAtt;
  float4 angAtt;
};

struct LightingUniform {
  Light lights[URDE_MAX_LIGHTS];
  float4 ambient;
  float4 colorReg0;
  float4 colorReg1;
  float4 colorReg2;
  float4 mulColor;
  float4 addColor;
  Fog fog;
};
#define colorReg0 lu.colorReg0
#define colorReg1 lu.colorReg1
#define colorReg2 lu.colorReg2
#else
constant float4 colorReg0 = float4(1.0, 1.0, 1.0, 1.0);
constant float4 colorReg1 = float4(1.0, 1.0, 1.0, 1.0);
constant float4 colorReg2 = float4(1.0, 1.0, 1.0, 1.0);
#endif

#if defined(URDE_LIGHTING) || defined(URDE_LIGHTING_CUBE_REFLECTION)
float3 LightingFunc(thread VertToFrag& vtf, constant LightingUniform& lu, texture2d<float> extTex0, sampler clampSamp) {
  float4 ret = lu.ambient;

  for (int i = 0; i < URDE_MAX_LIGHTS; ++i) {
    float3 delta = vtf.mvPos.xyz - lu.lights[i].pos.xyz;
    float dist = length(delta);
    float3 deltaNorm = delta / dist;
    float angDot = max(dot(deltaNorm, lu.lights[i].dir.xyz), 0.0);
    float att = 1.0 / (lu.lights[i].linAtt[2] * dist * dist +
                       lu.lights[i].linAtt[1] * dist +
                       lu.lights[i].linAtt[0]);
    float angAtt = lu.lights[i].angAtt[2] * angDot * angDot +
                   lu.lights[i].angAtt[1] * angDot +
                   lu.lights[i].angAtt[0];
    ret += lu.lights[i].color * angAtt * att * max(dot(-deltaNorm, vtf.mvNorm.xyz), 0.0);
  }

  return saturate(ret.rgb);
}
#endif

#if defined(URDE_THERMAL_HOT)
struct LightingUniform {
  float4 tmulColor;
  float4 taddColor;
};
float3 LightingFunc(thread VertToFrag& vtf, constant LightingUniform& lu, texture2d<float> extTex0, sampler clampSamp) {
  return float3(1.0, 1.0, 1.0);
}
#endif

#if defined(URDE_THERMAL_COLD)
struct LightingUniform {};
float3 LightingFunc(thread VertToFrag& vtf, constant LightingUniform& lu, texture2d<float> extTex0, sampler clampSamp) {
  return float3(1.0, 1.0, 1.0);
}
#endif

#if defined(URDE_SOLID)
struct LightingUniform {
  float4 solidColor;
};
float3 LightingFunc(thread VertToFrag& vtf, constant LightingUniform& lu, texture2d<float> extTex0, sampler clampSamp) {
  return float3(1.0, 1.0, 1.0);
}
#endif

#if defined(URDE_MB_SHADOW)
struct LightingUniform {
  float4 shadowUp;
  float shadowId;
};
float3 LightingFunc(thread VertToFrag& vtf, constant LightingUniform& lu, texture2d<float> extTex0, sampler clampSamp) {
  return float3(1.0, 1.0, 1.0);
}
#endif

#if defined(URDE_LIGHTING_SHADOW) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
float3 LightingFunc(thread VertToFrag& vtf, constant LightingUniform& lu, texture2d<float> extTex0, sampler clampSamp) {
  float2 shadowUV = vtf.extUvs0;
  shadowUV.y = 1.0 - shadowUV.y;

  float4 ret = lu.ambient;

  float3 delta = vtf.mvPos.xyz - lu.lights[0].pos.xyz;
  float dist = length(delta);
  float3 deltaNorm = delta / dist;
  float angDot = max(dot(deltaNorm, lu.lights[0].dir.xyz), 0.0);
  float att = 1.0 / (lu.lights[0].linAtt[2] * dist * dist +
                     lu.lights[0].linAtt[1] * dist +
                     lu.lights[0].linAtt[0]);
  float angAtt = lu.lights[0].angAtt[2] * angDot * angDot +
                 lu.lights[0].angAtt[1] * angDot +
                 lu.lights[0].angAtt[0];
  ret += lu.lights[0].color * angAtt * att * max(dot(-deltaNorm, vtf.mvNorm.xyz), 0.0) *
         extTex0.sample(clampSamp, shadowUV).r;

  for (int i = 1; i < URDE_MAX_LIGHTS; ++i) {
    float3 delta = vtf.mvPos.xyz - lu.lights[i].pos.xyz;
    float dist = length(delta);
    float3 deltaNorm = delta / dist;
    float angDot = max(dot(deltaNorm, lu.lights[i].dir.xyz), 0.0);
    float att = 1.0 / (lu.lights[i].linAtt[2] * dist * dist +
                       lu.lights[i].linAtt[1] * dist +
                       lu.lights[i].linAtt[0]);
    float angAtt = lu.lights[i].angAtt[2] * angDot * angDot +
                   lu.lights[i].angAtt[1] * angDot +
                   lu.lights[i].angAtt[0];
    ret += lu.lights[i].color * angAtt * att * max(dot(-deltaNorm, vtf.mvNorm.xyz), 0.0);
  }

  return saturate(ret.rgb);
}
#endif

#if defined(URDE_DISINTEGRATE)
struct LightingUniform {
  float4 daddColor;
  Fog fog;
};
float3 LightingFunc(thread VertToFrag& vtf, constant LightingUniform& lu, texture2d<float> extTex0, sampler clampSamp) {
  return float3(1.0, 1.0, 1.0);
}
#endif

#if defined(URDE_LIGHTING) || defined(URDE_LIGHTING_SHADOW) || defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW) || defined(URDE_DISINTEGRATE)
float4 FogFunc(thread VertToFrag& vtf, constant LightingUniform& lu, float4 colorIn) {
  float fogZ;
  float fogF = saturate((lu.fog.A / (lu.fog.B - (1.0 - vtf.mvpPos.z))) - lu.fog.C);
  switch (lu.fog.mode) {
  case 2:
    fogZ = fogF;
    break;
  case 4:
    fogZ = 1.0 - exp2(-8.0 * fogF);
    break;
  case 5:
    fogZ = 1.0 - exp2(-8.0 * fogF * fogF);
    break;
  case 6:
    fogZ = exp2(-8.0 * (1.0 - fogF));
    break;
  case 7:
    fogF = 1.0 - fogF;
    fogZ = exp2(-8.0 * fogF * fogF);
    break;
  default:
    fogZ = 0.0;
    break;
  }
#ifdef BLEND_DST_ONE
  return float4(mix(colorIn, float4(0.0, 0.0, 0.0, 0.0), saturate(fogZ)).rgb, colorIn.a);
#else
  return float4(mix(colorIn, lu.fog.color, saturate(fogZ)).rgb, colorIn.a);
#endif
}
#endif

#if defined(URDE_LIGHTING) || defined(URDE_LIGHTING_SHADOW) || defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
float4 PostFunc(thread VertToFrag& vtf, constant LightingUniform& lu,
                texture2d<float> extTex0, texture2d<float> extTex1, texture2d<float> extTex2,
                sampler samp, sampler clampSamp, sampler clampEdgeSamp, float4 colorIn) {
  return FogFunc(vtf, lu, colorIn) * lu.mulColor + lu.addColor;
}
#endif

#if defined(URDE_THERMAL_HOT)
float4 PostFunc(thread VertToFrag& vtf, constant LightingUniform& lu,
                texture2d<float> extTex0, texture2d<float> extTex1, texture2d<float> extTex2,
                sampler samp, sampler clampSamp, sampler clampEdgeSamp, float4 colorIn) {
  return extTex0.sample(samp, vtf.extUvs0).rrrr * lu.tmulColor + lu.taddColor;
}
#endif

#if defined(URDE_THERMAL_COLD)
float4 PostFunc(thread VertToFrag& vtf, constant LightingUniform& lu,
                texture2d<float> extTex0, texture2d<float> extTex1, texture2d<float> extTex2,
                sampler samp, sampler clampSamp, sampler clampEdgeSamp, float4 colorIn) {
  return colorIn * float4(0.75, 0.75, 0.75, 0.75);
}
#endif

#if defined(URDE_SOLID)
float4 PostFunc(thread VertToFrag& vtf, constant LightingUniform& lu,
                texture2d<float> extTex0, texture2d<float> extTex1, texture2d<float> extTex2,
                sampler samp, sampler clampSamp, sampler clampEdgeSamp, float4 colorIn) {
  return lu.solidColor;
}
#endif

#if defined(URDE_MB_SHADOW)
float4 PostFunc(thread VertToFrag& vtf, constant LightingUniform& lu,
                texture2d<float> extTex0, texture2d<float> extTex1, texture2d<float> extTex2,
                sampler samp, sampler clampSamp, sampler clampEdgeSamp, float4 colorIn) {
  float idTexel = extTex0.sample(samp, vtf.extUvs0).a;
  float sphereTexel = extTex1.sample(clampEdgeSamp, vtf.extUvs1).r;
  float fadeTexel = extTex2.sample(clampEdgeSamp, vtf.extUvs2).a;
  float val = ((abs(idTexel - lu.shadowId) < 0.001) ?
      (dot(vtf.mvNorm.xyz, lu.shadowUp.xyz) * lu.shadowUp.w) : 0.0) *
      sphereTexel * fadeTexel;
  return float4(0.0, 0.0, 0.0, val);
}
#endif

#if defined(URDE_DISINTEGRATE)
float4 PostFunc(thread VertToFrag& vtf, constant LightingUniform& lu,
                texture2d<float> extTex0, texture2d<float> extTex1, texture2d<float> extTex2,
                sampler samp, sampler clampSamp, sampler clampEdgeSamp, float4 colorIn) {
  float4 texel0 = extTex0.sample(samp, vtf.extUvs0);
  float4 texel1 = extTex0.sample(samp, vtf.extUvs1);
  colorIn = mix(float4(0.0, 0.0, 0.0, 0.0), texel1, texel0);
  colorIn.rgb += lu.daddColor.rgb;
  return FogFunc(vtf, lu, colorIn);
}
#endif

#if defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
#define ReflectionFunc(roughness) \
  (reflectionTex.sample(reflectSamp, float3(reflectionCoords.x, -reflectionCoords.y, reflectionCoords.z), bias(roughness)).rgb)
#elif defined(URDE_REFLECTION_SIMPLE)
#define ReflectionFunc() \
  (reflectionTex.sample(reflectSamp, vtf.dynReflectionUvs1).rgb * vtf.dynReflectionAlpha)
#elif defined(URDE_REFLECTION_INDIRECT)
#define ReflectionFunc() \
  (reflectionTex.sample(reflectSamp, (reflectionIndTex.sample(samp, vtf.dynReflectionUvs0).ab - \
  float2(0.5, 0.5)) * float2(0.5, 0.5) + vtf.dynReflectionUvs1).rgb * vtf.dynReflectionAlpha)
#else
#define ReflectionFunc() float3(0.0, 0.0, 0.0)
#endif

#if !defined(URDE_ALPHA_TEST)
[[ early_fragment_tests ]]
#endif
fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(0) ]],
                      sampler clampSamp [[ sampler(1) ]],
                      sampler reflectSamp [[ sampler(2) ]],
                      sampler clampEdgeSamp [[ sampler(3) ]],
                      texture2d<float> lightmap [[ texture(0) ]],
                      texture2d<float> diffuse [[ texture(1) ]],
                      texture2d<float> emissive [[ texture(2) ]],
                      texture2d<float> specular [[ texture(3) ]],
                      texture2d<float> extendedSpecular [[ texture(4) ]],
                      texture2d<float> reflection [[ texture(5) ]],
                      texture2d<float> alpha [[ texture(6) ]],
                      texture2d<float> reflectionIndTex [[ texture(7) ]],
                      texture2d<float> extTex0 [[ texture(8) ]],
                      texture2d<float> extTex1 [[ texture(9) ]],
                      texture2d<float> extTex2 [[ texture(10) ]],
#if defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
                      texturecube<float> reflectionTex [[ texture(11) ]],
#else
                      texture2d<float> reflectionTex [[ texture(11) ]],
#endif
                      constant LightingUniform& lu [[ buffer(4) ]]) {
  float3 lighting = LightingFunc(vtf, lu, extTex0, clampSamp);
  float4 tmp;
#if defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
  float3 reflectionCoords = reflect(vtf.mvPos.xyz, vtf.mvNorm.xyz);
  tmp.rgb = (SampleTexture_lightmap() * colorReg1.rgb + lighting) * SampleTexture_diffuse() +
  SampleTexture_emissive() + (SampleTexture_specular() + SampleTexture_extendedSpecular() * lighting) *
  (SampleTexture_reflection() * ReflectionFunc(saturate(0.5 - SampleTextureAlpha_specular())) * 2.0);
  tmp.a = SampleTextureAlpha_alpha();
#elif defined(URDE_DIFFUSE_ONLY)
  tmp.rgb = SampleTexture_diffuse();
  tmp.a = SampleTextureAlpha_alpha();
#elif defined(RETRO_SHADER)
  tmp.rgb = (SampleTexture_lightmap() * colorReg1.rgb + lighting) * SampleTexture_diffuse() +
  SampleTexture_emissive() + (SampleTexture_specular() + SampleTexture_extendedSpecular() * lighting) *
  SampleTexture_reflection() + ReflectionFunc();
  tmp.a = SampleTextureAlpha_alpha();
#elif defined(RETRO_DYNAMIC_SHADER)
  tmp.rgb = (SampleTexture_lightmap() * colorReg1.rgb + lighting) * SampleTexture_diffuse() * colorReg1.rgb +
  SampleTexture_emissive() * colorReg1.rgb + (SampleTexture_specular() + SampleTexture_extendedSpecular() * lighting) *
  SampleTexture_reflection() + ReflectionFunc();
  tmp.a = SampleTextureAlpha_alpha();
#elif defined(RETRO_DYNAMIC_ALPHA_SHADER)
  tmp.rgb = (SampleTexture_lightmap() * colorReg1.rgb + lighting) * SampleTexture_diffuse() * colorReg1.rgb +
  SampleTexture_emissive() * colorReg1.rgb + (SampleTexture_specular() + SampleTexture_extendedSpecular() * lighting) *
  SampleTexture_reflection() + ReflectionFunc();
  tmp.a = SampleTextureAlpha_alpha() * colorReg1.a;
#elif defined(RETRO_DYNAMIC_CHARACTER_SHADER)
  tmp.rgb = (SampleTexture_lightmap() + lighting) * SampleTexture_diffuse() +
  SampleTexture_emissive() * colorReg1.rgb + (SampleTexture_specular() + SampleTexture_extendedSpecular() * lighting) *
  SampleTexture_reflection() + ReflectionFunc();
  tmp.a = SampleTextureAlpha_alpha();
#endif
  float4 colorOut = PostFunc(vtf, lu, extTex0, extTex1, extTex2, samp, clampSamp, clampEdgeSamp, tmp);
#if defined(URDE_ALPHA_TEST)
  if (colorOut.a < 0.25)
    discard_fragment();
#endif
  return colorOut;
}
