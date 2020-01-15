#include "Runtime/MP1/CCredits.hpp"

#include "Runtime/Graphics/CGraphics.hpp"

namespace urde::MP1 {

CCredits::CCredits() : CIOWin("Credits") {}

CIOWin::EMessageReturn CCredits::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) {
  return EMessageReturn::Normal;
}

void CCredits::Draw() const {
  SCOPED_GRAPHICS_DEBUG_GROUP("CCredits::Draw", zeus::skGreen);
}

} // namespace urde::MP1
