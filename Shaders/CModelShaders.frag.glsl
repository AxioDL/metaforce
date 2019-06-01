#if !defined(URDE_ALPHA_TEST) && defined(GL_ARB_shader_image_load_store)
layout(early_fragment_tests) in;
#endif

SBINDING(0) in VertToFrag vtf;
TBINDING0 uniform sampler2D lightmap;
TBINDING1 uniform sampler2D diffuse;
TBINDING2 uniform sampler2D emissive;
TBINDING3 uniform sampler2D specular;
TBINDING4 uniform sampler2D extendedSpecular;
TBINDING5 uniform sampler2D reflection;
TBINDING6 uniform sampler2D alpha;
TBINDING7 uniform sampler2D reflectionIndTex;
TBINDING8 uniform sampler2D extTex0;
TBINDING9 uniform sampler2D extTex1;
TBINDING10 uniform sampler2D extTex2;

#if defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
TBINDING11 uniform samplerCube reflectionTex;
#else
TBINDING11 uniform sampler2D reflectionTex;
#endif

const vec3 kRGBToYPrime = vec3(0.257, 0.504, 0.098);

/*
#if defined(ALPHA_%s)
vec3 SampleTexture_%s() { return texture(%s, vtf.%sUv).aaa; }
float SampleTextureAlpha_%s() { return texture(%s, vtf.%sUv).a; }
#else
vec3 SampleTexture_%s() { return texture(%s, vtf.%sUv).rgb; }
float SampleTextureAlpha_%s() { return dot(texture(%s, vtf.%sUv).rgb, kRGBToYPrime); }
#endif
 */

#if defined(ALPHA_lightmap)
vec3 SampleTexture_lightmap() { return texture(lightmap, vtf.lightmapUv).aaa; }
float SampleTextureAlpha_lightmap() { return texture(lightmap, vtf.lightmapUv).a; }
#else
vec3 SampleTexture_lightmap() { return texture(lightmap, vtf.lightmapUv).rgb; }
float SampleTextureAlpha_lightmap() { return dot(texture(lightmap, vtf.lightmapUv).rgb, kRGBToYPrime); }
#endif

#if defined(ALPHA_diffuse)
vec3 SampleTexture_diffuse() { return texture(diffuse, vtf.diffuseUv).aaa; }
float SampleTextureAlpha_diffuse() { return texture(diffuse, vtf.diffuseUv).a; }
#else
vec3 SampleTexture_diffuse() { return texture(diffuse, vtf.diffuseUv).rgb; }
float SampleTextureAlpha_diffuse() { return dot(texture(diffuse, vtf.diffuseUv).rgb, kRGBToYPrime); }
#endif

#if defined(ALPHA_emissive)
vec3 SampleTexture_emissive() { return texture(emissive, vtf.emissiveUv).aaa; }
float SampleTextureAlpha_emissive() { return texture(emissive, vtf.emissiveUv).a; }
#else
vec3 SampleTexture_emissive() { return texture(emissive, vtf.emissiveUv).rgb; }
float SampleTextureAlpha_emissive() { return dot(texture(emissive, vtf.emissiveUv).rgb, kRGBToYPrime); }
#endif

#if defined(ALPHA_specular)
vec3 SampleTexture_specular() { return texture(specular, vtf.specularUv).aaa; }
float SampleTextureAlpha_specular() { return texture(specular, vtf.specularUv).a; }
#else
vec3 SampleTexture_specular() { return texture(specular, vtf.specularUv).rgb; }
float SampleTextureAlpha_specular() { return dot(texture(specular, vtf.specularUv).rgb, kRGBToYPrime); }
#endif

#if defined(ALPHA_extendedSpecular)
vec3 SampleTexture_extendedSpecular() { return texture(extendedSpecular, vtf.extendedSpecularUv).aaa; }
float SampleTextureAlpha_extendedSpecular() { return texture(extendedSpecular, vtf.extendedSpecularUv).a; }
#else
vec3 SampleTexture_extendedSpecular() { return texture(extendedSpecular, vtf.extendedSpecularUv).rgb; }
float SampleTextureAlpha_extendedSpecular() { return dot(texture(extendedSpecular, vtf.extendedSpecularUv).rgb, kRGBToYPrime); }
#endif

#if defined(ALPHA_reflection)
vec3 SampleTexture_reflection() { return texture(reflection, vtf.reflectionUv).aaa; }
float SampleTextureAlpha_reflection() { return texture(reflection, vtf.reflectionUv).a; }
#else
vec3 SampleTexture_reflection() { return texture(reflection, vtf.reflectionUv).rgb; }
float SampleTextureAlpha_reflection() { return dot(texture(reflection, vtf.reflectionUv).rgb, kRGBToYPrime); }
#endif

#if defined(ALPHA_alpha)
vec3 SampleTexture_alpha() { return texture(alpha, vtf.alphaUv).aaa; }
float SampleTextureAlpha_alpha() { return texture(alpha, vtf.alphaUv).a; }
#else
vec3 SampleTexture_alpha() { return texture(alpha, vtf.alphaUv).rgb; }
float SampleTextureAlpha_alpha() { return dot(texture(alpha, vtf.alphaUv).rgb, kRGBToYPrime); }
#endif

#if defined(URDE_LIGHTING) || defined(URDE_LIGHTING_SHADOW) || defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW) || defined(URDE_DISINTEGRATE)
struct Fog {
  vec4 color;
  float A;
  float B;
  float C;
  int mode;
};
#endif

#if defined(URDE_LIGHTING) || defined(URDE_LIGHTING_SHADOW) || defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
struct Light {
  vec4 pos;
  vec4 dir;
  vec4 color;
  vec4 linAtt;
  vec4 angAtt;
};

UBINDING2 uniform LightingUniform {
  Light lights[URDE_MAX_LIGHTS];
  vec4 ambient;
  vec4 colorReg0;
  vec4 colorReg1;
  vec4 colorReg2;
  vec4 mulColor;
  vec4 addColor;
  Fog fog;
};
#else
const vec4 colorReg0 = vec4(1.0);
const vec4 colorReg1 = vec4(1.0);
const vec4 colorReg2 = vec4(1.0);
#endif

#if defined(URDE_LIGHTING) || defined(URDE_LIGHTING_CUBE_REFLECTION)
vec3 LightingFunc() {
  vec4 ret = ambient;

  for (int i = 0; i < URDE_MAX_LIGHTS; ++i) {
    vec3 delta = vtf.mvPos.xyz - lights[i].pos.xyz;
    float dist = length(delta);
    vec3 deltaNorm = delta / dist;
    float angDot = max(dot(deltaNorm, lights[i].dir.xyz), 0.0);
    float att = 1.0 / (lights[i].linAtt[2] * dist * dist +
                       lights[i].linAtt[1] * dist +
                       lights[i].linAtt[0]);
    float angAtt = lights[i].angAtt[2] * angDot * angDot +
                   lights[i].angAtt[1] * angDot +
                   lights[i].angAtt[0];
    ret += lights[i].color * angAtt * att * max(dot(-deltaNorm, vtf.mvNorm.xyz), 0.0);
  }

  return clamp(ret.rgb, vec3(0.0), vec3(1.0));
}
#endif

#if defined(URDE_THERMAL_HOT)
vec3 LightingFunc() {
  return vec3(1.0);
}
UBINDING2 uniform ThermalUniform {
  vec4 tmulColor;
  vec4 taddColor;
};
#endif

#if defined(URDE_THERMAL_COLD)
vec3 LightingFunc() {
  return vec3(1.0);
}
#endif

#if defined(URDE_SOLID)
vec3 LightingFunc() {
  return vec3(1.0);
}
UBINDING2 uniform SolidUniform {
  vec4 solidColor;
};
#endif

#if defined(URDE_MB_SHADOW)
vec3 LightingFunc() {
  return vec3(1.0);
}
UBINDING2 uniform MBShadowUniform {
  vec4 shadowUp;
  float shadowId;
};
#endif

#if defined(URDE_LIGHTING_SHADOW) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
vec3 LightingFunc() {
  vec2 shadowUV = vtf.extUvs[0];
  shadowUV.y = 1.0 - shadowUV.y;

  vec4 ret = ambient;

  vec3 delta = vtf.mvPos.xyz - lights[0].pos.xyz;
  float dist = length(delta);
  vec3 deltaNorm = delta / dist;
  float angDot = max(dot(deltaNorm, lights[0].dir.xyz), 0.0);
  float att = 1.0 / (lights[0].linAtt[2] * dist * dist +
                     lights[0].linAtt[1] * dist +
                     lights[0].linAtt[0]);
  float angAtt = lights[0].angAtt[2] * angDot * angDot +
                 lights[0].angAtt[1] * angDot +
                 lights[0].angAtt[0];
  ret += lights[0].color * angAtt * att * max(dot(-deltaNorm, vtf.mvNorm.xyz), 0.0) *
         texture(extTex0, shadowUV).r;

  for (int i = 1; i < URDE_MAX_LIGHTS; ++i) {
    vec3 delta = vtf.mvPos.xyz - lights[i].pos.xyz;
    float dist = length(delta);
    vec3 deltaNorm = delta / dist;
    float angDot = max(dot(deltaNorm, lights[i].dir.xyz), 0.0);
    float att = 1.0 / (lights[i].linAtt[2] * dist * dist +
                       lights[i].linAtt[1] * dist +
                       lights[i].linAtt[0]);
    float angAtt = lights[i].angAtt[2] * angDot * angDot +
                   lights[i].angAtt[1] * angDot +
                   lights[i].angAtt[0];
    ret += lights[i].color * angAtt * att * max(dot(-deltaNorm, vtf.mvNorm.xyz), 0.0);
  }

  return clamp(ret.rgb, vec3(0.0), vec3(1.0));
}
#endif

#if defined(URDE_DISINTEGRATE)
UBINDING2 uniform DisintegrateUniform {
  vec4 daddColor;
  Fog fog;
};
vec3 LightingFunc() {
  return vec3(1.0);
}
#endif

#if defined(URDE_LIGHTING) || defined(URDE_LIGHTING_SHADOW) || defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW) || defined(URDE_DISINTEGRATE)
vec4 FogFunc(vec4 colorIn) {
  float fogZ;
  float fogF = clamp((fog.A / (fog.B - gl_FragCoord.z)) - fog.C, 0.0, 1.0);
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
  return vec4(mix(colorIn, vec4(0.0), clamp(fogZ, 0.0, 1.0)).rgb, colorIn.a);
#else
  return vec4(mix(colorIn, fog.color, clamp(fogZ, 0.0, 1.0)).rgb, colorIn.a);
#endif
}
#endif

#if defined(URDE_LIGHTING) || defined(URDE_LIGHTING_SHADOW) || defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
vec4 PostFunc(vec4 colorIn) {
  return FogFunc(colorIn) * mulColor + addColor;
}
#endif

#if defined(URDE_THERMAL_HOT)
vec4 PostFunc(vec4 colorIn) {
  return texture(extTex0, vtf.extUvs[0]).rrrr * tmulColor + taddColor;
}
#endif

#if defined(URDE_THERMAL_COLD)
vec4 PostFunc(vec4 colorIn) {
  return colorIn * vec4(0.75);
}
#endif

#if defined(URDE_SOLID)
vec4 PostFunc(vec4 colorIn) {
  return solidColor;
}
#endif

#if defined(URDE_MB_SHADOW)
vec4 PostFunc(vec4 colorIn) {
  float idTexel = texture(extTex0, vtf.extUvs[0]).a;
  float sphereTexel = texture(extTex1, vtf.extUvs[1]).a;
  float fadeTexel = texture(extTex2, vtf.extUvs[2]).a;
  float val = ((abs(idTexel - shadowId) < 0.001) ?
      (dot(vtf.mvNorm.xyz, shadowUp.xyz) * shadowUp.w) : 0.0) *
      sphereTexel * fadeTexel;
  return vec4(0.0, 0.0, 0.0, val);
}
#endif

#if defined(URDE_DISINTEGRATE)
vec4 PostFunc(vec4 colorIn) {
  vec4 texel0 = texture(extTex0, vtf.extUvs[0]);
  vec4 texel1 = texture(extTex0, vtf.extUvs[1]);
  colorIn = mix(vec4(0.0), texel1, texel0);
  colorIn.rgb += daddColor.rgb;
  return FogFunc(colorIn);
}
#endif

#if defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
vec3 ReflectionFunc(float roughness) { return texture(reflectionTex, reflect(vtf.mvPos.xyz, vtf.mvNorm.xyz), roughness).rgb; }
#elif defined(URDE_REFLECTION_SIMPLE)
vec3 ReflectionFunc() { return texture(reflectionTex, vtf.dynReflectionUvs[1]).rgb * vtf.dynReflectionAlpha; }
#elif defined(URDE_REFLECTION_INDIRECT)
vec3 ReflectionFunc() { return texture(reflectionTex, (texture(reflectionIndTex, vtf.dynReflectionUvs[0]).ab -
                        vec2(0.5, 0.5)) * vec2(0.5, 0.5) + vtf.dynReflectionUvs[1]).rgb * vtf.dynReflectionAlpha; }
#else
vec3 ReflectionFunc() { return vec3(0.0); }
#endif

layout(location=0) out vec4 colorOut;
void main() {
  vec3 lighting = LightingFunc();
  vec4 tmp;
#if defined(URDE_LIGHTING_CUBE_REFLECTION) || defined(URDE_LIGHTING_CUBE_REFLECTION_SHADOW)
  tmp.rgb = (SampleTexture_lightmap() * colorReg1.rgb + lighting) * SampleTexture_diffuse() +
  SampleTexture_emissive() + (SampleTexture_specular() + SampleTexture_extendedSpecular() * lighting) *
  (SampleTexture_reflection() * ReflectionFunc(clamp(0.5 - SampleTextureAlpha_specular(), 0.0, 1.0)) * 2.0);
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
  colorOut = PostFunc(tmp);
#if defined(URDE_ALPHA_TEST)
  if (colorOut.a < 0.25)
    discard;
#endif
}
