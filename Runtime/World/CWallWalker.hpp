#pragma once

#include <string_view>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Collision/CCollidableSphere.hpp"
#include "Runtime/Collision/CCollisionSurface.hpp"
#include "Runtime/World/CPatterned.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
class CWallWalker : public CPatterned {
public:
  enum class EWalkerType { Parasite = 0, Oculus = 1, Geemer = 2, IceZoomer = 3, Seedling = 4 };

protected:
  CCollisionSurface x568_alignNormal =
      CCollisionSurface(zeus::CVector3f(), zeus::skForward, zeus::skRight, -1);
  CCollidableSphere x590_colSphere;
  float x5b0_collisionCloseMargin;
  float x5b4_alignAngVel;
  float x5b8_tumbleAngle = 0.f;
  float x5bc_patrolPauseRemTime = 0.f;
  float x5c0_advanceWpRadius;
  float x5c4_playerObstructionMinDist;
  float x5c8_bendingHackWeight = 0.f;
  s32 x5cc_bendingHackAnim;
  EWalkerType x5d0_walkerType;
  s16 x5d4_thinkCounter = 0;
  bool x5d6_24_alignToFloor : 1 = false;
  bool x5d6_25_hasAlignSurface : 1 = false;
  bool x5d6_26_playerObstructed : 1 = false;
  bool x5d6_27_disableMove : 1;
  bool x5d6_28_addBendingWeight : 1 = true;
  bool x5d6_29_applyBendingHack : 1 = false;
  static zeus::CVector3f ProjectVectorToPlane(const zeus::CVector3f& pt, const zeus::CVector3f& plane) {
    return pt - plane * pt.dot(plane);
  }
  static zeus::CVector3f ProjectPointToPlane(const zeus::CVector3f& p0, const zeus::CVector3f& p1,
                                             const zeus::CVector3f& plane) {
    return p0 - (p0 - p1).dot(plane) * plane;
  }
  void OrientToSurfaceNormal(const zeus::CVector3f& normal, float clampAngle);
  static bool PointOnSurface(const CCollisionSurface& surf, const zeus::CVector3f& point);
  void AlignToFloor(CStateManager&, float, const zeus::CVector3f&, float);
  void GotoNextWaypoint(CStateManager& mgr);

public:
  CWallWalker(ECharacter chr, TUniqueId uid, std::string_view name, EFlavorType flavType, const CEntityInfo& eInfo,
              const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, EMovementType mType,
              EColliderType colType, EBodyType bType, const CActorParameters& aParms, float collisionCloseMargin,
              float alignAngVel, EKnockBackVariant kbVariant, float advanceWpRadius, EWalkerType wType,
              float playerObstructionMinDist, bool disableMove);

  void PreThink(float, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void Render(CStateManager&) override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &x590_colSphere; }
  void UpdateWPDestination(CStateManager&);
};
} // namespace urde
