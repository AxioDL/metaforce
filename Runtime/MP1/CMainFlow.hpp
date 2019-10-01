#pragma once

#include "Runtime/CMainFlowBase.hpp"

namespace urde {
class CArchitectureMessage;
class CArchitectureQueue;

namespace MP1 {

class CMainFlow : public CMainFlowBase {
public:
  CMainFlow() : CMainFlowBase("CMainFlow") {}
  void AdvanceGameState(CArchitectureQueue& queue) override;
  void SetGameState(EClientFlowStates state, CArchitectureQueue& queue) override;
  bool GetIsContinueDraw() const override { return false; }
  void Draw() const override {}
};

} // namespace MP1

} // namespace urde
