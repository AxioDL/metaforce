#ifndef URDE_CMODELPOINTSELECTOR_HPP
#define URDE_CMODELPOINTSELECTOR_HPP

#include "RetroTypes.hpp"

namespace urde
{

class CModelPointSelector
{
public:
    CModelPointSelector(const zeus::CVector3f& scale, int, int, float, float);
    void GeneratePoints(const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn);
};

}

#endif // URDE_CMODELPOINTSELECTOR_HPP
