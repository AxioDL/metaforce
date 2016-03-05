#ifndef __PSHAG_ICONTROLLER_HPP__
#define __PSHAG_ICONTROLLER_HPP__

#include "../RetroTypes.hpp"

namespace urde
{

class IController
{
public:
    enum class EMotorState
    {
        Stop = 0,
        Rumble = 1,
        StopHard = 2
    };
    virtual void Poll()=0;
    virtual void SetMotorState(EMotorState state)=0;
};

}

#endif // __PSHAG_ICONTROLLER_HPP__
