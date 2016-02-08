#ifndef __RETRO_CELEMENTGEN_HPP__
#define __RETRO_CELEMENTGEN_HPP__

#include "../RetroTypes.hpp"
#include "CTransform.hpp"
#include "CColor.hpp"
#include "CAABox.hpp"
#include "CToken.hpp"

namespace Retro
{
class CWarp;
class CLight;
class CGenDescription;

class CElementGen
{
public:
    enum class EModelOrientationType
    {
    };
    enum class EOptionalSystemFlags
    {
    };
private:
    TLockedToken<CGenDescription> x1c_genDesc;
    EModelOrientationType x28_orientType;
    u32 x30 = 0;
    u32 x34 = 0;
    u32 x38 = 0;
    u32 x40 = 0;
    u32 x44 = 0;
    u32 x48 = 0;
    u32 x4c = 0;
    u32 x50 = 0;
    float x58 = 0.f;
    u32 x64 = -1;
    bool x68_particleEmission = true;
    float x6c = 0.f;
    u32 x70 = 0;
    u16 x74 = 99;
    float x78_generatorRate = 1.f;
    float x7c = 0.f;
    float x80 = 0.f;
    float x84 = 0.f;
    float x88 = 0.f;
    float x8c = 0.f;
    float x90 = 0.f;
    float x94 = 0.f;
    float x98 = 0.f;
    float x9c = 0.f;
    float xa0 = 1.f;
    float xa4 = 1.f;
    float xa8 = 1.f;
    Zeus::CTransform xac = Zeus::CTransform::Identity();
    Zeus::CVector3f x88_globalTranslation;
    Zeus::CTransform x1d8_globalOrientation;
    std::vector<CElementGen> x240_children;
    std::vector<CElementGen> x254_children;
public:
    CElementGen(const TToken<CGenDescription>& gen, EModelOrientationType orientType, EOptionalSystemFlags flags);
    virtual ~CElementGen() = default;

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
