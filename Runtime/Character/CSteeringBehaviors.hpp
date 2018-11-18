#pragma once

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class CPhysicsActor;
class CStateManager;

class CSteeringBehaviors
{
    float x0_ = M_PIF / 2.f;
public:
    zeus::CVector3f Flee(const CPhysicsActor& actor, const zeus::CVector3f& v0) const;
    zeus::CVector3f Seek(const CPhysicsActor& actor, const zeus::CVector3f& target) const;
    zeus::CVector3f Arrival(const CPhysicsActor& actor, const zeus::CVector3f& dest, float dampingRadius) const;
    zeus::CVector3f Pursuit(const CPhysicsActor& actor, const zeus::CVector3f& v0, const zeus::CVector3f& v1) const;
    zeus::CVector3f Separation(const CPhysicsActor& actor, const zeus::CVector3f& pos, float separation) const;
    zeus::CVector3f Alignment(const CPhysicsActor& actor, rstl::reserved_vector<TUniqueId, 1024>& list,
                              const CStateManager& mgr) const;
    zeus::CVector3f Cohesion(const CPhysicsActor& actor, rstl::reserved_vector<TUniqueId, 1024>& list,
                             float dampingRadius, const CStateManager& mgr) const;
    zeus::CVector2f Flee2D(const CPhysicsActor& actor, const zeus::CVector2f& v0) const;
    zeus::CVector2f Arrival2D(const CPhysicsActor& actor, const zeus::CVector2f& v0, float f1) const;
    static bool SolveQuadratic(float, float, float, float&, float&);
    static bool SolveCubic(const rstl::reserved_vector<float, 4>& in, rstl::reserved_vector<float, 4>& out);
    static bool SolveQuartic(const rstl::reserved_vector<float, 5>& in, rstl::reserved_vector<float, 4>& out);
    static bool ProjectLinearIntersection(const zeus::CVector3f& v0, float f1,
        const zeus::CVector3f& v1, const zeus::CVector3f& v2, zeus::CVector3f& v3);
    static bool ProjectLinearIntersection(const zeus::CVector3f& v0, float f1,
        const zeus::CVector3f& v1, const zeus::CVector3f& v2, const zeus::CVector3f& v3,
        zeus::CVector3f& v4);
    static bool ProjectOrbitalIntersection(const zeus::CVector3f& v0, float f1, float f2,
        const zeus::CVector3f& v1, const zeus::CVector3f& v2, const zeus::CVector3f& v3,
        zeus::CVector3f& v4);
    static bool ProjectOrbitalIntersection(const zeus::CVector3f& v0, float f1, float f2,
        const zeus::CVector3f& v1, const zeus::CVector3f& v2, const zeus::CVector3f& v3,
        const zeus::CVector3f& v4, zeus::CVector3f& v5);
    static zeus::CVector3f ProjectOrbitalPosition(const zeus::CVector3f& pos, const zeus::CVector3f& vel,
        const zeus::CVector3f& orbitPoint, float dt, float preThinkDt);
};

}

