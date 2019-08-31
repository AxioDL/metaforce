#pragma once

#include "CIOWin.hpp"

namespace urde {

enum class EClientFlowStates { Unspecified = -1, PreFrontEnd = 7, FrontEnd = 8, Game = 14, GameExit = 15 };

class CMainFlowBase : public CIOWin {
protected:
  EClientFlowStates x14_gameState = EClientFlowStates::Unspecified;

public:
  CMainFlowBase(const char* name) : CIOWin(name) {}
  EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) override;
  virtual void AdvanceGameState(CArchitectureQueue& queue) = 0;
  virtual void SetGameState(EClientFlowStates state, CArchitectureQueue& queue) = 0;
};

} // namespace urde
