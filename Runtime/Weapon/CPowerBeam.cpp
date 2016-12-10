#include "CPowerBeam.hpp"

namespace urde
{
CPowerBeam::CPowerBeam(u32 w1, EWeaponType wType, TUniqueId uid, EMaterialTypes mType, const zeus::CVector3f& vec)
    : CGunWeapon(w1, wType, uid, mType, vec)
{
}
}
