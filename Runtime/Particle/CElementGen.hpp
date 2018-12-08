#pragma once

#include "RetroTypes.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CAABox.hpp"
#include "CToken.hpp"
#include "Graphics/CLight.hpp"
#include "Graphics/CGraphics.hpp"
#include "CRandom16.hpp"
#include "CParticleGen.hpp"
#include "Graphics/Shaders/CElementGenShaders.hpp"
#include "Graphics/CLineRenderer.hpp"
#include "Particle/CGenDescription.hpp"

namespace urde
{
class CWarp;
class CLight;
class IGenDescription;
class CGenDescription;
class CParticleSwoosh;
class CParticleElectric;
class CActorLights;

class CElementGen : public CParticleGen
{
    static u16 g_GlobalSeed;
    static bool g_subtractBlend;
public:
    static void SetGlobalSeed(u16 seed) { g_GlobalSeed = seed; }
    static void SetSubtractBlend(bool s) { g_subtractBlend = s; }
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
    enum class LightType
    {
        None        = 0,
        Custom      = 1,
        Directional = 2,
        Spot        = 3
    };
    class CParticleListItem
    {
        friend class CElementGen;
        s16 x0_partIdx;
        zeus::CVector3f x4_viewPoint;
    public:
        CParticleListItem(s16 idx)
        : x0_partIdx(idx) {}
    };
    static CParticle* g_currentParticle;
private:
    friend class CElementGenShaders;
    TLockedToken<CGenDescription> x1c_genDesc;
    CGenDescription* x28_loadedGenDesc;
    EModelOrientationType x2c_orientType;
    std::vector<CParticle> x30_particles;
    std::vector<u32> x40;
    std::vector<zeus::CMatrix3f> x50_parentMatrices;
    std::vector<std::array<float, 8>> x60_advValues;

    int x70_internalStartFrame = 0;
    int x74_curFrame = 0;
    double x78_curSeconds = 0.f;
    float x80_timeDeltaScale;
    int x84_prevFrame = -1;
    bool x88_particleEmission = true;
    float x8c_generatorRemainder = 0.f;
    int x90_MAXP = 0;
    int m_maxMAXP = 256;
    u16 x94_randomSeed = 99;
    float x98_generatorRate = 1.f;
    float x9c_cextValues[16] = {};

    zeus::CVector3f xdc_translation;
    zeus::CVector3f xe8_globalTranslation;
    zeus::CVector3f xf4_POFS;
    zeus::CVector3f x100_globalScale = {1.f, 1.f, 1.f};
    zeus::CTransform x10c_globalScaleTransform = zeus::CTransform::Identity();
    zeus::CTransform x13c_globalScaleTransformInverse = zeus::CTransform::Identity();
    zeus::CVector3f x16c_localScale = {1.f, 1.f, 1.f};
    zeus::CTransform x178_localScaleTransform = zeus::CTransform::Identity();
    zeus::CTransform x1a8_localScaleTransformInverse = zeus::CTransform::Identity();
    zeus::CTransform x1d8_orientation = zeus::CTransform::Identity();
    zeus::CTransform x208_orientationInverse = zeus::CTransform::Identity();
    zeus::CTransform x22c_globalOrientation = zeus::CTransform::Identity();

    u32 x25c_activeParticleCount = 0;
    u32 x260_cumulativeParticles = 0;
    u32 x264_recursiveParticleCount = 0;
    int x268_PSLT;

    union
    {
        struct
        {
            bool x26c_24_translationDirty : 1;
            bool x26c_25_LIT_ : 1;
            bool x26c_26_AAPH : 1;
            bool x26c_27_ZBUF : 1;
            bool x26c_28_zTest : 1;
            bool x26c_29_ORNT : 1;
            bool x26c_30_MBLR : 1;
            bool x26c_31_LINE : 1;
            bool x26d_24_FXLL : 1;
            bool x26d_25_warmedUp : 1;
            bool x26d_26_modelsUseLights : 1;
            bool x26d_27_enableOPTS : 1;
            bool x26d_28_enableADV : 1;
        };
        u32 _dummy = 0;
    };

    int x270_MBSP = 0;
    int m_maxMBSP = 0;
    ERglLightBits x274_backupLightActive = ERglLightBits::None;
    bool x278_hasVMD[4] = {};
    CRandom16 x27c_randState;
    CModVectorElement* x280_VELSources[4] = {};

    std::vector<std::unique_ptr<CParticleGen>> x290_activePartChildren;
    int x2a0_CSSD = 0;
    int x2a4_SISY = 16;
    int x2a8_PISY = 16;
    int x2ac_SSSD = 0;
    zeus::CVector3f x2b0_SSPO;
    int x2bc_SESD = 0;
    zeus::CVector3f x2c0_SEPO;
    float x2cc = 0.f;
    float x2d0 = 0.f;
    zeus::CVector3f x2d4_aabbMin;
    zeus::CVector3f x2e0_aabbMax;
    float x2ec_maxSize = 0.f;
    zeus::CAABox x2f0_systemBounds = zeus::CAABox::skInvertedBox;
    LightType x308_lightType;
    zeus::CColor x30c_LCLR = zeus::CColor::skWhite;
    float x310_LINT = 1.f;
    zeus::CVector3f x314_LOFF;
    zeus::CVector3f x320_LDIR = {1.f, 0.f, 0.f};
    EFalloffType x32c_falloffType = EFalloffType::Linear;
    float x330_LFOR = 1.f;
    float x334_LSLA = 45.f;
    zeus::CColor x338_moduColor = {1.f, 1.f, 1.f, 1.f};

    std::unique_ptr<CLineRenderer> m_lineRenderer;
    CElementGenShaders::EShaderClass m_shaderClass;

    void AccumulateBounds(const zeus::CVector3f& pos, float size);

public:
    CElementGen(const TToken<CGenDescription>& gen,
                EModelOrientationType orientType = EModelOrientationType::Normal,
                EOptionalSystemFlags flags = EOptionalSystemFlags::One);
    ~CElementGen();

    boo::ObjToken<boo::IShaderDataBinding> m_normalDataBind;
    boo::ObjToken<boo::IShaderDataBinding> m_normalSubDataBind;
    boo::ObjToken<boo::IShaderDataBinding> m_redToAlphaDataBind;
    boo::ObjToken<boo::IShaderDataBinding> m_redToAlphaSubDataBind;
    boo::ObjToken<boo::IGraphicsBufferD> m_instBuf;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniformBuf;

    boo::ObjToken<boo::IShaderDataBinding> m_normalDataBindPmus;
    boo::ObjToken<boo::IShaderDataBinding> m_redToAlphaDataBindPmus;
    boo::ObjToken<boo::IGraphicsBufferD> m_instBufPmus;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniformBufPmus;

    CGenDescription* GetDesc() {return x1c_genDesc.GetObj();}
    const SObjectTag* GetDescTag() const {return x1c_genDesc.GetObjectTag();}

    static bool g_ParticleSystemInitialized;
    static int g_ParticleAliveCount;
    static int g_ParticleSystemAliveCount;
    static bool sMoveRedToAlphaBuffer;
    static void Initialize();
    static void Shutdown();

    void UpdateAdvanceAccessParameters(u32 activeParticleCount, u32 particleFrame);
    bool UpdateVelocitySource(u32 idx, u32 particleFrame, CParticle& particle);
    void UpdateExistingParticles();
    void CreateNewParticles(int);
    void UpdatePSTranslationAndOrientation();
    void UpdateChildParticleSystems(double);
    std::unique_ptr<CParticleGen> ConstructChildParticleSystem(const TToken<CGenDescription>&);
    void UpdateLightParameters();
    void BuildParticleSystemBounds();
    u32 GetEmitterTime() const { return x74_curFrame; }
    u32 GetSystemCount();
    u32 GetCumulativeParticleCount() const { return x260_cumulativeParticles; }
    u32 GetParticleCountAllInternal() const;
    u32 GetParticleCountAll() const { return x264_recursiveParticleCount; }
    void EndLifetime();
    void ForceParticleCreation(int amount);
    float GetCEXTValue(int i) const { return x9c_cextValues[i]; }
    void SetCEXTValue(int i, float v) { x9c_cextValues[i] = v; }

    bool InternalUpdate(double);
    void RenderModels(const CActorLights* actLights);
    void RenderLines();
    void RenderParticles();
    void RenderParticlesIndirectTexture();

    bool Update(double);
    void Render(const CActorLights* = nullptr);
    void SetOrientation(const zeus::CTransform&);
    void SetTranslation(const zeus::CVector3f&);
    void SetGlobalOrientation(const zeus::CTransform&);
    void SetGlobalTranslation(const zeus::CVector3f&);
    void SetGlobalScale(const zeus::CVector3f&);
    void SetLocalScale(const zeus::CVector3f&);
    void SetGlobalOrientAndTrans(const zeus::CTransform& xf);
    void SetParticleEmission(bool);
    void SetModulationColor(const zeus::CColor&);
    void SetGeneratorRate(float rate);
    const zeus::CTransform& GetOrientation() const;
    const zeus::CVector3f& GetTranslation() const;
    const zeus::CTransform& GetGlobalOrientation() const;
    const zeus::CVector3f& GetGlobalTranslation() const;
    const zeus::CVector3f& GetGlobalScale() const;
    const zeus::CColor& GetModulationColor() const;
    float GetGeneratorRate() const { return x98_generatorRate; }
    bool IsSystemDeletable() const;
    std::experimental::optional<zeus::CAABox> GetBounds() const;
    u32 GetParticleCount() const;
    bool SystemHasLight() const;
    CLight GetLight() const;
    bool GetParticleEmission() const;
    void DestroyParticles();
    void Reset();
    FourCC Get4CharId() const { return FOURCC('PART'); }
    size_t GetNumActiveChildParticles() const { return x290_activePartChildren.size(); }
    CParticleGen& GetActiveChildParticle(size_t idx) const { return *x290_activePartChildren[idx]; }
    bool IsIndirectTextured() const { return x28_loadedGenDesc->x54_x40_TEXR && x28_loadedGenDesc->x58_x44_TIND; }
    void SetModelsUseLights(bool v) { x26d_26_modelsUseLights = v; }
    static void SetMoveRedToAlphaBuffer(bool);

    s32 GetMaxParticles() const { return x90_MAXP; }
};
ENABLE_BITWISE_ENUM(CElementGen::EOptionalSystemFlags)

}

