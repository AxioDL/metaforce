#include "CScriptDoor.hpp"
#include "CScriptDock.hpp"
#include "Collision/CMaterialList.hpp"
#include "Character/CAnimData.hpp"
#include "Character/CAnimPlaybackParms.hpp"
#include "AutoMapper/CMapWorldInfo.hpp"
#include "CStateManager.hpp"
#include "CWorld.hpp"

namespace urde
{

static CMaterialList MakeDoorMaterialList(bool material)
{
    CMaterialList ret;
    ret.Add(EMaterialTypes::Nineteen);
    ret.Add(EMaterialTypes::FourtyThree);
    ret.Add(EMaterialTypes::FourtyOne);
    if (material)
        ret.Add(EMaterialTypes::FourtyTwo);

    return ret;
}

CScriptDoor::CScriptDoor(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                         const zeus::CTransform& xf, CModelData&& mData, const CActorParameters& actParms,
                         const zeus::CVector3f&, const zeus::CAABox& aabb, bool active,
                         bool material, bool b2, float, bool ballDoor)
    : CPhysicsActor(uid, active, name, info, xf, std::move(mData), MakeDoorMaterialList(material),
                aabb, SMoverData(1.f), actParms, 0.3f, 0.1f),
      x2a8_29_ballDoor(ballDoor),
      x2a8_25_(material),
      x2a8_26_(material),
      x2a8_28_(b2),
      x2a8_27_(true)
{
    x264_ = GetBoundingBox();
    x284_modelBounds = x64_modelData->GetBounds(xf.getRotation());

    if (material)
        SetDoorAnimation(EDoorAnimType::Open);

    SetMass(0.f);
}

/* ORIGINAL 0-00 OFFSET: 8007F054 */
zeus::CVector3f CScriptDoor::GetOrbitPosition(const CStateManager& /*mgr*/) const
{
    return x34_transform.origin + x29c_;
}

/* ORIGINAL 0-00 OFFSET: 8007E550 */
void CScriptDoor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager &mgr)
{
    (void)mgr;
    (void)uid;
    (void)msg;
}

void CScriptDoor::Think(float dt, CStateManager& mgr)
{
    if (!GetActive())
        return;

    if (!x2a8_26_)
    {
        if (x25c_ < 0.5f)
            x25c_ += dt;
    }
}

void CScriptDoor::AddToRenderer(const zeus::CFrustum& /*frustum*/, CStateManager &mgr)
{
    if (!xe4_30_)
        CPhysicsActor::Render(mgr);
}

/* ORIGINAL 0-00 OFFSET: 8007E0BC */
void CScriptDoor::ForceClosed(CStateManager & mgr)
{
    if (x2a8_26_)
    {
        x2a8_26_ = false;
        x2a8_25_ = false;

        /* TODO: Figure out what goes here this */
        /*
         * mgr->x870->x80_->sub_80083118(x8_uid)
         * mgr->x870->x80_->sub_800830F4(x8_uid)
         */

        SetDoorAnimation(EDoorAnimType::Close);
        SendScriptMsgs(EScriptObjectState::Closed, mgr, EScriptObjectMessage::None);

        x25c_ = 0.f;
        x2a8_27_ = false;
        x2a8_30_ = false;
    }
    else if (x2a8_27_)
    {
        x2a8_27_ = false;
        x2a8_30_ = false;
        SendScriptMsgs(EScriptObjectState::Closed, mgr, EScriptObjectMessage::None);
    }
}

/* ORIGINAL 0-00 OFFSET: 8007E1C4 */
bool CScriptDoor::IsConnectedToArea(const CStateManager& mgr, TAreaId area)
{
    const CScriptDock* dock = dynamic_cast<const CScriptDock*>(mgr.GetObjectById(x282_dockId));
    if (dock)
    {
        if (dock->x260_area == area)
            return true;
        /*
         * TODO: Determine what's going on here
         * CWorld* world = mgr.GetWorld();
         */
    }
    return false;
}

/* ORIGINAL 0-00 OFFSET: 8007EA64 */
void CScriptDoor::OpenDoor(TUniqueId uid, CStateManager& mgr)
{
    TEditorId eid = mgr.GetEditorIdForUniqueId(uid);
    mgr.MapWorldInfo()->SetDoorVisited(eid, true);

    const CScriptDoor* door = dynamic_cast<const CScriptDoor*>(mgr.GetObjectById(uid));

    if (door)
        x27c_partner = door->GetUniqueId();

    SetDoorAnimation(EDoorAnimType::Open);
    if (x27c_partner != kInvalidUniqueId)
        SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
    else
    {
        SendScriptMsgs(EScriptObjectState::Open, mgr, EScriptObjectMessage::None);

        /* TODO: Finish this */
    }
}

/* ORIGINAL 0-00 OFFSET: 8007ED4C */
u32 CScriptDoor::GetDoorOpenCondition(CStateManager& mgr)
{
    const CScriptDock* dock = dynamic_cast<const CScriptDock*>(mgr.GetObjectById(x282_dockId));

    if (!dock)
        return 2;

    if (x25c_ < 0.05f)
        return 1;

    /* TODO: Finish this */
    return 0;
}

/* ORIGINAL 0-00 OFFSET: 8007E9D0 */
void CScriptDoor::SetDoorAnimation(CScriptDoor::EDoorAnimType type)
{
    x260_doorState = type;
    CModelData* modelData = x64_modelData.get();
    if (modelData && modelData->AnimationData())
        modelData->AnimationData()->SetAnimation(CAnimPlaybackParms(s32(type), -1, 1.f, true), false);
}

}
