#include <Runtime/CStateManager.hpp>
#include "CPatterned.hpp"
#include "CPatternedInfo.hpp"
#include "TCastTo.hpp"
#include "CActorParameters.hpp"
#include "Character/CPASAnimParmData.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CPlayer.hpp"
#include "Weapon/CGameProjectile.hpp"
#include "Character/CAnimData.hpp"

namespace urde
{

const zeus::CColor CPatterned::skDamageColor(0.5f, 0.f, 0.f);
CMaterialList gkPatternedGroundMaterialList(EMaterialTypes::Character, EMaterialTypes::Solid,
                                            EMaterialTypes::Orbit, EMaterialTypes::GroundCollider,
                                            EMaterialTypes::Target);
CMaterialList gkPatternedFlyerMaterialList(EMaterialTypes::Character, EMaterialTypes::Solid,
                                           EMaterialTypes::Orbit, EMaterialTypes::Target);

CPatterned::CPatterned(ECharacter character, TUniqueId uid, std::string_view name, CPatterned::EFlavorType flavor,
                       const CEntityInfo& info, const zeus::CTransform& xf, CModelData&& mData,
                       const CPatternedInfo& pInfo, CPatterned::EMovementType moveType,
                       CPatterned::EColliderType colliderType, EBodyType bodyType, const CActorParameters& actorParms,
                       int variant)
: CAi(uid, pInfo.xf8_active, name, info, xf, std::move(mData),
      zeus::CAABox(pInfo.xcc_bodyOrigin - zeus::CVector3f{pInfo.xc4_halfExtent, pInfo.xc4_halfExtent, 0.f},
                   pInfo.xcc_bodyOrigin +
                       zeus::CVector3f{pInfo.xc4_halfExtent, pInfo.xc4_halfExtent, pInfo.xc8_height}),
      pInfo.x0_mass, pInfo.x54_healthInfo, pInfo.x5c_damageVulnerability,
      moveType == EMovementType::Flyer ? gkPatternedFlyerMaterialList : gkPatternedGroundMaterialList,
      pInfo.xfc_stateMachineId, actorParms, pInfo.xd8_stepUpHeight, 0.8f),
x2fc_minAttackRange(pInfo.x18_minAttackRange),
x300_maxAttackRange(pInfo.x1c_maxAttackRange),
x304_averageAttackTime(pInfo.x20_averageAttackTime),
x308_attackTimeVariation(pInfo.x24_attackTimeVariation),
x34c_character(character),
x388_anim(pInfo.GetAnimationParameters().GetInitialAnimation()),
x3b4_speed(pInfo.x4_speed),
x3b8_turnSpeed(pInfo.x8_turnSpeed),
x3bc_detectionRange(pInfo.xc_detectionRange),
x3c0_detectionHeightRange(pInfo.x10_detectionHeightRange),
x3c4_detectionAngle(std::cos(zeus::degToRad(pInfo.x14_dectectionAngle))),
x3c8_leashRadius(pInfo.x28_leashRadius),
x3cc_playerLeashRadius(pInfo.x2c_playerLeashRadius),
x3d0_playerLeashTime(pInfo.x30_playerLeashTime),
x3fc_flavor(flavor)
{
    x400_25_ = true;
    x400_31_ = moveType == CPatterned::EMovementType::Flyer;
    x402_29_ = true;
    x402_30_ = x402_31_ = actorParms.HasThermalHeat();
    x403_25_ = true;
    x403_26_ = true;
    x404_ = pInfo.x34_damageInfo;
    x424_damageWaitTime = pInfo.x50_damageWaitTime;
    x454_deathSfx = pInfo.xe8_deathSfx;
    x458_iceShatterSfx = pInfo.x134_iceShatterSfx;
    x4f4_ = pInfo.x100_;
    x4f8_ = pInfo.x104_;
    x4fc_ = pInfo.x108_;
    x508_colliderType = colliderType;
    x50c_thermalMag = actorParms.GetThermalMag();
    x510_.reset(new CVertexMorphEffect);
    x514_ = pInfo.x110_particle1Scale;
    x540_ = pInfo.x124_particle2Scale;

    if (pInfo.x11c_particle1.IsValid())
        x520_ = { g_SimplePool->GetObj({FOURCC('PART'), pInfo.x11c_particle1})};

    if (pInfo.x120_electric.IsValid())
        x530_ = { g_SimplePool->GetObj({FOURCC('ELSC'), pInfo.x120_electric})};

    if (pInfo.x130_particle2.IsValid())
        x54c_ = { g_SimplePool->GetObj({FOURCC('PART'), pInfo.x130_particle2})};

    if (x404_.GetRadius() > 0.f)
        x404_.SetRadius(0.f);

    xe6_29_renderParticleDBInside = false;
    x402_27_ = x64_modelData->HasModel(CModelData::EWhichModel::XRay);
    BuildBodyController(bodyType);
}

void CPatterned::Think(float dt, urde::CStateManager& mgr)
{
    if (!GetActive())
        return;

#if 0
    if (x402_30_)
        sub80077854(x450_bodyController->GetPercentageFrozen() == 0.f);
#endif

    if (x64_modelData->GetAnimationData()->GetIceModel())
        x510_->Update(dt);

    if (x402_26_)
    {
        float froz = x450_bodyController->GetPercentageFrozen();
        if (froz > 0.8f)
            x400_29_ = true;
    }
}

void CPatterned::Touch(CActor& act, CStateManager& mgr)
{
    if (!x400_25_)
        return;

    if (TCastToPtr<CGameProjectile> proj = act)
    {
        if (mgr.GetPlayer().GetUniqueId() == proj->GetOwnerId())
            x400_24_ = true;
    }
}

zeus::CVector3f CPatterned::GetAimPosition(const urde::CStateManager& mgr, float dt) const
{
    zeus::CVector3f offset;
     if (dt > 0.f)
         offset = PredictMotion(dt).x0_translation;

     CSegId segId = GetModelData()->GetAnimationData()->GetLocatorSegId("lockon_target_LCTR"sv);
     if (segId != 0xFF)
     {
         zeus::CTransform xf = GetModelData()->GetAnimationData()->GetLocatorTransform(segId, nullptr);
         zeus::CVector3f scaledOrigin = GetModelData()->GetScale() * xf.origin;
         if (GetTouchBounds())
            return offset + GetTouchBounds()->clampToBox(x34_transform * scaledOrigin);

         zeus::CAABox aabox = GetBaseBoundingBox();

         zeus::CAABox primBox(aabox.min + GetPrimitiveOffset(), aabox.max + GetPrimitiveOffset());

         return offset + (x34_transform * primBox.clampToBox(scaledOrigin));
     }

    return offset + GetBoundingBox().center();
}

void CPatterned::BuildBodyController(EBodyType bodyType)
{
    if (x450_bodyController)
        return;

    x450_bodyController.reset(new CBodyController(*this, x3b8_turnSpeed, bodyType));
    auto anim = x450_bodyController->GetPASDatabase().FindBestAnimation(CPASAnimParmData(24,
        CPASAnimParm::FromEnum(0)), -1);
    /* TODO: Double check this */
     x460_.x81_26_ = anim.first != 0.f;
}

zeus::CVector3f CPatterned::GetGunEyePos() const
{
    zeus::CVector3f origin = GetOrigin();
    zeus::CAABox baseBox = GetBaseBoundingBox();
    origin.z = 0.6f * (baseBox.max.z - baseBox.min.z) + origin.z;

    return origin;
}

void CPatterned::SetupPlayerCollision(bool v)
{
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    CMaterialList exclude = GetMaterialFilter().GetExcludeList();
    CMaterialList* modList = (v ? &exclude : &include);
    modList->Add(EMaterialTypes::Player);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));

}

}
