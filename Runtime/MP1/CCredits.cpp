#include "CCredits.hpp"

namespace urde::MP1 {

CCredits::CCredits() : CIOWin("Credits") {}

CIOWin::EMessageReturn CCredits::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) {
  return EMessageReturn::Normal;
}

void CCredits::Draw() const {}

} // namespace urde::MP1
