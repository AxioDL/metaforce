#include <Runtime/MP1/World/CSpacePirate.hpp>
#include "CPatterned.hpp"
#include "Runtime/CStateManager.hpp"
#include "CPatternedInfo.hpp"
#include "TCastTo.hpp"
#include "CActorParameters.hpp"
#include "Character/CPASAnimParmData.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CPlayer.hpp"
#include "Weapon/CGameProjectile.hpp"
#include "Character/CAnimData.hpp"
#include "TCastTo.hpp"
#include "MP1/World/CSpacePirate.hpp"
#include "World/CStateMachine.hpp"

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
                       EKnockBackVariant kbVariant)
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
x3fc_flavor(flavor),
x460_knockBackController(kbVariant)
{
    x400_25_alive = true;
    x400_31_ = moveType == CPatterned::EMovementType::Flyer;
    x402_29_ = true;
    x402_30_ = x402_31_thawed = actorParms.HasThermalHeat();
    x403_25_ = true;
    x403_26_ = true;
    x404_contactDamage = pInfo.x34_contactDamageInfo;
    x424_damageWaitTime = pInfo.x50_damageWaitTime;
    x454_deathSfx = pInfo.xe8_deathSfx;
    x458_iceShatterSfx = pInfo.x134_iceShatterSfx;
    x4f4_ = pInfo.x100_;
    x4f8_ = pInfo.x104_;
    x4fc_ = pInfo.x108_;
    x508_colliderType = colliderType;
    x50c_thermalMag = actorParms.GetThermalMag();
    x514_ = pInfo.x110_particle1Scale;
    x540_ = pInfo.x124_particle2Scale;

    if (pInfo.x11c_particle1.IsValid())
        x520_ = { g_SimplePool->GetObj({FOURCC('PART'), pInfo.x11c_particle1})};

    if (pInfo.x120_electric.IsValid())
        x530_ = { g_SimplePool->GetObj({FOURCC('ELSC'), pInfo.x120_electric})};

    if (pInfo.x130_particle2.IsValid())
        x54c_ = { g_SimplePool->GetObj({FOURCC('PART'), pInfo.x130_particle2})};

    if (x404_contactDamage.GetRadius() > 0.f)
        x404_contactDamage.SetRadius(0.f);

    xe6_29_renderParticleDBInside = false;
    x402_27_ = !x64_modelData->HasModel(CModelData::EWhichModel::XRay);
    BuildBodyController(bodyType);
}

void CPatterned::Accept(urde::IVisitor& visitor)
{
    visitor.Visit(this);
}

void CPatterned::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CAi::AcceptScriptMsg(msg, uid, mgr);

    switch(msg)
    {
    case EScriptObjectMessage::Registered:
    {
        if (x508_colliderType != EColliderType::One)
        {
            CMaterialList include = GetMaterialFilter().GetIncludeList();
            CMaterialList exclude = GetMaterialFilter().GetExcludeList();
            include.Remove(EMaterialTypes::Character);
            exclude.Add(EMaterialTypes::Character);
            SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
        }

        if (HasModelData() && GetModelData()->HasAnimData() && GetModelData()->GetAnimationData()->GetIceModel())
        {
            const auto& baseAABB = GetBaseBoundingBox();
            float diagExtent = (baseAABB.max - baseAABB.min).magnitude() * 0.5f;
            x510_vertexMorph = std::make_shared<CVertexMorphEffect>(zeus::CVector3f::skRight,
                               zeus::CVector3f{}, diagExtent, 0.f, *mgr.GetActiveRandom());
        }

        xf8_25_angularEnabled = true;
        break;
    }
    case EScriptObjectMessage::OnFloor:
    {
        if (!x328_25_)
        {
            x150_momentum = {};
            AddMaterial(EMaterialTypes::GroundCollider, mgr);
        }
        x328_27_onGround = true;
        break;
    }
    case EScriptObjectMessage::Falling:
    {
        if (!x328_25_)
        {
            if (x450_bodyController->GetPercentageFrozen() == 0.f)
            {
                x150_momentum = {0.f, 0.f, -GetWeight()};
                RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
            }
        }
        x328_27_onGround = false;
        break;
    }
    case EScriptObjectMessage::Activate:
        x3a0_ = GetTranslation();
        break;
    case EScriptObjectMessage::Deleted:
        if (x330_stateMachineState.GetActorState() != nullptr)
            x330_stateMachineState.GetActorState()->CallFunc(mgr, *this, EStateMsg::Two, 0.f);
        break;
    case EScriptObjectMessage::Damage:
    {
        if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(uid))
        {
            const CDamageInfo& info = proj->GetDamageInfo();
            if (info.GetWeaponMode().GetType() == EWeaponType::Wave)
            {
                if (x460_knockBackController.x81_26_ && info.GetWeaponMode().IsComboed() && HealthInfo(mgr))
                {
                    x401_31_ = true;
                    KnockBack(GetTransform().frontVector(), mgr, info, EKnockBackType::One, false,
                              info.GetKnockBackPower());
                    x460_knockBackController.DeferKnockBack(EWeaponType::Wave);
                }
            }
            else if (info.GetWeaponMode().GetType() == EWeaponType::Plasma)
            {
                if (x460_knockBackController.x81_27_ && info.GetWeaponMode().IsCharged() && HealthInfo(mgr))
                {
                    KnockBack(GetTransform().frontVector(), mgr, info, EKnockBackType::One,
                              false, info.GetKnockBackPower());
                    x460_knockBackController.DeferKnockBack(EWeaponType::Plasma);
                }
            }
            if (mgr.GetPlayer().GetUniqueId() == proj->GetOwnerId())
                x400_24_ = true;
        }
        break;
    }
    case EScriptObjectMessage::InvulnDamage:
    {
        if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(uid))
        {
            if (proj->GetOwnerId() == mgr.GetPlayer().GetUniqueId())
                x400_24_ = true;
        }
        break;
    }
    default: break;
    }
}

void CPatterned::UpdateFrozenState(bool thawed)
{
    x402_31_thawed = thawed;
    if (x403_24_keepThermalVisorState)
        return;
    xe6_27_thermalVisorFlags = thawed ? 2 : 1;
}

void CPatterned::Think(float dt, CStateManager& mgr)
{
    if (!GetActive())
        return;

    if (x402_30_)
        UpdateFrozenState(x450_bodyController->GetPercentageFrozen() == 0.f);

    if (x64_modelData->GetAnimationData()->GetIceModel())
        x510_vertexMorph->Update(dt);

    if (x402_26_)
    {
        float froz = x450_bodyController->GetPercentageFrozen();
        if (froz > 0.8f)
            x400_29_ = true;
    }

    if (x400_25_alive && !x400_28_ && x400_29_)
    {
        if (x3e0_ > 0.f && x400_29_)
        {
            SendScriptMsgs(EScriptObjectState::UNKS4, mgr, EScriptObjectMessage::None);
            sub8007ab34(mgr);
        }
        else
        {
            SendScriptMsgs(EScriptObjectState::UNKS4, mgr, EScriptObjectMessage::None);
            sub8007ace8(mgr);
        }
    }
    else if (!x400_29_)
    {
        x3e0_ -= dt;
        if (x403_26_)
        {
            if (x330_stateMachineState.sub8007FB9C())
            {
                u32 unk = x330_stateMachineState.sub8007FB9C();
                bool isDead = false; //sub8007A454(unk, "Dead"sv);

                if (!isDead && x330_stateMachineState.x8_time > 15.f)
                    sub8007ace8(mgr);
            }
        }
    }

    sub8007a68c(dt, mgr);

    x3e4_ = HealthInfo(mgr)->GetHP();
    if (!x330_stateMachineState.x4_state)
        x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "Start"sv);

    zeus::CVector3f diffVec = x4e4_ - GetTranslation();
    if (!x328_25_)
        diffVec.z = 0.f;

    if (diffVec.magSquared() > (0.1f * dt))
        x4f0_ += dt;
    else
        x4f0_ = 0.f;

    if (x460_knockBackController.x81_26_ && x401_31_ && x402_24_)
        Shock(0.5f + mgr.GetActiveRandom()->Range(0.f, 0.5f), 0.2f);

    x402_24_ = x401_24_;
    x401_31_ = false;
    if (x450_bodyController->IsElectrocuting())
    {
        mgr.GetActorModelParticles()->StartElectric(*this);
        if (x3f0_ > 0.f && x400_25_alive)
        {
            CDamageInfo dInfo({EWeaponType::Wave}, x3f0_, 0.f, 0.f);
            mgr.ApplyDamage(kInvalidUniqueId, GetUniqueId(), kInvalidUniqueId, dInfo,
                            CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
        }
    }
    else
    {
        if (x3f0_!= 0.f)
        {
            //x450_bodyController->sub80139f0c(x3f0_);
            mgr.GetActorModelParticles()->StopElectric(*this);
        }
    }

    if (x450_bodyController->IsOnFire())
    {
        if (x400_25_alive)
        {
            mgr.GetActorModelParticles()->LightDudeOnFire(*this);
            CDamageInfo dInfo({EWeaponType::Wave}, x3f0_, 0.f, 0.f);
            mgr.ApplyDamage(kInvalidUniqueId, GetUniqueId(), kInvalidUniqueId, dInfo,
                            CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
        }
    }
    else if (x3ec_ > 0.f)
        x3ec_ = 0.f;


    //if (x450_bodyController->IsFrozen())
        //mgr.GetActorModelParticles()->sub801e5044(*this);

    if (!x401_27_ || x401_28_)
        x3e8_ = -0.33333334f;

    if (x401_30_)
    {
        x401_30_ = false;
        Death(GetTransform().frontVector(), mgr, EScriptObjectState::DeathRattle);
    }

    float thinkDt = (x400_25_alive ? dt : dt * sub80078a88());

    x450_bodyController->Update(thinkDt, mgr);
    x450_bodyController->MultiplyPlaybackRate(x3b4_speed);
    SAdvancementDeltas deltas = UpdateAnimation(thinkDt, mgr, !x450_bodyController->IsFrozen());
    x434_posDelta = deltas.x0_posDelta;
    x440_rotDelta = deltas.xc_rotDelta;

    if (x403_25_ && x450_bodyController->GetPercentageFrozen() < 1.f)
        x330_stateMachineState.Update(mgr, *this, thinkDt);


    ThinkAboutMove(thinkDt);

    x460_knockBackController.Update(thinkDt, mgr, *this);
    x4e4_ = GetTranslation() + PredictMotion(thinkDt).x0_translation;
    x328_26_ = false;
    if (x420_curDamageTime > 0.f)
        x420_curDamageTime -= dt;

    if (x401_28_ && x3f4_ > dt)
        x3f4_ -= dt;

    xd0_thermalMag = x50c_thermalMag;
    sub8007a5b8(dt);

    if (!x450_bodyController->IsFrozen())
    {
        if (x3a0_ == zeus::CVector3f())
            x3a0_ = GetTranslation();

        if (x3cc_playerLeashRadius != 0.f)
        {
            zeus::CVector3f diffVec = (GetTranslation() - mgr.GetPlayer().GetTranslation());
            if (diffVec.magSquared() > x3cc_playerLeashRadius)
                x3d4_curPlayerLeashTime += dt;
            else
                x3d4_curPlayerLeashTime = 0.f;
        }
    }
    else
        RemoveEmitter();

    if (x2f8_ > 0.f)
        x2f8_ -= dt;
}

void CPatterned::Touch(CActor& act, CStateManager& mgr)
{
    if (!x400_25_alive)
        return;

    if (TCastToPtr<CGameProjectile> proj = act)
    {
        if (mgr.GetPlayer().GetUniqueId() == proj->GetOwnerId())
            x400_24_ = true;
    }
}

std::experimental::optional<zeus::CAABox> CPatterned::GetTouchBounds() const
{
    return GetBoundingBox();
}

bool CPatterned::CanRenderUnsorted(const urde::CStateManager& mgr) const
{
    return x64_modelData->GetAnimationData()->GetParticleDB().AreAnySystemsDrawnWithModel() ? false :
           CActor::CanRenderUnsorted(mgr);
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

void CPatterned::KnockBack(const zeus::CVector3f& backVec, CStateManager& mgr, const CDamageInfo& info,
                           EKnockBackType type, bool inDeferred, float magnitude)
{
    CHealthInfo* hInfo = HealthInfo(mgr);
    if (!x401_27_ && !x401_28_ && hInfo)
    {
        x460_knockBackController.KnockBack(backVec, mgr, *this, info, type, magnitude);
        /* TODO: Finish */
    }
}

void CPatterned::BuildBodyController(EBodyType bodyType)
{
    if (x450_bodyController)
        return;

    x450_bodyController.reset(new CBodyController(*this, x3b8_turnSpeed, bodyType));
    auto anim = x450_bodyController->GetPASDatabase().FindBestAnimation(CPASAnimParmData(24,
        CPASAnimParm::FromEnum(0)), -1);
    /* TODO: Double check this */
     x460_knockBackController.x81_26_ = anim.first != 0.f;
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

float CPatterned::sub80078a88()
{
    float f0 = (x401_28_ ? x3f4_ / 1.f : 1.f);
    return zeus::max(0.1f, f0);
}

void CPatterned::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum)
{
#if 1
    if (mgr.GetPlayerState()->GetActiveVisor(mgr  ) == CPlayerState::EPlayerVisor::Thermal)
    {
        SetCalculateLighting(false);
        x90_actorLights->BuildConstantAmbientLighting(zeus::CColor::skWhite);
    }
    else
        SetCalculateLighting(true);

    zeus::CColor col = x42c_;
    u8 alpha = GetModelAlphau8(mgr);
    if (x402_27_ && mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay)
        alpha = 0x4C;

    if (alpha < 0xFF)
    {
        if (col.r == 0.f && col.g == 0.f && col.b == 0.f)
            col = zeus::CColor::skWhite;
    }

    if (x401_29_ && alpha > 0x7F)
    {

    }
#endif

    CActor::PreRender(mgr, frustum);

}

void CPatterned::ThinkAboutMove(float dt)
{
    bool doMove = true;
    if (!x328_25_ && ! x328_27_onGround)
    {
        x310_.zeroOut();
        doMove = false;
    }

    if (doMove && x39c_ < x390_)
    {
        zeus::CVector3f frontVec = GetTransform().frontVector();
        zeus::CVector3f x31cCpy = x31c_;
        if (x31c_.magSquared() > 0.1f)
            x31cCpy.normalize();
        float mag = frontVec.dot(x31cCpy);

        switch (x3f8_)
        {
        case 0:
            if (!x328_26_)
                break;
        case 1:
        {
            doMove = false;
            if (mag > 0.85)
            {
                doMove = true;
                x3f8_ = 2;
                break;
            }
            x3f8_ = 1;
        }
        case 2:
            x3f8_ = 3;
        case 3:
        {
            doMove = true;
            if (!x328_26_)
            {
                x3f8_ = 0;
                break;
            }
            if (mag > 0.89)
                x3f8_ = 4;
            break;
        }
        case 4:
        {
            x328_24_ = true;
            doMove = false;
            x3f8_ = 0;
        }
        default:
            break;
        }
    }

    if (!x401_26_ && doMove)
    {
        const CBodyState* state = x450_bodyController->GetBodyStateInfo().GetCurrentState();
        if (state->ApplyAnimationDeltas() && !zeus::close_enough(x2e0_destPos - GetTranslation(), {}))
            MoveToOR((x64_modelData->GetScale() * x434_posDelta) * x55c_, dt);
    }

    RotateToOR(x440_rotDelta, dt);
}
}
