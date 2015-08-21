#ifndef __RETRO_CPARTICLEGEN_HPP__
#define __RETRO_CPARTICLEGEN_HPP__

#include "../RetroTypes.hpp"
#include "CParticleGen.hpp"
#include "CTransform.hpp"
#include "CColor.hpp"
#include "CAABox.hpp"

namespace Retro
{
class CWarp;
class CLight;

class CParticleGen
{
public:
    virtual ~CParticleGen() {}
    virtual void Update(double);
    virtual void Render();
    virtual void SetOrientation(const CTransform&);
    virtual void SetTranslation(const CVector3f&);
    virtual void SetGlobalOrientation(const CTransform&);
    virtual void SetGlobalTranslation(const CVector3f&);
    virtual void SetGlobalScale(const CVector3f&);
    virtual void SetLocalScale(const CVector3f&);
    virtual void SetParticleEmission(bool);
    virtual void SetModulationColor(const CColor&);
    virtual const CTransform& GetOrientation() const;
    virtual const CVector3f& GetTranslation() const;
    virtual const CTransform& GetGlobalOrientation() const;
    virtual const CVector3f& GetGlobalTranslation() const;
    virtual const CVector3f& GetGlobalScale() const;
    virtual bool GetParticleEmission() const;
    virtual const CColor& GetModulationColor() const;
    virtual bool IsSystemDeletable() const;
    virtual CAABox GetBounds() const;
    virtual u32 GetParticleCount() const;
    virtual bool SystemHasLight() const;
    virtual CLight GetLight() const;
    virtual void DestroyParticles();
    virtual void AddModifier(CWarp*);
};

}

#endif // __RETRO_CPARTICLEGEN_HPP__
