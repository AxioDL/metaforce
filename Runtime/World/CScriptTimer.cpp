#include "CScriptTimer.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde {

CScriptTimer::CScriptTimer(TUniqueId uid, std::string_view name, const CEntityInfo& info, float startTime,
                           float maxRandDelay, bool loop, bool autoStart, bool active)
: CEntity(uid, info, active, name)
, x34_time(startTime)
, x38_startTime(startTime)
, x3c_maxRandDelay(maxRandDelay)
, x40_loop(loop)
, x41_autoStart(autoStart)
, x42_isTiming(autoStart) {}

void CScriptTimer::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptTimer::Think(float dt, CStateManager& mgr) {
  if (GetActive() && IsTiming())
    ApplyTime(dt, mgr);
}

void CScriptTimer::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  if (GetActive()) {
    if (msg == EScriptObjectMessage::Start) {
      StartTiming(true);
    } else if (msg == EScriptObjectMessage::Stop) {
      StartTiming(false);
    } else if (msg == EScriptObjectMessage::ResetAndStart) {
      Reset(stateMgr);
      StartTiming(true);
    } else if (msg == EScriptObjectMessage::Reset) {
      Reset(stateMgr);
    } else if (msg == EScriptObjectMessage::StopAndReset) {
      Reset(stateMgr);
      StartTiming(false);
    }
  }
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

bool CScriptTimer::IsTiming() const { return x42_isTiming; }

void CScriptTimer::StartTiming(bool isTiming) { x42_isTiming = isTiming; }

void CScriptTimer::Reset(CStateManager& mgr) {
  float rDt = mgr.GetActiveRandom()->Float();
  x34_time = (x3c_maxRandDelay * rDt) + x38_startTime;
}

void CScriptTimer::ApplyTime(float dt, CStateManager& mgr) {
  if (x34_time <= 0.f || !GetActive())
    return;

  x34_time -= dt;
  if (x34_time <= 0.f) {
    SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);

    x42_isTiming = false;
    if (x40_loop) {
      Reset(mgr);
      if (x41_autoStart)
        x42_isTiming = true;
    }
  }
}
} // namespace urde
