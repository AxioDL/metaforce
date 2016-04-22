#include "CScriptDoor.hpp"
#include "Collision/CMaterialList.hpp"
#include "Character/CAnimData.hpp"
#include "Character/CAnimPlaybackParms.hpp"
#include "AutoMapper/CMapWorldInfo.hpp"
#include "CStateManager.hpp"

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
                         bool material, bool, float, bool ballDoor)
    : CPhysicsActor(uid, active, name, info, xf, std::move(mData), MakeDoorMaterialList(material),
                aabb, SMoverData(1.f), actParms, 0.3f, 0.1f)
{
    x264_ = GetBoundingBox();
}

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

        SetDoorAnimation(EDoorAnimType::One);
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

void CScriptDoor::OpenDoor(TUniqueId uid, CStateManager& mgr)
{
    TEditorId eid = mgr.GetEditorIdForUniqueId(uid);
    mgr.MapWorldInfo()->SetDoorVisited(eid, true);

    const CScriptDoor* door = dynamic_cast<const CScriptDoor*>(mgr.GetObjectById(uid));

    if (door)
        x27c_otherId = door->x8_uid;

    SetDoorAnimation(EDoorAnimType::Zero);
    if (x27c_otherId != kInvalidUniqueId)
        SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
    else
    {
        SendScriptMsgs(EScriptObjectState::Open, mgr, EScriptObjectMessage::None);

        /* TODO: Finish this */
    }
}

void CScriptDoor::SetDoorAnimation(CScriptDoor::EDoorAnimType type)
{
    CModelData* modelData = x64_modelData.get();
    if (x260_doorState == EDoorAnimType::Zero)
        return;

    if (modelData->AnimationData())
        modelData->AnimationData()->SetAnimation(CAnimPlaybackParms(0, -1, 1.f, true), false);
}

}
