#ifndef __URDE_CPOWERBEAM_HPP__
#define __URDE_CPOWERBEAM_HPP__

#include "CGunWeapon.hpp"

namespace urde
{

class CPowerBeam : public CGunWeapon
{
public:
    CPowerBeam(u32, EWeaponType, TUniqueId, EMaterialTypes, const zeus::CVector3f&);
};

}

#endif // __URDE_CPOWERBEAM_HPP__
