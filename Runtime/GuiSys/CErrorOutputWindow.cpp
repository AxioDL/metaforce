#include "Runtime/GuiSys/CErrorOutputWindow.hpp"

#include "Runtime/Graphics/CGraphics.hpp"

namespace metaforce {

CErrorOutputWindow::CErrorOutputWindow(bool flag) : CIOWin("Error Output Window") {
  x18_24_ = false;
  x18_25_ = true;
  x18_26_ = true;
  x18_27_ = true;
  x18_28_ = flag;
}

CIOWin::EMessageReturn CErrorOutputWindow::OnMessage(const CArchitectureMessage&, CArchitectureQueue&) {
  return EMessageReturn::Normal;
}

void CErrorOutputWindow::Draw() {
  // SCOPED_GRAPHICS_DEBUG_GROUP("CErrorOutputWindow::Draw", zeus::skGreen);
}

} // namespace metaforce
