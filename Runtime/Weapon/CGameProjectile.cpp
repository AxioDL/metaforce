#include "Weapon/CGameProjectile.hpp"
#include "World/CGameLight.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
#include "World/CPlayer.hpp"
#include "World/CHUDBillboardEffect.hpp"
#include "World/CWallCrawlerSwarm.hpp"
#include "World/CScriptDoor.hpp"
#include "Collision/CInternalRayCastStructure.hpp"

namespace urde
{
CGameProjectile::CGameProjectile(bool active, const TToken<CWeaponDescription>& wDesc, std::string_view name,
                                 EWeaponType wType, const zeus::CTransform& xf, EMaterialTypes matType,
                                 const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
                                 TUniqueId homingTarget, EProjectileAttrib attribs, bool underwater,
                                 const zeus::CVector3f& scale,
                                 const rstl::optional_object<TLockedToken<CGenDescription>>& visorParticle,
                                 u16 visorSfx, bool sendCollideMsg)
: CWeapon(uid, aid, active, owner, wType, name, xf,
          CMaterialFilter::MakeIncludeExclude(
              {EMaterialTypes::NonSolidDamageable, matType},
              {EMaterialTypes::Projectile, EMaterialTypes::ProjectilePassthrough, matType, EMaterialTypes::Solid}),
          CMaterialList(), dInfo, attribs | GetBeamAttribType(wType), CModelData::CModelDataNull()),
  x158_visorParticle(visorParticle), x168_visorSfx(visorSfx), x170_projectile(wDesc, xf.origin, xf.basis, scale,
  (attribs & EProjectileAttrib::Sixteen) == EProjectileAttrib::Sixteen), x298_(xf.origin),
  x2a4_((xe8_projectileAttribs & EProjectileAttrib::Ten) == EProjectileAttrib::Ten ? 0.25f : 0.1f),
  x2c0_homingTargetId(homingTarget), x2cc_wpscId(wDesc.GetObjectTag()->id)
{
    x2e4_24_ = true;
    x2e4_25_ = underwater;
    x2e4_26_waterUpdate = underwater;
    x2e4_27_inWater = underwater;
    x2e4_28_sendProjectileCollideMsg = sendCollideMsg;
}

void CGameProjectile::Accept(urde::IVisitor& visitor) { visitor.Visit(this); }

void CGameProjectile::ResolveCollisionWithActor(const CRayCastResult& res, CActor& act, CStateManager& mgr)
{
    zeus::CVector3f revDir = -x34_transform.basis[1].normalized();
    if (TCastToPtr<CPlayer>(act))
    {
        if (x158_visorParticle && mgr.GetPlayer().GetCameraState() == CPlayer::EPlayerCameraState::FirstPerson)
        {
            if (zeus::radToDeg(
                std::acos(mgr.GetCameraManager()->GetCurrentCameraTransform(mgr).
                    basis[1].normalized().dot(revDir))) <= 45.f)
            {
                /* Hit us head on! Draw Billboard! */
                std::experimental::optional<TToken<CGenDescription>> bb = {*x158_visorParticle};
                CHUDBillboardEffect* effect = new CHUDBillboardEffect(bb, {},
                                                                      mgr.AllocateUniqueId(), true, "VisorAcid",
                                                                      CHUDBillboardEffect::GetNearClipDistance(mgr),
                                                                      CHUDBillboardEffect::GetScaleForPOV(mgr),
                                                                      zeus::CColor::skWhite, zeus::CVector3f::skOne,
                                                                      zeus::CVector3f::skZero);
                mgr.AddObject(effect);
                CSfxManager::SfxStart(x168_visorSfx, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
                if (x2e4_28_sendProjectileCollideMsg)
                    mgr.SendScriptMsg(&mgr.GetPlayer(), GetUniqueId(), EScriptObjectMessage::ProjectileCollide);
            }
        }
    }
}

void CGameProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId /*uid*/, CStateManager& mgr)
{
    if (msg == EScriptObjectMessage::AddSplashInhabitant)
    {
        if (!x2e4_27_inWater)
        {
            x2e4_27_inWater = true;
            x2e4_26_waterUpdate = true;
        }
    }
    else if (msg == EScriptObjectMessage::UpdateSplashInhabitant)
    {
        if (!x2e4_26_waterUpdate)
            x2e4_26_waterUpdate = true;
    }
    else if (msg == EScriptObjectMessage::RemoveSplashInhabitant)
    {
        if (x2e4_26_waterUpdate)
        {
            x2e4_26_waterUpdate = false;
            x2e4_27_inWater = false;
        }
    }
    else if (msg == EScriptObjectMessage::Deleted)
        DeleteProjectileLight(mgr);
}

CWeapon::EProjectileAttrib CGameProjectile::GetBeamAttribType(EWeaponType wType)
{
    if (wType == EWeaponType::Ice)
        return EProjectileAttrib::Ice;
    else if (wType == EWeaponType::Wave)
        return EProjectileAttrib::Wave;
    else if (wType == EWeaponType::Plasma)
        return EProjectileAttrib::Plasma;
    else if (wType == EWeaponType::Phazon)
        return EProjectileAttrib::Phazon;

    return EProjectileAttrib::None;
}

void CGameProjectile::DeleteProjectileLight(CStateManager& mgr)
{
    if (x2c8_projectileLight != kInvalidUniqueId)
    {
        mgr.FreeScriptObject(x2c8_projectileLight);
        x2c8_projectileLight = kInvalidUniqueId;
    }
}

void CGameProjectile::CreateProjectileLight(std::string_view name, const CLight& light, CStateManager& mgr)
{
    DeleteProjectileLight(mgr);
    x2c8_projectileLight = mgr.AllocateUniqueId();
    mgr.AddObject(new CGameLight(x2c8_projectileLight, GetAreaId(), GetActive(), name, GetTransform(), GetUniqueId(),
                                 light, u32(x2cc_wpscId.Value()), 0, 0.f));
}

void CGameProjectile::Chase(float dt, CStateManager& mgr)
{
    if (!x170_projectile.IsProjectileActive() ||
        x2c0_homingTargetId == kInvalidUniqueId)
        return;

    if (TCastToConstPtr<CActor> act = mgr.GetObjectById(x2c0_homingTargetId))
    {
        if (!act->GetMaterialList().HasMaterial(EMaterialTypes::Target) &&
            !act->GetMaterialList().HasMaterial(EMaterialTypes::Player))
        {
            x2c0_homingTargetId = kInvalidUniqueId;
        }
        else
        {
            zeus::CVector3f homingPos = act->GetHomingPosition(mgr, 0.f);

            TCastToConstPtr<CWallCrawlerSwarm> swarm = act.GetPtr();
            if (swarm)
            {
                int lockOnId = swarm->GetCurrentLockOnId();
                if (swarm->GetLockOnLocationValid(lockOnId))
                {
                    homingPos = swarm->GetLockOnLocation(lockOnId);
                }
                else
                {
                    x2c0_homingTargetId = kInvalidUniqueId;
                    return;
                }
            }

            zeus::CVector3f projToPos = homingPos - x170_projectile.GetTranslation();
            if (x2e0_minHomingDist > 0.f && projToPos.magnitude() < x2e0_minHomingDist)
            {
                x2c0_homingTargetId = kInvalidUniqueId;
                return;
            }

            if (!swarm && !TCastToConstPtr<CPhysicsActor>(act.GetPtr()))
                if (auto tb = act->GetTouchBounds())
                    projToPos.z += (tb->max.z - tb->min.z) * 0.5f;

            zeus::CQuaternion qDelta =
                zeus::CQuaternion::shortestRotationArc(x170_projectile.GetTransform().basis[1], projToPos);

            float wThres = qDelta.w * qDelta.w * 2.f - 1.f;
            if (wThres > 0.99f)
                return;

            float turnRate;
            if (x2e4_26_waterUpdate)
                turnRate = x170_projectile.GetMaxTurnRate() * 0.5f;
            else
                turnRate = x170_projectile.GetMaxTurnRate();

            float maxTurnDelta = zeus::degToRad(turnRate * dt);
            float turnDelta = std::acos(wThres);
            if (maxTurnDelta < turnDelta)
            {
                /* Clamp quat to max delta */
                qDelta = zeus::CQuaternion(std::cos(maxTurnDelta * 0.5f),
                (std::sin(maxTurnDelta * 0.5f) / std::sin(turnDelta * 0.5f)) * qDelta.getImaginary());
            }

            zeus::CTransform xf = qDelta.toTransform() * x170_projectile.GetTransform();
            xf.orthonormalize();
            x170_projectile.SetWorldSpaceOrientation(xf);
        }
    }
}

void CGameProjectile::UpdateHoming(float dt, CStateManager& mgr)
{
    if (!x2e4_24_ || x2c0_homingTargetId == kInvalidUniqueId || x2a8_homingDt <= 0.f)
        return;

    x2b0_targetHomingTime += dt;

    while (x2b0_targetHomingTime >= x2b8_curHomingTime)
    {
        Chase(x2a8_homingDt, mgr);
        x2b8_curHomingTime += x2a8_homingDt;
    }
}

void CGameProjectile::UpdateProjectileMovement(float dt, CStateManager& mgr)
{
    float useDt = dt;
    if (x2e4_26_waterUpdate)
        useDt = 37.5f * dt * dt;

    x298_ = x34_transform.origin;
    x170_projectile.Update(useDt);
    SetTransform(x170_projectile.GetTransform());
    SetTranslation(x170_projectile.GetTranslation());
    UpdateHoming(dt, mgr);
}

CRayCastResult
CGameProjectile::DoCollisionCheck(TUniqueId& idOut, CStateManager& mgr)
{
    CRayCastResult res;
    if (x2e4_24_)
    {
        zeus::CVector3f posDelta = x34_transform.origin - x298_;
        rstl::reserved_vector<TUniqueId, 1024> nearList;
        mgr.BuildNearList(nearList, GetProjectileBounds(),
            CMaterialFilter::MakeExclude(EMaterialTypes::ProjectilePassthrough), this);

        res = RayCollisionCheckWithWorld(idOut, x298_, x34_transform.origin,
                                         posDelta.magnitude(), nearList, mgr);

    }
    return res;
}

void CGameProjectile::ApplyDamageToActors(CStateManager& mgr, const CDamageInfo& dInfo)
{
    if (x2c6_ != kInvalidUniqueId)
    {
        if (TCastToPtr<CActor> act = mgr.ObjectById(x2c6_))
        {
            mgr.ApplyDamage(GetUniqueId(), act->GetUniqueId(), xec_ownerId, dInfo, xf8_filter, x34_transform.basis[1]);
            if ((xe8_projectileAttribs & EProjectileAttrib::PlayerUnFreeze) == EProjectileAttrib::PlayerUnFreeze &&
                mgr.GetPlayer().GetUniqueId() == act->GetUniqueId() && mgr.GetPlayer().GetFrozenState())
                mgr.GetPlayer().UnFreeze(mgr);
        }
        x2c6_ = kInvalidUniqueId;
    }

    for (CProjectileTouchResult& res : x2d0_touchResults)
    {
        if (TCastToConstPtr<CActor> act = mgr.GetObjectById(res.GetActorId()))
        {
            mgr.ApplyDamage(GetUniqueId(), act->GetUniqueId(), xec_ownerId, dInfo, xf8_filter, x34_transform.basis[1]);
            if ((xe8_projectileAttribs & EProjectileAttrib::PlayerUnFreeze) == EProjectileAttrib::PlayerUnFreeze &&
                mgr.GetPlayer().GetUniqueId() == act->GetUniqueId() && mgr.GetPlayer().GetFrozenState())
                mgr.GetPlayer().UnFreeze(mgr);
        }
    }

    x2d0_touchResults.clear();
}

void CGameProjectile::FluidFxThink(EFluidState state, CScriptWater& water, CStateManager& mgr)
{
    if (x170_projectile.GetWeaponDescription()->xa6_SWTR)
        CWeapon::FluidFXThink(state, water, mgr);
}

CRayCastResult
CGameProjectile::RayCollisionCheckWithWorld(TUniqueId& idOut, const zeus::CVector3f& start,
                                            const zeus::CVector3f& end, float mag,
                                            const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                            CStateManager& mgr)
{
    x2d0_touchResults.clear();
    idOut = kInvalidUniqueId;
    x2c6_ = kInvalidUniqueId;
    CRayCastResult res;
    zeus::CVector3f delta = end - start;
    if (!delta.canBeNormalized())
        return res;

    float bestMag = mag;
    zeus::CVector3f dir = delta.normalized();
    CRayCastResult res2 = mgr.RayStaticIntersection(start, dir, mag, xf8_filter);
    if (res2.IsValid())
    {
        bestMag = res2.GetT();
        res = res2;
    }

    for (TUniqueId id : nearList)
    {
        if (CActor* ent = static_cast<CActor*>(mgr.ObjectById(id)))
        {
            CProjectileTouchResult tRes = CanCollideWith(*ent, mgr);
            if (tRes.GetActorId() == kInvalidUniqueId)
                continue;
            if (tRes.HasRayCastResult())
            {
                if (tRes.GetRayCastResult().GetT() < bestMag)
                {
                    ent->Touch(*this, mgr);
                    bestMag = tRes.GetRayCastResult().GetT();
                    res = tRes.GetRayCastResult();
                    x2c6_ = idOut = tRes.GetActorId();
                }
            }
            else
            {
                auto tb = ent->GetTouchBounds();
                CGameProjectile* projObj = nullptr;
                if (TCastToPtr<CScriptDoor> door = ent)
                {
                    tb = door->GetProjectileBounds();
                }
                else if (TCastToPtr<CGameProjectile> proj = ent)
                {
                    tb.emplace(proj->GetProjectileBounds());
                    projObj = proj.GetPtr();
                }
                if (!tb)
                    continue;

                CCollidableAABox prim(*tb, ent->GetMaterialList());
                CRayCastResult res3 =
                prim.CastRayInternal(CInternalRayCastStructure(start, dir, mag, {},
                                                               CMaterialFilter::skPassEverything));
                if (res3.IsValid())
                {
                    if (res3.GetT() < bestMag)
                    {
                        bestMag = res3.GetT();
                        res = res3;
                        x2c6_ = idOut = tRes.GetActorId();
                    }
                }
                else if (tb->pointInside(start) || (projObj && projObj->GetProjectileBounds().intersects(*tb)))
                {
                    x2c6_ = idOut = ent->GetUniqueId();
                    zeus::CUnitVector3f norm(-dir);
                    res = CRayCastResult(0.f, start, {norm, norm.dot(start)}, ent->GetMaterialList());
                    break;
                }
            }
        }
    }

    if (x2e4_27_inWater && idOut == kInvalidUniqueId)
        x2e4_27_inWater = false;

    return res;
}

CProjectileTouchResult CGameProjectile::CanCollideWith(CActor& act, CStateManager& mgr)
{
    return {{}, {}};
}

zeus::CAABox CGameProjectile::GetProjectileBounds() const
{
    return {};
}

}
