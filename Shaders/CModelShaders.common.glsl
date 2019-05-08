#extension GL_ARB_enhanced_layouts: enable
#extension GL_ARB_shader_image_load_store: enable

struct VertToFrag {
  vec4 mvPos;
  vec4 mvNorm;
  vec4 color;
  vec2 lightmapUv;
  vec2 diffuseUv;
  vec2 emissiveUv;
  vec2 specularUv;
  vec2 extendedSpecularUv;
  vec2 reflectionUv;
  vec2 alphaUv;
  vec2 extUvs[3];
  vec2 dynReflectionUvs[2];
  float dynReflectionAlpha;
};
