#include "Runtime/Character/CSkinRules.hpp"

#include "Runtime/CToken.hpp"
#include "Runtime/Character/CPoseAsTransforms.hpp"
#include "Runtime/Graphics/CModel.hpp"

namespace urde {

CSkinRules::CSkinRules(CInputStream& in) {
  u32 bankCount = in.readUint32Big();
  x0_skinBanks.reserve(bankCount);
  for (u32 i = 0; i < bankCount; ++i)
    x0_skinBanks.emplace_back(in);

  u32 virtualBoneCount = in.readUint32Big();
  m_virtualBones.reserve(virtualBoneCount);
  for (u32 i = 0; i < virtualBoneCount; ++i)
    m_virtualBones.emplace_back(in);

  u32 poolSz = in.readUint32Big();
  m_poolToSkinIdx.reserve(poolSz);
  for (u32 i = 0; i < poolSz; ++i)
    m_poolToSkinIdx.push_back(in.readUint32Big());
}

void CSkinRules::TransformVerticesCPU(std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vnOut,
                                      const CPoseAsTransforms& pose, const CModel& model) const {
  vnOut.resize(m_poolToSkinIdx.size());
  for (size_t i = 0; i < m_poolToSkinIdx.size(); ++i) {
    const CVirtualBone& vb = m_virtualBones[m_poolToSkinIdx[i]];
    zeus::CVector3f origVertex = model.GetPoolVertex(i);
    zeus::CVector3f vertex;
    zeus::CVector3f origNormal = model.GetPoolNormal(i);
    zeus::CVector3f normal;
    for (const SSkinWeighting& w : vb.GetWeights()) {
      const zeus::CTransform& xf = pose.GetRestToAccumTransform(w.m_id);
      vertex += (xf * origVertex) * w.m_weight;
      normal += (xf.basis.inverted().transposed() * origNormal) * w.m_weight;
    }
    vnOut[i] = std::make_pair(vertex, normal.normalized());
  }
}

CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& params,
                                   CObjectReference* selfRef) {
  return TToken<CSkinRules>::GetIObjObjectFor(std::make_unique<CSkinRules>(in));
}

} // namespace urde
