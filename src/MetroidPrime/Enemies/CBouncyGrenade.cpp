#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TCastTo.hpp"

static CElementGen* CreateElementGen(CAssetId id) {
  TLockedToken< CGenDescription > desc = gpSimplePool->GetObj(SObjectTag('PART', id));
  return rs_new CElementGen(desc);
}

CBouncyGrenade::CBouncyGrenade(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                               const CTransform4f& xf, const CModelData& mData,
                               const CActorParameters& actParams, TUniqueId parentId,
                               const CBouncyGrenadeData& data, float velocity,
                               float explodePlayerDistance)
: CPhysicsActor(uid, true, name, info, xf, mData, CMaterialList(kMT_Solid, kMT_Projectile),
                mData.GetBounds(), SMoverData(data.GetVelocityInfo().GetMass()), actParams, 0.3f,
                0.1f)
, mData(data)
, mNumBounces(data.GetNumBounces())
, mParentId(parentId)
, mElapsedTime(0.f)
, mElementGenCombat(CreateElementGen(data.GetElementGenId1()))
, mElementGenXRay(CreateElementGen(data.GetElementGenId2()))
, mElementGenThermal(CreateElementGen(data.GetElementGenId3()))
, mElementGenTrail(CreateElementGen(data.GetElementGenId4()))
, mExplodePlayerDistance(explodePlayerDistance)
, mExploded(false)
, x2b4_25_(false) {
  const float mass = GetMass();
  SetMomentumWR(CVector3f(0.f, 0.f, -kGravityAccel * mass));
  SetVelocityWR(velocity * xf.GetForward());
  mElementGenCombat->SetParticleEmission(false);
  mElementGenXRay->SetParticleEmission(false);
  mElementGenThermal->SetParticleEmission(false);
  mElementGenTrail->SetParticleEmission(true);
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  exclude.Add(CMaterialList(kMT_Character));
  SetMaterialFilter(
      CMaterialFilter::MakeIncludeExclude(GetMaterialFilter().GetIncludeList(), exclude));
}

ENTITY_ACCEPT_IMPL(CBouncyGrenade)

void CBouncyGrenade::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    const CTransform4f orientation = GetTransform().GetRotation();
    const CVector3f translation = GetTranslation();
    const CVector3f scale = GetModelData()->ScaleCopy();
    if (mExploded) {
      Stop();
      mElementGenCombat->SetOrientation(orientation);
      mElementGenCombat->SetGlobalTranslation(translation);
      mElementGenCombat->SetGlobalScale(scale);
      mElementGenCombat->Update(dt);
      mElementGenXRay->SetOrientation(orientation);
      mElementGenXRay->SetGlobalTranslation(translation);
      mElementGenXRay->SetGlobalScale(scale);
      mElementGenXRay->Update(dt);
      mElementGenThermal->SetOrientation(orientation);
      mElementGenThermal->SetGlobalTranslation(translation);
      mElementGenThermal->SetGlobalScale(scale);
      mElementGenThermal->Update(dt);
    } else {
      mElementGenTrail->SetOrientation(orientation);
      mElementGenTrail->SetGlobalTranslation(translation);
      mElementGenTrail->SetGlobalScale(scale);
      mElementGenTrail->Update(dt);
    }
    mElapsedTime += dt;
    if (mElapsedTime > 0.3f) {
      x2b4_25_ = true;
    }
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation() +
                                CVector3f(0.f, 0.f, 0.5f * mgr.GetPlayer()->GetEyeHeight());
    const CVector3f& delta = CVector3f(playerPos - translation);
    if (delta.MagSquared() < mExplodePlayerDistance * mExplodePlayerDistance) {
      Explode(mgr, kInvalidUniqueId);
    }
  }
  if (mElementGenCombat->IsSystemDeletable() && mElementGenXRay->IsSystemDeletable() &&
      mElementGenThermal->IsSystemDeletable()) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

void CBouncyGrenade::Touch(CActor& act, CStateManager& mgr) { CActor::Touch(act, mgr); }

rstl::optional_object< CAABox > CBouncyGrenade::GetTouchBounds() const {
  return GetModelData()->GetBounds(GetTransform());
}

void CBouncyGrenade::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                  CStateManager& mgr) {
  static const CMaterialList skSolidTypes(kMT_Solid, kMT_Ceiling, kMT_Wall, kMT_Floor,
                                          kMT_Character);
  bool shouldExplode = false;
  if (id != mParentId) {
    if (const CEntity* entity = mgr.GetObjectById(id)) {
      if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(entity)) {
        shouldExplode = actor->GetOwnerId() != mParentId;
      } else {
        shouldExplode = true;
      }
    }
  }
  if (shouldExplode) {
    Explode(mgr, id);
  } else {
    for (int i = 0; i < list.GetCount(); ++i) {
      const CCollisionInfo& info = list[i];
      if (info.GetMaterialLeft().SharesMaterials(skSolidTypes)) {
        if (mNumBounces != 0) {
          const CVector3f bounceNormal = CVector3f::Dot(GetVelocityWR(), info.GetNormalLeft()) > 0.f
                                             ? info.GetNormalRight()
                                             : info.GetNormalLeft();
          const CVector3f impulse =
              (mData.GetVelocityInfo().GetSpeed() * GetConstantForceWR().Magnitude()) *
              bounceNormal;
          const CAxisAngle angle = -mData.GetVelocityInfo().GetSpeed() * GetAngularMomentumWR();
          ApplyImpulseWR(impulse, angle);
          CSfxManager::AddEmitter(mData.GetBounceSfx(), GetTranslation(), CVector3f::Up(),
                                  false, false, CSfxManager::kMedPriority,
                                  GetCurrentAreaId().Value());
          --mNumBounces;
        } else {
          Explode(mgr, kInvalidUniqueId);
        }
        break;
      }
    }
  }
  CPhysicsActor::CollidedWith(id, list, mgr);
}

void CBouncyGrenade::Render(const CStateManager& mgr) const {
  if (!mExploded) {
    GetModelData()->Render(mgr, GetTransform(), nullptr, CModelFlags::Normal());
  } else if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
    CElementGen::SetSubtractBlend(true);
    CElementGen::SetMoveRedToAlphaBuffer(true);
    CGraphics::SetFog(kRFM_PerspLin, 0.f, 75.f, CColor::Black());
    mElementGenXRay->Render();
    mgr.SetupFogForArea(GetCurrentAreaId());
    CElementGen::SetSubtractBlend(false);
    CElementGen::SetMoveRedToAlphaBuffer(false);
  }
}

void CBouncyGrenade::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  CActor::AddToRenderer(frustum, mgr);
  if (mExploded) {
    switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
    case CPlayerState::kPV_Combat:
    case CPlayerState::kPV_Scan:
      gpRender->AddParticleGen(*mElementGenCombat);
      break;
    case CPlayerState::kPV_Thermal:
      gpRender->AddParticleGen(*mElementGenThermal);
      break;
    }
  } else {
    gpRender->AddParticleGen(*mElementGenTrail);
  }
}

void CBouncyGrenade::Explode(CStateManager& mgr, const TUniqueId uid) {
  if (mExploded) {
    return;
  }
  mExploded = true;
  CSfxManager::AddEmitter(mData.GetExplodeSfx(), GetTranslation(), CVector3f::Up(), false,
                          false, CSfxManager::kMedPriority, GetCurrentAreaId().Value());
  mElementGenCombat->SetParticleEmission(true);
  mElementGenXRay->SetParticleEmission(true);
  mElementGenThermal->SetParticleEmission(true);
  mElementGenTrail->SetParticleEmission(false);
  bool isParent = uid == mParentId;
  if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(uid))) {
    isParent = actor->GetOwnerId() == mParentId;
  }
  const CDamageInfo& dInfo = mData.GetDamageInfo();
  if (uid != kInvalidUniqueId && !isParent) {
    mgr.ApplyDamage(
        GetUniqueId(), uid, GetUniqueId(), dInfo,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
        CVector3f::Zero());
  }
  if (dInfo.GetRadius() > 1.f) {
    const CVector3f pos = GetTranslation();
    const CVector3f extent(dInfo.GetRadius(), dInfo.GetRadius(), dInfo.GetRadius());
    const CAABox bounds(pos - extent, pos + extent);
    const CMaterialFilter filter =
        CMaterialFilter::MakeInclude(CMaterialList(kMT_Character, kMT_Player));
    TEntityList nearList;
    mgr.BuildNearList(nearList, bounds, filter, nullptr);
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      bool isParent = *it == mParentId;
      if (const CCollisionActor* actor =
              TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(*it))) {
        isParent = actor->GetOwnerId() == mParentId;
      }
      if (isParent) {
        continue;
      }
      CActor* actor = static_cast< CActor* >(mgr.ObjectById(*it));
      if (actor != nullptr) {
        const CVector3f delta = actor->GetTranslation() - GetTranslation();
        const float magnitude = delta.Magnitude();
        if (magnitude < dInfo.GetRadius()) {
          const float scale = (dInfo.GetRadius() - magnitude) / dInfo.GetRadius();
          const CDamageInfo info(dInfo.GetWeaponMode(), scale * dInfo.GetDamage(),
                                 dInfo.GetRadius(), scale * dInfo.GetKnockBackPower());
          mgr.ApplyDamage(
              GetUniqueId(), *it, GetUniqueId(), info,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
              CVector3f::Zero());
        }
      }
    }
  }
}

CBouncyGrenade::~CBouncyGrenade() {}
