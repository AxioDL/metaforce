#ifndef __URDE_MP1_CCREDITS_HPP__
#define __URDE_MP1_CCREDITS_HPP__

#include "CIOWin.hpp"

namespace urde
{
namespace MP1
{

class CCredits : public CIOWin
{
public:
    CCredits();
    EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&);
    bool GetIsContinueDraw() const { return false; }
    void Draw() const;
};

}
}

#endif // __URDE_MP1_CCREDITS_HPP__
