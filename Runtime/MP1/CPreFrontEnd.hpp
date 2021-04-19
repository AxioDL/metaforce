#pragma once

#include "Runtime/CIOWin.hpp"

namespace metaforce::MP1 {

class CPreFrontEnd : public CIOWin {
  bool x14_resourceTweaksRegistered = false;

public:
  CPreFrontEnd();
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
};

} // namespace metaforce::MP1
