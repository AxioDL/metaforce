#ifndef __PSHAG_CPARTICLESWOOSH_HPP__
#define __PSHAG_CPARTICLESWOOSH_HPP__

#include "CParticleGen.hpp"
#include "CToken.hpp"

namespace pshag
{
class CSwooshDescription;

class CParticleSwoosh : public CParticleGen
{
public:
    CParticleSwoosh(const TToken<CSwooshDescription>& desc, int);

    void Update(double);
    void Render();
    void SetOrientation(const Zeus::CTransform&);
    void SetTranslation(const Zeus::CVector3f&);
    void SetGlobalOrientation(const Zeus::CTransform&);
    void SetGlobalTranslation(const Zeus::CVector3f&);
    void SetGlobalScale(const Zeus::CVector3f&);
    void SetLocalScale(const Zeus::CVector3f&);
    void SetParticleEmission(bool);
    void SetModulationColor(const Zeus::CColor&);
    const Zeus::CTransform& GetOrientation() const;
    const Zeus::CVector3f& GetTranslation() const;
    const Zeus::CTransform& GetGlobalOrientation() const;
    const Zeus::CVector3f& GetGlobalTranslation() const;
    const Zeus::CVector3f& GetGlobalScale() const;
    const Zeus::CColor& GetModulationColor() const;
    bool IsSystemDeletable() const;
    std::pair<Zeus::CAABox, bool> GetBounds() const;
    u32 GetParticleCount() const;
    bool SystemHasLight() const;
    CLight GetLight() const;
    bool GetParticleEmission() const;
    void DestroyParticles();
    void Reset() {}
};

}

#endif // __PSHAG_CPARTICLESWOOSH_HPP__
