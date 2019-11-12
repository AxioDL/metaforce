#include "CCollisionActor.hpp"
#include "Camera/CGameCamera.hpp"
#include "CStateManager.hpp"
#include "World/CActorParameters.hpp"
#include "Collision/CCollidableOBBTreeGroup.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
static const CMaterialList gkDefaultCollisionActorMaterials =
    CMaterialList(EMaterialTypes::Solid, EMaterialTypes::CollisionActor, EMaterialTypes::ScanPassthrough,
                  EMaterialTypes::CameraPassthrough);

CCollisionActor::CCollisionActor(TUniqueId uid1, TAreaId aId, TUniqueId uid2, const zeus::CVector3f& extent,
                                 const zeus::CVector3f& center, bool active, float mass, std::string_view name)
: CPhysicsActor(uid1, active, "CollisionActor", CEntityInfo(aId, CEntity::NullConnectionList),
                zeus::CTransform(), CModelData::CModelDataNull(), gkDefaultCollisionActorMaterials,
                zeus::skNullBox, SMoverData(mass), CActorParameters::None(), 0.3f, 0.1f)
, x258_primitiveType(EPrimitiveType::OBBTreeGroup)
, x25c_owner(uid2)
, x260_boxSize(extent)
, x26c_center(center)
, x278_obbContainer(std::make_unique<CCollidableOBBTreeGroupContainer>(extent, center))
, x27c_obbTreeGroupPrimitive(std::make_unique<CCollidableOBBTreeGroup>(x278_obbContainer.get(), GetMaterialList())) {
  x10_name += ' ';
  x10_name += name;
  SetCoefficientOfRestitutionModifier(0.5f);
  SetCallTouch(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      {EMaterialTypes::Solid}, {EMaterialTypes::CollisionActor, EMaterialTypes::NoStaticCollision}));
}

CCollisionActor::CCollisionActor(TUniqueId uid1, TAreaId aId, TUniqueId uid2, const zeus::CVector3f& boxSize,
                                 bool active, float mass, std::string_view name)
: CPhysicsActor(uid1, active, "CollisionActor", CEntityInfo(aId, CEntity::NullConnectionList),
                zeus::CTransform(), CModelData::CModelDataNull(), gkDefaultCollisionActorMaterials,
                zeus::skNullBox, SMoverData(mass), CActorParameters::None(), 0.3f, 0.1f)
, x258_primitiveType(EPrimitiveType::AABox)
, x25c_owner(uid2)
, x260_boxSize(boxSize)
, x280_aaboxPrimitive(
      std::make_unique<CCollidableAABox>(zeus::CAABox(-0.5f * boxSize, 0.5f * boxSize),
                                         CMaterialList(EMaterialTypes::Solid, EMaterialTypes::NoStaticCollision))) {
  x10_name += ' ';
  x10_name += name;
  SetCoefficientOfRestitutionModifier(0.5f);
  SetCallTouch(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      {EMaterialTypes::Solid}, {EMaterialTypes::CollisionActor, EMaterialTypes::NoStaticCollision}));
}

CCollisionActor::CCollisionActor(TUniqueId uid1, TAreaId aId, TUniqueId uid2, bool active, float radius, float mass,
                                 std::string_view name)
: CPhysicsActor(uid1, active, "CollisionActor", CEntityInfo(aId, CEntity::NullConnectionList),
                zeus::CTransform(), CModelData::CModelDataNull(), gkDefaultCollisionActorMaterials,
                zeus::skNullBox, SMoverData(mass), CActorParameters::None(), 0.3f, 0.1f)
, x258_primitiveType(EPrimitiveType::Sphere)
, x25c_owner(uid2)
, x284_spherePrimitive(std::make_unique<CCollidableSphere>(
      zeus::CSphere(zeus::skZero3f, radius), CMaterialList(EMaterialTypes::NoStaticCollision, EMaterialTypes::Solid)))
, x288_sphereRadius(radius) {
  x10_name += ' ';
  x10_name += name;
  SetCoefficientOfRestitutionModifier(0.5f);
  SetCallTouch(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      {EMaterialTypes::Solid}, {EMaterialTypes::CollisionActor, EMaterialTypes::NoStaticCollision}));
}

void CCollisionActor::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CCollisionActor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Falling:
  case EScriptObjectMessage::Registered:
  case EScriptObjectMessage::Deleted:
  case EScriptObjectMessage::InitializedInArea:
    break;
  case EScriptObjectMessage::Touched:
  case EScriptObjectMessage::Damage:
  case EScriptObjectMessage::InvulnDamage: {
    if (CEntity* ent = mgr.ObjectById(x25c_owner)) {
      x2fc_lastTouched = uid;
      mgr.SendScriptMsg(ent, GetUniqueId(), msg);
    }
  } break;
  default:
    mgr.SendScriptMsgAlways(x25c_owner, GetUniqueId(), msg);
    break;
  }

  CActor::AcceptScriptMsg(msg, uid, mgr);
}

CHealthInfo* CCollisionActor::HealthInfo(CStateManager&) { return &x28c_healthInfo; }

const CDamageVulnerability* CCollisionActor::GetDamageVulnerability() const { return &x294_damageVuln; }

const CDamageVulnerability* CCollisionActor::GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                                    const CDamageInfo&) const {
  return GetDamageVulnerability();
}

void CCollisionActor::SetDamageVulnerability(const CDamageVulnerability& vuln) { x294_damageVuln = vuln; }

zeus::CVector3f CCollisionActor::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  const CGameCamera* gameCamera = static_cast<const CGameCamera*>(mgr.GetCameraManager()->GetCurrentCamera(mgr));

  float scanScale;
  if (x258_primitiveType == EPrimitiveType::Sphere)
    scanScale = GetSphereRadius();
  else {
    const zeus::CVector3f v = GetBoxSize();
    float comp = (v.x() < v.y() ? v.y() : v.z());
    comp = (comp < v.z() ? v.x() : comp);

    scanScale = 0.5f * comp;
  }
  scanScale *= 3.0f;
  zeus::CVector3f orbitPos = GetOrbitPosition(mgr);
  return orbitPos - scanScale * (orbitPos - gameCamera->GetTranslation()).normalized();
}

const CCollisionPrimitive* CCollisionActor::GetCollisionPrimitive() const {
  if (x258_primitiveType == EPrimitiveType::OBBTreeGroup)
    return x27c_obbTreeGroupPrimitive.get();
  if (x258_primitiveType == EPrimitiveType::AABox)
    return x280_aaboxPrimitive.get();
  return x284_spherePrimitive.get();
}

EWeaponCollisionResponseTypes CCollisionActor::GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                                        const CWeaponMode&, EProjectileAttrib) const {
  return x300_responseType;
}

zeus::CTransform CCollisionActor::GetPrimitiveTransform() const {
  zeus::CTransform xf = x34_transform;
  xf.origin = CPhysicsActor::GetPrimitiveTransform().origin;
  return xf;
}

std::optional<zeus::CAABox> CCollisionActor::GetTouchBounds() const {
  std::optional<zeus::CAABox> aabox;
  if (x258_primitiveType == EPrimitiveType::OBBTreeGroup)
    aabox = {x27c_obbTreeGroupPrimitive->CalculateAABox(x34_transform)};
  else if (x258_primitiveType == EPrimitiveType::AABox)
    aabox = {x280_aaboxPrimitive->CalculateAABox(x34_transform)};
  else if (x258_primitiveType == EPrimitiveType::Sphere)
    aabox = {x284_spherePrimitive->CalculateAABox(x34_transform)};

  aabox->accumulateBounds(aabox->max + x304_extendedTouchBounds);
  aabox->accumulateBounds(aabox->min - x304_extendedTouchBounds);

  return aabox;
}

void CCollisionActor::OnScanStateChanged(CActor::EScanState state, CStateManager& mgr) {
  TCastToPtr<CActor> actor = mgr.ObjectById(x25c_owner);
  if (actor)
    actor->OnScanStateChanged(state, mgr);

  CActor::OnScanStateChanged(state, mgr);
}

void CCollisionActor::Touch(CActor& actor, CStateManager& mgr) {
  x2fc_lastTouched = actor.GetUniqueId();
  mgr.SendScriptMsgAlways(x25c_owner, GetUniqueId(), EScriptObjectMessage::Touched);
}

zeus::CVector3f CCollisionActor::GetOrbitPosition(const CStateManager&) const { return GetTouchBounds()->center(); }

void CCollisionActor::SetSphereRadius(float radius) {
  if (x258_primitiveType != EPrimitiveType::Sphere)
    return;

  x288_sphereRadius = radius;
  x284_spherePrimitive->SetSphereRadius(radius);
}
} // namespace urde
