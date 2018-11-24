#include "CRagDoll.hpp"
#include "Collision/CMetroidAreaCollider.hpp"
#include "Collision/CGameCollision.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfo.hpp"
#include "CStateManager.hpp"

namespace urde
{

void CRagDoll::CRagDollLengthConstraint::Update()
{
    zeus::CVector3f delta = x4_p2->x4_curPos - x0_p1->x4_curPos;
    float magSq = delta.magSquared();
    float lenSq = x8_length * x8_length;
    bool doSolve = true;
    switch (xc_ineqType)
    {
    case 1: // Min
        doSolve = magSq < lenSq;
        break;
    case 2: // Max
        doSolve = magSq > lenSq;
        break;
    default:
        break;
    }
    if (!doSolve)
        return;
    zeus::CVector3f solveVec = delta * (lenSq / (magSq + lenSq) - 0.5f);
    x0_p1->x4_curPos -= solveVec;
    x4_p2->x4_curPos += solveVec;
}

void CRagDoll::CRagDollJointConstraint::Update()
{
    // L_hip, R_shoulder, L_shoulder, L_hip, L_knee, L_ankle
    zeus::CVector3f P4ToP5 = x10_p5->x4_curPos - xc_p4->x4_curPos; // L_hip->L_knee
    zeus::CVector3f cross = P4ToP5.cross(
        (x8_p3->x4_curPos - x0_p1->x4_curPos).cross(x4_p2->x4_curPos - x0_p1->x4_curPos));
    // L_hip->L_knee X (L_hip->L_shoulder X L_hip->R_shoulder)
    if (cross.canBeNormalized())
    {
        zeus::CVector3f hipUp = cross.cross(P4ToP5).normalized();
        float dot = (x14_p6->x4_curPos - x10_p5->x4_curPos).dot(hipUp);
        if (dot > 0.f)
        {
            zeus::CVector3f solveVec = 0.5f * dot * hipUp;
            x14_p6->x4_curPos -= solveVec;
            x10_p5->x4_curPos += solveVec;
        }
    }
}

void CRagDoll::CRagDollPlaneConstraint::Update()
{
    zeus::CVector3f P1ToP2 = (x4_p2->x4_curPos - x0_p1->x4_curPos).normalized();
    float dot = P1ToP2.dot(xc_p4->x4_curPos - x8_p3->x4_curPos);
    if (dot < 0.f)
    {
        zeus::CVector3f solveVec = 0.5f * dot * P1ToP2;
        xc_p4->x4_curPos -= solveVec;
        x10_p5->x4_curPos += solveVec;
    }
}

CRagDoll::CRagDoll(float f1, float f2, float f3, u32 flags)
: x44_f1(f1), x48_f2(f2), x50_f3(f3)
{
    x68_27_ = bool(flags & 0x1);
    x68_28_ = bool(flags & 0x2);
    x68_29_ = bool(flags & 0x4);
}

void CRagDoll::AccumulateForces(float dt, float f2)
{
    float fps = 1.f / dt;
    x64_angTimer += dt;
    if (x64_angTimer > 4.f)
        x64_angTimer -= 4.f;
    float f25 = std::sin(zeus::degToRad(90.f) * x64_angTimer) * 0.1f + (f2 - 0.2f);
    zeus::CVector3f f29;
    float f24 = 0.f;
    for (auto& particle : x4_particles)
    {
        float cubed = particle.x10_radius * particle.x10_radius * particle.x10_radius;
        f24 += cubed;
        f29 += particle.x4_curPos * cubed;
        float f7 = particle.x4_curPos.z - f25;
        float f8 = x48_f2;
        float f23 = 0.f;
        if (std::fabs(f7) < 0.5f)
        {
            f23 = 0.5f * f7 / 0.5f + 0.5f;
            f8 = x48_f2 * -f7 / 0.5f;
        }
        else if (f7 > 0.f)
        {
            f8 = x44_f1;
            f23 = 1.f;
        }
        particle.x20_acceleration.z += f8;
        zeus::CVector3f vel = (particle.x4_curPos - particle.x14_prevPos) * fps;
        float velMag = vel.magnitude();
        if (velMag > FLT_EPSILON)
        {
            particle.x20_acceleration -= vel * (1.f / velMag) *
                ((velMag * velMag * 0.75f * (1.2f * f23 + 1000.f * (1.f - f23))) / (8000.f * particle.x10_radius));
        }
    }
    zeus::CVector3f _c8;
    f29 = f29 / f24;
    for (const auto& particle : x4_particles)
    {
        float cubed = particle.x10_radius * particle.x10_radius * particle.x10_radius;
        _c8 += (particle.x4_curPos - f29).cross(particle.x4_curPos) * cubed;
    }
    _c8 = _c8 * (fps / f24);
    if (_c8.canBeNormalized())
        for (auto& particle : x4_particles)
            particle.x20_acceleration -= _c8.cross(particle.x4_curPos - f29) * 25.f;
}

void CRagDoll::AddParticle(CSegId id, const zeus::CVector3f& prevPos, const zeus::CVector3f& curPos, float f1)
{
    x4_particles.emplace_back(id, curPos, f1, prevPos);
}

void CRagDoll::AddLengthConstraint(int i1, int i2)
{
    x14_lengthConstraints.emplace_back(&x4_particles[i1], &x4_particles[i2],
        (x4_particles[i1].x4_curPos - x4_particles[i2].x4_curPos).magnitude(), 0);
}

void CRagDoll::AddMaxLengthConstraint(int i1, int i2, float length)
{
    x14_lengthConstraints.emplace_back(&x4_particles[i1], &x4_particles[i2], length, 2);
}

void CRagDoll::AddMinLengthConstraint(int i1, int i2, float length)
{
    x14_lengthConstraints.emplace_back(&x4_particles[i1], &x4_particles[i2], length, 1);
}

void CRagDoll::AddJointConstraint(int i1, int i2, int i3, int i4, int i5, int i6)
{
    x24_jointConstraints.emplace_back(&x4_particles[i1], &x4_particles[i2], &x4_particles[i3],
                                      &x4_particles[i4], &x4_particles[i5], &x4_particles[i6]);
}

zeus::CQuaternion CRagDoll::BoneAlign(CHierarchyPoseBuilder& pb, const CCharLayoutInfo& charInfo,
                                      int i1, int i2, const zeus::CQuaternion& q)
{
    zeus::CVector3f fromParent = charInfo.GetFromParentUnrotated(x4_particles[i2].x0_id);
    zeus::CVector3f delta = x4_particles[i2].x4_curPos - x4_particles[i1].x4_curPos;
    delta = q.inverse().transform(delta);
    zeus::CQuaternion ret = zeus::CQuaternion::shortestRotationArc(fromParent, delta);
    pb.GetTreeMap()[x4_particles[i1].x0_id].x4_rotation = ret;
    return ret;
}

zeus::CAABox CRagDoll::CalculateRenderBounds() const
{
    zeus::CAABox aabb;
    for (const auto& particle : x4_particles)
    {
        aabb.accumulateBounds(zeus::CAABox(particle.x4_curPos - particle.x10_radius,
                                           particle.x4_curPos + particle.x10_radius));
    }
    return aabb;
}

void CRagDoll::CheckStatic(float dt)
{
    x4c_ = 0;
    x54_ = 0.f;
    float f1 = 0.5f * dt;
    float f31 = f1 * f1;
    x58_ = zeus::CVector3f::skZero;
    bool r31 = true;
    for (auto& particle : x4_particles)
    {
        zeus::CVector3f delta = particle.x4_curPos - particle.x14_prevPos;
        x58_ += delta;
        if (delta.magSquared() > f31)
            r31 = false;
        if (particle.x3c_24_nextDampVel)
        {
            x4c_ += 1;
            x54_ = std::max(particle.x38_, x54_);
        }
    }
    if (!x4_particles.empty())
        x58_ = x58_ * (1.f / (dt * x4_particles.size()));
    x54_ /= dt;
    if (!x68_28_)
    {
        x50_f3 -= dt;
        if (x50_f3 <= 0.f)
            x68_25_ = true;
    }
    if (r31 && x68_24_)
        x68_25_ = true;
    x68_24_ = r31;
}

void CRagDoll::ClearForces()
{
    for (auto& particle : x4_particles)
        particle.x20_acceleration = zeus::CVector3f::skZero;
}

void CRagDoll::SatisfyConstraints(CStateManager& mgr)
{
    for (auto& length : x14_lengthConstraints)
        length.Update();
    for (auto& joint : x24_jointConstraints)
        joint.Update();
    for (auto& plane : x34_planeConstraints)
        plane.Update();
    if (SatisfyWorldConstraints(mgr, 1))
        SatisfyWorldConstraints(mgr, 2);
}

bool CRagDoll::SatisfyWorldConstraints(CStateManager& mgr, int i1)
{
    zeus::CAABox aabb;
    for (const auto& particle : x4_particles)
    {
        if (i1 == 1 || particle.x3c_24_nextDampVel)
        {
            aabb.accumulateBounds(particle.x14_prevPos - particle.x10_radius);
            aabb.accumulateBounds(particle.x14_prevPos + particle.x10_radius);
            aabb.accumulateBounds(particle.x4_curPos - particle.x10_radius);
            aabb.accumulateBounds(particle.x4_curPos + particle.x10_radius);
        }
    }

    CAreaCollisionCache ccache(aabb);
    CGameCollision::BuildAreaCollisionCache(mgr, ccache);
    bool ret = false;

    TUniqueId bestId = kInvalidUniqueId;
    CMaterialList include;
    if (x68_29_)
        include = CMaterialList(EMaterialTypes::Solid);
    else
        include = CMaterialList(EMaterialTypes::Solid, EMaterialTypes::AIBlock);

    CMaterialList exclude;
    if (x68_29_)
        exclude = CMaterialList(EMaterialTypes::Character, EMaterialTypes::Player,
                                  EMaterialTypes::AIBlock, EMaterialTypes::Occluder);
    else
        exclude = CMaterialList(EMaterialTypes::Character, EMaterialTypes::Player);

    rstl::reserved_vector<TUniqueId, 1024> nearList;
    CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(include, exclude);
    mgr.BuildNearList(nearList, aabb, filter, nullptr);

    for (auto& particle : x4_particles)
    {
        if (i1 == 1 || particle.x3c_24_nextDampVel)
        {
            zeus::CVector3f delta = particle.x4_curPos - particle.x14_prevPos;
            float deltaMag = delta.magnitude();
            if (deltaMag > 0.0001f)
            {
                delta = delta * (1.f / deltaMag);
                double d = deltaMag;
                CCollidableSphere sphere(zeus::CSphere(particle.x14_prevPos, particle.x10_radius), include);
                CCollisionInfo info;
                CGameCollision::DetectCollision_Cached_Moving(mgr, ccache, sphere, {}, filter,
                                                              nearList, delta, bestId, info, d);
                if (info.IsValid())
                {
                    ret = true;
                    switch (i1)
                    {
                    case 1:
                    {
                        particle.x3c_24_nextDampVel = true;
                        float dot = delta.dot(info.GetNormalLeft());
                        particle.x2c_nextPosDelta = -0.125f * dot * deltaMag * info.GetNormalLeft();
                        particle.x38_ = -dot * deltaMag;
                        particle.x4_curPos += float(0.0001f - (deltaMag - d) * dot) * info.GetNormalLeft();
                        break;
                    }
                    case 2:
                        particle.x4_curPos = float(d - 0.0001) * delta + particle.x14_prevPos;
                        break;
                    default:
                        break;
                    }
                }
            }
            else if (!x68_27_)
            {
                particle.x4_curPos = particle.x14_prevPos;
            }
        }
    }

    return ret;
}

void CRagDoll::SatisfyWorldConstraintsOnConstruction(CStateManager& mgr)
{
    for (auto& particle : x4_particles)
        particle.x3c_24_nextDampVel = true;
    SatisfyWorldConstraints(mgr, 2);
    for (auto& particle : x4_particles)
        particle.x14_prevPos = particle.x4_curPos;
}

void CRagDoll::Verlet(float dt)
{
    for (auto& particle : x4_particles)
    {
        zeus::CVector3f oldPos = particle.x4_curPos;
        particle.x4_curPos += (particle.x4_curPos - particle.x14_prevPos) *
            (particle.x3c_24_nextDampVel ? 0.9f : 1.f);
        particle.x4_curPos += particle.x20_acceleration * (dt * dt);
        particle.x4_curPos += particle.x2c_nextPosDelta;
        particle.x14_prevPos = oldPos;
        zeus::CVector3f deltaPos = particle.x4_curPos - particle.x14_prevPos;
        if (deltaPos.magSquared() > 4.f)
            particle.x4_curPos = deltaPos.normalized() * 2.f + particle.x14_prevPos;
        particle.x3c_24_nextDampVel = false;
        particle.x2c_nextPosDelta = zeus::CVector3f::skZero;
    }
}

void CRagDoll::PreRender(const zeus::CVector3f& v, CModelData& mData)
{
    // Empty
}

void CRagDoll::Update(CStateManager& mgr, float dt, float f2)
{
    if (!x68_25_ || x68_27_)
    {
        AccumulateForces(dt, f2);
        Verlet(dt);
        SatisfyConstraints(mgr);
        ClearForces();
        CheckStatic(dt);
    }
}

void CRagDoll::Prime(CStateManager& mgr, const zeus::CTransform& xf, CModelData& mData)
{
    zeus::CVector3f scale = mData.GetScale();
    CAnimData* aData = mData.AnimationData();
    aData->BuildPose();
    for (auto& particle : x4_particles)
        if (particle.x0_id != 0xff)
            particle.x4_curPos = xf * aData->GetPose().GetOffset(particle.x0_id) * scale;
    SatisfyWorldConstraints(mgr, 2);
    for (auto& particle : x4_particles)
        particle.x3c_24_nextDampVel = false;
    x68_26_primed = true;
}

}
