#ifndef __PSHAG_CANIMCHARACTERSET_HPP__
#define __PSHAG_CANIMCHARACTERSET_HPP__

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
};

CFactoryFnReturn FAnimCharacterSet(const SObjectTag&, CInputStream&, const CVParamTransfer&);

}

#endif // __PSHAG_CANIMCHARACTERSET_HPP__
