#include "CBodyController.hpp"
#include "World/CActor.hpp"
#include "TCastTo.hpp"
#include "World/CPhysicsActor.hpp"

namespace urde
{

CBodyController::CBodyController(CActor& actor, float f1, EBodyType bodyType)
: x0_actor(actor), x2a4_bodyStateInfo(actor, bodyType),
  x2f4_bodyType(bodyType), x2fc_rotRate(f1)
{
    x300_28_playDeathAnims = true;
    x2a4_bodyStateInfo.x18_bodyController = this;
}

void CBodyController::EnableAnimation(bool e)
{
    x0_actor.ModelData()->AnimationData()->EnableAnimation(e);
}

float CBodyController::GetAnimTimeRemaining() const
{
    return x0_actor.GetModelData()->GetAnimationData()->GetAnimTimeRemaining("Whole Body");
}

const CPASDatabase& CBodyController::GetPASDatabase() const
{
    return x0_actor.GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase();
}

void CBodyController::FaceDirection(const zeus::CVector3f& v0, float dt)
{
    if (x300_26_)
        return;
    zeus::CVector3f noZ = v0;
    noZ.z = 0.f;
    if (noZ.canBeNormalized())
    {
        if (TCastToPtr<CPhysicsActor> act = x0_actor)
        {
            zeus::CQuaternion rot =
                zeus::CQuaternion::lookAt(act->GetTransform().basis[1],
                noZ.normalized(), zeus::degToRad(dt * x2fc_rotRate));
            rot.setImaginary(act->GetTransform().transposeRotate(rot.getImaginary()));
            act->RotateInOneFrameOR(rot, dt);
        }
    }
}

void CBodyController::FaceDirection3D(const zeus::CVector3f& v0, const zeus::CVector3f& v1, float dt)
{
    if (x300_26_)
        return;
    if (v0.canBeNormalized() && v1.canBeNormalized())
    {
        if (TCastToPtr<CPhysicsActor> act = x0_actor)
        {
            zeus::CUnitVector3f uv0 = v0;
            zeus::CUnitVector3f uv1 = v1;
            float dot = uv0.dot(uv1);
            if (std::fabs(dot - 1.f) > 0.00001f)
            {
                if (dot < -0.9999f)
                {
                    zeus::CQuaternion rot =
                        zeus::CQuaternion::fromAxisAngle(act->GetTransform().basis[2],
                        zeus::degToRad(dt * x2fc_rotRate));
                    rot.setImaginary(act->GetTransform().transposeRotate(rot.getImaginary()));
                    act->RotateInOneFrameOR(rot, dt);
                }
                else
                {
                    /* TODO: Finish */
                }
            }
        }
    }
}

}
