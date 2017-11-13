#include "CScriptCameraShaker.hpp"
#include "TCastTo.hpp"
#include "CStateManager.hpp"
#include "CWorld.hpp"

namespace urde
{

CScriptCameraShaker::CScriptCameraShaker(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                         bool active, const CCameraShakeData& shakeData)
: CEntity(uid, info, active, name), x34_shakeData(shakeData)
{}

void CScriptCameraShaker::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptCameraShaker::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr)
{
    switch (msg)
    {
    case EScriptObjectMessage::Action:
    {
        TAreaId aid = GetAreaIdAlways();
        if (GetActive() && aid != kInvalidAreaId)
        {
            const CGameArea* area = stateMgr.GetWorld()->GetAreaAlways(aid);
            CGameArea::EOcclusionState occState = CGameArea::EOcclusionState::Occluded;
            if (area->IsPostConstructed())
                occState = area->GetPostConstructed()->x10dc_occlusionState;
            if (occState == CGameArea::EOcclusionState::Visible)
                x34_shakeData.SetShakerId(stateMgr.GetCameraManager()->AddCameraShaker(x34_shakeData, false));
        }
        break;
    }
    case EScriptObjectMessage::Deactivate:
    {
        if (GetActive())
            stateMgr.GetCameraManager()->RemoveCameraShaker(x34_shakeData.GetShakerId());
        break;
    }
    default: break;
    }
    CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

}
