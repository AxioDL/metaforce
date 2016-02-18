#ifndef __PSHAG_CPARTICLEGEN_HPP__
#define __PSHAG_CPARTICLEGEN_HPP__

#include "RetroTypes.hpp"
#include "CLight.hpp"
#include "CWarp.hpp"
#include "CColor.hpp"
#include "CVector3f.hpp"
#include "CTransform.hpp"
#include "CAABox.hpp"
#include <list>

namespace pshag
{

class CParticleGen
{
    std::list<CWarp*> x8_modifierList;
public:
    virtual ~CParticleGen() = default;

    virtual void Update(double)=0;
    virtual void Render()=0;
    virtual void SetOrientation(const Zeus::CTransform&)=0;
    virtual void SetTranslation(const Zeus::CVector3f&)=0;
    virtual void SetGlobalOrientation(const Zeus::CTransform&)=0;
    virtual void SetGlobalTranslation(const Zeus::CVector3f&)=0;
    virtual void SetGlobalScale(const Zeus::CVector3f&)=0;
    virtual void SetLocalScale(const Zeus::CVector3f&)=0;
    virtual void SetParticleEmission(bool)=0;
    virtual void SetModulationColor(const Zeus::CColor&)=0;
    virtual const Zeus::CTransform& GetOrientation() const=0;
    virtual const Zeus::CVector3f& GetTranslation() const=0;
    virtual const Zeus::CTransform& GetGlobalOrientation() const=0;
    virtual const Zeus::CVector3f& GetGlobalTranslation() const=0;
    virtual const Zeus::CVector3f& GetGlobalScale() const=0;
    virtual const Zeus::CColor& GetModulationColor() const=0;
    virtual bool IsSystemDeletable() const=0;
    virtual std::pair<Zeus::CAABox, bool> GetBounds() const=0;
    virtual u32 GetParticleCount() const=0;
    virtual bool SystemHasLight() const=0;
    virtual CLight GetLight() const=0;
    virtual bool GetParticleEmission() const=0;
    virtual void DestroyParticles()=0;
    virtual void Reset()=0;

    virtual void AddModifier(CWarp* mod);
};

}

#endif // __PSHAG_CPARTICLEGEN_HPP__
