#pragma once

#include <string_view>
#include "Runtime/World/CEntity.hpp"

namespace metaforce {
class CScriptSwitch : public CEntity {
  bool x34_opened;
  bool x35_closeOnOpened;

public:
  DEFINE_ENTITY
  CScriptSwitch(TUniqueId, std::string_view, const CEntityInfo&, bool, bool, bool);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
};
} // namespace metaforce
