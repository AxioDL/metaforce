#include "CFireFlea.hpp"
#include "CStateManager.hpp"
#include "CPlayerState.hpp"
#include "TCastTo.hpp"

namespace urde::MP1
{

//region Fire Flea Death Camera

const zeus::CColor CFireFlea::CDeathCameraEffect::skEndFadeColor{1.f, 1.f, 0.5f, 1.f};
const zeus::CColor CFireFlea::CDeathCameraEffect::skStartFadeColor{1.f, 0.f, 0.f, 0.f};
zeus::CColor CFireFlea::CDeathCameraEffect::sCurrentFadeColor = zeus::CColor::skClear;

CFireFlea::CDeathCameraEffect::CDeathCameraEffect(TUniqueId uid, TAreaId areaId, std::string_view name)
    : CEntity(uid, CEntityInfo(areaId, CEntity::NullConnectionList), true, name)
{
}

void CFireFlea::CDeathCameraEffect::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CFireFlea::CDeathCameraEffect::PreThink(float dt, CStateManager& mgr)
{
    CCameraFilterPassPoly& filterPass = mgr.GetCameraFilterPass(5);
    u32 r5 = x34_ + x38_;
    u32 r8 = r5 + x3c_;
    u32 r31 = r8 + x40_;
    if (x44_ >= x34_ && x44_ <= r5)
    {
        sCurrentFadeColor += zeus::CColor::lerp(skStartFadeColor, skEndFadeColor, x34_- x44_);
        filterPass.SetFilter(EFilterType::Blend, EFilterShape::Fullscreen, 0.f, sCurrentFadeColor, CAssetId());
    }
    else if (x44_ >= r8 && x44_ <= r31)
    {
        sCurrentFadeColor += zeus::CColor::lerp(skEndFadeColor, skStartFadeColor, r8 - x44_);
        filterPass.SetFilter(EFilterType::Blend, EFilterShape::Fullscreen, 0.f, sCurrentFadeColor, CAssetId());
    }
    else if (x44_ > r5)
    {
        sCurrentFadeColor = skEndFadeColor;
        filterPass.SetFilter(EFilterType::Blend, EFilterShape::Fullscreen, 0.f, sCurrentFadeColor, CAssetId());
    }

    if (r31 == x44_)
    {
        filterPass.DisableFilter(0.f);
        mgr.FreeScriptObject(GetUniqueId());
        x44_ = 0;
    } else
        x44_++;

    if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::Thermal)
        filterPass.DisableFilter(0.f);
}

void CFireFlea::CDeathCameraEffect::Think(float dt, CStateManager& mgr)
{
    sCurrentFadeColor = zeus::CColor::skClear;
}

//endregion

CFireFlea::CFireFlea(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo, float)
    : CPatterned(ECharacter::FireFlea, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
                 EMovementType::Flyer, EColliderType::One, EBodyType::NewFlyer, actParms, EKnockBackVariant::Small)
{

}
}