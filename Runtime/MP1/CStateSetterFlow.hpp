#pragma once

#include "Runtime/CIOWin.hpp"

namespace metaforce::MP1 {

class CStateSetterFlow : public CIOWin {
public:
  CStateSetterFlow();
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
};

} // namespace metaforce::MP1
