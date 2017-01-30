#ifndef CSCRIPTMEMORYRELAY_HPP
#define CSCRIPTMEMORYRELAY_HPP

#include "CEntity.hpp"

namespace urde
{
class CScriptMemoryRelay : public CEntity
{
    union
    {
        struct
        {
            bool x34_24_;
            bool x34_25_skipSendActive;
            bool x34_26_ignoreMessages;
        };
        u8 dummy = 0;
    };
public:
    CScriptMemoryRelay(TUniqueId, const std::string& name, const CEntityInfo&, bool, bool, bool);
    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr);
};
}

#endif // CSCRIPTMEMORYRELAY_HPP
