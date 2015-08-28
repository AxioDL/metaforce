#ifndef __RETRO_ICONTROLLER_HPP__
#define __RETRO_ICONTROLLER_HPP__

namespace Retro
{

class IController
{
public:
    class CControllerAxis
    {
    };
    class CControllerButton
    {
    };
    class IControllerGamepadData
    {
    };

    void Poll();
    u32 GetDeviceCount();

};

}

#endif // __RETRO_ICONTROLLER_HPP__
