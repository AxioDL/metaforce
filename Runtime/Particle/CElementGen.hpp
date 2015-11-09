#ifndef __RETRO_CELEMENTGEN_HPP__
#define __RETRO_CELEMENTGEN_HPP__

#include "CParticleGen.hpp"

namespace Retro
{

class CElementGen : public CParticleGen
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
};

}

#endif // __RETRO_CELEMENTGEN_HPP__
