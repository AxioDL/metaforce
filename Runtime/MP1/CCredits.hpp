#pragma once

#include "CIOWin.hpp"

namespace urde::MP1 {

class CCredits : public CIOWin {
public:
  CCredits();
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&);
  bool GetIsContinueDraw() const { return false; }
  void Draw() const;
};

} // namespace urde::MP1
