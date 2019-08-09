#pragma once

#include "CIOWin.hpp"

namespace urde::MP1 {

class CStateSetterFlow : public CIOWin {
public:
  CStateSetterFlow();
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
};

} // namespace urde::MP1
