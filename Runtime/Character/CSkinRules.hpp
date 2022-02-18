#pragma once

#include <vector>

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/Character/CSkinBank.hpp"
#include "Runtime/RetroTypes.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
class CPoseAsTransforms;
class CModel;

struct SSkinWeighting {
  CSegId x0_id;
  float x4_weight;
  explicit SSkinWeighting(CInputStream& in) : x0_id(in), x4_weight(in.ReadFloat()) {}
};

class CVirtualBone {
  rstl::reserved_vector<SSkinWeighting, 3> x0_weights;
  u32 x1c_vertexCount;
  zeus::CTransform x20_xf;
  zeus::CMatrix3f x50_mtx;

public:
  explicit CVirtualBone(CInputStream& in);

  [[nodiscard]] const rstl::reserved_vector<SSkinWeighting, 3>& GetWeights() const { return x0_weights; }
};

class CSkinRules {
  std::vector<CVirtualBone> x0_bones;
  u32 x10_vertexCount;
  u32 x14_normalCount;

public:
  explicit CSkinRules(CInputStream& in);

//  void GetBankTransforms(std::vector<const zeus::CTransform*>& out, const CPoseAsTransforms& pose,
//                         int skinBankIdx) const {
//    // FIXME: This is definitely not proper behavior, this is here to fix the phazon suit crashing
//    if (x0_skinBanks.size() <= skinBankIdx) {
//      return;
//    }
//    x0_skinBanks[skinBankIdx].GetBankTransforms(out, pose);
//  }

//  void TransformVerticesCPU(std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vnOut,
//                            const CPoseAsTransforms& pose, const CModel& model) const;
};

CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& params,
                                   CObjectReference* selfRef);

} // namespace metaforce
