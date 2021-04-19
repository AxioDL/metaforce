#include "Runtime/MP1/CCredits.hpp"

#include "Runtime/Graphics/CGraphics.hpp"

namespace metaforce::MP1 {

CCredits::CCredits() : CIOWin("Credits") {}

CIOWin::EMessageReturn CCredits::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) {
  return EMessageReturn::Normal;
}

void CCredits::Draw() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CCredits::Draw", zeus::skGreen);
}

} // namespace metaforce::MP1
