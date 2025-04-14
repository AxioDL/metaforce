#pragma once

#include <vector>

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/Character/CSegId.hpp"
#include "Runtime/Graphics/CCubeModel.hpp"
#include "Runtime/RetroTypes.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
class CCharLayoutInfo;
class CPoseAsTransforms;
class CModel;

struct SSkinWeighting {
  CSegId x0_id;
  float x4_weight;
  explicit SSkinWeighting(CInputStream& in) : x0_id(in), x4_weight(in.ReadFloat()) {}
};

class CVirtualBone {
  friend class CSkinnedModel;

  rstl::reserved_vector<SSkinWeighting, 3> x0_weights;
  u32 x1c_vertexCount;
  zeus::CTransform x20_xf;
  zeus::CMatrix3f x50_rotation;

public:
  explicit CVirtualBone(CInputStream& in);

  void BuildPoints(const aurora::Vec3<float>* in, TVectorRef out, u32 count) const;
  void BuildNormals(const aurora::Vec3<float>* in, TVectorRef out, u32 count) const;
  void BuildAccumulatedTransform(const CPoseAsTransforms& pose, const zeus::CVector3f* points);

  [[nodiscard]] const auto& GetWeights() const { return x0_weights; }
  [[nodiscard]] u32 GetVertexCount() const { return x1c_vertexCount; }

private:
  void BuildFinalPosMatrix(const CPoseAsTransforms& pose, const zeus::CVector3f* points);
};

class CSkinRules {
  friend class CSkinnedModel;

  std::vector<CVirtualBone> x0_bones;
  u32 x10_vertexCount = 0;
  u32 x14_normalCount = 0;

public:
  explicit CSkinRules(CInputStream& in);

  void BuildPoints(TConstVectorRef positions, TVectorRef out);
  void BuildNormals(TConstVectorRef normals, TVectorRef out);
  void BuildAccumulatedTransforms(const CPoseAsTransforms& pose, const CCharLayoutInfo& info);

  [[nodiscard]] u32 GetVertexCount() const { return x10_vertexCount; }
  [[nodiscard]] u32 GetNormalCount() const { return x14_normalCount; }
};

CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& params,
                                   CObjectReference* selfRef);

} // namespace metaforce
