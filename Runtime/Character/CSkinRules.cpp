#include "Runtime/Character/CSkinRules.hpp"

#include "Runtime/CToken.hpp"
#include "Runtime/Character/CPoseAsTransforms.hpp"
#include "Runtime/Graphics/CModel.hpp"

namespace metaforce {

static u32 ReadCount(CInputStream& in) {
  u32 result = in.ReadLong();
  if (result == UINT32_MAX) {
    return in.ReadLong();
  }
  u8 junk[784];
  for (u32 i = 0; i < (result * 3); ++i) {
    u32 iVar2 = ((result * 3) - i);
    iVar2 = 192 < iVar2 ? 192 : iVar2;
    in.Get(junk, iVar2 * 4);
  }
  return result;
}

CSkinRules::CSkinRules(CInputStream& in) {
  u32 weightCount = in.ReadLong();
  x0_bones.reserve(weightCount);
  for (int i = 0; i < weightCount; ++i) {
    x0_bones.emplace_back(in);
  }
  x10_vertexCount = ReadCount(in);
  x14_normalCount = ReadCount(in);
}

// void CSkinRules::TransformVerticesCPU(std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vnOut,
//                                       const CPoseAsTransforms& pose, const CModel& model) const {
//   OPTICK_EVENT();
//   vnOut.resize(m_poolToSkinIdx.size());
//   for (size_t i = 0; i < m_poolToSkinIdx.size(); ++i) {
//     const CVirtualBone& vb = m_virtualBones[m_poolToSkinIdx[i]];
//     zeus::CVector3f origVertex = model.GetPoolVertex(i);
//     zeus::CVector3f vertex;
//     zeus::CVector3f origNormal = model.GetPoolNormal(i);
//     zeus::CVector3f normal;
//     for (const SSkinWeighting& w : vb.GetWeights()) {
//       const zeus::CTransform& xf = pose.GetRestToAccumTransform(w.m_id);
//       vertex += (xf * origVertex) * w.m_weight;
//       normal += (xf.basis.inverted().transposed() * origNormal) * w.m_weight;
//     }
//     vnOut[i] = std::make_pair(vertex, normal.normalized());
//   }
// }

CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& params,
                                   CObjectReference* selfRef) {
  return TToken<CSkinRules>::GetIObjObjectFor(std::make_unique<CSkinRules>(in));
}

auto StreamInSkinWeighting(CInputStream& in) {
  rstl::reserved_vector<SSkinWeighting, 3> weights;
  u32 weightCount = in.ReadLong();
  for (int i = 0; i < std::min(3u, weightCount); ++i) {
    weights.emplace_back(in);
  }
  for (int i = 3; i < weightCount; ++i) {
    SSkinWeighting{in};
  }
  return weights;
}

CVirtualBone::CVirtualBone(CInputStream& in)
: x0_weights(StreamInSkinWeighting(in)), x1c_vertexCount(in.ReadLong()) {}
} // namespace metaforce
