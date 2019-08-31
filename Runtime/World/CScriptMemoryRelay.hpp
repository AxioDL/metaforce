#pragma once

#include "CEntity.hpp"

namespace urde {
class CScriptMemoryRelay : public CEntity {
  union {
    struct {
      bool x34_24_defaultActive;
      bool x34_25_skipSendActive;
      bool x34_26_ignoreMessages;
    };
    u8 dummy = 0;
  };

public:
  CScriptMemoryRelay(TUniqueId, std::string_view name, const CEntityInfo&, bool, bool, bool);
  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
};
} // namespace urde
