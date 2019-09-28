#pragma once

#include <utility>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/Character/CPoseAsTransforms.hpp"

#include <zeus/CUnitVector.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CRandom16;
class CSkinRules;

class CVertexMorphEffect {
  zeus::CUnitVector3f x0_;
  float xc_ = 0.f;
  float x10_ = 0.f;
  float x14_ = 0.f;
  float x18_ = 0.f;
  float x1c_ = 0.f;
  float x20_diagExtent;
  CRandom16& x24_random;
  std::vector<u32> x28_;
  std::vector<u32> x38_;

public:
  CVertexMorphEffect(const zeus::CUnitVector3f& v1, const zeus::CVector3f& v2, float diagExtent, float f2,
                     CRandom16& random);
  void MorphVertices(std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn, const float* magnitudes,
                     const TLockedToken<CSkinRules>& skinRules, const CPoseAsTransforms& pose) const;
  void Reset(const zeus::CVector3f& dir, const zeus::CVector3f& pos, float duration) {}
  void Update(float) {}
};

} // namespace urde
