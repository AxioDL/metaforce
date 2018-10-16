#pragma once

#include "CIOWin.hpp"
#include "RetroTypes.hpp"

namespace urde
{

class CErrorOutputWindow : public CIOWin
{
public:
    enum class State
    {
        Zero,
        One,
        Two
    };
private:
    State x14_state = State::Zero;
    union
    {
        struct
        {
            bool x18_24_;
            bool x18_25_;
            bool x18_26_;
            bool x18_27_;
            bool x18_28_;
        };
        u16 dummy = 0;
    };
    const wchar_t* x1c_msg;
public:
    CErrorOutputWindow(bool);
    EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&);
    bool GetIsContinueDraw() const { return int(x14_state) < 2; }
    void Draw() const;
};

}

