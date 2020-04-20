#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Input/ControlMapper.hpp"
#include "Runtime/World/CEntity.hpp"

namespace urde {

class CScriptControllerAction : public CEntity {
  ControlMapper::ECommands x34_command;
  u32 x38_mapScreenSubaction;
  bool x3c_24_mapScreenResponse : 1;
  bool x3c_25_deactivateOnClose : 1;
  bool x3c_26_pressed : 1 = false;

public:
  CScriptControllerAction(TUniqueId uid, std::string_view name, const CEntityInfo& info, bool active,
                          ControlMapper::ECommands command, bool b1, u32 w1, bool b2);
  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager&) override;
};

} // namespace urde
