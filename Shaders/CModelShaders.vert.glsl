UBINDING0 uniform HECLVertUniform {
#if URDE_SKIN_SLOTS
  mat4 objs[URDE_SKIN_SLOTS];
  mat4 objsInv[URDE_SKIN_SLOTS];
#endif
  mat4 mv;
  mat4 mvInv;
  mat4 proj;
};

struct HECLTCGMatrix {
  mat4 mtx;
  mat4 postMtx;
};
UBINDING1 uniform HECLTexMtxUniform {
  HECLTCGMatrix texMtxs[8];
};

UBINDING3 uniform HECLReflectMtx {
  mat4 indMtx;
  mat4 reflectMtx;
  float reflectAlpha;
};

layout(location=0) in vec3 posIn;
layout(location=1) in vec3 normIn;
#if URDE_COL_SLOTS
layout(location=2) in vec4 colIn[URDE_COL_SLOTS];
#endif
#if URDE_UV_SLOTS
layout(location=2 + URDE_COL_SLOTS) in vec2 uvIn[URDE_UV_SLOTS];
#endif
#if URDE_WEIGHT_SLOTS
layout(location=2 + URDE_COL_SLOTS + URDE_UV_SLOTS) in vec4 weightIn[URDE_WEIGHT_SLOTS];
#endif

SBINDING(0) out VertToFrag vtf;
void main() {
#if URDE_SKIN_SLOTS
  vec4 objPos = vec4(0.0,0.0,0.0,0.0);
  vec4 objNorm = vec4(0.0,0.0,0.0,0.0);
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
  URDE_TCG_EXPR
}
