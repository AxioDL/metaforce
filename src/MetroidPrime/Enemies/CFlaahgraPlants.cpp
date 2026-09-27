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
, mElementGen(rs_new CElementGen(genDesc))
, mOwnerId(owner)
, mDamageInfo(dInfo)
, mLastDt(0.f)
, mObbox(xf, extents)
, mColAct(kInvalidUniqueId) {
  mElementGen->SetOrientation(xf.GetRotation());
  mElementGen->SetTranslation(xf.GetTranslation());
  mElementGen->SetLeaveLightsEnabledForModelRender(true);
  mAabox = mObbox.CalculateAABox(CTransform4f::Identity());
}

CFlaahgraPlants::~CFlaahgraPlants() {}

void CFlaahgraPlants::Touch(CActor& act, CStateManager& mgr) {
  if (act.GetUniqueId() == mgr.GetPlayer()->GetUniqueId() && mAabox) {
    COBBox plObb = COBBox::FromAABox(mgr.GetPlayer()->GetBoundingBox(), CTransform4f::Identity());

    if (!mObbox.OBBIntersectsBox(plObb)) {
      return;
    }

    CDamageInfo dInfo(mDamageInfo.GetWeaponMode(), mLastDt * mDamageInfo.GetDamage(),
                      mDamageInfo.GetRadius(), mDamageInfo.GetKnockBackPower(), true);
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
    return mAabox;
  } else {
    return rstl::optional_object_null();
  }
}

void CFlaahgraPlants::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  gpRender->AddParticleGen(*mElementGen);
  CActor::AddToRenderer(frustum, mgr);
}

ENTITY_ACCEPT_IMPL(CFlaahgraPlants)

void CFlaahgraPlants::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    mElementGen->Update(dt);
    mLastDt = dt;
  }

  if (mElementGen->IsSystemDeletable()) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

void CFlaahgraPlants::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Registered: {
    mElementGen->SetParticleEmission(true);
    SetActive(true);
    if (mColAct == kInvalidUniqueId) {
      mColAct = mgr.AllocateUniqueId();
      CVector3f extent = mObbox.GetSize() + CVector3f(0.f, 5.f, 10.f);
      CCollisionActor* colAct = rs_new CCollisionActor(mColAct, GetCurrentAreaId(), GetUniqueId(),
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
    if (mColAct != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mColAct);
    }
    break;
  }
  }
}
