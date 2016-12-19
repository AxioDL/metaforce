#ifndef __URDE_CSCRIPTSWITCH_HPP__
#define __URDE_CSCRIPTSWITCH_HPP__

#include "CEntity.hpp"

namespace urde
{
class CScriptSwitch : public CEntity
{
    bool x34_;
    bool x35_;
public:
    CScriptSwitch(TUniqueId, const std::string&, const CEntityInfo&, bool, bool, bool);

    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr);
};
}
#endif // __URDE_CSCRIPTSWITCH_HPP__
