#include "Runtime/Character/CSkinRules.hpp"

#include "Runtime/CToken.hpp"
#include "Runtime/Character/CPoseAsTransforms.hpp"
#include "Runtime/Graphics/CModel.hpp"

namespace metaforce {

static u32 ReadCount(CInputStream& in) {
  s32 result = in.ReadLong();
  if (result == -1) {
    return in.ReadLong();
  }
  u8 junk[784];
  u32 iVar2 = 0;
  for (u32 i = 0; i < (result * 3); i += iVar2) {
    iVar2 = ((result * 3) - i);
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

void CSkinRules::BuildAccumulatedTransforms(const CPoseAsTransforms& pose, const CCharLayoutInfo& info) {
  std::array<zeus::CVector3f, 100> points;
  CSegId segId = pose.GetLastInserted();
  while (segId != 0) {
    zeus::CVector3f origin;
    if (segId != 3) { // root ID
      origin = info.GetFromRootUnrotated(segId);
    }
    const auto rotatedOrigin = pose.GetRotation(segId) * origin;
    points[segId] = pose.GetOffset(segId) - rotatedOrigin;
    segId = pose.GetParent(segId);
  }
  for (auto& bone : x0_bones) {
    bone.BuildAccumulatedTransform(pose, points.data());
  }
}

void CSkinRules::BuildPoints(TConstVectorRef positions, TVectorRef out) {
  size_t offset = 0;
  for (auto& bone : x0_bones) {
    u32 vertexCount = bone.GetVertexCount();
    bone.BuildPoints(positions->data() + offset, out, vertexCount);
    offset += vertexCount;
  }
}

void CSkinRules::BuildNormals(TConstVectorRef normals, TVectorRef out) {
  size_t offset = 0;
  for (auto& bone : x0_bones) {
    u32 vertexCount = bone.GetVertexCount();
    bone.BuildNormals(normals->data() + offset, out, vertexCount);
    offset += vertexCount;
  }
}

CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& params,
                                   CObjectReference* selfRef) {
  return TToken<CSkinRules>::GetIObjObjectFor(std::make_unique<CSkinRules>(in));
}

static inline auto StreamInSkinWeighting(CInputStream& in) {
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

CVirtualBone::CVirtualBone(CInputStream& in) : x0_weights(StreamInSkinWeighting(in)), x1c_vertexCount(in.ReadLong()) {}

void CVirtualBone::BuildPoints(const zeus::CVector3f* in, TVectorRef out, u32 count) const {
  for (u32 i = 0; i < count; ++i) {
    out->emplace_back(x20_xf * in[i]);
  }
}

void CVirtualBone::BuildNormals(const zeus::CVector3f* in, TVectorRef out, u32 count) const {
  for (u32 i = 0; i < count; ++i) {
    out->emplace_back(x50_rotation * in[i]);
  }
}

void CVirtualBone::BuildAccumulatedTransform(const CPoseAsTransforms& pose, const zeus::CVector3f* points) {
  BuildFinalPosMatrix(pose, points);
  x50_rotation = pose.GetRotation(x0_weights[0].x0_id);
}

static inline zeus::CMatrix3f WeightedMatrix(const zeus::CMatrix3f& m1, float w1, const zeus::CMatrix3f& m2, float w2) {
  return {
      m1[0] * w1 + m2[0] * w2,
      m1[1] * w1 + m2[1] * w2,
      m1[2] * w1 + m2[2] * w2,
  };
}

static inline zeus::CVector3f WeightedVector(const zeus::CVector3f& v1, float w1, const zeus::CVector3f& v2, float w2) {
  return v1 * w1 + v2 * w2;
}

void CVirtualBone::BuildFinalPosMatrix(const CPoseAsTransforms& pose, const zeus::CVector3f* points) {
  if (x0_weights.size() == 1) {
    const auto id = x0_weights[0].x0_id;
    x20_xf = {pose.GetRotation(id), points[id]};
  } else if (x0_weights.size() == 2) {
    const auto w0 = x0_weights[0];
    const auto w1 = x0_weights[1];
    x20_xf = {
        WeightedMatrix(pose.GetRotation(w0.x0_id), w0.x4_weight, pose.GetRotation(w1.x0_id), w1.x4_weight),
        WeightedVector(points[w0.x0_id], w0.x4_weight, points[w1.x0_id], w1.x4_weight),
    };
  } else if (x0_weights.size() == 3) {
    const auto w0 = x0_weights[0];
    const auto w1 = x0_weights[1];
    const auto w2 = x0_weights[2];
    auto rot = WeightedMatrix(pose.GetRotation(w0.x0_id), w0.x4_weight, pose.GetRotation(w1.x0_id), w1.x4_weight);
    auto pos = WeightedVector(points[w0.x0_id], w0.x4_weight, points[w1.x0_id], w1.x4_weight);
    pose.AccumulateScaledTransform(w2.x0_id, rot, w2.x4_weight);
    x20_xf = {rot, pos + points[w2.x0_id] * w2.x4_weight};
  } else {
    x20_xf = {};
  }
}
} // namespace metaforce
