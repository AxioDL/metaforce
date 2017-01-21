#ifndef __URDE_CPARTICLESWOOSH_HPP__
#define __URDE_CPARTICLESWOOSH_HPP__

#include "CParticleGen.hpp"
#include "CToken.hpp"

namespace urde
{
class CSwooshDescription;

class CParticleSwoosh : public CParticleGen
{
public:
    CParticleSwoosh(const TToken<CSwooshDescription>& desc, int);

    void Update(double);
    void Render();
    void SetOrientation(const zeus::CTransform&);
    void SetTranslation(const zeus::CVector3f&);
    void SetGlobalOrientation(const zeus::CTransform&);
    void SetGlobalTranslation(const zeus::CVector3f&);
    void SetGlobalScale(const zeus::CVector3f&);
    void SetLocalScale(const zeus::CVector3f&);
    void SetParticleEmission(bool);
    void SetModulationColor(const zeus::CColor&);
    const zeus::CTransform& GetOrientation() const;
    const zeus::CVector3f& GetTranslation() const;
    const zeus::CTransform& GetGlobalOrientation() const;
    const zeus::CVector3f& GetGlobalTranslation() const;
    const zeus::CVector3f& GetGlobalScale() const;
    const zeus::CColor& GetModulationColor() const;
    bool IsSystemDeletable() const;
    rstl::optional_object<zeus::CAABox> GetBounds() const;
    u32 GetParticleCount() const;
    bool SystemHasLight() const;
    CLight GetLight() const;
    bool GetParticleEmission() const;
    void DestroyParticles();
    void Reset() {}
};

}

#endif // __URDE_CPARTICLESWOOSH_HPP__
