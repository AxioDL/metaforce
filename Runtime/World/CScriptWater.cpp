#include "CScriptWater.hpp"
#include "CStateManager.hpp"

namespace urde
{

CScriptWater::CScriptWater(CStateManager& mgr, TUniqueId uid, const std::string& name,
                           const CEntityInfo& info, const zeus::CVector3f& pos, const zeus::CAABox& box,
                           const urde::CDamageInfo& dInfo, zeus::CVector3f& orientedForce, u32 triggerFlags,
                           bool b1, bool b2, ResId, ResId, ResId, ResId, ResId, ResId, ResId,
                           const zeus::CVector3f&, float, float, float, bool active, CFluidPlane::EFluidType,
                           bool, float, const CFluidUVMotion&,
                           float, float, float, float, float, float, float, float,
                           const zeus::CColor&, const zeus::CColor&, ResId, ResId, ResId, ResId,
                           ResId, s32, s32, s32, s32, s32, float, u32, float, float, float, float, float, float, float, float,
                           const zeus::CColor&, urde::ResId, float, float, float, u32, u32, bool, s32, s32, const u32*)
    : CScriptTrigger(uid, name, info, pos, box, dInfo, orientedForce, triggerFlags, active, false, false)
{
}

}
