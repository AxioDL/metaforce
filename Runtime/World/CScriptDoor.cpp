#include "CScriptDoor.hpp"
#include "CScriptDock.hpp"
#include "Collision/CMaterialList.hpp"
#include "Character/CAnimData.hpp"
#include "Character/CAnimPlaybackParms.hpp"
#include "AutoMapper/CMapWorldInfo.hpp"
#include "CStateManager.hpp"
#include "CWorld.hpp"
#include "Camera/CCameraManager.hpp"
#include "Camera/CBallCamera.hpp"
#include "TCastTo.hpp"

namespace urde
{

static CMaterialList MakeDoorMaterialList(bool open)
{
    CMaterialList ret;
    ret.Add(EMaterialTypes::Solid);
    ret.Add(EMaterialTypes::Immovable);
    ret.Add(EMaterialTypes::Orbit);
    if (!open)
        ret.Add(EMaterialTypes::Occluder);

    return ret;
}

CScriptDoor::CScriptDoor(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                         const zeus::CTransform& xf, CModelData&& mData, const CActorParameters& actParms,
                         const zeus::CVector3f& vec, const zeus::CAABox& aabb, bool active,
                         bool open, bool projectilesCollide, float animLen, bool ballDoor)
    : CPhysicsActor(uid, active, name, info, xf, std::move(mData), MakeDoorMaterialList(open),
                aabb, SMoverData(1.f), actParms, 0.3f, 0.1f)
{
    x258_animLen = animLen;
    x2a8_24_closing = false;
    x2a8_25_wasOpen = open;
    x2a8_26_isOpen = open;
    x2a8_27_conditionsMet = false;
    x2a8_28_projectilesCollide = projectilesCollide;
    x2a8_29_ballDoor = ballDoor;
    x2a8_30_doClose = false;
    x264_ = GetBoundingBox();
    x284_modelBounds = x64_modelData->GetBounds(xf.getRotation());
    x29c_ = vec;

    xe6_27_thermalVisorFlags = 1;
    if (open)
        SetDoorAnimation(EDoorAnimType::Open);

    SetMass(0.f);
}

void CScriptDoor::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

/* ORIGINAL 0-00 OFFSET: 8007F054 */
zeus::CVector3f CScriptDoor::GetOrbitPosition(const CStateManager& /*mgr*/) const
{
    return x34_transform.origin + x29c_;
}

/* ORIGINAL 0-00 OFFSET: 8007E550 */
void CScriptDoor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager &mgr)
{
    switch (msg)
    {
    case EScriptObjectMessage::Close:
    {
        if (!GetActive())
            return;

        if (x27c_partner1 != kInvalidUniqueId && x27c_partner1 != uid)
            return;

        if (x2a8_26_isOpen)
        {
            if (x27e_partner2 != kInvalidUniqueId)
                if (CEntity* ent = mgr.ObjectById(x27e_partner2))
                    mgr.SendScriptMsg(ent, GetUniqueId(), EScriptObjectMessage::Close);
            x2a8_26_isOpen = false;
            SetDoorAnimation(EDoorAnimType::Close);
            mgr.GetCameraManager()->GetBallCamera()->DoorClosing(GetUniqueId());
        }
        else if (x2a8_27_conditionsMet)
        {
            x2a8_27_conditionsMet = false;
            SendScriptMsgs(EScriptObjectState::Closed, mgr, EScriptObjectMessage::None);
        }
        break;
    }
    case EScriptObjectMessage::Action:
    {
        if (x27c_partner1 != kInvalidUniqueId)
        {
            if (TCastToPtr<CScriptDoor> door = mgr.ObjectById(x27c_partner1))
            {
                if (x2a8_26_isOpen)
                    return;
                x2a8_30_doClose = true;
                mgr.SendScriptMsg(door, GetUniqueId(), EScriptObjectMessage::Close);
            }
        }
        else if (x2a8_26_isOpen)
        {
            x2a8_30_doClose = true;
            if (TCastToPtr<CScriptDoor> door = mgr.ObjectById(x27e_partner2))
            {
                mgr.SendScriptMsg(door, GetUniqueId(), EScriptObjectMessage::Close);
                x2a8_30_doClose = true;
            }
            x2a8_26_isOpen = false;
            SetDoorAnimation(EDoorAnimType::Close);
            mgr.GetCameraManager()->GetBallCamera()->DoorClosing(GetUniqueId());
        }
        break;
    }
    case EScriptObjectMessage::Open:
    {
        if (!GetActive() || x2a8_26_isOpen)
            return;

        u32 doorCond = TCastToConstPtr<CScriptDoor>(mgr.GetObjectById(uid)) ? 2 : GetDoorOpenCondition(mgr);
        switch(doorCond)
        {
        case 1:
            x2a8_27_conditionsMet = true;
            x280_prevDoor = uid;
            break;
        case 2:
            OpenDoor(uid, mgr);
            break;
        default:
            x2a8_25_wasOpen= false;
            x2a8_24_closing = true;
            break;
        }
        break;
    }
    case EScriptObjectMessage::InitializedInArea:
    {
        for (const SConnection& conn : x20_conns)
        {
            if (conn.x4_msg == EScriptObjectMessage::Increment)
            {
                TUniqueId dock = mgr.GetIdForScript(conn.x8_objId);
                if (TCastToConstPtr<CScriptDock> d = mgr.GetObjectById(dock))
                    x282_dockId = d->GetUniqueId();
            }
        }
        break;
    }
    case EScriptObjectMessage::SetToZero:
    {
        x2a8_28_projectilesCollide = true;
        mgr.MapWorldInfo()->SetDoorVisited(mgr.GetEditorIdForUniqueId(GetUniqueId()), true);
        break;
    }
    case EScriptObjectMessage::SetToMax:
    {
        x2a8_28_projectilesCollide = false;
        break;
    }
    default:
        CActor::AcceptScriptMsg(msg, uid, mgr);
    }
}


void CScriptDoor::Think(float dt, CStateManager& mgr)
{
    if (!GetActive())
        return;

    if (!x2a8_26_isOpen && x25c_animTime < 0.5f)
        x25c_animTime += dt;

    if (x2a8_27_conditionsMet && GetDoorOpenCondition(mgr) == 2)
    {
        x2a8_27_conditionsMet = false;
        OpenDoor(x280_prevDoor, mgr);
    }

    if (x2a8_24_closing)
    {
        x2a8_25_wasOpen = false;
        mgr.GetCameraManager()->GetBallCamera()->DoorClosed(GetUniqueId());
        x2a8_28_projectilesCollide = false;
        x2a8_24_closing = false;
        SendScriptMsgs(EScriptObjectState::Closed, mgr, EScriptObjectMessage::Decrement);
        x25c_animTime = 0.f;
        x2a8_30_doClose = false;
    }

    if (x2a8_26_isOpen && !x64_modelData->IsAnimating())
        RemoveMaterial(EMaterialTypes::Solid, EMaterialTypes::Occluder, EMaterialTypes::Orbit, EMaterialTypes::Scannable, mgr);
    else
    {
        if (x2a8_25_wasOpen && !x64_modelData->IsAnimating())
        {
            x2a8_25_wasOpen = false;
            mgr.GetCameraManager()->GetBallCamera()->DoorClosed(GetUniqueId());
            x2a8_28_projectilesCollide = false;
            x2a8_27_conditionsMet = false;
            SendScriptMsgs(EScriptObjectState::Closed, mgr, EScriptObjectMessage::None);
        }

        if (GetScannableObjectInfo())
            AddMaterial(EMaterialTypes::Solid, EMaterialTypes::Metal, EMaterialTypes::Occluder, EMaterialTypes::Orbit, EMaterialTypes::Scannable, mgr);
        else
            AddMaterial(EMaterialTypes::Solid, EMaterialTypes::Metal, EMaterialTypes::Occluder, EMaterialTypes::Orbit, mgr);
    }

    if (x64_modelData->IsAnimating())
        UpdateAnimation((x64_modelData->GetAnimationDuration(s32(x260_doorState)) / x258_animLen) * dt, mgr, true);

    xe7_31_targetable = mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan;
}

void CScriptDoor::AddToRenderer(const zeus::CFrustum& /*frustum*/, const CStateManager &mgr) const
{
    if (!xe4_30_outOfFrustum)
        CPhysicsActor::Render(mgr);
}

/* ORIGINAL 0-00 OFFSET: 8007E0BC */
void CScriptDoor::ForceClosed(CStateManager & mgr)
{
    if (x2a8_26_isOpen)
    {
        x2a8_26_isOpen = false;
        x2a8_25_wasOpen = false;

        mgr.GetCameraManager()->GetBallCamera()->DoorClosing(x8_uid);
        mgr.GetCameraManager()->GetBallCamera()->DoorClosed(x8_uid);

        SetDoorAnimation(EDoorAnimType::Close);
        SendScriptMsgs(EScriptObjectState::Closed, mgr, EScriptObjectMessage::None);

        x25c_animTime = 0.f;
        x2a8_27_conditionsMet = false;
        x2a8_30_doClose = false;
    }
    else if (x2a8_27_conditionsMet)
    {
        x2a8_27_conditionsMet = false;
        x2a8_30_doClose = false;
        SendScriptMsgs(EScriptObjectState::Closed, mgr, EScriptObjectMessage::None);
    }
}

/* ORIGINAL 0-00 OFFSET: 8007E1C4 */
bool CScriptDoor::IsConnectedToArea(const CStateManager& mgr, TAreaId areaId)
{
    const CScriptDock* dockEnt = TCastToConstPtr<CScriptDock>(mgr.GetObjectById(x282_dockId));
    if (dockEnt)
    {
        if (dockEnt->GetAreaId() == areaId)
            return true;

        const CWorld* world = mgr.GetWorld();
        const CGameArea* area = world->GetAreaAlways(dockEnt->GetAreaId());
        const CGameArea::Dock* dock = area->GetDock(dockEnt->GetDockId());
        if (dock->GetConnectedAreaId(dockEnt->GetDockReference(mgr)) == areaId)
            return true;
    }
    return false;
}

/* ORIGINAL 0-00 OFFSET: 8007EA64 */
void CScriptDoor::OpenDoor(TUniqueId uid, CStateManager& mgr)
{
    TEditorId eid = mgr.GetEditorIdForUniqueId(uid);
    mgr.MapWorldInfo()->SetDoorVisited(eid, true);
    x2a8_26_isOpen = true;
    x2a8_25_wasOpen = true;
    x2a8_27_conditionsMet = false;

    if (const CScriptDoor* door = TCastToConstPtr<CScriptDoor>(mgr.GetObjectById(uid)))
        x27c_partner1 = door->GetUniqueId();

    SetDoorAnimation(EDoorAnimType::Open);
    if (x27c_partner1 == kInvalidUniqueId)
        SendScriptMsgs(EScriptObjectState::Open, mgr, EScriptObjectMessage::None);
    else
        SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);

    if (TCastToConstPtr<CScriptDock> dock1 = mgr.GetObjectById(x282_dockId))
    {
        for (CEntity* ent : mgr.GetPlatformAndDoorObjectList())
        {
            TCastToConstPtr<CScriptDoor> door = ent;
            if (!door || door->GetUniqueId() == GetUniqueId())
                continue;

            if(TCastToConstPtr<CScriptDock> dock2 = mgr.GetObjectById(door->x282_dockId))
            {
                if (dock1->GetCurrentConnectedAreaId(mgr) == dock2->GetAreaId() && dock2->GetCurrentConnectedAreaId(mgr) == dock1->GetAreaId())
                {
                    x27e_partner2 = door->GetUniqueId();
                    mgr.SendScriptMsg(ent, GetUniqueId(), EScriptObjectMessage::Open);
                }
            }
        }
    }

    if (x27c_partner1 == kInvalidUniqueId && x27e_partner2 == kInvalidUniqueId)
    {
        for (const SConnection& conn : x20_conns)
        {
            if (conn.x4_msg != EScriptObjectMessage::Open)
                continue;
            if (TCastToConstPtr<CScriptDoor> door = mgr.GetObjectById(mgr.GetIdForScript(conn.x8_objId)))
            {
                x27e_partner2 = door->GetUniqueId();
                break;
            }
        }
    }
}

/* ORIGINAL 0-00 OFFSET: 8007ED4C */
u32 CScriptDoor::GetDoorOpenCondition(CStateManager& mgr)
{
    const TCastToPtr<CScriptDock> dock = mgr.ObjectById(x282_dockId);
    if (!dock)
        return 2;

    if (x25c_animTime < 0.05f || x2a8_30_doClose)
        return 1;

    TAreaId destArea = dock->GetAreaId();
    if (destArea < 0 || destArea >= mgr.GetWorld()->GetNumAreas())
        return 0;

    if (!mgr.GetWorld()->AreSkyNeedsMet())
        return 1;

    TAreaId connArea = mgr.GetWorld()->GetAreaAlways(
                dock->GetAreaId())->GetDock(
                dock->GetDockId())->GetConnectedAreaId(dock->GetDockReference(mgr));

    if (connArea == kInvalidAreaId)
        return 0;

    const CWorld* world = mgr.GetWorld();
    const CGameArea* area = world->GetAreaAlways(connArea);

    if (!area->IsPostConstructed())
    {
        mgr.SendScriptMsg(dock, GetUniqueId(), EScriptObjectMessage::SetToMax);
        return 1;
    }

    if (area->GetPostConstructed()->x113c_playerActorsLoading != 0)
        return 1;

    for (CEntity* ent : mgr.GetPlatformAndDoorObjectList())
    {
        TCastToPtr<CScriptDoor> door(ent);
        if (!door || door->GetUniqueId() == GetUniqueId())
            continue;

        if (door->GetAreaIdAlways() == GetAreaIdAlways() && door->x2a8_25_wasOpen)
        {
            if (door->x282_dockId != kInvalidUniqueId)
                return 1;
        }
    }

    for (const CGameArea& aliveArea : *world)
    {
        if (aliveArea.GetAreaId() == area->GetAreaId())
            continue;

        if (!aliveArea.IsFinishedOccluding())
            return 1;
    }

    //if (area->TryTakingOutOfARAM())
    {
        if (world->GetMapWorld()->IsMapAreasStreaming())
            return 1;
    }

    return 2;
}

/* ORIGINAL 0-00 OFFSET: 8007E9D0 */
void CScriptDoor::SetDoorAnimation(CScriptDoor::EDoorAnimType type)
{
    x260_doorState = type;
    CModelData* modelData = x64_modelData.get();
    if (modelData && modelData->AnimationData())
        modelData->AnimationData()->SetAnimation(CAnimPlaybackParms(s32(type), -1, 1.f, true), false);
}

std::experimental::optional<zeus::CAABox> CScriptDoor::GetProjectileBounds() const
{
    if (x2a8_28_projectilesCollide)
        return {{x284_modelBounds.min + GetTranslation(), x284_modelBounds.max + GetTranslation()}};
    return {};
}

}
