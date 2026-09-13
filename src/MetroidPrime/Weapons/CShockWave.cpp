#define CSTATEMANAGER_OUT_OF_LINE_GETPLAYER
#include "MetroidPrime/Weapons/CShockWave.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include <Kyoto/CSimplePool.hpp>
#include <Kyoto/Particles/CElectricDescription.hpp>
#include <Kyoto/Particles/CElementGen.hpp>
#include <rstl/optional_object.hpp>

CShockWave::CShockWave(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, const TUniqueId parent, const CShockWaveInfo& data,
                       const float minActiveTime, const float knockback)
: CActor(uid, true, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_Projectile),
         CActorParameters::None(), kInvalidUniqueId)
, mParentId(parent)
, mDamageInfo(data.GetDamageInfo())
, mElementGenDesc(gpSimplePool->GetObj(SObjectTag('PART', data.GetParticleDescId())))
, mElementGen(rs_new CElementGen(mElementGenDesc))
, mShockWaveInfo(data)
, mRadius(data.GetInitialRadius())
, mExpansionSpeed(data.GetInitialExpansionSpeed())
, mActiveTime(0.f)
, mMinActiveTime(minActiveTime)
, mKnockBack(knockback)
, mTimeSinceHitPlayerInAir(0.f)
, mTimeSinceHitPlayer(0.f)
, mHitPlayerInAir(false)
, mHitPlayer(false)
, mElectricDesc(data.GetWeaponDescId() != kInvalidAssetId
                    ? rstl::optional_object< TToken< CElectricDescription > >(
                          gpSimplePool->GetObj(SObjectTag('ELSC', data.GetWeaponDescId())))
                    : rstl::optional_object_null())
, mLightId(kInvalidUniqueId) {
  mElementGen->SetParticleEmission(true);
  mElementGen->SetOrientation(GetTransform().GetRotation());
  mElementGen->SetGlobalTranslation(GetTranslation());
  SetThermalFlags(kTF_Hot);
}

ENTITY_ACCEPT_IMPL(CShockWave)

void CShockWave::Think(const float dt, CStateManager& mgr) {
  if (GetActive()) {

    mElementGen->Update(dt);
    mActiveTime += dt;
    mRadius += mExpansionSpeed * dt;
    mExpansionSpeed += dt * mShockWaveInfo.GetSpeedIncrease();
    mElementGen->SetExternalVar(0, mRadius);
    for (int i = 0; i < mElementGen->GetNumActiveChildParticles(); ++i) {
      CParticleGen* gen = mElementGen->GetActiveChildParticle(i);
      if (gen->Get4CharId() != 'PART') {
        continue;
      }

      static_cast< CElementGen* >(gen)->SetExternalVar(0, mRadius);
    }

    if (mHitPlayerInAir) {
      mTimeSinceHitPlayerInAir += dt;
      mHitPlayerInAir = false;
    }

    if (mHitPlayer) {
      mTimeSinceHitPlayer += dt;
      mHitPlayer = false;
    }
  }

  if (mElementGen->IsSystemDeletable() && mMinActiveTime > 0.f && mActiveTime >= mMinActiveTime) {
    mgr.DeleteObjectRequest(GetUniqueId());
    return;
  }

  if (mLightId == kInvalidUniqueId) {
    return;
  }

  CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId));
  if (!light) {
    return;
  }
  if (!GetActive()) {
    return;
  }
  light->SetLight(mElementGen->GetLight());
}

void CShockWave::Render(const CStateManager& mgr) const {
  CActor::Render(mgr);
  mElementGen->Render();
}

void CShockWave::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  CActor::AddToRenderer(frustum, mgr);
  gpRender->AddParticleGen(*mElementGen.get());
}

void CShockWave::Touch(CActor& actor, CStateManager& mgr) {
  if (mActiveTime >= mMinActiveTime) {
    return;
  }

  bool isParent = actor.GetUniqueId() == mParentId;

  if (const CCollisionActor* colAct =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(actor.GetUniqueId()))) {
    isParent = colAct->GetOwnerId() == mParentId;
  }

  if (isParent) {
    return;
  }

  float maxDistance = mRadius * mRadius;
  CVector3f distance = actor.GetTranslation() - GetTranslation();
  float minDistance =
      maxDistance * mShockWaveInfo.GetWidthPercent() * mShockWaveInfo.GetWidthPercent();
  CDamageInfo damageInfo = mDamageInfo;
  float knockbackScale = rstl::max_val(1.f - mKnockBack * mActiveTime, 0.f);
  const bool isPlayer = actor.GetUniqueId() == mgr.GetPlayer()->GetUniqueId();
  const bool isPlayerInAir =
      isPlayer && mgr.Player()->GetPlayerMovementState() != NPlayer::kMS_OnGround;
  distance.SetZ(0.f);
  const float distanceSquared = CVector3f::Dot(distance, distance);
  if (distanceSquared >= minDistance && distanceSquared <= maxDistance) {
    if (isPlayer) {
      if (mgr.Player()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
        const CTransform4f playerTransform = mgr.Player()->GetTransform();
        CVector3f playerDirection = GetTranslation() - playerTransform.GetTranslation();
        if (playerDirection.CanBeNormalized()) {
          playerDirection.Normalize();
          const float dot =
              CMath::AbsF(CVector3f::Dot(playerDirection, playerTransform.GetForward()));
          const float minKnockbackScale = 1.f - 0.88f;
          knockbackScale *= rstl::max_val(minKnockbackScale, 0.88f * (dot * dot));
        }
      }
      const CVector3f velocity = mgr.Player()->GetVelocityWR();
      if (velocity.Magnitude() > 40.f) {
        mTimeSinceHitPlayer = 0.2666f;
      }
    }
    damageInfo.SetKnockBackPower(knockbackScale * mDamageInfo.GetKnockBackPower());

    bool canDamage = true;
    if (isPlayer && (mTimeSinceHitPlayerInAir >= 0.1333f || mTimeSinceHitPlayer >= 0.2666f)) {
      canDamage = false;
    }
    if (canDamage) {
      if (!WasAlreadyDamaged(actor.GetUniqueId())) {
        mgr.ApplyDamage(
            GetUniqueId(), actor.GetUniqueId(), GetUniqueId(), damageInfo,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
            CVector3f::Zero());
        if (isPlayer && mElectricDesc) {
          mgr.AddObject(rs_new CHUDBillboardEffect(
              rstl::optional_object_null(), mElectricDesc, mgr.AllocateUniqueId(), true,
              rstl::string_l("VisorElectricFx"), CHUDBillboardEffect::GetNearClipDistance(mgr),
              CHUDBillboardEffect::GetScaleForPOV(mgr), CColor(1.f, 1.f, 1.f, 1.f),
              CVector3f(1.f, 1.f, 1.f), CVector3f(0.f, 0.f, 0.f)));
          CSfxManager::SfxStart(mShockWaveInfo.GetElectrocuteSfx());
        }
        mHitIds.push_back(actor.GetUniqueId());
      } else {
        damageInfo.SetDamage(0.f);
        mgr.ApplyDamage(
            GetUniqueId(), actor.GetUniqueId(), GetUniqueId(), damageInfo,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
            CVector3f::Zero());
      }
      if (isPlayerInAir) {
        mHitPlayerInAir = true;
      }
      if (isPlayer) {
        mHitPlayer = true;
      }
    }
  }
}

rstl::optional_object< CAABox > CShockWave::GetTouchBounds() const {
  if (mRadius > 0.f) {
    CAABox bounds(CVector3f(-mRadius, -mRadius, 0.f), CVector3f(mRadius, mRadius, 1.f));
    return bounds.GetTransformedAABox(GetTransform());
  }

  return rstl::optional_object_null();
}

bool CShockWave::WasAlreadyDamaged(const TUniqueId uid) const {
  for (TEntityList::const_iterator it = mHitIds.begin(); it != mHitIds.end(); ++it) {
    if (*it == uid) {
      return true;
    }
  }

  return false;
}

void CShockWave::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered: {
    if (!mElementGen->SystemHasLight()) {
      break;
    }

    mLightId = mgr.AllocateUniqueId();
    CAssetId partId = static_cast< CAssetId >(mShockWaveInfo.GetParticleDescId());
    mgr.AddObject(rs_new CGameLight(mLightId, GetCurrentAreaId(), GetActive(),
                                    rstl::string_l("ShockWaveLight_") + GetDebugName(),
                                    GetTransform(), GetUniqueId(), mElementGen->GetLight(), partId,
                                    1, 0.f));
  } break;
  case kSM_Deleted: {
    if (mLightId == kInvalidUniqueId) {
      break;
    }
    mgr.DeleteObjectRequest(mLightId);
    mLightId = kInvalidUniqueId;
  } break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(msg, sender, mgr);
  mgr.SendScriptMsgAlways(mLightId, sender, msg);
}
