#pragma once

#include "CEntity.hpp"

namespace urde {
class CScriptSwitch : public CEntity {
  bool x34_opened;
  bool x35_closeOnOpened;

public:
  CScriptSwitch(TUniqueId, std::string_view, const CEntityInfo&, bool, bool, bool);

  void Accept(IVisitor& visitor);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);
};
} // namespace urde
