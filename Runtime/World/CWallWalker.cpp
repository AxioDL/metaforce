#include "CWallWalker.hpp"
#include "CPatternedInfo.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
#include "CScriptWaypoint.hpp"

namespace urde
{
CWallWalker::CWallWalker(ECharacter chr, TUniqueId uid, std::string_view name, EFlavorType flavType,
                         const CEntityInfo& eInfo, const zeus::CTransform& xf,
                         CModelData&& mData, const CPatternedInfo& pInfo, EMovementType mType,
                         EColliderType colType, EBodyType bType, const CActorParameters& aParms, float f1, float f2,
                         s32 w1, float f3, u32 w2, float f4, bool b1)
: CPatterned(chr, uid, name, flavType, eInfo, xf, std::move(mData), pInfo, mType, colType, bType, aParms, w1)
, x590_colSphere(zeus::CSphere(zeus::CVector3f::skZero, pInfo.GetHalfExtent()), x68_material)
, x5b0_(f1)
, x5b4_(f2)
, x5c0_(f3)
, x5c4_(f4)
, x5cc_bendingHackAnim(GetModelData()->GetAnimationData()->GetCharacterInfo().GetAnimationIndex("BendingAnimationHack"sv))
, x5d0_(w2)
, x5d6_24_(false)
, x5d6_25_(false)
, x5d6_26_(false)
, x5d6_27_(b1)
, x5d6_28_addBendingWeight(true)
, x5d6_29_applyBendingHack(false)
{
}

void CWallWalker::PreThink(float dt, CStateManager& mgr)
{
    CPatterned::PreThink(dt, mgr);
    if (!GetActive() || x5d6_26_ || x5bc_ > 0.f || x5d6_27_ || x450_bodyController->IsFrozen() || !x5d6_24_)
        return;

    zeus::CQuaternion quat(GetTransform().buildMatrix3f());
    AddMotionState(PredictMotion(dt));
    zeus::CQuaternion quat2(GetTransform().buildMatrix3f());
    ClearForcesAndTorques();
    if (x5d6_25_)
    {
        zeus::CPlane plane = x568_.GetPlane();
        const float futureDt = (10.f * dt);
        SetTranslation(GetTranslation() * (1.f - futureDt) +
                       (((GetTranslation() - ((plane.vec.dot(GetTranslation())) - plane.d) -
                          x590_colSphere.GetSphere().radius - 0.1f) * plane.vec) * futureDt));

    }
    else
        MoveCollisionPrimitive(zeus::CVector3f::skZero);

}

void CWallWalker::Think(float dt, CStateManager& mgr)
{
    if (!x450_bodyController->GetActive())
        x450_bodyController->Activate(mgr);
    CPatterned::Think(dt, mgr);

    if (x5cc_bendingHackAnim == -1)
        return;

    if (x5d6_28_addBendingWeight)
    {
        if (x5c8_bendingHackWeight < 1.f)
        {
            x5c8_bendingHackWeight += (dt * x138_velocity.magnitude()) / 0.6f;
            if (x5c8_bendingHackWeight >= 1.f)
                x5c8_bendingHackWeight = 1.f;
        }
    }
    else if (x5c8_bendingHackWeight > 0.f)
    {
        x5c8_bendingHackWeight -= (dt * x138_velocity.magnitude()) / 1.5f;
        if (x5c8_bendingHackWeight < 0.f)
            x5c8_bendingHackWeight = 0.f;
    }

    if (x5c8_bendingHackWeight <= 0.f && !x5d6_29_applyBendingHack)
        return;

    if (x5c8_bendingHackWeight > 0.0001f)
    {
        ModelData()->AnimationData()->AddAdditiveAnimation(x5cc_bendingHackAnim, x5c8_bendingHackWeight, true, false);
        x5d6_29_applyBendingHack = true;
    }
    else
    {
        ModelData()->AnimationData()->DelAdditiveAnimation(x5cc_bendingHackAnim);
        x5d6_29_applyBendingHack = false;
    }
}

void CWallWalker::UpdateWPDestination(CStateManager& mgr)
{
    if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x2dc_))
    {
        zeus::CVector3f wpPos = wp->GetTranslation();
        if ((wpPos - GetTranslation()).magSquared() < x5c0_ * x5c0_)
        {
            x2dc_ = wp->NextWaypoint(mgr);
            if (std::fabs(wp->GetF0()) > 0.00001f)
            {
                x5bc_ = wp->GetF0();
                if (x5d0_ == 0)
                    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
                mgr.SendScriptMsg(wp, GetUniqueId(), EScriptObjectMessage::Arrived);
            }
        }

        SetDestPos(wpPos);
    }
}
}
