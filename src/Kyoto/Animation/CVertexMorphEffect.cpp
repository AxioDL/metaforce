#include "Kyoto/Animation/CVertexMorphEffect.hpp"

#include "Kyoto/Animation/CSkinRules.hpp"

#include <rstl/math.hpp>

void CVertexMorphEffect::Update(float dt) {
  mElapsed = rstl::min_val(mElapsed + dt, mDuration);
}

void CVertexMorphEffect::MorphVertices(CVector3f* workspace, const CVector3f* averageNormals,
                                       const TLockedToken< CSkinRules >& skinRules,
                                       const CPoseAsTransforms& pose, uint vertexCount) const {
  if (mIndices.empty()) {
    // We want to build the vertex buffer, but not construct them just yet.
    CVector3f* out = reinterpret_cast< CVector3f* >(rs_new uchar[vertexCount * sizeof(CVector3f)]);
    skinRules->BuildNormalsFrom(averageNormals, out);
    rstl::vector< uint >& indices = mIndices;
    uint count = static_cast< float >(vertexCount) * 0.5f;
    indices.reserve(count);
    rstl::vector< float >& weights = mFloats;
    weights.reserve(count);

    for (uint i = 0; i < vertexCount; ++i) {
      float thisMag = CVector3f::Dot(out[i], mDir);
      if (thisMag > 0.5f) {
        const CVector3f& vec = workspace[i];
        indices.push_back(i);
        const float sum = vec.GetX() + vec.GetY() + vec.GetZ();
        const float fraction = sum - static_cast< int >(sum);
        weights.push_back(fraction * (thisMag - 0.5f));
      }
    }

    if (out) {
      delete[] reinterpret_cast< uchar* >(out);
    }
  }

  const uint count = mIndices.size();
  for (uint i = 0; i < count; ++i) {
    workspace[mIndices[i]] +=
        (mElapsed / mDuration) * (mDiagExtent * (mFloats[i] * mDir));
  }
}

void CVertexMorphEffect::Reset(const CUnitVector3f& dir, const CVector3f& pos, float duration) {
  mDir = dir;
  mPos = pos;
  mDuration = duration;
  mElapsed = 0.f;
  mIndices.clear();
  mFloats.clear();
}
