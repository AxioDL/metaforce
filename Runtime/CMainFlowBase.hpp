#pragma once

#include "Runtime/CIOWin.hpp"

namespace metaforce {

enum class EClientFlowStates {
  Unspecified = -1,
  None = 0,
  WinBad = 1,
  WinGood = 2,
  WinBest = 3,
  LoseGame = 4,
  Default = 5,
  StateSetter = 6,
  PreFrontEnd = 7,
  FrontEnd = 8,
  Game = 14,
  GameExit = 15
};

class CMainFlowBase : public CIOWin {
protected:
  EClientFlowStates x14_gameState = EClientFlowStates::Unspecified;

public:
  explicit CMainFlowBase(const char* name) : CIOWin(name) {}
  EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) override;
  virtual void AdvanceGameState(CArchitectureQueue& queue) = 0;
  virtual void SetGameState(EClientFlowStates state, CArchitectureQueue& queue) = 0;
};

} // namespace metaforce
