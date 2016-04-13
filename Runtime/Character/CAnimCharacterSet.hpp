#ifndef __URDE_CANIMCHARACTERSET_HPP__
#define __URDE_CANIMCHARACTERSET_HPP__

#include "CFactoryMgr.hpp"
#include "CCharacterSet.hpp"
#include "CAnimationSet.hpp"

namespace urde
{

class CAnimCharacterSet
{
    u16 x0_version;
    CCharacterSet x4_characterSet;
    CAnimationSet x1c_animationSet;
public:
    CAnimCharacterSet(CInputStream& in);
    const CCharacterSet& GetCharacterSet() const {return x4_characterSet;}
    const CAnimationSet& GetAnimationSet() const {return x1c_animationSet;}
};

CFactoryFnReturn FAnimCharacterSet(const SObjectTag&, CInputStream&, const CVParamTransfer&);

}

#endif // __URDE_CANIMCHARACTERSET_HPP__
