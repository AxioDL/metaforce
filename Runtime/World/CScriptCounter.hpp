#ifndef __URDE_CSCRIPTCOUNTER_HPP__
#define __URDE_CSCRIPTCOUNTER_HPP__

#include "CEntity.hpp"

namespace urde
{

class CScriptCounter : public CEntity
{
    s32 x34_initial;
    s32 x38_current;
    s32 x3c_max;
    bool x40_autoReset;
public:
    CScriptCounter(TUniqueId, const std::string& name, const CEntityInfo& info,
                   s32, s32, bool, bool);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr);
};

}

#endif // __URDE_CSCRIPTCOUNTER_HPP__
