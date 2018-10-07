#pragma once

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

