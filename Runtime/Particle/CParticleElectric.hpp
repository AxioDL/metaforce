#ifndef __PSHAG_CPARTICLEELECTRIC_HPP__
#define __PSHAG_CPARTICLEELECTRIC_HPP__

#include "CParticleGen.hpp"
#include "CToken.hpp"
#include "CRandom16.hpp"

namespace urde
{
class CElectricDescription;

class CParticleElectric : public CParticleGen
{
public:
    class CLineManager
    {

    };
private:
    TLockedToken<CElectricDescription> x1c_elecDesc;
    u32              x28 = 0;
    u32              x2c = 0;
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
    u32              x150 = 0;
    u32              x154 = 1;
    u32              x158 = 0;
    float            x15c = 0.f;
    zeus::CAABox     x160_systemBounds = zeus::CAABox::skInvertedBox;
    bool             x184 = false;
    bool             x194 = false;
    bool             x1b4 = false;
    zeus::CColor     x1b8_moduColor;
    bool             x1bc_hasModuColor = false;
    rstl::reserved_vector<std::unique_ptr<CLineManager>, 32> x1c0_lineManagers;

    union
    {
        struct { bool x438_25 : 1; bool x438_26 : 1; bool x438_28: 1; };
        u8 dummy = 0;
    };
public:


    CParticleElectric(const TToken<CElectricDescription>& desc);

    void SetupLineGXMaterial();
    void RenderLines();
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
    std::pair<zeus::CAABox, bool> GetBounds() const;
    u32 GetParticleCount() const;
    bool SystemHasLight() const;
    CLight GetLight() const;
    bool GetParticleEmission() const;
    void DestroyParticles();
    void Reset() {}
};

}

#endif // __PSHAG_CPARTICLEELECTRIC_HPP__
