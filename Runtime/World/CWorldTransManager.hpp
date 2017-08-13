#ifndef __URDE_CWORLDTRANSMANAGER_HPP__
#define __URDE_CWORLDTRANSMANAGER_HPP__

#include "RetroTypes.hpp"
#include "CRandom16.hpp"
#include "Character/CModelData.hpp"
#include "GuiSys/CGuiTextSupport.hpp"
#include "Graphics/CLight.hpp"
#include "Graphics/Shaders/CColoredQuadFilter.hpp"
#include "Graphics/Shaders/CTexturedQuadFilter.hpp"
#include "Graphics/Shaders/CCameraBlurFilter.hpp"
#include "Audio/CSfxManager.hpp"

namespace urde
{
class CSimplePool;
class CStringTable;

class CWorldTransManager
{
public:
    enum class ETransType
    {
        Disabled,
        Enabled,
        Text
    };

    struct SModelDatas
    {
        CAnimRes x0_samusRes;
        CModelData x1c_samusModelData;
        CModelData x68_beamModelData;
        CModelData xb4_platformModelData;
        CModelData x100_bgModelData[3];
        TLockedToken<CModel> x14c_beamModel;
        TLockedToken<CModel> x158_suitModel;
        TLockedToken<CSkinRules> x164_suitSkin;
        zeus::CTransform x170_gunXf;
        std::vector<CLight> x1a0_lights;
        //std::unique_ptr<u8> x1b0_dissolveTextureBuffer;
        zeus::CVector2f x1b4_shakeResult;
        zeus::CVector2f x1bc_shakeDelta;
        float x1c4_randTimeout = 0.f;
        float x1c8_blurResult = 0.f;
        float x1cc_blurDelta = 0.f;
        float x1d0_dissolveStartTime = 99999.f;
        float x1d4_dissolveEndTime = 99999.f;
        float x1d8_transCompleteTime = 99999.f;
        bool x1dc_dissolveStarted = false;

        SModelDatas(const CAnimRes& samusRes);
    };

private:
    float x0_curTime = 0.f;
    std::unique_ptr<SModelDatas> x4_modelData;
    std::unique_ptr<CGuiTextSupport> x8_textData;
    TLockedToken<CStringTable> xc_strTable;
    u8 x14_ = 0;
    float x18_bgOffset;
    float x1c_bgHeight;
    CRandom16 x20_random = CRandom16(99);
    u16 x24_sfx = 1189;
    CSfxHandle x28_sfxHandle;
    u8 x2c_volume = 127;
    u8 x2d_panning = 64;
    ETransType x30_type = ETransType::Disabled;
    float x34_stopTime;
    float x38_textStartTime = 0.f;
    float x3c_sfxInterval;
    bool x40_strIdx;
    union
    {
        struct
        {
            bool x44_24_transFinished : 1;
            bool x44_25_stopSoon : 1;
            bool x44_26_goingUp : 1;
            bool x44_27_fadeWhite : 1;
            bool x44_28_textDirty : 1;
        };
        u8 dummy = 0;
    };

    CColoredQuadFilter m_fadeToBlack = { EFilterType::Blend };
    CTexturedQuadFilter m_dissolve = { EFilterType::Blend,
                                       CGraphics::g_SpareTexture };
    CWideScreenFilter m_widescreen = { EFilterType::Blend };
    CCameraBlurFilter m_camblur;

    static int GetSuitCharIdx();
    void DrawFirstPass();
    void DrawSecondPass();
    void DrawAllModels();
    void UpdateLights(float dt);
    void UpdateEnabled(float);
    void UpdateDisabled(float);
    void UpdateText(float);
    void DrawEnabled();
    void DrawDisabled();
    void DrawText();

public:
    CWorldTransManager() { x44_24_transFinished = true; }

    void Update(float);
    void Draw();

    void EnableTransition(const CAnimRes& samusRes,
                          CAssetId platRes, const zeus::CVector3f& platScale,
                          CAssetId bgRes, const zeus::CVector3f& bgScale, bool goingUp);
    void EnableTransition(CAssetId fontId, CAssetId stringId, u32 strIdx, bool fadeWhite,
                          float chFadeTime, float chFadeRate, float textStartTime);

    void StartTransition();
    void EndTransition();
    bool IsTransitionFinished() const { return x44_24_transFinished; }
    void PleaseStopSoon() { x44_25_stopSoon = true; }
    void StartTextFadeOut();
    bool IsTransitionEnabled() const { return x30_type != ETransType::Disabled; }
    void DisableTransition();
    void TouchModels();
    ETransType GetTransType() { return x30_type; }
    void SetSfx(u16 sfx, u8 volume, u8 panning)
    {
        x24_sfx = sfx;
        x2c_volume = volume;
        x2d_panning = panning;
    }
    void SfxStart();
    void SfxStop();

    static bool WaitForModelsAndTextures();
};

}

#endif // __URDE_CWORLDTRANSMANAGER_HPP__
