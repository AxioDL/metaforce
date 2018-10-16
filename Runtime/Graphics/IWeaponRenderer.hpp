#pragma once

namespace urde
{
class CParticleGen;

class IWeaponRenderer
{
public:
    virtual ~IWeaponRenderer() = default;
    virtual void AddParticleGen(const CParticleGen&)=0;
};

class CDefaultWeaponRenderer : public IWeaponRenderer
{
public:
    void AddParticleGen(const CParticleGen&);
};

}

