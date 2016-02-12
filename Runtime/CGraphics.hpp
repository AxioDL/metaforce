#ifndef __RETRO_CGRAPHICS_HPP__
#define __RETRO_CGRAPHICS_HPP__

#include "RetroTypes.hpp"

namespace Retro
{

enum class ERglLight : u8
{
    None  = 0,
    Zero  = 1,
    One   = 1 << 1,
    Two   = 1 << 2,
    Three = 1 << 3,
    Four  = 1 << 4,
    Five  = 1 << 5,
    Six   = 1 << 6,
    Seven = 1 << 7
};
ENABLE_BITWISE_ENUM(ERglLight)

class CGraphics
{
public:
    static u32 g_NumLightsActive;
    static ERglLight g_LightActive;
    static ERglLight g_LightsWereOn;
    static void DisableAllLights();
    static void EnableLight(ERglLight light);
    static void SetLightState(ERglLight lightState);
};

}

#endif // __RETRO_CGRAPHICS_HPP__
