#ifndef __URDE_CPARTICLEELECTRIC_HPP__
#define __URDE_CPARTICLEELECTRIC_HPP__

#include "CParticleGen.hpp"
#include "CToken.hpp"
#include "CRandom16.hpp"

namespace urde
{
class CElectricDescription;
class CParticleSwoosh;
class CElementGen;

class CParticleElectric : public CParticleGen
{
public:
    struct CLineManager
    {
        std::unique_ptr<CParticleSwoosh> SSWH;
        std::unique_ptr<CElementGen> GPSM;
        std::unique_ptr<CElementGen> EPSM;
    };

private:
    TLockedToken<CElectricDescription> x1c_elecDesc;
    int              x28_currentFrame = 0;
    int              x2c_LIFE = 0x7FFFFF;
    double           x30 = 0.0;
    zeus::CVector3f  x38_translation;
    zeus::CTransform x44_orientation;
    zeus::CVector3f  xa4_globalTranslation;
    zeus::CTransform xb0_globalOrientation;
    zeus::CVector3f  xe0_globalScale;
    zeus::CVector3f  xec_localScale;
    float            x128 = 0.f;
    float            x12c = 0.f;
    zeus::CVector3f  x130;
    CRandom16        x14c_randState;
    int              x150_SSEG = 8;
    int              x154_SCNT = 1;
    int              x158 = 0;
    float            x15c = 0.f;
    zeus::CAABox     x160_systemBounds = zeus::CAABox::skInvertedBox;
    bool             x184 = false;
    bool             x194 = false;
    bool             x1b4 = false;
    zeus::CColor     x1b8_moduColor;
    rstl::reserved_vector<bool,32> x1c0_;
    rstl::reserved_vector<CLineManager, 32> x1e0_lineManagers;
    int              x414 = 0;
    int              x418 = 0;
    int              x41c = 0;
    int              x424 = 0;
    int              x428 = 0;
    int              x42c = 0;
    int              x434 = 0;
    int              x438 = 0;
    int              x43c = 0;
    int              x444 = 0;
    int              x448 = 0; // retail
    int              x44c = 0; // retail

    union
    {
        struct
        {
            bool x450_24 : 1; bool x450_25_HaveGPSM : 1; bool x450_26_HaveEPSM : 1;
            bool x450_27_HaveSSWH : 1; bool x450_28: 1; bool x450_29 : 1;
        };
        u8 dummy = 0;
    };

public:


    CParticleElectric(const TToken<CElectricDescription>& desc);

    void SetupLineGXMaterial();
    void Update(double);
    void RenderLines();
    void RenderSwooshes();
    void Render();
    void CalculateFractal(s32, s32, float, float);
    void CalculatePoints();
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

#endif // __URDE_CPARTICLEELECTRIC_HPP__
