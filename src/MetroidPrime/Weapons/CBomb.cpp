#include "MetroidPrime/Weapons/CBomb.hpp"
#include "Collision/CRayCastResult.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/EListenNoiseType.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/SFX/Weapons.h"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "rstl/math.hpp"

#include <Kyoto/Audio/CSfxManager.hpp>
#include <Kyoto/Math/CFrustumPlanes.hpp>
#include <Kyoto/Particles/CElementGen.hpp>

static float kBombBoxAllowance = 0.2f;

CBomb::CBomb(TToken< CGenDescription > particle1, TToken< CGenDescription > particle2,
             TUniqueId uid, TAreaId aid, TUniqueId playerId, float f1, const CTransform4f& xf,
             const CDamageInfo& dInfo)
: CWeapon(uid, aid, true, playerId, kWT_Bomb, rstl::string_l("Bomb"), xf,
          CMaterialFilter::MakeIncludeExclude(
              CMaterialList(kMT_Solid, kMT_Trigger, kMT_NonSolidDamageable),
              CMaterialList(kMT_Projectile, kMT_Bomb)),
          CMaterialList(kMT_Projectile, kMT_Bomb), dInfo, kPA_Bombs, CModelData::CModelDataNull())
, mVelocity(CVector3f::Zero())
, mAcceleration(CVector3f::Zero())
, mPrevLocation(xf.GetTranslation())
, mFuseTime(f1)
, mParticle1(rs_new CElementGen(particle1))
, mParticle2(rs_new CElementGen(particle2))
, mLightId(kInvalidUniqueId)
, mParticle2Ptr(CToken(particle2).GetTag().GetId())
, mIsNotDetonated(true)
, mBeingDragged(false)
, mDisableFuse(false) {
  mParticle1->SetGlobalTranslation(xf.GetTranslation());
  mParticle2->SetGlobalTranslation(xf.GetTranslation());
}

CBomb::~CBomb() {}

void CBomb::Explode(const CVector3f& pos, CStateManager& mgr) {
  mgr.ApplyDamageToWorld(GetOwnerId(), *this, pos, mCurDamageInfo, GetFilter());
  CSfxManager::AddEmitter(SFXsam_misl_expl_01, GetTranslation(), CVector3f::Zero(), true);
  mgr.InformListeners(pos, kLNT_BombExplode);
  mgr.RemoveWeaponId(GetOwnerId(), GetType());
  mIsNotDetonated = false;
}

void CBomb::Touch(CActor& actor, CStateManager& mgr) {
  if (!mIsNotDetonated) {
    return;
  }
}

void CBomb::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  CVector3f origin = GetTranslation();
  float ballRadius = mgr.GetPlayer()->GetMorphBall()->GetBallRadius();

  CAABox aabox(origin - CVector3f(0.9f * ballRadius, 0.9f * ballRadius, 0.9f * ballRadius),
               origin + CVector3f(0.9f * ballRadius, 0.9f * ballRadius, 0.9f * ballRadius));
  CVector3f forward = CGraphics::GetViewMatrix().GetForward();
  CVector3f closestPoint = aabox.ClosestPointAlongVector(forward);

  if (mIsNotDetonated) {
    if (mFuseTime > 0.5f) {
      gpRender->AddParticleGen(*mParticle1, closestPoint, aabox);
    } else {
      gpRender->AddParticleGen(*mParticle2, closestPoint, aabox);
    }
  } else {
    gpRender->AddParticleGen(*mParticle2, closestPoint, aabox);
  }
}

void CBomb::Render(const CStateManager& mgr) const {}

void CBomb::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);
  if (mIsNotDetonated) {
    if (mFuseTime <= 0.f) {
      Explode(GetTranslation(), mgr);
      if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId))) {
        light->SetActive(true);
      }
    }

    if (mFuseTime > 0.5f) {
      mParticle1->Update(dt);
    } else {
      UpdateLight(dt, mgr);
    }

    if (!mDisableFuse) {
      mFuseTime -= dt;
    }
  } else {
    UpdateLight(dt, mgr);

    if (mParticle2->IsSystemDeletable()) {
      mgr.DeleteObjectRequest(GetUniqueId());
    }
  }

  if (mIsNotDetonated) {
    if (mAcceleration.MagSquared() > 0.f) {
      mVelocity += dt * mAcceleration;
    }

    if (mVelocity.MagSquared() > 0.f) {
      mPrevLocation = GetTranslation();
      GlobalMove(dt * mVelocity);
      CVector3f diffVec = GetTranslation() - mPrevLocation;
      float diffMag = diffVec.Magnitude();
      if (diffMag == 0.f) {
        Explode(GetTranslation(), mgr);
      } else {
        static const CMaterialFilter kSolidFilter = CMaterialFilter::MakeIncludeExclude(
            CMaterialList(kMT_Solid),
            CMaterialList(kMT_Character, kMT_Player, kMT_ProjectilePassthrough));
        CRayCastResult res =
            mgr.RayStaticIntersection(mPrevLocation, diffVec / diffMag, diffMag, kSolidFilter);

        if (res.IsValid()) {
          Explode(GetTranslation(), mgr);
        }
      }
    }
  }

  mParticle1->SetGlobalTranslation(GetTranslation());
  mParticle2->SetGlobalTranslation(GetTranslation());
}

ENTITY_ACCEPT_IMPL(CBomb)

void CBomb::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered: {
    if (mParticle2->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      const int sourceId = mParticle2Ptr;
      mgr.AddObject(rs_new CGameLight(
          mLightId, GetCurrentAreaId(), false, rstl::string_l("BombPLight_") + GetDebugName(),
          GetTransform(), GetUniqueId(), mParticle2->GetLight(), sourceId, 1, 0.f));
    }
    mgr.AddWeaponId(GetOwnerId(), GetType());
    CSfxManager::AddEmitter(SFXsam_a_bombset_00, GetTranslation(), CVector3f::Zero(), true);
    mgr.InformListeners(GetTranslation(), kLNT_BombExplode);
  } break;
  case kSM_Deleted: {
    if (mLightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mLightId);
    }

    if (mIsNotDetonated) {
      mgr.RemoveWeaponId(GetOwnerId(), GetType());
    }
  } break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(msg, sender, mgr);
}

rstl::optional_object< CAABox > CBomb::GetTouchBounds() const {
  float radius = mIsNotDetonated ? kBombBoxAllowance : mCurDamageInfo.GetRadius();
  return CAABox(rstl::min_val(mPrevLocation.GetX(), GetTranslation().GetX()) - radius,
                rstl::min_val(mPrevLocation.GetY(), GetTranslation().GetY()) - radius,
                rstl::min_val(mPrevLocation.GetZ(), GetTranslation().GetZ()) - radius,
                rstl::max_val(mPrevLocation.GetX(), GetTranslation().GetX()) + radius,
                rstl::max_val(mPrevLocation.GetY(), GetTranslation().GetY()) + radius,
                rstl::max_val(mPrevLocation.GetZ(), GetTranslation().GetZ()) + radius);
}

void CBomb::UpdateLight(float dt, CStateManager& mgr) {
  mParticle2->Update(dt);
  if (mLightId == kInvalidUniqueId) {
    return;
  }

  CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId));
  if (light && GetActive()) {
    light->SetLight(mParticle2->GetLight());
  }
}
