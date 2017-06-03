#ifndef __URDE_CPARTICLEDATA_HPP__
#define __URDE_CPARTICLEDATA_HPP__

#include "IOStreams.hpp"
#include "RetroTypes.hpp"

namespace urde
{

class CParticleData
{
public:
    enum class EParentedMode
    {
        Initial,
        ContinuousEmitter,
        ContinuousSystem
    };
private:
    u32 x0_duration = 0;
    SObjectTag x4_particle;
    std::string xc_boneName = "root";
    float x1c_scale = 1.f;
    EParentedMode x20_parentMode = EParentedMode::Initial;
public:
    CParticleData() = default;
    CParticleData(CInputStream& in);
    u32 GetDuration() const { return x0_duration; }
    const SObjectTag& GetTag() const { return x4_particle; }
    const std::string& GetSegmentName() const { return xc_boneName; }
    float GetScale() const { return x1c_scale; }
    EParentedMode GetParentedMode() const { return x20_parentMode; }
};

class CAuxiliaryParticleData
{
public:
};

}

#endif // __URDE_CPARTICLEDATA_HPP__
