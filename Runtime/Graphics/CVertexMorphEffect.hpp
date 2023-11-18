#pragma once

#include <utility>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/Character/CPoseAsTransforms.hpp"
#include "Runtime/Graphics/CCubeModel.hpp"

#include <zeus/CUnitVector.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CRandom16;
class CSkinRules;
struct SSkinningWorkspace;

class CVertexMorphEffect {
  zeus::CUnitVector3f x0_dir;
  zeus::CVector3f xc_pos;
  float x18_duration;
  float x1c_elapsed = 0.f;
  float x20_diagExtent;
  CRandom16& x24_random;
  std::vector<u32> x28_indices;
  std::vector<float> x38_floats;

public:
  CVertexMorphEffect(const zeus::CUnitVector3f& dir, const zeus::CVector3f& pos, float duration, float diagExtent,
                     CRandom16& random);
  void MorphVertices(SSkinningWorkspace& workspace, TConstVectorRef averagedNormals,
                     TLockedToken<CSkinRules>& skinRules, const CPoseAsTransforms& pose, u32 vertexCount);
  void Reset(const zeus::CVector3f& dir, const zeus::CVector3f& pos, float duration);
  void Update(float dt);
};

} // namespace metaforce
