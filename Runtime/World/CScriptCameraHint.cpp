#include "CScriptCameraHint.hpp"
#include "CActorParameters.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptCameraHint::CScriptCameraHint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                     const zeus::CTransform& xf, bool active, s32 priority,
                                     CBallCamera::EBallCameraBehaviour behaviour, u32 overrideFlags, float minDist,
                                     float maxDist, float backwardsDist, const zeus::CVector3f& lookAtOffset,
                                     const zeus::CVector3f& chaseLookAtOffset, const zeus::CVector3f& ballToCam,
                                     float fov, float attitudeRange, float azimuthRange, float anglePerSecond,
                                     float clampVelRange, float f9, float elevation, float f11,
                                     float clampVelTime, float controlInterpDur)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId), xe8_priority(priority),
  xec_hint(overrideFlags, behaviour, minDist, maxDist, backwardsDist, lookAtOffset, chaseLookAtOffset, ballToCam,
           fov, attitudeRange, azimuthRange, anglePerSecond, clampVelRange, f9, elevation, f11, clampVelTime,
           controlInterpDur),
  x168_origXf(xf)
{
}

void CScriptCameraHint::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptCameraHint::InitializeInArea(CStateManager& mgr)
{
    x164_delegatedCamera = kInvalidUniqueId;
    for (CEntity* ent : mgr.GetAllObjectList())
    {
        for (const SConnection& conn : ent->GetConnectionList())
        {
            if (mgr.GetIdForScript(conn.x8_objId) != GetUniqueId())
                continue;
            if (conn.x4_msg != EScriptObjectMessage::Increment &&
                conn.x4_msg != EScriptObjectMessage::Decrement)
                continue;

            for (auto it = ent->GetConnectionList().begin() ; it != ent->GetConnectionList().cend() ; ++it)
            {
                const SConnection& conn2 = *it;
                if (conn2.x4_msg != EScriptObjectMessage::Increment &&
                    conn2.x4_msg != EScriptObjectMessage::Decrement)
                    continue;
                TUniqueId id = mgr.GetIdForScript(conn2.x8_objId);
                if (TCastToPtr<CPathCamera>(mgr.ObjectById(id)) ||
                    TCastToPtr<CScriptSpindleCamera>(mgr.ObjectById((id))))
                {
                    ent->ConnectionList().erase(it);
                    if (x164_delegatedCamera != id)
                        x164_delegatedCamera = id;
                    break;
                }
            }
            break;
        }
    }
}

void CScriptCameraHint::AddHelper(TUniqueId id)
{
    auto search = std::find_if(x150_helpers.begin(), x150_helpers.end(),
                               [id](TUniqueId tid) { return tid == id; });
    if (search == x150_helpers.end())
        x150_helpers.push_back(id);
}

void CScriptCameraHint::RemoveHelper(TUniqueId id)
{
    auto search = std::find_if(x150_helpers.begin(), x150_helpers.end(),
                               [id](TUniqueId tid) { return tid == id; });
    if (search != x150_helpers.end())
        x150_helpers.erase(search);
}

void CScriptCameraHint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr)
{
    switch (msg)
    {
    case EScriptObjectMessage::Deleted:
    case EScriptObjectMessage::Deactivate:
        mgr.GetCameraManager()->DeleteCameraHint(GetUniqueId(), mgr);
        break;
    case EScriptObjectMessage::InitializedInArea:
        InitializeInArea(mgr);
        break;
    default:
        break;
    }

    if (GetActive())
    {
        switch (msg)
        {
        case EScriptObjectMessage::Increment:
            AddHelper(sender);
            mgr.GetCameraManager()->AddActiveCameraHint(GetUniqueId(), mgr);
            x166_inactive = false;
            break;
        case EScriptObjectMessage::Decrement:
            RemoveHelper(sender);
            mgr.GetCameraManager()->AddInactiveCameraHint(GetUniqueId(), mgr);
            break;
        default:
            break;
        }
    }

    if (msg == EScriptObjectMessage::Follow)
    {
        if (!GetActive())
        {
            if (TCastToConstPtr<CActor> act = mgr.GetObjectById(sender))
            {
                zeus::CVector3f followerToThisFlat = x168_origXf.origin - act->GetTranslation();
                followerToThisFlat.z = 0.f;
                if (followerToThisFlat.canBeNormalized())
                    followerToThisFlat.normalize();
                else
                    followerToThisFlat = act->GetTransform().basis[1];
                zeus::CVector3f target = act->GetTranslation() + followerToThisFlat;
                target.z = x168_origXf.origin.z + followerToThisFlat.z;
                SetTransform(zeus::lookAt(act->GetTranslation(), target));
            }
        }
        AddHelper(sender);
        mgr.GetCameraManager()->AddActiveCameraHint(GetUniqueId(), mgr);
    }

    CActor::AcceptScriptMsg(msg, sender, mgr);
}

}
