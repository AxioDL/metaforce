#include "Runtime/GuiSys/CConsoleOutputWindow.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

namespace metaforce {

CConsoleOutputWindow::CConsoleOutputWindow(int, float, float) : CIOWin("Console Output Window") {}

CIOWin::EMessageReturn CConsoleOutputWindow::OnMessage(const CArchitectureMessage&, CArchitectureQueue&) {
  return EMessageReturn::Normal;
}

void CConsoleOutputWindow::Draw() {
  //SCOPED_GRAPHICS_DEBUG_GROUP("CConsoleOutputWindow::Draw", zeus::skGreen);
}

} // namespace metaforce
