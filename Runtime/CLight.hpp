#ifndef __RETRO_CLIGHT_HPP__
#define __RETRO_CLIGHT_HPP__

namespace Retro
{

enum class ELightType
{
    LocalAmbient,
    Directional,
    Custom,
    Spot,
    Spot2,
    LocalAmbient2
};
enum class EFalloffType
{
    Constant,
    Linear,
    Quadratic
};

class CLight
{
public:

};

}

#endif // __RETRO_CLIGHT_HPP__
