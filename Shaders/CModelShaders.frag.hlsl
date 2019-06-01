SamplerState samp : register(s0);
SamplerState clampSamp : register(s1);
SamplerState reflectSamp : register(s2);
Texture2D lightmap : register(t0);
Texture2D diffuse : register(t1);
Texture2D emissive : register(t2);
Texture2D specular : register(t3);
Texture2D extendedSpecular : register(t4);
Texture2D reflection : register(t5);
Texture2D alpha : register(t6);
Texture2D reflectionIndTex : register(t7);
Texture2D extTex0 : register(t8);
Texture2D extTex1 : register(t9);
Texture2D extTex2 : register(t10);

#if defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
TextureCube reflectionTex : register(t11);
#else
Texture2D reflectionTex : register(t11);
#endif

static const float3 kRGBToYPrime = float3(0.257, 0.504, 0.098);

/*
#if defined(ALPHA_%s)
float3 SampleTexture_%s(in VertToFrag vtf) { return %s.Sample(samp, vtf.%sUv).aaa; }
float SampleTextureAlpha_%s(in VertToFrag vtf) { return %s.Sample(samp, vtf.%sUv).a; }
#else
float3 SampleTexture_%s(in VertToFrag vtf) { return %s.Sample(samp, vtf.%sUv).rgb; }
float SampleTextureAlpha_%s(in VertToFrag vtf) { return dot(%s.Sample(samp, vtf.%sUv).rgb, kRGBToYPrime); }
#endif
 */

#if defined(ALPHA_lightmap)
float3 SampleTexture_lightmap(in VertToFrag vtf) { return lightmap.Sample(samp, vtf.lightmapUv).aaa; }
float SampleTextureAlpha_lightmap(in VertToFrag vtf) { return lightmap.Sample(samp, vtf.lightmapUv).a; }
#else
float3 SampleTexture_lightmap(in VertToFrag vtf) { return lightmap.Sample(samp, vtf.lightmapUv).rgb; }
float SampleTextureAlpha_lightmap(in VertToFrag vtf) { return dot(lightmap.Sample(samp, vtf.lightmapUv).rgb, kRGBToYPrime); }
#endif

#if defined(ALPHA_diffuse)
float3 SampleTexture_diffuse(in VertToFrag vtf) { return diffuse.Sample(samp, vtf.diffuseUv).aaa; }
float SampleTextureAlpha_diffuse(in VertToFrag vtf) { return diffuse.Sample(samp, vtf.diffuseUv).a; }
#else
float3 SampleTexture_diffuse(in VertToFrag vtf) { return diffuse.Sample(samp, vtf.diffuseUv).rgb; }
float SampleTextureAlpha_diffuse(in VertToFrag vtf) { return dot(diffuse.Sample(samp, vtf.diffuseUv).rgb, kRGBToYPrime); }
#endif

#if defined(ALPHA_emissive)
float3 SampleTexture_emissive(in VertToFrag vtf) { return emissive.Sample(samp, vtf.emissiveUv).aaa; }
float SampleTextureAlpha_emissive(in VertToFrag vtf) { return emissive.Sample(samp, vtf.emissiveUv).a; }
#else
float3 SampleTexture_emissive(in VertToFrag vtf) { return emissive.Sample(samp, vtf.emissiveUv).rgb; }
float SampleTextureAlpha_emissive(in VertToFrag vtf) { return dot(emissive.Sample(samp, vtf.emissiveUv).rgb, kRGBToYPrime); }
#endif

#if defined(ALPHA_specular)
float3 SampleTexture_specular(in VertToFrag vtf) { return specular.Sample(samp, vtf.specularUv).aaa; }
float SampleTextureAlpha_specular(in VertToFrag vtf) { return specular.Sample(samp, vtf.specularUv).a; }
#else
float3 SampleTexture_specular(in VertToFrag vtf) { return specular.Sample(samp, vtf.specularUv).rgb; }
float SampleTextureAlpha_specular(in VertToFrag vtf) { return dot(specular.Sample(samp, vtf.specularUv).rgb, kRGBToYPrime); }
#endif

#if defined(ALPHA_extendedSpecular)
float3 SampleTexture_extendedSpecular(in VertToFrag vtf) { return extendedSpecular.Sample(samp, vtf.extendedSpecularUv).aaa; }
float SampleTextureAlpha_extendedSpecular(in VertToFrag vtf) { return extendedSpecular.Sample(samp, vtf.extendedSpecularUv).a; }
#else
float3 SampleTexture_extendedSpecular(in VertToFrag vtf) { return extendedSpecular.Sample(samp, vtf.extendedSpecularUv).rgb; }
float SampleTextureAlpha_extendedSpecular(in VertToFrag vtf) { return dot(extendedSpecular.Sample(samp, vtf.extendedSpecularUv).rgb, kRGBToYPrime); }
#endif

#if defined(ALPHA_reflection)
float3 SampleTexture_reflection(in VertToFrag vtf) { return reflection.Sample(samp, vtf.reflectionUv).aaa; }
float SampleTextureAlpha_reflection(in VertToFrag vtf) { return reflection.Sample(samp, vtf.reflectionUv).a; }
#else
float3 SampleTexture_reflection(in VertToFrag vtf) { return reflection.Sample(samp, vtf.reflectionUv).rgb; }
float SampleTextureAlpha_reflection(in VertToFrag vtf) { return dot(reflection.Sample(samp, vtf.reflectionUv).rgb, kRGBToYPrime); }
#endif

#if defined(ALPHA_alpha)
float3 SampleTexture_alpha(in VertToFrag vtf) { return alpha.Sample(samp, vtf.alphaUv).aaa; }
float SampleTextureAlpha_alpha(in VertToFrag vtf) { return alpha.Sample(samp, vtf.alphaUv).a; }
#else
float3 SampleTexture_alpha(in VertToFrag vtf) { return alpha.Sample(samp, vtf.alphaUv).rgb; }
float SampleTextureAlpha_alpha(in VertToFrag vtf) { return dot(alpha.Sample(samp, vtf.alphaUv).rgb, kRGBToYPrime); }
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

cbuffer LightingUniform : register(b2) {
  Light lights[URDE_MAX_LIGHTS];
  float4 ambient;
  float4 colorReg0;
  float4 colorReg1;
  float4 colorReg2;
  float4 mulColor;
  float4 addColor;
  Fog fog;
};
#else
static const float4 colorReg0 = float4(1.0, 1.0, 1.0, 1.0);
static const float4 colorReg1 = float4(1.0, 1.0, 1.0, 1.0);
static const float4 colorReg2 = float4(1.0, 1.0, 1.0, 1.0);
#endif

#if defined(URDE_LIGHTING) || defined(URDE_LIGHTING_CUBE_REFLECTION)
float3 LightingFunc(in VertToFrag vtf) {
  float4 ret = ambient;

  for (int i = 0; i < URDE_MAX_LIGHTS; ++i) {
    float3 delta = vtf.mvPos.xyz - lights[i].pos.xyz;
    float dist = length(delta);
    float3 deltaNorm = delta / dist;
    float angDot = max(dot(deltaNorm, lights[i].dir.xyz), 0.0);
    float att = 1.0 / (lights[i].linAtt[2] * dist * dist +
                       lights[i].linAtt[1] * dist +
                       lights[i].linAtt[0]);
    float angAtt = lights[i].angAtt[2] * angDot * angDot +
                   lights[i].angAtt[1] * angDot +
                   lights[i].angAtt[0];
    ret += lights[i].color * angAtt * att * max(dot(-deltaNorm, vtf.mvNorm.xyz), 0.0);
  }

  return saturate(ret.rgb);
}
#endif

#if defined(URDE_THERMAL_HOT)
float3 LightingFunc(in VertToFrag vtf) {
  return float3(1.0, 1.0, 1.0);
}
cbuffer ThermalUniform : register(b2) {
  float4 tmulColor;
  float4 taddColor;
};
#endif

#if defined(URDE_THERMAL_COLD)
float3 LightingFunc(in VertToFrag vtf) {
  return float3(1.0, 1.0, 1.0);
}
#endif

#if defined(URDE_SOLID)
float3 LightingFunc(in VertToFrag vtf) {
  return float3(1.0, 1.0, 1.0);
}
cbuffer SolidUniform : register(b2) {
  float4 solidColor;
};
#endif

#if defined(URDE_MB_SHADOW)
float3 LightingFunc(in VertToFrag vtf) {
  return float3(1.0, 1.0, 1.0);
}
cbuffer MBShadowUniform : register(b2) {
  float4 shadowUp;
  float shadowId;
};
#endif

#if defined(URDE_LIGHTING_SHADOW) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
float3 LightingFunc(in VertToFrag vtf) {
  float2 shadowUV = vtf.extUvs[0];
  shadowUV.y = 1.0 - shadowUV.y;

  float4 ret = ambient;

  float3 delta = vtf.mvPos.xyz - lights[0].pos.xyz;
  float dist = length(delta);
  float3 deltaNorm = delta / dist;
  float angDot = max(dot(deltaNorm, lights[0].dir.xyz), 0.0);
  float att = 1.0 / (lights[0].linAtt[2] * dist * dist +
                     lights[0].linAtt[1] * dist +
                     lights[0].linAtt[0]);
  float angAtt = lights[0].angAtt[2] * angDot * angDot +
                 lights[0].angAtt[1] * angDot +
                 lights[0].angAtt[0];
  ret += lights[0].color * angAtt * att * max(dot(-deltaNorm, vtf.mvNorm.xyz), 0.0) *
         extTex0.Sample(clampSamp, shadowUV).r;

  for (int i = 1; i < URDE_MAX_LIGHTS; ++i) {
    float3 delta = vtf.mvPos.xyz - lights[i].pos.xyz;
    float dist = length(delta);
    float3 deltaNorm = delta / dist;
    float angDot = max(dot(deltaNorm, lights[i].dir.xyz), 0.0);
    float att = 1.0 / (lights[i].linAtt[2] * dist * dist +
                       lights[i].linAtt[1] * dist +
                       lights[i].linAtt[0]);
    float angAtt = lights[i].angAtt[2] * angDot * angDot +
                   lights[i].angAtt[1] * angDot +
                   lights[i].angAtt[0];
    ret += lights[i].color * angAtt * att * max(dot(-deltaNorm, vtf.mvNorm.xyz), 0.0);
  }

  return saturate(ret.rgb);
}
#endif

#if defined(URDE_DISINTEGRATE)
cbuffer DisintegrateUniform : register(b2) {
  float4 daddColor;
  Fog fog;
};
float3 LightingFunc(in VertToFrag vtf) {
  return float3(1.0, 1.0, 1.0);
}
#endif

#if defined(URDE_LIGHTING) || defined(URDE_LIGHTING_SHADOW) || defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW) || defined(URDE_DISINTEGRATE)
float4 FogFunc(in VertToFrag vtf, float4 colorIn) {
  float fogZ;
  float fogF = saturate((fog.A / (fog.B - (1.0 - vtf.mvpPos.z))) - fog.C);
  switch (fog.mode) {
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
  return float4(lerp(colorIn, float4(0.0, 0.0, 0.0, 0.0), saturate(fogZ)).rgb, colorIn.a);
#else
  return float4(lerp(colorIn, fog.color, saturate(fogZ)).rgb, colorIn.a);
#endif
}
#endif

#if defined(URDE_LIGHTING) || defined(URDE_LIGHTING_SHADOW) || defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
float4 PostFunc(in VertToFrag vtf, float4 colorIn) {
  return FogFunc(vtf, colorIn) * mulColor + addColor;
}
#endif

#if defined(URDE_THERMAL_HOT)
float4 PostFunc(in VertToFrag vtf, float4 colorIn) {
  return extTex0.Sample(samp, vtf.extUvs[0]).rrrr * tmulColor + taddColor;
}
#endif

#if defined(URDE_THERMAL_COLD)
float4 PostFunc(in VertToFrag vtf, float4 colorIn) {
  return colorIn * float4(0.75, 0.75, 0.75, 0.75);
}
#endif

#if defined(URDE_SOLID)
float4 PostFunc(in VertToFrag vtf, float4 colorIn) {
  return solidColor;
}
#endif

#if defined(URDE_MB_SHADOW)
float4 PostFunc(in VertToFrag vtf, float4 colorIn) {
  float idTexel = extTex0.Sample(samp, vtf.extUvs[0]).a;
  float sphereTexel = extTex1.Sample(samp, vtf.extUvs[1]).a;
  float fadeTexel = extTex2.Sample(samp, vtf.extUvs[2]).a;
  float val = ((abs(idTexel - shadowId) < 0.001) ?
      (dot(vtf.mvNorm.xyz, shadowUp.xyz) * shadowUp.w) : 0.0) *
      sphereTexel * fadeTexel;
  return float4(0.0, 0.0, 0.0, val);
}
#endif

#if defined(URDE_DISINTEGRATE)
float4 PostFunc(in VertToFrag vtf, float4 colorIn) {
  float4 texel0 = extTex0.Sample(samp, vtf.extUvs[0]);
  float4 texel1 = extTex0.Sample(samp, vtf.extUvs[1]);
  colorIn = lerp(float4(0.0, 0.0, 0.0, 0.0), texel1, texel0);
  colorIn.rgb += daddColor.rgb;
  return FogFunc(vtf, colorIn);
}
#endif

#if defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
float3 ReflectionFunc(in VertToFrag vtf, float roughness) {
  return reflectionTex.SampleBias(reflectSamp, reflect(vtf.mvPos.xyz, vtf.mvNorm.xyz), roughness).rgb;
}
#elif defined(URDE_REFLECTION_SIMPLE)
float3 ReflectionFunc(in VertToFrag vtf)
{ return reflectionTex.Sample(reflectSamp, vtf.dynReflectionUvs[1]).rgb * vtf.dynReflectionAlpha; }
#elif defined(URDE_REFLECTION_INDIRECT)
float3 ReflectionFunc(in VertToFrag vtf)
{ return reflectionTex.Sample(reflectSamp, (reflectionIndTex.Sample(samp, vtf.dynReflectionUvs[0]).ab -
  float2(0.5, 0.5)) * float2(0.5, 0.5) + vtf.dynReflectionUvs[1]).rgb * vtf.dynReflectionAlpha; }
#else
float3 ReflectionFunc(in VertToFrag vtf) { return float3(0.0, 0.0, 0.0); }
#endif

#if !defined(URDE_ALPHA_TEST)
[earlydepthstencil]
#endif
float4 main(in VertToFrag vtf) : SV_Target0 {
  float3 lighting = LightingFunc(vtf);
  float4 tmp;
#if defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
  tmp.rgb = (SampleTexture_lightmap(vtf) * colorReg1.rgb + lighting) * SampleTexture_diffuse(vtf) +
  SampleTexture_emissive(vtf) + (SampleTexture_specular(vtf) + SampleTexture_extendedSpecular(vtf) * lighting) *
  (SampleTexture_reflection(vtf) * ReflectionFunc(vtf, clamp(0.5 - SampleTextureAlpha_specular(vtf), 0.0, 1.0)) * 2.0);
  tmp.a = SampleTextureAlpha_alpha(vtf);
#elif defined(URDE_DIFFUSE_ONLY)
  tmp.rgb = SampleTexture_diffuse(vtf);
  tmp.a = SampleTextureAlpha_alpha(vtf);
#elif defined(RETRO_SHADER)
  tmp.rgb = (SampleTexture_lightmap(vtf) * colorReg1.rgb + lighting) * SampleTexture_diffuse(vtf) +
  SampleTexture_emissive(vtf) + (SampleTexture_specular(vtf) + SampleTexture_extendedSpecular(vtf) * lighting) *
  SampleTexture_reflection(vtf) + ReflectionFunc(vtf);
  tmp.a = SampleTextureAlpha_alpha(vtf);
#elif defined(RETRO_DYNAMIC_SHADER)
  tmp.rgb = (SampleTexture_lightmap(vtf) * colorReg1.rgb + lighting) * SampleTexture_diffuse(vtf) * colorReg1.rgb +
  SampleTexture_emissive(vtf) * colorReg1.rgb + (SampleTexture_specular(vtf) + SampleTexture_extendedSpecular(vtf) * lighting) *
  SampleTexture_reflection(vtf) + ReflectionFunc(vtf);
  tmp.a = SampleTextureAlpha_alpha(vtf);
#elif defined(RETRO_DYNAMIC_ALPHA_SHADER)
  tmp.rgb = (SampleTexture_lightmap(vtf) * colorReg1.rgb + lighting) * SampleTexture_diffuse(vtf) * colorReg1.rgb +
  SampleTexture_emissive(vtf) * colorReg1.rgb + (SampleTexture_specular(vtf) + SampleTexture_extendedSpecular(vtf) * lighting) *
  SampleTexture_reflection(vtf) + ReflectionFunc(vtf);
  tmp.a = SampleTextureAlpha_alpha(vtf) * colorReg1.a;
#elif defined(RETRO_DYNAMIC_CHARACTER_SHADER)
  tmp.rgb = (SampleTexture_lightmap(vtf) + lighting) * SampleTexture_diffuse(vtf) +
  SampleTexture_emissive(vtf) * colorReg1.rgb + (SampleTexture_specular(vtf) + SampleTexture_extendedSpecular(vtf) * lighting) *
  SampleTexture_reflection(vtf) + ReflectionFunc(vtf);
  tmp.a = SampleTextureAlpha_alpha(vtf);
#endif
  float4 colorOut = PostFunc(vtf, tmp);
#if defined(URDE_ALPHA_TEST)
  if (colorOut.a < 0.25)
    discard;
#endif
  return colorOut;
}
