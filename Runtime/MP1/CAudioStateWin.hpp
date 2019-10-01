#pragma once

#include "Runtime/CIOWin.hpp"

namespace urde::MP1 {
class CAudioStateWin : public CIOWin {
public:
  CAudioStateWin() : CIOWin("CAudioStateWin") {}
  CIOWin::EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) override;
};

} // namespace urde::MP1
