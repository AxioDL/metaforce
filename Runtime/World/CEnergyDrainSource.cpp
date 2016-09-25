#include "CEnergyDrainSource.hpp"

namespace urde
{

CEnergyDrainSource::CEnergyDrainSource(TUniqueId src, float intensity)
    : x0_source(src)
    , x4_intensity(intensity)
{
}

TUniqueId CEnergyDrainSource::GetEnergyDrainSourceId() const
{
    return x0_source;
}

void CEnergyDrainSource::SetEnergyDrainIntensity(float in)
{
    x4_intensity = in;
}

float CEnergyDrainSource::GetEnergyDrainIntensity() const
{
    return x4_intensity;
}

bool CEnergyDrainSource::operator<(const CEnergyDrainSource& other)
{
    return x0_source < other.x0_source;
}

}
