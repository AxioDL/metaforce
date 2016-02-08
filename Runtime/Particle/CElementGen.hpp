#ifndef __RETRO_CELEMENTGEN_HPP__
#define __RETRO_CELEMENTGEN_HPP__

#include "../RetroTypes.hpp"
#include "CTransform.hpp"
#include "CColor.hpp"
#include "CAABox.hpp"

namespace Retro
{
class CWarp;
class CLight;
class CGenDescription
{
};

class CElementGen
{
    bool x68_particleEmission;
    float x78_generatorRate;
    Zeus::CVector3f x88_globalTranslation;
    Zeus::CTransform x1d8_globalOrientation;
    std::vector<CElementGen> x240_children;
    std::vector<CElementGen> x254_children;
public:
    virtual const Zeus::CVector3f& GetGlobalTranslation() const
    { return x88_globalTranslation; }
    virtual const Zeus::CTransform& GetGlobalOrientation() const
    { return x1d8_globalOrientation; }
    virtual bool GetParticleEmission() const
    { return x68_particleEmission; }

    void SetGeneratorRateScalar(float scalar)
    {
        if (scalar >= 0.0f)
            x78_generatorRate = scalar;
        else
            x78_generatorRate = 0.0f;

        for (CElementGen& child : x240_children)
            child.SetGeneratorRateScalar(x78_generatorRate);

        for (CElementGen& child : x254_children)
            child.SetGeneratorRateScalar(x78_generatorRate);
    }

    static void Initialize()
    {
    }
    void BuildParticleSystemBounds();

    virtual ~CElementGen() {}
    virtual void Update(double);
    virtual void Render();
    virtual void SetOrientation(const Zeus::CTransform&);
    virtual void SetTranslation(const Zeus::CVector3f&);
    virtual void SetGlobalOrientation(const Zeus::CTransform&);
    virtual void SetGlobalTranslation(const Zeus::CVector3f&);
    virtual void SetGlobalScale(const Zeus::CVector3f&);
    virtual void SetLocalScale(const Zeus::CVector3f&);
    virtual void SetParticleEmission(bool);
    virtual void SetModulationColor(const Zeus::CColor&);
    virtual const Zeus::CTransform& GetOrientation() const;
    virtual const Zeus::CVector3f& GetTranslation() const;
    virtual const Zeus::CVector3f& GetGlobalScale() const;
    virtual const Zeus::CColor& GetModulationColor() const;
    virtual bool IsSystemDeletable() const;
    virtual Zeus::CAABox GetBounds() const;
    virtual u32 GetParticleCount() const;
    virtual bool SystemHasLight() const;
    virtual CLight GetLight() const;
    virtual void DestroyParticles();
    virtual void AddModifier(CWarp*);
};

}

#endif // __RETRO_CELEMENTGEN_HPP__
