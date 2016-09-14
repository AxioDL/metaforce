#ifndef __URDE_CCONSOLEOUTPUTWINDOW_HPP__
#define __URDE_CCONSOLEOUTPUTWINDOW_HPP__

#include "CIOWin.hpp"

namespace urde
{

class CConsoleOutputWindow : public CIOWin
{
public:
    CConsoleOutputWindow(int, float, float);
    EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&);
    void Draw() const;
};

}

#endif // __URDE_CCONSOLEOUTPUTWINDOW_HPP__
