#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CEntity.hpp"

namespace urde {
class CScriptMemoryRelay : public CEntity {
  bool x34_24_defaultActive;
  bool x34_25_skipSendActive;
  bool x34_26_ignoreMessages;

public:
  CScriptMemoryRelay(TUniqueId, std::string_view name, const CEntityInfo&, bool, bool, bool);
  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
};
} // namespace urde
