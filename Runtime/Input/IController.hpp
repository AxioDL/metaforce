#ifndef __RETRO_ICONTROLLER_HPP__
#define __RETRO_ICONTROLLER_HPP__

#include "../RetroTypes.hpp"

namespace Retro
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

#endif // __RETRO_ICONTROLLER_HPP__
