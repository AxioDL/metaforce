#ifndef __PSHAG_CELEMENTGEN_HPP__
#define __PSHAG_CELEMENTGEN_HPP__

#include "RetroTypes.hpp"
#include "CTransform.hpp"
#include "CVector3f.hpp"
#include "CColor.hpp"
#include "CAABox.hpp"
#include "CToken.hpp"
#include "CLight.hpp"
#include "CGraphics.hpp"
#include "CRandom16.hpp"
#include "CParticleGen.hpp"
#include "CElementGenShaders.hpp"

namespace pshag
{
class CWarp;
class CLight;
class IGenDescription;
class CGenDescription;
class CParticleSwoosh;
class CParticleElectric;

class CElementGen : public CParticleGen
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
        Zeus::CVector3f x4_viewPoint;
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
    friend class CElementGenShaders;
    TLockedToken<CGenDescription> x1c_genDesc;
    EModelOrientationType x28_orientType;
    std::vector<CParticleListItem> x2c_particleLists;
    std::vector<Zeus::CMatrix3f> x3c_parentMatrices;
    u32 x4c_internalStartFrame = 0;
    u32 x50_curFrame = 0;
    double x58_curSeconds = 0.f;
    float x60_timeDeltaScale;
    u32 x64_prevFrame = -1;
    bool x68_particleEmission = true;
    float x6c_generatorRemainder = 0.f;
    int x70_MAXP = 0;
    u16 x74_randomSeed = 99;
    float x78_generatorRate = 1.f;
    Zeus::CVector3f x7c_translation;
    Zeus::CVector3f x88_globalTranslation;
    Zeus::CVector3f x94_POFS;
    Zeus::CVector3f xa0_globalScale = {1.f, 1.f, 1.f};
    Zeus::CTransform xac_globalScaleTransform = Zeus::CTransform::Identity();
    Zeus::CTransform xdc_globalScaleTransformInverse = Zeus::CTransform::Identity();
    Zeus::CVector3f x10c_localScale = {1.f, 1.f, 1.f};
    Zeus::CTransform x118_localScaleTransform = Zeus::CTransform::Identity();
    Zeus::CTransform x148_localScaleTransformInverse = Zeus::CTransform::Identity();
    Zeus::CTransform x178_orientation = Zeus::CTransform::Identity();
    Zeus::CTransform x1a8_orientationInverse = Zeus::CTransform::Identity();
    Zeus::CTransform x1d8_globalOrientation = Zeus::CTransform::Identity();
    u32 x208_activeParticleCount = 0;
    u32 x20c_recursiveParticleCount = 0;
    u32 x210_curEmitterFrame = 0;
    int x214_PSLT = 0x7fffff;
    Zeus::CVector3f x218_PSIV;
    bool x224_24_translationDirty = false;
    bool x224_25_LIT_;
    bool x224_26_AAPH;
    bool x224_27_ZBUF;
    bool x224_28_zTest = false;
    bool x224_29_MBLR;
    bool x224_30_VMD1;
    bool x224_31_VMD2;
    bool x225_24_VMD3;
    bool x225_25_VMD4;
    bool x225_26_LINE;
    bool x225_27_FXLL;
    bool x225_28_warmedUp = false;
    bool x225_29_modelsUseLights = false;
    bool x226_enableOPTS;
    int x228_MBSP = 0; int m_maxMBSP = 0;
    ERglLight x22c_backupLightActive = ERglLight::None;
    CRandom16 x230_randState;
    std::vector<std::unique_ptr<CElementGen>> x234_activePartChildren;
    int x244_CSSD = 0;
    std::vector<std::unique_ptr<CElementGen>> x248_finishPartChildren;
    int x258_SISY = 16;
    int x25c_PISY = 16;
    std::vector<std::unique_ptr<CParticleSwoosh>> x260_swhcChildren;
    int x270_SSSD = 0;
    Zeus::CVector3f x274_SSPO;
    std::vector<std::unique_ptr<CParticleElectric>> x280_elscChildren;
    int x290_SESD = 0;
    Zeus::CVector3f x294_SEPO;
    float x2a0 = 0.f;
    float x2a4 = 0.f;
    Zeus::CVector3f x2a8_aabbMin;
    Zeus::CVector3f x2b4_aabbMax;
    float x2c0_maxSize = 0.f;
    Zeus::CAABox x2c4_systemBounds = Zeus::CAABox::skInvertedBox;
    LightType x2dc_lightType;
    Zeus::CColor x2e0_LCLR = Zeus::CColor::skWhite;
    float x2e4_LINT = 1.f;
    Zeus::CVector3f x2e8_LOFF;
    Zeus::CVector3f x2f4_LDIR = {1.f, 0.f, 0.f};
    EFalloffType x300_falloffType = EFalloffType::Linear;
    float x304_LFOR = 1.f;
    float x308_LSLA = 45.f;
    Zeus::CColor x30c_moduColor = {1.f, 1.f, 1.f, 1.f};

    CElementGenShaders::EShaderClass m_shaderClass;

    void AccumulateBounds(Zeus::CVector3f& pos, float size);

public:
    CElementGen(const TToken<CGenDescription>& gen, EModelOrientationType orientType, EOptionalSystemFlags flags);
    ~CElementGen();

    boo::GraphicsDataToken m_gfxToken;
    boo::IShaderDataBinding* m_normalDataBind = nullptr;
    boo::IShaderDataBinding* m_redToAlphaDataBind = nullptr;
    boo::IGraphicsBufferD* m_instBuf = nullptr;
    boo::IGraphicsBufferD* m_uniformBuf = nullptr;

    CGenDescription* GetDesc() {return x1c_genDesc.GetObj();}

    static s32 g_FreeIndex;
    static bool g_StaticListInitialized;
    static int g_ParticleAliveCount;
    static int g_ParticleSystemAliveCount;
    static bool g_MoveRedToAlphaBuffer;
    static void Initialize();
    static void Shutdown();

    void SetGeneratorRateScalar(float scalar)
    {
        if (scalar >= 0.0f)
            x78_generatorRate = scalar;
        else
            x78_generatorRate = 0.0f;

        for (std::unique_ptr<CElementGen>& child : x234_activePartChildren)
            child->SetGeneratorRateScalar(x78_generatorRate);

        for (std::unique_ptr<CElementGen>& child : x248_finishPartChildren)
            child->SetGeneratorRateScalar(x78_generatorRate);
    }

    void UpdateExistingParticles();
    void CreateNewParticles(int);
    void UpdatePSTranslationAndOrientation();
    void UpdateChildParticleSystems(double);
    CElementGen* ConstructChildParticleSystem(const TToken<CGenDescription>&);
    void UpdateLightParameters();
    void BuildParticleSystemBounds();
    u32 GetSystemCount();
    u32 GetParticleCountAllInternal() const;
    u32 GetParticleCountAll() const {return x20c_recursiveParticleCount;}

    bool InternalUpdate(double);
    void RenderModels();
    void RenderLines();
    void RenderParticles();
    void RenderParticlesIndirectTexture();

    void Update(double);
    void Render();
    void SetOrientation(const Zeus::CTransform&);
    void SetTranslation(const Zeus::CVector3f&);
    void SetGlobalOrientation(const Zeus::CTransform&);
    void SetGlobalTranslation(const Zeus::CVector3f&);
    void SetGlobalScale(const Zeus::CVector3f&);
    void SetLocalScale(const Zeus::CVector3f&);
    void SetParticleEmission(bool);
    void SetModulationColor(const Zeus::CColor&);
    const Zeus::CTransform& GetOrientation() const;
    const Zeus::CVector3f& GetTranslation() const;
    const Zeus::CTransform& GetGlobalOrientation() const;
    const Zeus::CVector3f& GetGlobalTranslation() const;
    const Zeus::CVector3f& GetGlobalScale() const;
    const Zeus::CColor& GetModulationColor() const;
    bool IsSystemDeletable() const;
    std::pair<Zeus::CAABox, bool> GetBounds() const;
    u32 GetParticleCount() const;
    bool SystemHasLight() const;
    CLight GetLight() const;
    bool GetParticleEmission() const;
    void DestroyParticles();
};
ENABLE_BITWISE_ENUM(CElementGen::EOptionalSystemFlags)

}

#endif // __PSHAG_CELEMENTGEN_HPP__
