#ifndef __URDE_CSCRIPTDOCKAREACHANGE_HPP__
#define __URDE_CSCRIPTDOCKAREACHANGE_HPP__

#include "CEntity.hpp"

namespace urde
{
class CScriptDockAreaChange : public CEntity
{
    s32 x34_dockReference;
public:
    CScriptDockAreaChange(TUniqueId, std::string_view, const CEntityInfo&, s32, bool);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr);
};
}

#endif //__URDE_CSCRIPTDOCKAREACHANGE_HPP__
