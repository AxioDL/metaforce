#ifndef __URDE_CENERGYDRAINSOURCE_HPP__
#define __URDE_CENERGYDRAINSOURCE_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CEnergyDrainSource
{
    TUniqueId x0_source;
    float x4_intensity;
public:
    CEnergyDrainSource(TUniqueId src, float intensity);
    TUniqueId GetEnergyDrainSourceId() const;
    void SetEnergyDrainIntensity(float);
    float GetEnergyDrainIntensity() const;

    bool operator<(const CEnergyDrainSource& other);
};
}
#endif // __URDE_CENERGYDRAINSOURCE_HPP__
