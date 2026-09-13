#include "MetroidPrime/Enemies/CFlaahgraPlants.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

#include "MetaRender/CCubeRenderer.hpp"

CFlaahgraPlants::CFlaahgraPlants(const TToken< CGenDescription >& genDesc,
                                 const CActorParameters& actParms, TUniqueId uid, TAreaId aId,
                                 TUniqueId owner, const CTransform4f& xf, const CDamageInfo& dInfo,
                                 const CVector3f& extents)
: CActor(uid, true, "Flaahgra Plants", CEntityInfo(aId, NullConnectionList), xf,
         CModelData::CModelDataNull(), CMaterialList(kMT_Projectile), actParms, kInvalidUniqueId)
, xe8_elementGen(rs_new CElementGen(genDesc))
, xf0_ownerId(owner)
, xf4_damageInfo(dInfo)
, x12c_lastDt(0.f)
, x130_obbox(xf, extents)
, x16c_colAct(kInvalidUniqueId) {
  xe8_elementGen->SetOrientation(xf.GetRotation());
  xe8_elementGen->SetTranslation(xf.GetTranslation());
  xe8_elementGen->SetLeaveLightsEnabledForModelRender(true);
  x110_aabox = x130_obbox.CalculateAABox(CTransform4f::Identity());
}

CFlaahgraPlants::~CFlaahgraPlants() {}

void CFlaahgraPlants::Touch(CActor& act, CStateManager& mgr) {
  if (act.GetUniqueId() == mgr.GetPlayer()->GetUniqueId() && x110_aabox) {
    COBBox plObb = COBBox::FromAABox(mgr.GetPlayer()->GetBoundingBox(), CTransform4f::Identity());

    if (!x130_obbox.OBBIntersectsBox(plObb)) {
      return;
    }

    CDamageInfo dInfo(xf4_damageInfo.GetWeaponMode(), x12c_lastDt * xf4_damageInfo.GetDamage(),
                      xf4_damageInfo.GetRadius(), xf4_damageInfo.GetKnockBackPower(), true);
    CMaterialFilter filter =
        CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList());
    CVector3f diffVec = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    CVector3f right = GetTransform().GetColumn(kDX);
    CVector3f knockbackVec = CVector3f::Dot(diffVec, right) > 0.f ? right : -right;
    mgr.ApplyDamage(GetUniqueId(), act.GetUniqueId(), GetUniqueId(), dInfo, filter, knockbackVec);
  }
}

rstl::optional_object< CAABox > CFlaahgraPlants::GetTouchBounds() const {
  if (GetActive()) {
    return x110_aabox;
  } else {
    return rstl::optional_object_null();
  }
}

void CFlaahgraPlants::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  gpRender->AddParticleGen(*xe8_elementGen);
  CActor::AddToRenderer(frustum, mgr);
}

ENTITY_ACCEPT_IMPL(CFlaahgraPlants)

void CFlaahgraPlants::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    xe8_elementGen->Update(dt);
    x12c_lastDt = dt;
  }

  if (xe8_elementGen->IsSystemDeletable()) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

void CFlaahgraPlants::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Registered: {
    xe8_elementGen->SetParticleEmission(true);
    SetActive(true);
    if (x16c_colAct == kInvalidUniqueId) {
      x16c_colAct = mgr.AllocateUniqueId();
      CVector3f extent = x130_obbox.GetSize() + CVector3f(0.f, 5.f, 10.f);
      CCollisionActor* colAct = rs_new CCollisionActor(x16c_colAct, GetCurrentAreaId(), GetUniqueId(),
                                                    extent, CVector3f::Zero(), true, 0.001f);
      if (colAct != nullptr) {
        colAct->SetTransform(GetTransform());
        colAct->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
            CMaterialList(kMT_Player),
            CMaterialList(kMT_Trigger, kMT_CollisionActor, kMT_NoStaticCollision, kMT_Character)));
        colAct->MaterialList().Add(kMT_Immovable);
        colAct->MaterialList().Add(kMT_ProjectilePassthrough);
        mgr.AddObject(colAct);
        mgr.SetActorAreaId(*colAct, GetCurrentAreaId());
      }
    }
    break;
  }
  case kSM_Deleted: {
    if (x16c_colAct != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(x16c_colAct);
    }
    break;
  }
  }
}
