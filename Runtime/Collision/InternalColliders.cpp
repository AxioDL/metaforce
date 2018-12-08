#include "InternalColliders.hpp"
#include "CCollidableAABox.hpp"
#include "CCollidableCollisionSurface.hpp"
#include "CCollidableSphere.hpp"

namespace urde::InternalColliders {
void AddTypes() {
  CCollisionPrimitive::InitAddType(CCollidableAABox::GetType());
  CCollisionPrimitive::InitAddType(CCollidableCollisionSurface::GetType());
  CCollisionPrimitive::InitAddType(CCollidableSphere::GetType());
}

void AddColliders() {
  CCollisionPrimitive::InitAddCollider(Collide::AABox_AABox, "CCollidableAABox", "CCollidableAABox");
  CCollisionPrimitive::InitAddCollider(Collide::Sphere_AABox, "CCollidableSphere", "CCollidableAABox");
  CCollisionPrimitive::InitAddCollider(Collide::Sphere_Sphere, "CCollidableSphere", "CCollidableSphere");
  CCollisionPrimitive::InitAddBooleanCollider(Collide::AABox_AABox_Bool, "CCollidableAABox", "CCollidableAABox");
  CCollisionPrimitive::InitAddBooleanCollider(Collide::Sphere_AABox_Bool, "CCollidableSphere", "CCollidableAABox");
  CCollisionPrimitive::InitAddBooleanCollider(Collide::Sphere_Sphere_Bool, "CCollidableSphere", "CCollidableSphere");
  CCollisionPrimitive::InitAddMovingCollider(CCollidableAABox::CollideMovingAABox, "CCollidableAABox",
                                             "CCollidableAABox");
  CCollisionPrimitive::InitAddMovingCollider(CCollidableAABox::CollideMovingSphere, "CCollidableAABox",
                                             "CCollidableSphere");
  CCollisionPrimitive::InitAddMovingCollider(CCollidableSphere::CollideMovingAABox, "CCollidableSphere",
                                             "CCollidableAABox");
  CCollisionPrimitive::InitAddMovingCollider(CCollidableSphere::CollideMovingSphere, "CCollidableSphere",
                                             "CCollidableSphere");
}
} // namespace urde::InternalColliders
