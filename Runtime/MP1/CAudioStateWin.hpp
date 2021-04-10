#pragma once

#include "Runtime/CIOWin.hpp"

namespace metaforce::MP1 {
class CAudioStateWin : public CIOWin {
public:
  CAudioStateWin() : CIOWin("CAudioStateWin") {}
  CIOWin::EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) override;
};

} // namespace metaforce::MP1
