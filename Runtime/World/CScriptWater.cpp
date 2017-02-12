#include "CScriptWater.hpp"
#include "CStateManager.hpp"

namespace urde
{

const float CScriptWater::kSplashScales[6] =
{
    1.0f, 3.0f, 0.709f,
    1.19f, 0.709f, 1.f
};

CScriptWater::CScriptWater(CStateManager& mgr, TUniqueId uid, const std::string& name, const CEntityInfo& info,
                           const zeus::CVector3f& pos, const zeus::CAABox& box, const urde::CDamageInfo& dInfo,
                           zeus::CVector3f& orientedForce, ETriggerFlags triggerFlags, bool b1, bool b2, ResId, ResId, ResId,
                           ResId, ResId, ResId, ResId, const zeus::CVector3f&, float, float, float, bool active,
                           CFluidPlane::EFluidType, bool, float, const CFluidUVMotion&, float, float, float, float,
                           float, float, float, float, const zeus::CColor&, const zeus::CColor&, ResId, ResId, ResId,
                           ResId, ResId, s32, s32, s32, s32, s32, float, u32, float, float, float, float, float, float,
                           float, float, const zeus::CColor&, urde::ResId, float, float, float, u32, u32, bool, s32,
                           s32, const u32*)
: CScriptTrigger(uid, name, info, pos, box, dInfo, orientedForce, triggerFlags, active, false, false)
{
}

void CScriptWater::Think(float dt, CStateManager& mgr) { CScriptTrigger::Think(dt, mgr); }

void CScriptWater::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr)
{
    CScriptTrigger::AcceptScriptMsg(msg, other, mgr);
}

void CScriptWater::PreRender(CStateManager &, const zeus::CFrustum &) {}

void CScriptWater::AddToRenderer(const zeus::CFrustum& /*frustum*/, const CStateManager& mgr) const
{
    zeus::CPlane plane;
    plane.vec = x34_transform.origin.normalized();
    plane.d = x34_transform.origin.z + x130_bounds.max.z;
    zeus::CAABox renderBounds = GetSortingBounds(mgr);
    mgr.AddDrawableActorPlane(*this, plane, renderBounds);
}

void CScriptWater::Render(const CStateManager&) const {}

void CScriptWater::Touch(CActor&, CStateManager&)
{
}

void CScriptWater::CalculateRenderBounds()
{

}

zeus::CAABox CScriptWater::GetSortingBounds(const CStateManager&) const
{
    return {};
}

EWeaponCollisionResponseTypes CScriptWater::GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&, CWeaponMode&, int)
{
    return EWeaponCollisionResponseTypes::Water;
}

s16 CScriptWater::GetSplashSound(float dt) const
{
    return x29c_splashSounds[GetSplashIndex(dt)];
}

const TLockedToken<CParticleGen>& CScriptWater::GetSplashEffect(float dt) const
{
    return x268_splashEffects[GetSplashIndex(dt)];
}

float CScriptWater::GetSplashEffectScale(float dt) const
{
    if (std::fabs(dt - 1.f) < 0.00001f)
        return kSplashScales[5];

    u32 idx = GetSplashIndex(dt);
    float s = dt - zeus::floorF(dt * 3.f);
    return ((1.f - s) * (s * kSplashScales[idx * 2])) + kSplashScales[idx];
}

u32 CScriptWater::GetSplashIndex(float dt) const
{
    u32 idx = dt * 3.f;
    return (idx < 3 ? idx : idx - 1);
}
}
