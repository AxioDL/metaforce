#pragma once

#include "CEntity.hpp"

namespace urde {

class CScriptTimer : public CEntity {
  float x34_time;
  float x38_startTime;
  float x3c_maxRandDelay;
  bool x40_loop;
  bool x41_autoStart;
  bool x42_isTiming;

public:
  CScriptTimer(TUniqueId, std::string_view name, const CEntityInfo& info, float, float, bool, bool, bool);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  bool IsTiming() const;
  void StartTiming(bool isTiming);
  void Reset(CStateManager&);
  void ApplyTime(float, CStateManager&);
};

} // namespace urde
