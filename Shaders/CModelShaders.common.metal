#include <metal_stdlib>
using namespace metal;

struct VertToFrag {
  float4 mvpPos [[ position ]];
  float4 mvPos;
  float4 mvNorm;
  float2 lightmapUv;
  float2 diffuseUv;
  float2 emissiveUv;
  float2 specularUv;
  float2 extendedSpecularUv;
  float2 reflectionUv;
  float2 alphaUv;
  float2 extUvs0;
  float2 extUvs1;
  float2 extUvs2;
  float2 dynReflectionUvs0;
  float2 dynReflectionUvs1;
  float dynReflectionAlpha;
};
