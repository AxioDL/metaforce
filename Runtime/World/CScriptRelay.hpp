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

    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr);
};
}

#endif // CSCRIPTRELAY_HPP
