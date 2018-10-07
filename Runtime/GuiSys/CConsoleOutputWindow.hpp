#pragma once

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

