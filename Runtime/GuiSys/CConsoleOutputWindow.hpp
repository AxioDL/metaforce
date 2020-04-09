#pragma once

#include "Runtime/CIOWin.hpp"

namespace urde {

class CConsoleOutputWindow : public CIOWin {
public:
  CConsoleOutputWindow(int, float, float);
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  void Draw() override;
};

} // namespace urde
