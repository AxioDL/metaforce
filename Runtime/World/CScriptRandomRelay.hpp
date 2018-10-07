#pragma once

#include "CEntity.hpp"

namespace urde
{
class CScriptRandomRelay : public CEntity
{
    u32 x34_connectionCount;
    u32 x38_variance;
    bool x3c_clamp;
public:
    CScriptRandomRelay(TUniqueId, std::string_view, const CEntityInfo&, s32, s32, bool, bool);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr);
    void SendLocalScriptMsgs(EScriptObjectState state, CStateManager& stateMgr);
};
}

