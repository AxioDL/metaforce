#ifndef __URDE_CVERTEXMORPHEFFECT_HPP__
#define __URDE_CVERTEXMORPHEFFECT_HPP__

#include "CToken.hpp"
#include "Character/CPoseAsTransforms.hpp"

namespace urde
{
class CSkinRules;

class CVertexMorphEffect
{
    u32 x0_ = 0;
    u32 x4_ = 1;
public:
    void MorphVertices(std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn,
                       const float* magnitudes, const TLockedToken<CSkinRules>& skinRules,
                       const CPoseAsTransforms& pose) const;

    void Update(float) {}
};

}

#endif // __URDE_CVERTEXMORPHEFFECT_HPP__
