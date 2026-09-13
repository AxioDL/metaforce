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
, x258_data(data)
, x294_numBounces(data.GetNumBounces())
, x298_parentId(parentId)
, x29c_elapsedTime(0.f)
, x2a0_elementGenCombat(CreateElementGen(data.GetElementGenId1()))
, x2a4_elementGenXRay(CreateElementGen(data.GetElementGenId2()))
, x2a8_elementGenThermal(CreateElementGen(data.GetElementGenId3()))
, x2ac_elementGenTrail(CreateElementGen(data.GetElementGenId4()))
, x2b0_explodePlayerDistance(explodePlayerDistance)
, x2b4_24_exploded(false)
, x2b4_25_(false) {
  const float mass = GetMass();
  SetMomentumWR(CVector3f(0.f, 0.f, -kGravityAccel * mass));
  SetVelocityWR(velocity * xf.GetForward());
  x2a0_elementGenCombat->SetParticleEmission(false);
  x2a4_elementGenXRay->SetParticleEmission(false);
  x2a8_elementGenThermal->SetParticleEmission(false);
  x2ac_elementGenTrail->SetParticleEmission(true);
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
    if (x2b4_24_exploded) {
      Stop();
      x2a0_elementGenCombat->SetOrientation(orientation);
      x2a0_elementGenCombat->SetGlobalTranslation(translation);
      x2a0_elementGenCombat->SetGlobalScale(scale);
      x2a0_elementGenCombat->Update(dt);
      x2a4_elementGenXRay->SetOrientation(orientation);
      x2a4_elementGenXRay->SetGlobalTranslation(translation);
      x2a4_elementGenXRay->SetGlobalScale(scale);
      x2a4_elementGenXRay->Update(dt);
      x2a8_elementGenThermal->SetOrientation(orientation);
      x2a8_elementGenThermal->SetGlobalTranslation(translation);
      x2a8_elementGenThermal->SetGlobalScale(scale);
      x2a8_elementGenThermal->Update(dt);
    } else {
      x2ac_elementGenTrail->SetOrientation(orientation);
      x2ac_elementGenTrail->SetGlobalTranslation(translation);
      x2ac_elementGenTrail->SetGlobalScale(scale);
      x2ac_elementGenTrail->Update(dt);
    }
    x29c_elapsedTime += dt;
    if (x29c_elapsedTime > 0.3f) {
      x2b4_25_ = true;
    }
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation() +
                                CVector3f(0.f, 0.f, 0.5f * mgr.GetPlayer()->GetEyeHeight());
    const CVector3f& delta = CVector3f(playerPos - translation);
    if (delta.MagSquared() < x2b0_explodePlayerDistance * x2b0_explodePlayerDistance) {
      Explode(mgr, kInvalidUniqueId);
    }
  }
  if (x2a0_elementGenCombat->IsSystemDeletable() && x2a4_elementGenXRay->IsSystemDeletable() &&
      x2a8_elementGenThermal->IsSystemDeletable()) {
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
  if (id != x298_parentId) {
    if (const CEntity* entity = mgr.GetObjectById(id)) {
      if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(entity)) {
        shouldExplode = actor->GetOwnerId() != x298_parentId;
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
        if (x294_numBounces != 0) {
          const CVector3f bounceNormal = CVector3f::Dot(GetVelocityWR(), info.GetNormalLeft()) > 0.f
                                             ? info.GetNormalRight()
                                             : info.GetNormalLeft();
          const CVector3f impulse =
              (x258_data.GetVelocityInfo().GetSpeed() * GetConstantForceWR().Magnitude()) *
              bounceNormal;
          const CAxisAngle angle = -x258_data.GetVelocityInfo().GetSpeed() * GetAngularMomentumWR();
          ApplyImpulseWR(impulse, angle);
          CSfxManager::AddEmitter(x258_data.GetBounceSfx(), GetTranslation(), CVector3f::Up(),
                                  false, false, CSfxManager::kMedPriority,
                                  GetCurrentAreaId().Value());
          --x294_numBounces;
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
  if (!x2b4_24_exploded) {
    GetModelData()->Render(mgr, GetTransform(), nullptr, CModelFlags::Normal());
  } else if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
    CElementGen::SetSubtractBlend(true);
    CElementGen::SetMoveRedToAlphaBuffer(true);
    CGraphics::SetFog(kRFM_PerspLin, 0.f, 75.f, CColor::Black());
    x2a4_elementGenXRay->Render();
    mgr.SetupFogForArea(GetCurrentAreaId());
    CElementGen::SetSubtractBlend(false);
    CElementGen::SetMoveRedToAlphaBuffer(false);
  }
}

void CBouncyGrenade::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  CActor::AddToRenderer(frustum, mgr);
  if (x2b4_24_exploded) {
    switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
    case CPlayerState::kPV_Combat:
    case CPlayerState::kPV_Scan:
      gpRender->AddParticleGen(*x2a0_elementGenCombat);
      break;
    case CPlayerState::kPV_Thermal:
      gpRender->AddParticleGen(*x2a8_elementGenThermal);
      break;
    }
  } else {
    gpRender->AddParticleGen(*x2ac_elementGenTrail);
  }
}

void CBouncyGrenade::Explode(CStateManager& mgr, const TUniqueId uid) {
  if (x2b4_24_exploded) {
    return;
  }
  x2b4_24_exploded = true;
  CSfxManager::AddEmitter(x258_data.GetExplodeSfx(), GetTranslation(), CVector3f::Up(), false,
                          false, CSfxManager::kMedPriority, GetCurrentAreaId().Value());
  x2a0_elementGenCombat->SetParticleEmission(true);
  x2a4_elementGenXRay->SetParticleEmission(true);
  x2a8_elementGenThermal->SetParticleEmission(true);
  x2ac_elementGenTrail->SetParticleEmission(false);
  bool isParent = uid == x298_parentId;
  if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(uid))) {
    isParent = actor->GetOwnerId() == x298_parentId;
  }
  const CDamageInfo& dInfo = x258_data.GetDamageInfo();
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
      bool isParent = *it == x298_parentId;
      if (const CCollisionActor* actor =
              TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(*it))) {
        isParent = actor->GetOwnerId() == x298_parentId;
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
