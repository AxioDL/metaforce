#ifndef __URDE_IWEAPONRENDERER_HPP__
#define __URDE_IWEAPONRENDERER_HPP__

namespace urde
{
class CParticleGen;

class IWeaponRenderer
{
public:
    virtual ~IWeaponRenderer() = default;
    virtual void AddParticleGen(const CParticleGen&);
};

class CDefaultWeaponRenderer : public IWeaponRenderer
{
public:
    void AddParticleGen(const CParticleGen&);
};

}

#endif // __URDE_IWEAPONRENDERER_HPP__
