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

}
