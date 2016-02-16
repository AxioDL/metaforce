#include "CEmitterElement.hpp"

namespace pshag
{

bool CEESimpleEmitter::GetValue(int frame, Zeus::CVector3f& pPos, Zeus::CVector3f& pVel) const
{
    return false;
}

bool CVESphere::GetValue(int frame, Zeus::CVector3f& pPos, Zeus::CVector3f& pVel) const
{
    return false;
}

bool CVEAngleSphere::GetValue(int frame, Zeus::CVector3f& pPos, Zeus::CVector3f& pVel) const
{
    return false;
}

}
