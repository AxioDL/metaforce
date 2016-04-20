#ifndef __URDE_CSCRIPTWATER_HPP__
#define __URDE_CSCRIPTWATER_HPP__

#include "CScriptTrigger.hpp"
#include "CFluidPlane.hpp"

namespace urde
{

class CDamageInfo;
class CFluidUVMotion;

class CScriptWater : public CScriptTrigger
{
public:
    CScriptWater(CStateManager &,TUniqueId, const std::string& name, const CEntityInfo&, const zeus::CVector3f&,
                 const zeus::CAABox&, CDamageInfo const &, zeus::CVector3f&, u32, bool, bool,
                 ResId, ResId, ResId, ResId, ResId, ResId, ResId,
                 const zeus::CVector3f&, float, float, float, bool, CFluidPlane::EFluidType, bool, float, const CFluidUVMotion&,
                 float, float, float, float, float, float, float, float, const zeus::CColor&, const zeus::CColor&,
                 ResId, ResId, ResId, ResId, ResId, s32, s32, s32, s32, s32,
                 float, u32, float, float, float, float, float, float, float, float,
                 const zeus::CColor&, ResId, float, float, float, u32, u32, bool, s32, s32, const u32*);
};

}

#endif // __URDE_CSCRIPTWATER_HPP__
