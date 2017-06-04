#ifndef __URDE_CPARTICLESWOOSH_HPP__
#define __URDE_CPARTICLESWOOSH_HPP__

#include "CParticleGen.hpp"
#include "CToken.hpp"
#include "CRandom16.hpp"

namespace urde
{
class CSwooshDescription;

class CParticleSwoosh : public CParticleGen
{
    struct SSwooshData
    {
        bool x0_active;
        float x4_;
        float x8_;
        zeus::CVector3f xc_translation;
        zeus::CVector3f x18_offset;
        zeus::CVector3f x24_;
        float x30_irot;
        float x34_;
        zeus::CTransform x38_orientation;
        int x68_frame;
        zeus::CColor x6c_color;
        int x70_;
        zeus::CVector3f x74_velocity;

        SSwooshData(const zeus::CVector3f& translation, const zeus::CVector3f& offset, float irot, float f2, int w, bool active,
                    const zeus::CTransform& orient, const zeus::CVector3f& vel, float f3, float f4,
                    const zeus::CColor& color)
        : x0_active(active), x4_(f3), x8_(f4), xc_translation(translation), x18_offset(offset), x24_(offset),
          x30_irot(irot), x34_(f2), x38_orientation(orient), x6c_color(color), x70_(w), x74_velocity(vel) {}
    };

    TLockedToken<CSwooshDescription> x1c_desc;
    u32 x28_curFrame = 0;
    int x2c_PSLT = 0;
    double x30_curTime = 0.0;
    zeus::CVector3f x38_translation;
    zeus::CTransform x44_orientation;
    zeus::CTransform x74_invOrientation;
    zeus::CVector3f xa4_globalTranslation;
    zeus::CTransform xb0_globalOrientation;
    zeus::CVector3f xe0_globalScale = {1.f, 1.f, 1.f};
    zeus::CTransform xec_scaleXf;
    zeus::CTransform x11c_invScaleXf;
    zeus::CVector3f x14c_localScale = {1.f, 1.f, 1.f};
    u32 x158_curParticle = 0;
    std::vector<SSwooshData> x15c_swooshes;
    std::vector<zeus::CVector3f> x16c_;
    std::vector<zeus::CVector3f> x17c_;
    std::vector<zeus::CVector3f> x18c_;
    std::vector<zeus::CVector3f> x19c_;
    u32 x1ac_particleCount = 0;
    int x1b0_SPLN = 0;
    int x1b4_LENG = 0;
    int x1b8_SIDE = 0;
    CRandom16 x1c0_rand;
    float x1c4_ = 0.f;
    float x1c8_ = 0.f;
    float x1cc_TSPN;

    union
    {
        struct
        {
            bool x1d0_24_emitting : 1;
            bool x1d0_25_AALP : 1;
            bool x1d0_26_ : 1;
            bool x1d0_27_ : 1;
            bool x1d0_28_LLRD : 1;
            bool x1d0_29_VLS1 : 1;
            bool x1d0_30_VLS2 : 1;
            bool x1d0_31_ : 1;
            bool x1d1_24_ : 1;
        };
        u32 _dummy = 0;
    };

    float x1d4_ = 0.f;
    float x1d8_ = 0.f;
    float x1dc_ = 0.f;
    float x1e0_ = 0.f;
    u32 x1e4_ = 0;
    float x1e8_ = 1.f;
    u32 x1ec_ = 0;
    zeus::CVector3f x1f0_;
    zeus::CVector3f x1fc_;
    float x208_maxRadius = 0.f;
    zeus::CColor x20c_moduColor = zeus::CColor::skWhite;

    static int g_ParticleSystemAliveCount;

    bool IsValid() const { return x1b4_LENG >= 2 && x1b8_SIDE >= 2; }
    void UpdateMaxRadius(float r);
    float GetLeftRadius(int i) const;
    float GetRightRadius(int i) const;
    void UpdateSwooshTranslation(const zeus::CVector3f& translation);
    void UpdateTranslationAndOrientation();

    void RenderNSidedSpline();
    void RenderNSidedNoSpline();
    void Render3SidedSolidSpline();
    void Render3SidedSolidNoSplineNoGaps();
    void Render2SidedSpline();
    void Render2SidedNoSplineGaps();
    void Render2SidedNoSplineNoGaps();

public:
    CParticleSwoosh(const TToken<CSwooshDescription>& desc, int);
    ~CParticleSwoosh();

    bool Update(double);
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
    FourCC Get4CharId() const { return FOURCC('SWHC'); }
};

}

#endif // __URDE_CPARTICLESWOOSH_HPP__
