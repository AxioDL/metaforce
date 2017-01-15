#ifndef CSCRIPTRELAY_HPP
#define CSCRIPTRELAY_HPP

#include "CEntity.hpp"

namespace urde
{
class CScriptRelay : public CEntity
{
    TUniqueId x34_ = kInvalidUniqueId;
    u32 x38_ = 0;
public:
    CScriptRelay(TUniqueId, const std::string&, const CEntityInfo&, bool);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr);
    void Think(float, CStateManager& stateMgr);
    void UpdateObjectRef(CStateManager& stateMgr);
};
}

#endif // CSCRIPTRELAY_HPP
