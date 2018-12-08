#include "CVertexMorphEffect.hpp"
#include "Character/CSkinRules.hpp"

namespace urde {

CVertexMorphEffect::CVertexMorphEffect(const zeus::CUnitVector3f& v1, const zeus::CVector3f& v2, float diagExtent,
                                       float f2, CRandom16& random)
: x0_(v1), x20_diagExtent(diagExtent), x24_random(random) {}

void CVertexMorphEffect::MorphVertices(std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn,
                                       const float* magnitudes, const TLockedToken<CSkinRules>& skinRules,
                                       const CPoseAsTransforms& pose) const {}

} // namespace urde