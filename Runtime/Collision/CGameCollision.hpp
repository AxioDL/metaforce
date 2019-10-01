#pragma once

#include <optional>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Collision/CCollisionPrimitive.hpp"
#include "Runtime/Collision/CMetroidAreaCollider.hpp"
#include "Runtime/Collision/CRayCastResult.hpp"

#include <zeus/CPlane.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {

class CActor;
class CCollisionInfo;
class CCollisionInfoList;
class CGameArea;
class CMaterialFilter;
class CMaterialList;
class CPhysicsActor;
class CStateManager;
class ICollisionFilter;

class CGameCollision {
  static void MovePlayer(CStateManager& mgr, CPhysicsActor& actor, float dt,
                         const rstl::reserved_vector<TUniqueId, 1024>* colliderList);
  static void MoveAndCollide(CStateManager& mgr, CPhysicsActor& actor, float dt, const ICollisionFilter& filter,
                             const rstl::reserved_vector<TUniqueId, 1024>* colliderList);
  static zeus::CVector3f GetActorRelativeVelocities(const CPhysicsActor& act0, const CPhysicsActor* act1);

public:
  static float GetCoefficientOfRestitution(const CCollisionInfo&) { return 0.f; }
  static bool NullMovingCollider(const CInternalCollisionStructure&, const zeus::CVector3f&, double&, CCollisionInfo&) {
    return false;
  }
  static bool NullBooleanCollider(const CInternalCollisionStructure&) { return false; }
  static bool NullCollisionCollider(const CInternalCollisionStructure&, CCollisionInfoList&) { return false; }
  static void InitCollision();
  static void Move(CStateManager& mgr, CPhysicsActor& actor, float dt,
                   const rstl::reserved_vector<TUniqueId, 1024>* colliderList);

  static bool CanBlock(const CMaterialList&, const zeus::CVector3f&);
  static bool IsFloor(const CMaterialList&, const zeus::CVector3f&);
  static void SendMaterialMessage(CStateManager&, const CMaterialList&, CActor&);
  static CRayCastResult RayStaticIntersection(const CStateManager& mgr, const zeus::CVector3f& pos,
                                              const zeus::CVector3f& dir, float mag, const CMaterialFilter& filter);
  static bool RayStaticIntersectionBool(const CStateManager& mgr, const zeus::CVector3f& start,
                                        const zeus::CVector3f& dir, float length, const CMaterialFilter& filter);
  static CRayCastResult RayDynamicIntersection(const CStateManager& mgr, TUniqueId& idOut, const zeus::CVector3f& pos,
                                               const zeus::CVector3f& dir, float mag, const CMaterialFilter& filter,
                                               const rstl::reserved_vector<TUniqueId, 1024>& nearList);
  static bool RayDynamicIntersectionBool(const CStateManager& mgr, const zeus::CVector3f& pos,
                                         const zeus::CVector3f& dir, const CMaterialFilter& filter,
                                         const rstl::reserved_vector<TUniqueId, 1024>& nearList, const CActor* damagee,
                                         float length);
  static CRayCastResult RayWorldIntersection(const CStateManager& mgr, TUniqueId& idOut, const zeus::CVector3f& pos,
                                             const zeus::CVector3f& dir, float mag, const CMaterialFilter& filter,
                                             const rstl::reserved_vector<TUniqueId, 1024>& nearList);
  static bool RayStaticIntersectionArea(const CGameArea& area, const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                        float mag, const CMaterialFilter& filter);
  static void BuildAreaCollisionCache(const CStateManager& mgr, CAreaCollisionCache& cache);
  static float GetMinExtentForCollisionPrimitive(const CCollisionPrimitive& prim);
  static bool DetectCollisionBoolean(const CStateManager& mgr, const CCollisionPrimitive& prim,
                                     const zeus::CTransform& xf, const CMaterialFilter& filter,
                                     const rstl::reserved_vector<TUniqueId, 1024>& nearList);
  static bool DetectCollisionBoolean_Cached(const CStateManager& mgr, CAreaCollisionCache& cache,
                                            const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                            const CMaterialFilter& filter,
                                            const rstl::reserved_vector<TUniqueId, 1024>& nearList);
  static bool DetectStaticCollisionBoolean(const CStateManager& mgr, const CCollisionPrimitive& prim,
                                           const zeus::CTransform& xf, const CMaterialFilter& filter);
  static bool DetectStaticCollisionBoolean_Cached(const CStateManager& mgr, CAreaCollisionCache& cache,
                                                  const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                                  const CMaterialFilter& filter);
  static bool DetectDynamicCollisionBoolean(const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                            const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                            const CStateManager& mgr);
  static bool DetectCollision_Cached(const CStateManager& mgr, CAreaCollisionCache& cache,
                                     const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                     const CMaterialFilter& filter,
                                     const rstl::reserved_vector<TUniqueId, 1024>& nearList, TUniqueId& idOut,
                                     CCollisionInfoList& infoList);
  static bool DetectCollision_Cached_Moving(const CStateManager& mgr, CAreaCollisionCache& cache,
                                            const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                            const CMaterialFilter& filter,
                                            const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                            const zeus::CVector3f& vec, TUniqueId& idOut, CCollisionInfo& infoOut,
                                            double&);
  static bool DetectStaticCollision(const CStateManager& mgr, const CCollisionPrimitive& prim,
                                    const zeus::CTransform& xf, const CMaterialFilter& filter,
                                    CCollisionInfoList& list);
  static bool DetectStaticCollision_Cached(const CStateManager& mgr, CAreaCollisionCache& cache,
                                           const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                           const CMaterialFilter& filter, CCollisionInfoList& list);
  static bool DetectStaticCollision_Cached_Moving(const CStateManager& mgr, CAreaCollisionCache& cache,
                                                  const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                                  const CMaterialFilter& filter, const zeus::CVector3f& vec,
                                                  CCollisionInfo& infoOut, double& d);
  static bool DetectDynamicCollision(const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                     const rstl::reserved_vector<TUniqueId, 1024>& nearList, TUniqueId& idOut,
                                     CCollisionInfoList& list, const CStateManager& mgr);
  static bool DetectDynamicCollisionMoving(const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                           const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                           const zeus::CVector3f& vec, TUniqueId& idOut, CCollisionInfo& infoOut,
                                           double& d, const CStateManager& mgr);
  static void MakeCollisionCallbacks(CStateManager& mgr, CPhysicsActor& actor, TUniqueId id,
                                     const CCollisionInfoList& list);
  static void SendScriptMessages(CStateManager& mgr, CActor& a0, CActor* a1, const CCollisionInfoList& list);
  static void ResolveCollisions(CPhysicsActor& a0, CPhysicsActor* a1, const CCollisionInfoList& list);
  static void CollideWithDynamicBodyNoRot(CPhysicsActor& a0, CPhysicsActor& a1, const CCollisionInfo& info,
                                          float restitution, bool);
  static void CollideWithStaticBodyNoRot(CPhysicsActor& a0, const CMaterialList& m0, const CMaterialList& m1,
                                         const zeus::CUnitVector3f& normal, float restitution, bool);
  static void CollisionFailsafe(const CStateManager& mgr, CAreaCollisionCache& cache, CPhysicsActor& actor,
                                const CCollisionPrimitive& prim, const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                float, u32 failsafeTicks);
  static std::optional<zeus::CVector3f>
  FindNonIntersectingVector(const CStateManager& mgr, CAreaCollisionCache& cache, CPhysicsActor& actor,
                            const CCollisionPrimitive& prim, const rstl::reserved_vector<TUniqueId, 1024>& nearList);
};
} // namespace urde
