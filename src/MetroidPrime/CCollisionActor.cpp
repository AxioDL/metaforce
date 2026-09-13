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
, x258_primitiveType(kPT_OBBTreeGroup)
, x25c_owner(owner)
, x260_boxSize(extent)
, x26c_center(center)
, x278_obbContainer(rs_new CCollidableOBBTreeGroupContainer(extent, center))
, x27c_obbTreeGroupPrimitive(
      rs_new CCollidableOBBTreeGroup(x278_obbContainer.get(), GetMaterialList()))
, x280_aaboxPrimitive(nullptr)
, x284_spherePrimitive(nullptr)
, x288_sphereRadius(0.f)
, x28c_healthInfo(0.f, 0.f)
, x294_damageVuln(CDamageVulnerability::NormalVulnerability())
, x2fc_lastTouched(kInvalidUniqueId)
, x300_responseType(kWCR_EnemyNormal)
, x304_extendedTouchBounds(CVector3f::Zero()) {
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
, x258_primitiveType(kPT_AABox)
, x25c_owner(owner)
, x260_boxSize(boxSize)
, x26c_center(CVector3f::Zero())
, x278_obbContainer(nullptr)
, x27c_obbTreeGroupPrimitive(nullptr)
, x280_aaboxPrimitive(rs_new CCollidableAABox(CAABox(-0.5f * x260_boxSize, 0.5f * x260_boxSize),
                                              CMaterialList(kMT_Solid, kMT_NoStaticCollision)))
, x284_spherePrimitive(nullptr)
, x288_sphereRadius(0.f)
, x28c_healthInfo(0.f, 0.f)
, x294_damageVuln(CDamageVulnerability::NormalVulnerability())
, x2fc_lastTouched(kInvalidUniqueId)
, x300_responseType(kWCR_EnemyNormal)
, x304_extendedTouchBounds(CVector3f::Zero()) {
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
, x258_primitiveType(kPT_Sphere)
, x25c_owner(owner)
, x260_boxSize(CVector3f::Zero())
, x26c_center(CVector3f::Zero())
, x278_obbContainer(nullptr)
, x27c_obbTreeGroupPrimitive(nullptr)
, x280_aaboxPrimitive(nullptr)
, x284_spherePrimitive(rs_new CCollidableSphere(CSphere(CVector3f::Zero(), radius),
                                                CMaterialList(kMT_Solid, kMT_NoStaticCollision)))
, x288_sphereRadius(radius)
, x28c_healthInfo(0.f, 0.f)
, x294_damageVuln(CDamageVulnerability::NormalVulnerability())
, x2fc_lastTouched(kInvalidUniqueId)
, x300_responseType(kWCR_EnemyNormal)
, x304_extendedTouchBounds(CVector3f::Zero()) {
  SetCoefficientOfRestitutionModifier(0.5f);
  SetCallTouch(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_CollisionActor, kMT_NoStaticCollision)));
}

ENTITY_ACCEPT_IMPL(CCollisionActor)

rstl::optional_object< CAABox > CCollisionActor::GetTouchBounds() const {
  rstl::optional_object< CAABox > bounds;
  if (x258_primitiveType == kPT_OBBTreeGroup) {
    bounds = x27c_obbTreeGroupPrimitive->CalculateAABox(GetTransform());
  } else if (x258_primitiveType == kPT_AABox) {
    bounds = x280_aaboxPrimitive->CalculateAABox(GetTransform());
  } else {
    bounds = x284_spherePrimitive->CalculateAABox(GetTransform());
  }
  bounds->AccumulateBounds(bounds->GetMaxPoint() + x304_extendedTouchBounds);
  bounds->AccumulateBounds(bounds->GetMinPoint() - x304_extendedTouchBounds);
  return bounds;
}

CVector3f CCollisionActor::GetOrbitPosition(const CStateManager&) const {
  return GetTouchBounds()->GetCenterPoint();
}

CVector3f CCollisionActor::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  float scanScale;
  if (x258_primitiveType == kPT_Sphere) {
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
  x2fc_lastTouched = actor.GetUniqueId();
  mgr.SendScriptMsgAlways(x25c_owner, GetUniqueId(), kSM_Touched);
}

const CCollisionPrimitive* CCollisionActor::GetCollisionPrimitive() const {
  if (x258_primitiveType == kPT_OBBTreeGroup) {
    return x27c_obbTreeGroupPrimitive.get();
  }
  if (x258_primitiveType == kPT_AABox) {
    return x280_aaboxPrimitive.get();
  }
  return x284_spherePrimitive.get();
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
    if (CEntity* ent = mgr.ObjectById(x25c_owner)) {
      x2fc_lastTouched = sender;
      mgr.DeliverScriptMsg(ent, GetUniqueId(), msg);
    }
    break;
  default:
    mgr.SendScriptMsgAlways(x25c_owner, GetUniqueId(), msg);
    break;
  }
  CActor::AcceptScriptMsg(msg, sender, mgr);
}

void CCollisionActor::OnScanStateChange(EScanState state, CStateManager& mgr) {
  if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(x25c_owner))) {
    actor->OnScanStateChange(state, mgr);
  }
  CActor::OnScanStateChange(state, mgr);
}

CHealthInfo* CCollisionActor::HealthInfo(CStateManager&) { return &x28c_healthInfo; }

const CDamageVulnerability* CCollisionActor::GetDamageVulnerability() const {
  return &x294_damageVuln;
}

const CDamageVulnerability* CCollisionActor::GetDamageVulnerability(const CVector3f&,
                                                                    const CVector3f&,
                                                                    const CDamageInfo&) const {
  return GetDamageVulnerability();
}

void CCollisionActor::SetDamageVulnerability(const CDamageVulnerability& vulnerability) {
  x294_damageVuln = vulnerability;
}

TUniqueId CCollisionActor::GetLastTouchedObject() const { return x2fc_lastTouched; }

const CVector3f& CCollisionActor::GetBoxSize() const { return x260_boxSize; }

float CCollisionActor::GetSphereRadius() const { return x288_sphereRadius; }

void CCollisionActor::SetSphereRadius(float radius) {
  if (x258_primitiveType == kPT_Sphere) {
    x288_sphereRadius = radius;
    x284_spherePrimitive->SetSphere(CSphere(x284_spherePrimitive->GetSphere().GetCenter(), radius));
  }
}

EWeaponCollisionResponseTypes CCollisionActor::GetCollisionResponseType(const CVector3f&,
                                                                        const CVector3f&,
                                                                        const CWeaponMode&,
                                                                        int) const {
  return x300_responseType;
}

CCollisionActor::~CCollisionActor() {}
