#ifndef __RETRO_CELEMENTGEN_HPP__
#define __RETRO_CELEMENTGEN_HPP__

#include "../RetroTypes.hpp"
#include "CTransform.hpp"
#include "CVector3f.hpp"
#include "CColor.hpp"
#include "CAABox.hpp"
#include "CToken.hpp"
#include "CLight.hpp"
#include "CRandom16.hpp"

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
        Normal,
        One
    };
    enum class EOptionalSystemFlags
    {
        None,
        One,
        Two
    };
    class CParticleListItem
    {
        friend class CElementGen;
        s16 x0_partIdx;
        Zeus::CVector3f x4_vec;
    public:
        CParticleListItem(s16 idx)
        : x0_partIdx(idx)
        {
            ++g_ParticleAliveCount;
        }
        ~CParticleListItem()
        {
            --g_ParticleAliveCount;
        }
    };
    class CParticle
    {
        friend class CElementGen;
        int x0_endFrame = 0;
        Zeus::CVector3f x4_pos;
        Zeus::CVector3f x10_prevPos;
        Zeus::CVector3f x1c_vel;
        int x28_startFrame = 0;
        float x2c_lineLengthOrSize = 0.f;
        float x30_lineWidthOrRota = 0.f;
        Zeus::CColor x34_color = {0.f, 0.f, 0.f, 1.f};
    };
private:
    TLockedToken<CGenDescription> x1c_genDesc;
    EModelOrientationType x28_orientType;
    std::vector<CParticleListItem> x2c_particleLists;
    std::vector<Zeus::CMatrix3f> x3c_parentMatrices;
    u32 x4c = 0;
    u32 x50_curFrame = 0;
    double x58_curSeconds = 0.f;
    float x60;
    u32 x64 = -1;
    bool x68_particleEmission = true;
    float x6c_generatorRemainder = 0.f;
    int x70_MAXP = 0;
    u16 x74 = 99;
    float x78_generatorRate = 1.f;
    Zeus::CVector3f x7c_translation;
    Zeus::CVector3f x88_globalTranslation;
    Zeus::CVector3f x94_POFS;
    float xa0 = 1.f;
    float xa4 = 1.f;
    float xa8 = 1.f;
    Zeus::CTransform xac = Zeus::CTransform::Identity();
    Zeus::CTransform xdc = Zeus::CTransform::Identity();
    float x10c = 1.f;
    float x110 = 1.f;
    float x114 = 1.f;
    Zeus::CTransform x118 = Zeus::CTransform::Identity();
    Zeus::CTransform x148 = Zeus::CTransform::Identity();
    Zeus::CTransform x178_orientation = Zeus::CTransform::Identity();
    Zeus::CTransform x1a8 = Zeus::CTransform::Identity();
    Zeus::CTransform x1d8_globalOrientation = Zeus::CTransform::Identity();
    u32 x208_activeParticleCount = 0;
    u32 x20c = 0;
    u32 x210_curEmitterFrame = 0;
    int x214_PSLT = 0x7fffff;
    Zeus::CVector3f x218_PSIV;
    bool x224_24 = false;
    bool x224_25_LIT_;
    bool x224_26_AAPH;
    bool x224_27_ZBUF;
    bool x224_28 = true;
    bool x224_29_MBLR;
    bool x224_30_VMD1;
    bool x224_31_VMD2;
    bool x225_24_VMD3;
    bool x225_25_VMD4;
    bool x225_26_LINE;
    bool x225_27_FXLL;
    bool x225_28_warmedUp = false;
    bool x225_29 = false;
    bool x226;
    int x228_MBSP;
    bool x22c = false;
    CRandom16 x230_randState;
    std::vector<std::unique_ptr<CElementGen>> x234_children;
    int x244_CSSD = 0;
    std::vector<std::unique_ptr<CElementGen>> x248_children;
    int x258_SISY = 16;
    int x25c_PISY = 16;
    u32 x264 = 0;
    u32 x268 = 0;
    u32 x26c = 0;
    int x270_SSSD = 0;
    Zeus::CVector3f x274_SSPO;
    u32 x284 = 0;
    u32 x288 = 0;
    u32 x28c = 0;
    int x290_SESD = 0;
    Zeus::CVector3f x294_SEPO;
    float x2a0 = 0.f;
    float x2a4 = 0.f;
    Zeus::CVector3f x2a8_aabbMin;
    Zeus::CVector3f x2b4_aabbMax;
    float x2c0_maxSize = 0.f;
    Zeus::CAABox x2c4 = Zeus::CAABox::skInvertedBox;
    ELightType x2dc_lightType;
    Zeus::CColor x2e0 = Zeus::CColor::skWhite;
    float x2e4 = 1.f;
    float x2e8 = 0.f;
    float x2ec = 0.f;
    float x2f0 = 0.f;
    float x2f4 = 1.f;
    float x2f8 = 0.f;
    float x2fc = 0.f;
    EFalloffType x300_falloffType = EFalloffType::Linear;
    float x304 = 1.f;
    float x308 = 45.f;
    u32 x30c = -1;

    void AccumulateBounds(Zeus::CVector3f& pos, float size);

public:
    CElementGen(const TToken<CGenDescription>& gen, EModelOrientationType orientType, EOptionalSystemFlags flags);
    virtual ~CElementGen();

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

        for (std::unique_ptr<CElementGen>& child : x234_children)
            child->SetGeneratorRateScalar(x78_generatorRate);

        for (std::unique_ptr<CElementGen>& child : x248_children)
            child->SetGeneratorRateScalar(x78_generatorRate);
    }

    static s32 g_FreeIndex;
    static bool g_StaticListInitialized;
    static int g_ParticleAliveCount;
    static int g_ParticleSystemAliveCount;
    static void Initialize();

    void UpdateExistingParticles();
    void CreateNewParticles(int);
    void UpdatePSTranslationAndOrientation();
    void UpdateChildParticleSystems(double);
    CElementGen* ConstructChildParticleSystem(const TToken<CGenDescription>&);
    void UpdateLightParameters();
    void BuildParticleSystemBounds();
    u32 GetSystemCount();
    u32 GetParticleCountAll();
    u32 GetParticleCountAllInternal();

    virtual void Update(double);
    bool InternalUpdate(double);
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
ENABLE_BITWISE_ENUM(CElementGen::EOptionalSystemFlags)

}

#endif // __RETRO_CELEMENTGEN_HPP__
