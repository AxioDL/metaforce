#include "CScriptControllerAction.hpp"
#include "TCastTo.hpp"
#include "CStateManager.hpp"
#include "Input/ControlMapper.hpp"

namespace urde
{

CScriptControllerAction::CScriptControllerAction(TUniqueId uid, const std::string& name,
                                                 const CEntityInfo& info, bool active,
                                                 ControlMapper::ECommands command, bool b1, u32 w1, bool b2)
: CEntity(uid, info, active, name), x34_command(command), x38_mapScreenSubaction(w1)
{
    x3c_24_mapScreenResponse = b1;
    x3c_25_deactivateOnClose = b2;
}

void CScriptControllerAction::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptControllerAction::Think(float dt, CStateManager& stateMgr)
{
    bool old26 = x3c_26_pressed;
    if (x3c_24_mapScreenResponse)
    {
        if (x38_mapScreenSubaction == 0)
            x3c_26_pressed = stateMgr.GetInMapScreen();
    }
    else
    {
        x3c_26_pressed = ControlMapper::GetDigitalInput(x34_command, stateMgr.GetFinalInput());
    }

    if (GetActive() && x3c_26_pressed != old26)
    {
        if (x3c_26_pressed)
        {
            SendScriptMsgs(EScriptObjectState::Open, stateMgr, EScriptObjectMessage::None);
        }
        else
        {
            SendScriptMsgs(EScriptObjectState::Closed, stateMgr, EScriptObjectMessage::None);
            if (x3c_25_deactivateOnClose)
            {
                SetActive(false);
                SendScriptMsgs(EScriptObjectState::Inactive, stateMgr, EScriptObjectMessage::None);
            }
        }
    }
}

}
