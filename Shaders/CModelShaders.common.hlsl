struct VertToFrag {
  float4 mvpPos : SV_Position;
  float4 mvPos : POSITION;
  float4 mvNorm : NORMAL;
  float2 lightmapUv : UV0;
  float2 diffuseUv : UV1;
  float2 emissiveUv : UV2;
  float2 specularUv : UV3;
  float2 extendedSpecularUv : UV4;
  float2 reflectionUv : UV5;
  float2 alphaUv : UV6;
  float2 extUvs[3] : EXTUV;
  float2 dynReflectionUvs[2] : REFLECTUV;
  float dynReflectionAlpha : REFLECTALPHA;
};
