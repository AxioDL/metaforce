#ifndef __URDE_CPARTICLEGEN_HPP__
#define __URDE_CPARTICLEGEN_HPP__

#include "RetroTypes.hpp"
#include "Graphics/CLight.hpp"
#include "CWarp.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CAABox.hpp"
#include <list>

namespace urde
{

class CParticleGen
{
    std::list<CWarp*> x4_modifierList;
public:
    virtual ~CParticleGen() = default;

    virtual bool Update(double)=0;
    virtual void Render()=0;
    virtual void SetOrientation(const zeus::CTransform&)=0;
    virtual void SetTranslation(const zeus::CVector3f&)=0;
    virtual void SetGlobalOrientation(const zeus::CTransform&)=0;
    virtual void SetGlobalTranslation(const zeus::CVector3f&)=0;
    virtual void SetGlobalScale(const zeus::CVector3f&)=0;
    virtual void SetLocalScale(const zeus::CVector3f&)=0;
    virtual void SetParticleEmission(bool)=0;
    virtual void SetModulationColor(const zeus::CColor&)=0;
    virtual void SetGeneratorRate(float rate) {}
    virtual const zeus::CTransform& GetOrientation() const=0;
    virtual const zeus::CVector3f& GetTranslation() const=0;
    virtual const zeus::CTransform& GetGlobalOrientation() const=0;
    virtual const zeus::CVector3f& GetGlobalTranslation() const=0;
    virtual const zeus::CVector3f& GetGlobalScale() const=0;
    virtual const zeus::CColor& GetModulationColor() const=0;
    virtual float GetGeneratorRate() const { return 1.f; }
    virtual bool IsSystemDeletable() const=0;
    virtual rstl::optional_object<zeus::CAABox> GetBounds() const=0;
    virtual u32 GetParticleCount() const=0;
    virtual bool SystemHasLight() const=0;
    virtual CLight GetLight() const=0;
    virtual bool GetParticleEmission() const=0;
    virtual void DestroyParticles()=0;
    virtual void Reset()=0;
    virtual FourCC Get4CharId() const=0;

    virtual void AddModifier(CWarp* mod);
};

}

#endif // __URDE_CPARTICLEGEN_HPP__
