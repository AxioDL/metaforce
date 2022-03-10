#include "Runtime/Graphics/CVertexMorphEffect.hpp"

#include "Runtime/Character/CSkinRules.hpp"

namespace metaforce {

CVertexMorphEffect::CVertexMorphEffect(const zeus::CUnitVector3f& v1, const zeus::CVector3f& v2, float diagExtent,
                                       float f2, CRandom16& random)
: x0_(v1), x20_diagExtent(diagExtent), x24_random(random) {}

void CVertexMorphEffect::MorphVertices(SSkinningWorkspace& workspace, TConstVectorRef magnitudes,
                                       const TLockedToken<CSkinRules>& skinRules, const CPoseAsTransforms& pose) const {
}

} // namespace metaforce
