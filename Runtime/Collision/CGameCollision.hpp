#ifndef __URDE_CGAMECOLLISION_HPP__
#define __URDE_CGAMECOLLISION_HPP__
#include "zeus/CVector3f.hpp"
#include "zeus/CPlane.hpp"
#include "rstl.hpp"
#include "RetroTypes.hpp"
#include "CRayCastResult.hpp"

namespace urde
{

class CInternalCollisionStructure
{
};

class CActor;
class CCollisionInfo;
class CCollisionInfoList;
class CMaterialList;
class CStateManager;
class CPhysicsActor;
class CMaterialFilter;

class CGameCollision
{
public:
    static bool NullMovingCollider(const CInternalCollisionStructure&, const zeus::CVector3f&, double&, CCollisionInfo&)
    {
        return false;
    }
    static bool NullBooleanCollider(const CInternalCollisionStructure&) { return false; }
    static bool NullCollisionCollider(const CInternalCollisionStructure&, CCollisionInfoList&) { return false; }
    static void InitCollision();
    static void Move(CStateManager& mgr, CPhysicsActor& actor, float dt, const rstl::reserved_vector<TUniqueId, 1024>*);

    static bool CanBlock(const CMaterialList&, const zeus::CVector3f&);
    static bool IsFloor(const CMaterialList&, const zeus::CVector3f&);
    void SendMaterialMessage(CStateManager&, const CMaterialList&, CActor&);
    static CRayCastResult RayStaticIntersection(const CStateManager& mgr, const zeus::CVector3f& pos,
                                                const zeus::CVector3f& dir, float mag, const CMaterialFilter& filter);
    static CRayCastResult RayDynamicIntersection(const CStateManager& mgr, TUniqueId& idOut, const zeus::CVector3f& pos,
                                                 const zeus::CVector3f& dir, float mag, const CMaterialFilter& filter,
                                                 const rstl::reserved_vector<TUniqueId, 1024>& nearList);
    static CRayCastResult RayWorldIntersection(const CStateManager& mgr, TUniqueId& idOut, const zeus::CVector3f& pos,
                                               const zeus::CVector3f& dir, float mag, const CMaterialFilter& filter,
                                               const rstl::reserved_vector<TUniqueId, 1024>& nearList);
};
}

#endif // __URDE_CGAMECOLLISION_HPP__
