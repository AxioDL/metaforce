cbuffer URDEVertUniform : register(b0) {
#if URDE_SKIN_SLOTS
  float4x4 objs[URDE_SKIN_SLOTS];
  float4x4 objsInv[URDE_SKIN_SLOTS];
#endif
  float4x4 mv;
  float4x4 mvInv;
  float4x4 proj;
};

struct URDETCGMatrix {
  float4x4 mtx;
  float4x4 postMtx;
};
cbuffer URDETexMtxUniform : register(b1) {
  URDETCGMatrix texMtxs[8];
};

cbuffer URDEReflectMtx : register(b3) {
  float4x4 indMtx;
  float4x4 reflectMtx;
  float reflectAlpha;
};

struct VertData {
  float3 posIn : POSITION;
  float3 normIn : NORMAL;
#if URDE_COL_SLOTS
  float4 colIn[URDE_COL_SLOTS] : COLOR;
#endif
#if URDE_UV_SLOTS
  float2 uvIn[URDE_UV_SLOTS] : UV;
#endif
#if URDE_WEIGHT_SLOTS
  float4 weightIn[URDE_WEIGHT_SLOTS] : WEIGHT;
#endif
};

VertToFrag main(in VertData v) {
  VertToFrag vtf;
#if URDE_SKIN_SLOTS
  float4 objPos = float4(0.0, 0.0, 0.0, 0.0);
  float4 objNorm = float4(0.0, 0.0, 0.0, 0.0);
  for (int i = 0; i < URDE_SKIN_SLOTS; ++i) {
    objPos += mul(objs[i], float4(v.posIn, 1.0)) * v.weightIn[i / 4][i % 4];
    objNorm += mul(objsInv[i], float4(v.normIn, 1.0)) * v.weightIn[i / 4][i % 4];
  }
  objPos[3] = 1.0;
  objNorm = float4(normalize(objNorm.xyz), 0.0);
  vtf.mvPos = mul(mv, objPos);
  vtf.mvNorm = float4(normalize(mul(mvInv, objNorm).xyz), 0.0);
  vtf.mvpPos = mul(proj, vtf.mvPos);
#else
  float4 objPos = float4(v.posIn, 1.0);
  float4 objNorm = float4(v.normIn, 0.0);
  vtf.mvPos = mul(mv, objPos);
  vtf.mvNorm = mul(mvInv, objNorm);
  vtf.mvpPos = mul(proj, vtf.mvPos);
#endif

  vtf.lightmapUv = float2(0.0, 0.0);
  vtf.diffuseUv = float2(0.0, 0.0);
  vtf.emissiveUv = float2(0.0, 0.0);
  vtf.specularUv = float2(0.0, 0.0);
  vtf.extendedSpecularUv = float2(0.0, 0.0);
  vtf.reflectionUv = float2(0.0, 0.0);
  vtf.alphaUv = float2(0.0, 0.0);
  float4 tmpProj;
  URDE_TCG_EXPR
  
  return vtf;
}
