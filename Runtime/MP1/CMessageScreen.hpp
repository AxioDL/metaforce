#ifndef __URDE_CMESSAGESCREEN_HPP__
#define __URDE_CMESSAGESCREEN_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CFinalInput;

namespace MP1
{

class CMessageScreen
{
public:
    CMessageScreen(ResId msg, float time);
    void ProcessControllerInput(const CFinalInput& input);
    bool Update(float dt, float blurAmt);
};

}
}

#endif // __URDE_CMESSAGESCREEN_HPP__
