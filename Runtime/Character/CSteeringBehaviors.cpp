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
    const zeus::CVector3f& target) const
{
    zeus::CVector3f posDiff = target - actor.GetTranslation();
    if (posDiff.canBeNormalized())
        return posDiff.normalized();

    return {};
}

zeus::CVector3f CSteeringBehaviors::Arrival(const CPhysicsActor& actor,
    const zeus::CVector3f& dest, float dampingRadius) const
{
    zeus::CVector3f posDiff = dest - actor.GetTranslation();
    if (!posDiff.canBeNormalized())
        return {};

    if (posDiff.magSquared() < (dampingRadius * dampingRadius))
        dampingRadius = posDiff.magSquared() / (dampingRadius * dampingRadius);
    else
        dampingRadius = 1.f;

    return dampingRadius * posDiff.normalized();
}

zeus::CVector3f CSteeringBehaviors::Pursuit(const CPhysicsActor& actor,
    const zeus::CVector3f& v0, const zeus::CVector3f& v1) const
{
    zeus::CVector3f target;
    if (!ProjectLinearIntersection(actor.GetTranslation(), actor.GetVelocity().magnitude(), v0, v1, target))
        target = v1 * 1.f + v0;
    return CSteeringBehaviors::Seek(actor, target);
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
            if (const CActor* act = static_cast<const CActor*>(mgr.GetObjectById(id)))
                align += act->GetTransform().frontVector();

        align *= zeus::CVector3f(1.f / float(list.size()));
    }

    float diff = zeus::CVector3f::getAngleDiff(actor.GetTransform().frontVector(), align);
    return align * (diff / M_PIF);
}

zeus::CVector3f CSteeringBehaviors::Cohesion(const CPhysicsActor& actor,
    rstl::reserved_vector<TUniqueId, 1024>& list, float dampingRadius, const CStateManager& mgr) const
{
    zeus::CVector3f dest;
    if (!list.empty())
    {
        for (const TUniqueId& id : list)
            if (const CActor* act = static_cast<const CActor*>(mgr.GetObjectById(id)))
                dest += act->GetTranslation();

        dest *= zeus::CVector3f(1.f / float(list.size()));
        return Arrival(actor, dest, dampingRadius);
    }
    return dest;
}

zeus::CVector2f CSteeringBehaviors::Flee2D(const CPhysicsActor& actor,
    const zeus::CVector2f& v0) const
{
    zeus::CVector2f diffVec = actor.GetTranslation().toVec2f() - v0;
    if (diffVec.magSquared() > FLT_EPSILON)
        return diffVec.normalized();
    else
        return actor.GetTransform().basis[1].toVec2f();
}

zeus::CVector2f CSteeringBehaviors::Arrival2D(const CPhysicsActor& actor,
    const zeus::CVector2f& v0, float f1) const
{
    zeus::CVector2f diffVec = v0 - actor.GetTranslation().toVec2f();
    if (diffVec.magSquared() > FLT_EPSILON)
        return diffVec.normalized();
    else
        return {};
}

bool CSteeringBehaviors::SolveQuadratic(float a, float b, float c, float& xPos, float& xNeg)
{
    float numSq = b * b - 4.f * a * c;
    if (numSq < FLT_EPSILON || std::fabs(a) < FLT_EPSILON)
        return false;

    numSq = std::sqrt(numSq);
    float denom = 2.f * a;
    xPos = (-b + numSq) / denom;
    xNeg = (-b - numSq) / denom;
    return true;
}

bool CSteeringBehaviors::SolveCubic(
    const rstl::reserved_vector<float, 4>& in, rstl::reserved_vector<float, 4>& out)
{
    if (in[3] != 0.f)
    {
        float f3 = 3.f * in[3];
        float f31 = in[2] / f3;
        float f4 = in[1] / f3 - f31 * f31;
        float f0 = (f31 * f4 - in[0]) / in[3];
        float f1 = 2.f * f31 * f31;
        f3 = f4 * f4 * f4;
        float f24 = -0.5f * (f31 * f1 - f0);
        f1 = f24 * f24 + f3;
        if (f1 < 0.f)
        {
            float f25 = std::acos(zeus::clamp(-1.f, f24 / std::sqrt(-f3), 1.f));
            f24 = 2.f * std::pow(-f3, 0.166667f);
            for (float f23 = 0.f; f23 < 2.01f; f23 += 1.f)
                out.push_back(std::cos((2.f * f23 * M_PIF + f25) / 3.f) * f24 - f31);
            if (out[1] < out[0])
                std::swap(out[1], out[0]);
            if (out[2] < out[1])
                std::swap(out[2], out[1]);
            if (out[1] < out[0])
                std::swap(out[1], out[0]);
        }
        else
        {
            float f30 = std::sqrt(f1);
            float f25 = std::pow(std::fabs(f24 + f30), 0.333333f);
            f1 = std::pow(std::fabs(f24 - f30), 0.333333f);
            f1 = (f24 - f30) > 0.f ? f1 : -f1;
            f25 = (f24 + f30) > 0.f ? f25 : -f25;
            out.push_back(f25 + f1 - f31);
        }
        for (float& f : out)
        {
            float f8 = (2.f * in[2] + 3.f * f * in[3]) * f + in[1];
            if (f8 != 0.f)
                f -= (((f * in[3] + in[2]) * f + in[1]) * f + in[0]) / f8;
        }
    }
    else if (in[2] != 0.f)
    {
        float f23 = 0.5f * in[1] / in[2];
        float f1 = f23 * f23 - (in[1] / in[2]);
        if (f1 >= 0.f)
        {
            f1 = std::sqrt(f1);
            out.push_back(-f23 - f1);
            out.push_back(-f23 + f1);
        }
    }
    else if (in[1] != 0.f)
    {
        out.push_back(-in[0] / in[1]);
    }
    return out.size() != 0;
}

bool CSteeringBehaviors::SolveQuartic(
    const rstl::reserved_vector<float, 5>& in, rstl::reserved_vector<float, 4>& out)
{
    if (in[4] == 0.f)
    {
        rstl::reserved_vector<float, 4> newIn;
        newIn.push_back(in[0]);
        newIn.push_back(in[1]);
        newIn.push_back(in[2]);
        newIn.push_back(in[3]);
        return SolveCubic(newIn, out);
    }
    else
    {
        rstl::reserved_vector<float, 4> newIn;
        float f30 = in[3] / (4.f * in[4]);
        float f2 = in[1] / in[4];
        float f29 = f30 * (8.f * f30 * f30 - 2.f * in[2] / in[4]) + f2;
        float f31 = -6.f * f30 * f30 + (in[2] / in[4]);
        float f28 = f30 * (f30 * (-3.f * f30 * f30 + (in[2] / in[4])) - f2) + (in[0] / in[4]);
        newIn.push_back(4.f * f28 * f31 - f29 * f29);
        newIn.push_back(-8.f * f28);
        newIn.push_back(-4.f * f31);
        newIn.push_back(8.f);
        rstl::reserved_vector<float, 4> newOut;
        if (SolveCubic(newIn, newOut))
        {
            float f26 = 2.f * newOut.back() - f31;
            f31 = std::sqrt(f26);
            float f1;
            if (f31 == 0.f)
            {
                f1 = newOut.back() * newOut.back() - f28;
                if (f1 < 0.f)
                    return false;
                f1 = std::sqrt(f1);
            }
            else
            {
                f1 = f29 / (2.f * f31);
            }
            float f1b = f26 - (newOut.back() + f1) * 4.f;
            f26 = f26 - (newOut.back() - f1) * 4.f;
            if (f1b >= 0.f)
            {
                f1b = std::sqrt(f1b);
                out.push_back((f31 - f1b) * 0.5f - f30);
                out.push_back((f31 + f1b) * 0.5f - f30);
            }
            if (f26 >= 0.f)
            {
                f1b = std::sqrt(f26);
                out.push_back((-f31 - f1b) * 0.5f - f30);
                out.push_back((-f31 + f1b) * 0.5f - f30);
            }
            for (float& f : out)
            {
                float f10 = ((3.f * in[3] + 4.f * f * in[4]) * f + 2.f * in[2]) * f + in[1];
                if (f10 != 0.f)
                    f -= ((((f * in[4] + in[3]) * f + in[2]) * f + in[1]) * f + in[0]) / f10;
            }
            if (out.size() > 2)
            {
                if (out[2] < out[0])
                    std::swap(out[2], out[0]);
                if (out[3] < out[1])
                    std::swap(out[3], out[1]);
                if (out[1] < out[0])
                    std::swap(out[1], out[0]);
                if (out[3] < out[2])
                    std::swap(out[3], out[2]);
                if (out[2] < out[1])
                    std::swap(out[2], out[1]);
            }
        }
        return out.size() != 0;
    }
}

bool CSteeringBehaviors::ProjectLinearIntersection(const zeus::CVector3f& v0, float f1,
    const zeus::CVector3f& v1, const zeus::CVector3f& v2, zeus::CVector3f& v3)
{
    zeus::CVector3f posDiff = v1 - v0;
    float xPos, xNeg;
    if (SolveQuadratic(v2.magSquared() - f1 * f1,
                       posDiff.dot(v2) * 2.f,
                       posDiff.magSquared(),
                       xPos, xNeg) && xNeg > 0.f)
    {
        v3 = v2 * xNeg + v1;
        return true;
    }
    return false;
}

bool CSteeringBehaviors::ProjectLinearIntersection(const zeus::CVector3f& v0, float f1,
    const zeus::CVector3f& v1, const zeus::CVector3f& v2, const zeus::CVector3f& v3, zeus::CVector3f& v4)
{
    rstl::reserved_vector<float, 5> newIn;
    rstl::reserved_vector<float, 4> newOut;
    zeus::CVector3f f7 = v1 - v0;
    newIn.push_back(f7.magSquared());
    newIn.push_back(f7.dot(v2) * 2.f);
    newIn.push_back(f7.dot(v3) + v2.magSquared() - f1 * f1);
    newIn.push_back(v2.dot(v3));
    newIn.push_back(v3.magSquared() * 0.25f);
    bool ret = false;
    if (SolveQuartic(newIn, newOut))
        for (float& f : newOut)
            if (f > 0.f)
            {
                ret = true;
                v4 = v1 + v2 * f + 0.5f * f * f * v3;
            }
    return ret;
}

bool CSteeringBehaviors::ProjectOrbitalIntersection(const zeus::CVector3f& v0, float f1, float f2,
    const zeus::CVector3f& v1, const zeus::CVector3f& v2, const zeus::CVector3f& v3, zeus::CVector3f& v4)
{
    if (f1 > 0.f)
    {
        if (v2.canBeNormalized())
        {
            zeus::CVector3f _12c = (v1 - v3).toVec2f();
            if (_12c.canBeNormalized())
            {
                zeus::CVector3f f25 = v1;
                zeus::CVector3f f22 = v2;
                float f17 = (f25 - v0).magnitude() / f1 - 0.f;
                float f18 = FLT_MAX;
                zeus::CVector3f _150 = _12c.normalized();
                float f26 = _150.dot(f22);
                float f27 = _150.cross(zeus::CVector3f::skUp).dot(f22);
                for (float f19 = 0.f; f17 < f18 && f19 < 4.f;)
                {
                    if (zeus::close_enough(f17, f2) || f17 < 0.f)
                    {
                        v4 = f25;
                        return true;
                    }
                    f25 += f2 * f22;
                    f18 = f17;
                    _12c = (f25 - v3).toVec2f();
                    if (!_12c.canBeNormalized())
                        break;
                    zeus::CVector3f _168 = _12c.normalized();
                    f22 = _168.cross(zeus::CVector3f::skUp) * f27 + f26 * _168;
                    f19 += f2;
                    f17 = (f25 - v0).magnitude() / f1 - f19;
                }
            }
            else
            {
                return ProjectLinearIntersection(v0, f1, v1, v2, v4);
            }
        }
        else
        {
            v4 = v1;
            return true;
        }
    }
    return false;
}

bool CSteeringBehaviors::ProjectOrbitalIntersection(const zeus::CVector3f& v0, float f1, float f2,
    const zeus::CVector3f& v1, const zeus::CVector3f& v2, const zeus::CVector3f& v3,
    const zeus::CVector3f& v4, zeus::CVector3f& v5)
{
    if (f1 > 0.f)
    {
        zeus::CVector3f _12c = (v1 - v4).toVec2f();
        if (v2.canBeNormalized() && _12c.canBeNormalized())
        {
            zeus::CVector3f f24 = v1;
            zeus::CVector3f f21 = v2;
            float f16 = (f24 - v0).magnitude() / f1 - 0.f;
            float f17 = FLT_MAX;
            zeus::CVector3f _150 = _12c.normalized();
            float f25 = _150.dot(f21);
            float f26 = _150.cross(zeus::CVector3f::skUp).dot(f21);
            for (float f18 = 0.f; f16 < f17 && f18 < 4.f;)
            {
                if (zeus::close_enough(f16, f2) || f16 < 0.f)
                {
                    v5 = f24;
                    return true;
                }
                f24 += f2 * f21;
                f17 = f16;
                f18 += f2;
                f16 = (f24 - v0).magnitude() / f1 - f18;
                _12c = (f24 - v4).toVec2f();
                if (!_12c.canBeNormalized())
                    break;
                zeus::CVector3f _168 = _12c.normalized();
                f21 = _168.cross(zeus::CVector3f::skUp) * f26 + f25 * _168;
            }
        }
        else
        {
            return ProjectLinearIntersection(v0, f1, v1, v2, v3, v5);
        }
    }
    return false;
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
