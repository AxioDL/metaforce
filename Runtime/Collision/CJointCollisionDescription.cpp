#include "Runtime/Collision/CJointCollisionDescription.hpp"

namespace urde {

CJointCollisionDescription::CJointCollisionDescription(ECollisionType colType, CSegId pivotId, CSegId nextId,
                                                       const zeus::CVector3f& bounds, const zeus::CVector3f& pivotPoint,
                                                       float radius, float maxSeparation, EOrientationType orientType,
                                                       std::string_view name, float mass)
: x0_colType(colType)
, x4_orientType(orientType)
, x8_pivotId(pivotId)
, x9_nextId(nextId)
, xc_bounds(bounds)
, x18_pivotPoint(pivotPoint)
, x24_radius(radius)
, x28_maxSeparation(maxSeparation)
, x2c_name(name)
, x40_mass(mass) {}

CJointCollisionDescription CJointCollisionDescription::SphereSubdivideCollision(CSegId pivotId, CSegId nextId,
                                                                                float radius, float maxSeparation,
                                                                                EOrientationType orientType,
                                                                                std::string_view name, float mass) {
  return CJointCollisionDescription(ECollisionType::SphereSubdivide, pivotId, nextId, zeus::skZero3f,
                                    zeus::skZero3f, radius, maxSeparation, orientType, name, mass);
}

CJointCollisionDescription CJointCollisionDescription::SphereCollision(CSegId pivotId, float radius,
                                                                       std::string_view name, float mass) {
  return CJointCollisionDescription(ECollisionType::Sphere, pivotId, {}, zeus::skZero3f,
                                    zeus::skZero3f, radius, 0.f, EOrientationType::Zero, name, mass);
}

CJointCollisionDescription CJointCollisionDescription::AABoxCollision(CSegId pivotId, const zeus::CVector3f& bounds,
                                                                      std::string_view name, float mass) {
  return CJointCollisionDescription(ECollisionType::AABox, pivotId, {}, bounds, zeus::skZero3f, 0.f, 0.f,
                                    EOrientationType::Zero, name, mass);
}

CJointCollisionDescription CJointCollisionDescription::OBBAutoSizeCollision(CSegId pivotId, CSegId nextId,
                                                                            const zeus::CVector3f& bounds,
                                                                            EOrientationType orientType,
                                                                            std::string_view name, float mass) {
  return CJointCollisionDescription(ECollisionType::OBBAutoSize, pivotId, nextId, bounds, zeus::skZero3f, 0.f,
                                    0.f, orientType, name, mass);
}

CJointCollisionDescription CJointCollisionDescription::OBBCollision(CSegId pivotId, const zeus::CVector3f& bounds,
                                                                    const zeus::CVector3f& pivotPoint,
                                                                    std::string_view name, float mass) {
  return CJointCollisionDescription(ECollisionType::OBB, pivotId, {}, bounds, pivotPoint, 0.f, 0.f,
                                    EOrientationType::Zero, name, mass);
}

void CJointCollisionDescription::ScaleAllBounds(const zeus::CVector3f& scale) {
  xc_bounds *= scale;
  x24_radius *= scale.x();
  x28_maxSeparation *= scale.x();
  x18_pivotPoint *= scale;
}
} // namespace urde
