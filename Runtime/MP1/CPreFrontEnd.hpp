#pragma once

#include "CIOWin.hpp"

namespace urde::MP1 {

class CPreFrontEnd : public CIOWin {
  bool x14_resourceTweaksRegistered = false;

public:
  CPreFrontEnd();
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&);
};

} // namespace urde::MP1
