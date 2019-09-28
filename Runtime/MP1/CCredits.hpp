#pragma once

#include "Runtime/CIOWin.hpp"

namespace urde::MP1 {

class CCredits : public CIOWin {
public:
  CCredits();
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  bool GetIsContinueDraw() const override { return false; }
  void Draw() const override;
};

} // namespace urde::MP1
