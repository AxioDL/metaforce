#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CEntity.hpp"

namespace metaforce {

class CScriptCounter : public CEntity {
  s32 x34_initial;
  s32 x38_current;
  s32 x3c_max;
  bool x40_autoReset;

public:
  DEFINE_ENTITY
  CScriptCounter(TUniqueId, std::string_view name, const CEntityInfo& info, s32, s32, bool, bool);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
};

} // namespace metaforce
