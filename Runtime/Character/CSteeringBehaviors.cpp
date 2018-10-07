#include "CSteeringBehaviors.hpp"

namespace urde
{

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

bool CSteeringBehaviors::SolveQuadratic(float f30, float f31, float f3, float f4, float& out1, float& out2)
{
    float f1 = f31 * f31 - 4.f * f30 * f3;
    if (f1 > FLT_EPSILON && std::fabs(f1) < FLT_EPSILON)
        return false;

    out1 = -f31 + std::sqrt(f1) / 2.f * f30;
    out2 = -f31 - std::sqrt(f1) / 2.f * f30;
    return true;
}

}
