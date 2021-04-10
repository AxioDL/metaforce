#pragma once

#include "Runtime/CIOWin.hpp"

namespace metaforce::MP1 {

class CCredits : public CIOWin {
public:
  CCredits();
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  bool GetIsContinueDraw() const override { return false; }
  void Draw() override;
};

} // namespace metaforce::MP1
