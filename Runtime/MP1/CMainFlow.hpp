#pragma once

#include "CMainFlowBase.hpp"

namespace urde {
class CArchitectureMessage;
class CArchitectureQueue;

namespace MP1 {

class CMainFlow : public CMainFlowBase {
public:
  CMainFlow() : CMainFlowBase("CMainFlow") {}
  void AdvanceGameState(CArchitectureQueue& queue);
  void SetGameState(EClientFlowStates state, CArchitectureQueue& queue);
  bool GetIsContinueDraw() const { return false; }
  void Draw() const {}
};

} // namespace MP1

} // namespace urde
