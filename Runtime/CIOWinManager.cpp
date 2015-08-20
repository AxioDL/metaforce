#include "CIOWinManager.hpp"

namespace Retro
{

bool CIOWinManager::OnIOWinMessage(const CArchitectureMessage& msg)
{
    switch (msg.GetType())
    {
    case MsgChangeIOWinPriority:
    {
        const CArchMsgParmInt32Int32VoidPtr& parm = MakeMsg::GetParmChangeIOWinPriority(msg);

    }
    default: break;
    }
    return false;
}

}
