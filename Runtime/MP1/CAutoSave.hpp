#pragma once

#include "Runtime/CIOWin.hpp"

namespace urde::MP1 {
class CSaveGameScreen;
class CAutoSave : CIOWin {
  std::unique_ptr<CSaveGameScreen> x14_savegameScreen;

public:
  CAutoSave();

  void Draw() override;
  bool GetIsContinueDraw() const override { return false; }
  EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) override;
};
} // namespace urde::MP1
