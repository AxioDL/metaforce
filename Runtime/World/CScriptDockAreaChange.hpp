#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CEntity.hpp"

namespace urde {
class CScriptDockAreaChange : public CEntity {
  s32 x34_dockReference;

public:
  CScriptDockAreaChange(TUniqueId, std::string_view, const CEntityInfo&, s32, bool);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
};
} // namespace urde
