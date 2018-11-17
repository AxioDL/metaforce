#include <Runtime/World/CPatternedInfo.hpp>
#include "CFireFlea.hpp"
#include "CStateManager.hpp"
#include "CPlayerState.hpp"
#include "World/CWorld.hpp"
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

s32 CFireFlea::sLightIdx = 0;

CFireFlea::CFireFlea(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo, float f1)
    : CPatterned(ECharacter::FireFlea, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
                 EMovementType::Flyer, EColliderType::One, EBodyType::NewFlyer, actParms, EKnockBackVariant::Small)
 , x56c_(f1)
 , xd8c_pathFind(nullptr, 2, pInfo.GetParticle1Frames(), 1.f, 1.f)
{
    CMaterialFilter filter = GetMaterialFilter();
    filter.ExcludeList().Add(EMaterialTypes::Character);
    SetMaterialFilter(filter);

    ModelData()->AnimationData()->SetParticleLightIdx(sLightIdx);
    ++sLightIdx;
}

void CFireFlea::Dead(CStateManager& mgr, EStateMsg msg, float dt)
{
    if (msg != EStateMsg::Activate || mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::Thermal)
        return;

    mgr.AddObject(new CDeathCameraEffect(mgr.AllocateUniqueId(), GetAreaIdAlways(), ""sv));
}

bool CFireFlea::Delay(CStateManager&, float arg)
{
    return x330_stateMachineState.GetTime() > 0.5f;
}

void CFireFlea::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CFireFlea::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CPatterned::AcceptScriptMsg(msg, uid, mgr);

    if (msg == EScriptObjectMessage::Registered)
    {
        x450_bodyController->Activate(mgr);
    }
    else if (msg == EScriptObjectMessage::InitializedInArea)
    {
        xd8c_pathFind.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
        xe64_ = 50.f;
    }
}

bool CFireFlea::InPosition(CStateManager& mgr, float dt)
{
    if (x2dc_destObj == kInvalidUniqueId)
        return false;
    return (xd80_ - GetTranslation()).magnitude() < 25.f;
}
}