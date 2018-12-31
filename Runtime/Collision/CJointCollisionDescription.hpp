#pragma once

#include "Character/CSegId.hpp"
#include "zeus/CAABox.hpp"

namespace urde {
struct SOBBJointInfo {
  const char* from;
  const char* to;
  zeus::CVector3f bounds;
};

struct SSphereJointInfo {
  const char* name;
  float radius;
};

class CJointCollisionDescription {
public:
  enum class ECollisionType {
    Sphere,
    SphereSubdivide,
    AABox,
    OBBAutoSize,
    OBB,
  };

  enum class EOrientationType { Zero, One };

private:
  ECollisionType x0_colType;
  EOrientationType x4_orientType;
  CSegId x8_pivotId;
  CSegId x9_nextId;
  zeus::CVector3f xc_bounds;
  zeus::CVector3f x18_pivotPoint;
  float x24_radius;
  float x28_maxSeparation;
  std::string x2c_name;
  TUniqueId x3c_actorId = kInvalidUniqueId;
  float x40_mass;

public:
  CJointCollisionDescription(ECollisionType, CSegId, CSegId, const zeus::CVector3f&, const zeus::CVector3f&, float,
                             float, EOrientationType, std::string_view, float);
  static CJointCollisionDescription SphereSubdivideCollision(CSegId pivotId, CSegId nextId, float radius,
                                                             float maxSeparation, EOrientationType orientType,
                                                             std::string_view name, float mass);
  static CJointCollisionDescription SphereCollision(CSegId pivotId, float radius, std::string_view name, float mass);
  static CJointCollisionDescription AABoxCollision(CSegId pivotId, const zeus::CVector3f& bounds, std::string_view name,
                                                   float mass);
  static CJointCollisionDescription OBBAutoSizeCollision(CSegId pivotId, CSegId nextId, const zeus::CVector3f& bounds,
                                                         EOrientationType orientType, std::string_view, float mass);
  static CJointCollisionDescription OBBCollision(CSegId pivotId, const zeus::CVector3f& bounds,
                                                 const zeus::CVector3f& pivotPoint, std::string_view name, float mass);
  std::string_view GetName() const { return x2c_name; }
  TUniqueId GetCollisionActorId() const { return x3c_actorId; }
  void SetCollisionActorId(TUniqueId id) { x3c_actorId = id; }
  const zeus::CVector3f& GetBounds() const { return xc_bounds; }
  float GetRadius() const { return x24_radius; }
  float GetMaxSeparation() const { return x28_maxSeparation; }
  EOrientationType GetOrientationType() const { return x4_orientType; }
  float GetMass() const { return x40_mass; }
  const zeus::CVector3f& GetPivotPoint() const { return x18_pivotPoint; }
  ECollisionType GetType() const { return x0_colType; }
  CSegId GetNextId() const { return x9_nextId; }
  CSegId GetPivotId() const { return x8_pivotId; }
  void ScaleAllBounds(const zeus::CVector3f& scale);
};
} // namespace urde
