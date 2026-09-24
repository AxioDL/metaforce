#pragma once
#include "MetroidPrime/CIOWin.hpp"

class CImGuiIOWin : public CIOWin {
public:
  CImGuiIOWin() : CIOWin(rstl::string_l("ImGuiIOWin")) {}
  
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  void PreDraw() const override;
  void Draw() const override;
  
private:
  
  void ShowDebugOverlay() const;
};