#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CEntity.hpp"

namespace urde {
class CScriptRandomRelay : public CEntity {
  s32 x34_sendSetSize;
  s32 x38_sendSetVariance;
  bool x3c_percentSize;

public:
  CScriptRandomRelay(TUniqueId uid, std::string_view name, const CEntityInfo& info, s32 sendSetSize,
                     s32 sendSetVariance, bool percentSize, bool active);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  void SendLocalScriptMsgs(EScriptObjectState state, CStateManager& stateMgr);
};
} // namespace urde
