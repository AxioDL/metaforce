#include "Runtime/Graphics/CVertexMorphEffect.hpp"

#include "Runtime/Character/CSkinRules.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"

namespace metaforce {

CVertexMorphEffect::CVertexMorphEffect(const zeus::CUnitVector3f& dir, const zeus::CVector3f& pos, float duration,
                                       float diagExtent, CRandom16& random)
: x0_dir(dir), xc_pos(pos), x18_duration(duration), x20_diagExtent(diagExtent), x24_random(random) {}

void CVertexMorphEffect::MorphVertices(SSkinningWorkspace& workspace, TConstVectorRef averagedNormals,
                                       TLockedToken<CSkinRules>& skinRules, const CPoseAsTransforms& pose,
                                       u32 vertexCount) {
  if (x28_indices.empty()) {
    std::vector<aurora::Vec3<float>> normalsOut;
    normalsOut.reserve(vertexCount);
    skinRules->BuildNormals(averagedNormals, &normalsOut);
    for (int i = 0; i < vertexCount; ++i) {
      const auto& nov = normalsOut[i];
      const zeus::CVector3f noz{nov.x, nov.y, nov.z};
      float dist = noz.dot(x0_dir);
      if (dist > 0.5f) {
        x28_indices.emplace_back(i);
        const auto vert = workspace.m_vertexWorkspace[i];
        const auto length = vert.x + vert.y + vert.z;
        x38_floats.emplace_back((length - std::trunc(length)) * (dist - 0.5f));
      }
    }
  }
  for (int i = 0; i < x28_indices.size(); ++i) {
    const auto scale = x1c_elapsed / x18_duration;
    auto& out = workspace.m_vertexWorkspace[x28_indices[i]];
    const auto add = scale * x20_diagExtent * x38_floats[i] * x0_dir;
    out.x += add.x();
    out.y += add.y();
    out.z += add.z();
  }
}

void CVertexMorphEffect::Reset(const zeus::CVector3f& dir, const zeus::CVector3f& pos, float duration) {
  x0_dir = dir;
  xc_pos = pos;
  x18_duration = duration;
  x1c_elapsed = 0.f;
  x28_indices.clear();
  x38_floats.clear();
}

void CVertexMorphEffect::Update(float dt) { x1c_elapsed = std::min(x1c_elapsed + dt, x18_duration); }

} // namespace metaforce
