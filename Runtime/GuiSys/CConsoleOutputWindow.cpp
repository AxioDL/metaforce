#include "CConsoleOutputWindow.hpp"

namespace urde
{

CConsoleOutputWindow::CConsoleOutputWindow(int, float, float)
: CIOWin("Console Output Window")
{
}

CIOWin::EMessageReturn CConsoleOutputWindow::OnMessage(const CArchitectureMessage&, CArchitectureQueue&)
{
    return EMessageReturn::Normal;
}

void CConsoleOutputWindow::Draw() const
{
}

}
