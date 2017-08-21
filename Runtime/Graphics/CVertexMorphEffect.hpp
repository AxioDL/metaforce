#ifndef __URDE_CVERTEXMORPHEFFECT_HPP__
#define __URDE_CVERTEXMORPHEFFECT_HPP__

#include "CToken.hpp"
#include "Character/CPoseAsTransforms.hpp"

namespace urde
{
class CSkinRules;

class CVertexMorphEffect
{
public:
    void MorphVertices(std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn,
                       const float* magnitudes, const TLockedToken<CSkinRules>& skinRules,
                       const CPoseAsTransforms& pose) const;
};

}

#endif // __URDE_CVERTEXMORPHEFFECT_HPP__
