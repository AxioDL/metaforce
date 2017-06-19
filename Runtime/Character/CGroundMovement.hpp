#ifndef __URDE_CGROUNDMOVEMENT_HPP__
#define __URDE_CGROUNDMOVEMENT_HPP__

#include "RetroTypes.hpp"
#include "Collision/CCollisionInfo.hpp"

namespace urde
{
class CPhysicsActor;
class CStateManager;
class CAreaCollisionCache;
class CMaterialFilter;
class CCollisionInfoList;

class CGroundMovement
{
public:
    struct SMovementOptions
    {
        bool x0_;
        float x4_;
        float x8_;
        float xc_;
        float x10_;
        float x14_;
        bool x18_;
        bool x19_;
        bool x1a_;
        u32 x1c_;
        float x20_;
        float x24_;
        float x28_;
        float x2c_;
        float x30_;
        float x34_;
        float x38_;
        std::experimental::optional<zeus::CVector3f> x3c_;
    };

    struct SMoveObjectResult
    {
        std::experimental::optional<TUniqueId> x0_id;
        std::experimental::optional<CCollisionInfo> x8_collision;
        u32 x6c_;
        float x70_;
    };

    static void CheckFalling(CPhysicsActor& actor, CStateManager& mgr, float);
    static void MoveGroundCollider(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                   const rstl::reserved_vector<TUniqueId, 1024>* nearList);
    static bool ResolveUpDown(CAreaCollisionCache& cache, CStateManager& mgr, CPhysicsActor& actor,
                              const CMaterialFilter& filter, rstl::reserved_vector<TUniqueId, 1024>& nearList,
                              float, float, float&, CCollisionInfoList& list);
    static bool MoveGroundColliderZ(CAreaCollisionCache& cache, CStateManager& mgr, CPhysicsActor& actor,
                                    const CMaterialFilter& filter, rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                    float, float&, CCollisionInfoList& list, TUniqueId& idOut);
    static void MoveGroundColliderXY(CAreaCollisionCache& cache, CStateManager& mgr, CPhysicsActor& actor,
                                     const CMaterialFilter& filter, rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                     float);
    static void CollisionDamping(const zeus::CVector3f&, const zeus::CVector3f&, const zeus::CVector3f&,
                                 float, float);
    static void MoveGroundCollider_New(CStateManager& mgr, CPhysicsActor& actor, float,
                                       const rstl::reserved_vector<TUniqueId, 1024>* nearList);
    static void RemoveNormalComponent(const zeus::CVector3f&, const zeus::CVector3f&, const zeus::CVector3f&, float&);
    static void RemoveNormalComponent(const zeus::CVector3f&, const zeus::CVector3f&);
    static CMaterialList MoveObjectAnalytical(CStateManager& mgr, CPhysicsActor& actor, float,
                                              rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                              CAreaCollisionCache& cache, const SMovementOptions& opts,
                                              SMoveObjectResult& result);
};

}

#endif // __URDE_CGROUNDMOVEMENT_HPP__
