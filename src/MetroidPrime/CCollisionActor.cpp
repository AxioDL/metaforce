#include "MetroidPrime/CCollisionActor.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "WorldFormat/CCollidableOBBTreeGroup.hpp"
#include "WorldFormat/COBBTree.hpp"

#include "rstl/math.hpp"

static const CMaterialList gkDefaultCollisionActorMaterials(kMT_Solid, kMT_CollisionActor,
                                                            kMT_ScanPassthrough,
                                                            kMT_CameraPassthrough);

CCollisionActor::CCollisionActor(TUniqueId uid, TAreaId areaId, TUniqueId owner,
                                 const CVector3f& extent, const CVector3f& center, bool active,
                                 float mass)
: CPhysicsActor(uid, active, rstl::string_l("CollisionActor"),
                CEntityInfo(areaId, NullConnectionList), CTransform4f::Identity(),
                CModelData::CModelDataNull(), gkDefaultCollisionActorMaterials,
                CAABox::MakeNullBox(), SMoverData(mass), CActorParameters::None(), 0.3f, 0.1f)
, mPrimitiveType(kPT_OBBTreeGroup)
, mOwner(owner)
, mBoxSize(extent)
, mCenter(center)
, mObbContainer(rs_new CCollidableOBBTreeGroupContainer(extent, center))
, mObbTreeGroupPrimitive(
      rs_new CCollidableOBBTreeGroup(mObbContainer.get(), GetMaterialList()))
, mAaboxPrimitive(nullptr)
, mSpherePrimitive(nullptr)
, mSphereRadius(0.f)
, mHealthInfo(0.f, 0.f)
, mDamageVuln(CDamageVulnerability::NormalVulnerability())
, mLastTouched(kInvalidUniqueId)
, mResponseType(kWCR_EnemyNormal)
, mExtendedTouchBounds(CVector3f::Zero()) {
  SetCoefficientOfRestitutionModifier(0.5f);
  SetCallTouch(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_CollisionActor, kMT_NoStaticCollision)));
}

CCollisionActor::CCollisionActor(TUniqueId uid, TAreaId areaId, TUniqueId owner,
                                 const CVector3f& boxSize, bool active, float mass)
: CPhysicsActor(uid, active, rstl::string_l("CollisionActor"),
                CEntityInfo(areaId, NullConnectionList), CTransform4f::Identity(),
                CModelData::CModelDataNull(), gkDefaultCollisionActorMaterials,
                CAABox::MakeNullBox(), SMoverData(mass), CActorParameters::None(), 0.3f, 0.1f)
, mPrimitiveType(kPT_AABox)
, mOwner(owner)
, mBoxSize(boxSize)
, mCenter(CVector3f::Zero())
, mObbContainer(nullptr)
, mObbTreeGroupPrimitive(nullptr)
, mAaboxPrimitive(rs_new CCollidableAABox(CAABox(-0.5f * mBoxSize, 0.5f * mBoxSize),
                                              CMaterialList(kMT_Solid, kMT_NoStaticCollision)))
, mSpherePrimitive(nullptr)
, mSphereRadius(0.f)
, mHealthInfo(0.f, 0.f)
, mDamageVuln(CDamageVulnerability::NormalVulnerability())
, mLastTouched(kInvalidUniqueId)
, mResponseType(kWCR_EnemyNormal)
, mExtendedTouchBounds(CVector3f::Zero()) {
  SetCoefficientOfRestitutionModifier(0.5f);
  SetCallTouch(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_CollisionActor, kMT_NoStaticCollision)));
}

CCollisionActor::CCollisionActor(TUniqueId uid, TAreaId areaId, TUniqueId owner, bool active,
                                 float radius, float mass)
: CPhysicsActor(uid, active, rstl::string_l("CollisionActor"),
                CEntityInfo(areaId, NullConnectionList), CTransform4f::Identity(),
                CModelData::CModelDataNull(), gkDefaultCollisionActorMaterials,
                CAABox::MakeNullBox(), SMoverData(mass), CActorParameters::None(), 0.3f, 0.1f)
, mPrimitiveType(kPT_Sphere)
, mOwner(owner)
, mBoxSize(CVector3f::Zero())
, mCenter(CVector3f::Zero())
, mObbContainer(nullptr)
, mObbTreeGroupPrimitive(nullptr)
, mAaboxPrimitive(nullptr)
, mSpherePrimitive(rs_new CCollidableSphere(CSphere(CVector3f::Zero(), radius),
                                                CMaterialList(kMT_Solid, kMT_NoStaticCollision)))
, mSphereRadius(radius)
, mHealthInfo(0.f, 0.f)
, mDamageVuln(CDamageVulnerability::NormalVulnerability())
, mLastTouched(kInvalidUniqueId)
, mResponseType(kWCR_EnemyNormal)
, mExtendedTouchBounds(CVector3f::Zero()) {
  SetCoefficientOfRestitutionModifier(0.5f);
  SetCallTouch(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_CollisionActor, kMT_NoStaticCollision)));
}

ENTITY_ACCEPT_IMPL(CCollisionActor)

rstl::optional_object< CAABox > CCollisionActor::GetTouchBounds() const {
  rstl::optional_object< CAABox > bounds;
  if (mPrimitiveType == kPT_OBBTreeGroup) {
    bounds = mObbTreeGroupPrimitive->CalculateAABox(GetTransform());
  } else if (mPrimitiveType == kPT_AABox) {
    bounds = mAaboxPrimitive->CalculateAABox(GetTransform());
  } else {
    bounds = mSpherePrimitive->CalculateAABox(GetTransform());
  }
  bounds->AccumulateBounds(bounds->GetMaxPoint() + mExtendedTouchBounds);
  bounds->AccumulateBounds(bounds->GetMinPoint() - mExtendedTouchBounds);
  return bounds;
}

CVector3f CCollisionActor::GetOrbitPosition(const CStateManager&) const {
  return GetTouchBounds()->GetCenterPoint();
}

CVector3f CCollisionActor::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  float scanScale;
  if (mPrimitiveType == kPT_Sphere) {
    scanScale = GetSphereRadius();
  } else {
    const CVector3f size = GetBoxSize();
    scanScale = 0.5f * rstl::max_val(rstl::max_val(size.GetX(), size.GetY()), size.GetZ());
  }
  scanScale *= 3.f;
  const CVector3f orbitPos = GetOrbitPosition(mgr);
  return orbitPos - scanScale * (orbitPos - camera.GetTranslation()).AsNormalized();
}

void CCollisionActor::Touch(CActor& actor, CStateManager& mgr) {
  mLastTouched = actor.GetUniqueId();
  mgr.SendScriptMsgAlways(mOwner, GetUniqueId(), kSM_Touched);
}

const CCollisionPrimitive* CCollisionActor::GetCollisionPrimitive() const {
  if (mPrimitiveType == kPT_OBBTreeGroup) {
    return mObbTreeGroupPrimitive.get();
  }
  if (mPrimitiveType == kPT_AABox) {
    return mAaboxPrimitive.get();
  }
  return mSpherePrimitive.get();
}

CTransform4f CCollisionActor::GetPrimitiveTransform() const {
  CTransform4f xf = GetTransform();
  xf.SetTranslation(CPhysicsActor::GetPrimitiveTransform().GetTranslation());
  return xf;
}

void CCollisionActor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                      CStateManager& mgr) {
  switch (msg) {
  case kSM_Falling:
  case kSM_Registered:
  case kSM_Deleted:
  case kSM_InitializedInArea:
    break;
  case kSM_Touched:
  case kSM_Damage:
  case kSM_InvulnDamage:
    if (CEntity* ent = mgr.ObjectById(mOwner)) {
      mLastTouched = sender;
      mgr.DeliverScriptMsg(ent, GetUniqueId(), msg);
    }
    break;
  default:
    mgr.SendScriptMsgAlways(mOwner, GetUniqueId(), msg);
    break;
  }
  CActor::AcceptScriptMsg(msg, sender, mgr);
}

void CCollisionActor::OnScanStateChange(EScanState state, CStateManager& mgr) {
  if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(mOwner))) {
    actor->OnScanStateChange(state, mgr);
  }
  CActor::OnScanStateChange(state, mgr);
}

CHealthInfo* CCollisionActor::HealthInfo(CStateManager&) { return &mHealthInfo; }

const CDamageVulnerability* CCollisionActor::GetDamageVulnerability() const {
  return &mDamageVuln;
}

const CDamageVulnerability* CCollisionActor::GetDamageVulnerability(const CVector3f&,
                                                                    const CVector3f&,
                                                                    const CDamageInfo&) const {
  return GetDamageVulnerability();
}

void CCollisionActor::SetDamageVulnerability(const CDamageVulnerability& vulnerability) {
  mDamageVuln = vulnerability;
}

TUniqueId CCollisionActor::GetLastTouchedObject() const { return mLastTouched; }

const CVector3f& CCollisionActor::GetBoxSize() const { return mBoxSize; }

float CCollisionActor::GetSphereRadius() const { return mSphereRadius; }

void CCollisionActor::SetSphereRadius(float radius) {
  if (mPrimitiveType == kPT_Sphere) {
    mSphereRadius = radius;
    mSpherePrimitive->SetSphere(CSphere(mSpherePrimitive->GetSphere().GetCenter(), radius));
  }
}

EWeaponCollisionResponseTypes CCollisionActor::GetCollisionResponseType(const CVector3f&,
                                                                        const CVector3f&,
                                                                        const CWeaponMode&,
                                                                        int) const {
  return mResponseType;
}

CCollisionActor::~CCollisionActor() {}
