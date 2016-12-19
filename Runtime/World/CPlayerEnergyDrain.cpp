#include "CPlayerEnergyDrain.hpp"
#include "CStateManager.hpp"

namespace urde
{

CPlayerEnergyDrain::CPlayerEnergyDrain(u32 capacity) { x0_sources.reserve(capacity); }

void CPlayerEnergyDrain::AddEnergyDrainSource(TUniqueId id, float intensity) { x0_sources.emplace_back(id, intensity); }

void CPlayerEnergyDrain::RemoveEnergyDrainSource(TUniqueId id)
{
    auto it = std::lower_bound(x0_sources.begin(), x0_sources.end(), CEnergyDrainSource(id, 0.f));
    if (it != x0_sources.end())
        x0_sources.erase(it);
}

float CPlayerEnergyDrain::GetEnergyDrainIntensity() const
{
    float intensity = 0.f;

    for (const CEnergyDrainSource& src : x0_sources)
        intensity += src.GetEnergyDrainIntensity();

    return intensity;
}

float CPlayerEnergyDrain::GetEnergyDrainTime() const { return x10_energyDrainTime; }

void CPlayerEnergyDrain::ProcessEnergyDrain(const CStateManager& mgr, float dt)
{
    auto it = x0_sources.begin();

    for (; it != x0_sources.end(); ++it)
    {
        if (mgr.GetObjectById((*it).GetEnergyDrainSourceId()) == nullptr)
            RemoveEnergyDrainSource((*it).GetEnergyDrainSourceId());
    }

    if (x0_sources.empty())
        x10_energyDrainTime = 0.f;
    else
        x10_energyDrainTime += dt;
}
}
