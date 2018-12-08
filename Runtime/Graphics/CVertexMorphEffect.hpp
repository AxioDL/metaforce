#pragma once

#include "CToken.hpp"
#include "Character/CPoseAsTransforms.hpp"

namespace urde {
class CSkinRules;
class CRandom16;

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
