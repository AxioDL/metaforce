UBINDING0 uniform URDEVertUniform {
#if URDE_SKIN_SLOTS
  mat4 objs[URDE_SKIN_SLOTS];
  mat4 objsInv[URDE_SKIN_SLOTS];
#endif
  mat4 mv;
  mat4 mvInv;
  mat4 proj;
};

struct URDETCGMatrix {
  mat4 mtx;
  mat4 postMtx;
};
UBINDING1 uniform URDETexMtxUniform {
  URDETCGMatrix texMtxs[8];
};

UBINDING3 uniform URDEReflectMtx {
  mat4 indMtx;
  mat4 reflectMtx;
  float reflectAlpha;
};

URDE_VERT_DATA_DECL

SBINDING(0) out VertToFrag vtf;
void main() {
#if URDE_SKIN_SLOTS
  vec4 objPos = vec4(0.0);
  vec4 objNorm = vec4(0.0);
  for (int i = 0; i < URDE_SKIN_SLOTS; ++i) {
    objPos += (objs[i] * vec4(posIn, 1.0)) * weightIn[i / 4][i % 4];
    objNorm += (objsInv[i] * vec4(normIn, 1.0)) * weightIn[i / 4][i % 4];
  }
  objPos[3] = 1.0;
  objNorm = vec4(normalize(objNorm.xyz), 0.0);
  vtf.mvPos = mv * objPos;
  vtf.mvNorm = vec4(normalize((mvInv * objNorm).xyz), 0.0);
  gl_Position = proj * vtf.mvPos;
#else
  vec4 objPos = vec4(posIn, 1.0);
  vec4 objNorm = vec4(normIn, 0.0);
  vtf.mvPos = mv * objPos;
  vtf.mvNorm = mvInv * objNorm;
  gl_Position = proj * vtf.mvPos;
#endif

  vec4 tmpProj;
  vtf.lightmapUv = vec2(0.0);
  vtf.diffuseUv = vec2(0.0);
  vtf.emissiveUv = vec2(0.0);
  vtf.specularUv = vec2(0.0);
  vtf.extendedSpecularUv = vec2(0.0);
  vtf.reflectionUv = vec2(0.0);
  vtf.alphaUv = vec2(0.0);
  URDE_TCG_EXPR
}
