struct URDEVertUniform {
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
struct URDETexMtxUniform {
  URDETCGMatrix texMtxs[8];
};

struct URDEReflectMtx {
  float4x4 indMtx;
  float4x4 reflectMtx;
  float reflectAlpha;
};

URDE_VERT_DATA_DECL

vertex VertToFrag vmain(VertData v [[ stage_in ]],
                        constant URDEVertUniform& vu [[ buffer(2) ]],
                        constant URDETexMtxUniform& tu [[ buffer(3) ]],
                        constant URDEReflectMtx& ru [[ buffer(5) ]]) {
  VertToFrag vtf;
#if URDE_SKIN_SLOTS
  float4 objPos = float4(0.0, 0.0, 0.0, 0.0);
  float4 objNorm = float4(0.0, 0.0, 0.0, 0.0);
URDE_WEIGHTING_EXPR
  objPos[3] = 1.0;
  objNorm = float4(normalize(objNorm.xyz), 0.0);
  vtf.mvPos = vu.mv * objPos;
  vtf.mvNorm = float4(normalize((vu.mvInv * objNorm).xyz), 0.0);
  vtf.mvpPos = vu.proj * vtf.mvPos;
#else
  float4 objPos = float4(v.posIn, 1.0);
  float4 objNorm = float4(v.normIn, 0.0);
  vtf.mvPos = vu.mv * objPos;
  vtf.mvNorm = vu.mvInv * objNorm;
  vtf.mvpPos = vu.proj * vtf.mvPos;
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
