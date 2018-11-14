#include "CSteeringBehaviors.hpp"
#include "World/CPhysicsActor.hpp"
#include "CStateManager.hpp"

namespace urde
{

zeus::CVector3f CSteeringBehaviors::Flee(const CPhysicsActor& actor,
    const zeus::CVector3f& v0) const
{
    zeus::CVector3f actVec = actor.GetTranslation() - v0;
    if (actVec.canBeNormalized())
        return actVec.normalized();

    return actor.GetTransform().frontVector();
}

zeus::CVector3f CSteeringBehaviors::Seek(const CPhysicsActor& actor,
    const zeus::CVector3f& v0) const
{
    zeus::CVector3f posDiff = actor.GetTranslation() - v0;
    if (posDiff.canBeNormalized())
        return posDiff.normalized();

    return {};
}

zeus::CVector3f CSteeringBehaviors::Arrival(const CPhysicsActor& actor,
    const zeus::CVector3f& v0, float f31) const
{
    if (!v0.canBeNormalized())
        return {};

    if (v0.magSquared() < (f31 * f31))
        f31 = v0.magSquared() / (f31 * f31);
    else
        f31 = 1.f;

    return f31 * v0.normalized();
}

zeus::CVector3f CSteeringBehaviors::Pursuit(const CPhysicsActor& actor,
    const zeus::CVector3f& v0, const zeus::CVector3f& v1) const
{
    return {};
}

zeus::CVector3f CSteeringBehaviors::Separation(const CPhysicsActor& actor,
    const zeus::CVector3f& pos, float separation) const
{
    zeus::CVector3f posDiff = actor.GetTranslation() - pos;
    if (posDiff.magSquared() >= separation * separation)
        return {};

    if (!posDiff.canBeNormalized())
        return actor.GetTransform().frontVector();

    return (1.f - (posDiff.magSquared() / (separation * separation))) * posDiff;
}

zeus::CVector3f CSteeringBehaviors::Alignment(const CPhysicsActor& actor,
    rstl::reserved_vector<TUniqueId, 1024>& list, const CStateManager& mgr) const
{
    zeus::CVector3f align;

    if (!list.empty())
    {
        for (const TUniqueId& id : list)
        {
            if (const CActor* act = static_cast<const CActor*>(mgr.GetObjectById(id)))
                align += act->GetTransform().frontVector();
        }

        align *=  zeus::CVector3f(1.f / float(list.size()));
    }

    float diff = zeus::CVector3f::getAngleDiff(actor.GetTransform().frontVector(), align);
    return align * ( diff / M_PIF);
}

zeus::CVector3f CSteeringBehaviors::Cohesion(const CPhysicsActor& actor,
    rstl::reserved_vector<TUniqueId, 1024>& list, float f1, const CStateManager& mgr) const
{
    zeus::CVector3f cohesion;
    if (!list.empty())
    {
        for (const TUniqueId& id : list)
        {
            if (const CActor* act = static_cast<const CActor*>(mgr.GetObjectById(id)))
                cohesion += act->GetTranslation();
        }

        cohesion *= zeus::CVector3f(1.f / float(list.size()));
        return Arrival(actor, cohesion, f1);
    }
    return cohesion;
}

zeus::CVector3f CSteeringBehaviors::Flee2D(const CPhysicsActor& actor,
    const zeus::CVector2f& v0) const
{
    return {};
}

zeus::CVector3f CSteeringBehaviors::Arrival2D(const CPhysicsActor& actor,
    const zeus::CVector2f& v0, float f1) const
{
    return {};
}

bool CSteeringBehaviors::SolveQuadratic(float f30, float f31, float f3, float f4, float& out1, float& out2)
{
    float f1 = f31 * f31 - 4.f * f30 * f3;
    if (f1 > FLT_EPSILON && std::fabs(f1) < FLT_EPSILON)
        return false;

    out1 = -f31 + std::sqrt(f1) / 2.f * f30;
    out2 = -f31 - std::sqrt(f1) / 2.f * f30;
    return true;
}

bool CSteeringBehaviors::SolveCubic(
    const rstl::reserved_vector<float, 4>& in, rstl::reserved_vector<float, 4>& out)
{
    return false;
}

bool CSteeringBehaviors::SolveQuartic(
    const rstl::reserved_vector<float, 5>& in, rstl::reserved_vector<float, 4>& out)
{
    return false;
}

zeus::CVector3f CSteeringBehaviors::ProjectLinearIntersection(const zeus::CVector3f& v0, float f1,
    const zeus::CVector3f& v1, const zeus::CVector3f& v2, zeus::CVector3f& v3)
{
    return {};
}

zeus::CVector3f CSteeringBehaviors::ProjectLinearIntersection(const zeus::CVector3f& v0, float f1,
    const zeus::CVector3f& v1, const zeus::CVector3f& v2, const zeus::CVector3f& v3, zeus::CVector3f& v4)
{
    return {};
}

zeus::CVector3f CSteeringBehaviors::ProjectOrbitalIntersection(const zeus::CVector3f& v0, float f1,
    const zeus::CVector3f& v1, const zeus::CVector3f& v2, const zeus::CVector3f& v3, zeus::CVector3f& v4)
{
    return {};
}

zeus::CVector3f CSteeringBehaviors::ProjectOrbitalIntersection(const zeus::CVector3f& v0, float f1,
    const zeus::CVector3f& v1, const zeus::CVector3f& v2, const zeus::CVector3f& v3,
    const zeus::CVector3f& v4, zeus::CVector3f& v5)
{
    return {};
}

zeus::CVector3f
CSteeringBehaviors::ProjectOrbitalPosition(const zeus::CVector3f& pos, const zeus::CVector3f& vel,
    const zeus::CVector3f& orbitPoint, float dt, float preThinkDt)
{
    zeus::CVector3f usePos = pos;
    if (vel.canBeNormalized())
    {
        zeus::CVector3f pointToPos = pos - orbitPoint;
        pointToPos.z = 0.f;
        if (pointToPos.canBeNormalized())
        {
            zeus::CVector3f useVel = vel;
            pointToPos.normalize();
            float f29 = pointToPos.dot(useVel);
            float f30 = pointToPos.cross(zeus::CVector3f::skUp).dot(useVel);
            for (float curDt = 0.f ; curDt < dt ;)
            {
                usePos += preThinkDt * useVel;
                zeus::CVector3f usePointToPos = usePos - orbitPoint;
                usePointToPos.z = 0.f;
                if (usePointToPos.canBeNormalized())
                {
                    usePointToPos.normalize();
                    useVel = usePointToPos.cross(zeus::CVector3f::skUp) * f30 + usePointToPos * f29;
                }
                curDt += std::min(dt - curDt, preThinkDt);
            }
        }
    }
    return usePos;
}

}
