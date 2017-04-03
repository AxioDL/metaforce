#ifndef __URDE_CSAMUSHUD_HPP__
#define __URDE_CSAMUSHUD_HPP__

#include "CInGameGuiManager.hpp"
#include "GuiSys/CTargetingManager.hpp"
#include "GuiSys/CHudBallInterface.hpp"
#include "GuiSys/CHudBossEnergyInterface.hpp"
#include "GuiSys/CHudDecoInterface.hpp"
#include "GuiSys/CHudEnergyInterface.hpp"
#include "GuiSys/CHudFreeLookInterface.hpp"
#include "GuiSys/CHudHelmetInterface.hpp"
#include "GuiSys/CHudMissileInterface.hpp"
#include "GuiSys/CHudRadarInterface.hpp"
#include "GuiSys/CHudThreatInterface.hpp"
#include "GuiSys/CHudVisorBeamMenu.hpp"

namespace urde
{
class CGuiFrame;
class CStateManager;
class CGuiLight;

struct CHUDMemoParms
{
    float x0_ = 0.f;
    bool x4_ = false;
    bool x5_ = false;
    bool x6_ = false;
    CHUDMemoParms() = default;
    CHUDMemoParms(float f1, bool b1, bool b2, bool b3)
    : x0_(f1), x4_(b1), x5_(b2), x6_(b3) {}
    CHUDMemoParms(CInputStream& in)
    {
        x0_ = in.readFloatBig();
        x4_ = in.readBool();
    }
};

enum class EHudState
{
    Combat,
    XRay,
    Thermal,
    Scan,
    Ball,
    None
};

namespace MP1
{

class CSamusHud
{
    enum class ELoadPhase
    {
        Zero,
        One,
        Two,
        Three
    };

    enum class ETransitionState
    {
        NotTransitioning,
        Countdown,
        Loading,
        Transitioning
    };

    struct SCachedHudLight
    {
        zeus::CVector3f x0_pos;
        zeus::CColor xc_color;
        float x10_ = 0.f;
        float x14_ = 0.f;
        float x18_ = 0.f;
        float x1c_ = 0.f;
        SCachedHudLight(const zeus::CVector3f& pos, const zeus::CColor& color,
                        float f1, float f2, float f3, float f4)
        : x0_pos(pos), xc_color(color), x10_(f1), x14_(f2), x18_(f3), x1c_(f4) {}
    };

    struct SVideoBand
    {
        CGuiModel* x0_videoband = nullptr;
        float x4_randA = 0.f;
        float x8_randB = 0.f;
    };

    struct SProfileInfo
    {
        u32 x0_ = 0;
        u32 x4_ = 0;
        u32 x8_ = 0;
        u32 xc_ = 0;
    };

    friend class CInGameGuiManager;
    ELoadPhase x4_loadPhase = ELoadPhase::Zero;
    CTargetingManager x8_targetingMgr;
    TLockedToken<CGuiFrame> x258_frmeHelmet;
    CGuiFrame* x264_loadedFrmeHelmet = nullptr;
    TLockedToken<CGuiFrame> x268_frmeBaseHud;
    CGuiFrame* x274_loadedFrmeBaseHud = nullptr;
    TLockedToken<CGuiFrame> x278_selectedHud; // used to be optional
    CGuiFrame* x288_loadedSelectedHud = nullptr;
    std::unique_ptr<CHudEnergyInterfaceBall> x28c_ballEnergyIntf;
    std::unique_ptr<IHudThreatInterface> x290_threatIntf;
    std::unique_ptr<CHudMissileInterfaceCombat> x294_missileIntf;
    std::unique_ptr<IFreeLookInterface> x298_freeLookIntf;
    std::unique_ptr<IHudDecoInterface> x29c_decoIntf;
    std::unique_ptr<CHudHelmetInterface> x2a0_helmetIntf;
    std::unique_ptr<CHudVisorBeamMenu> x2a4_visorMenu;
    std::unique_ptr<CHudVisorBeamMenu> x2a8_beamMenu;
    std::unique_ptr<CHudRadarInterface> x2ac_radarIntf;
    std::unique_ptr<CHudBallInterface> x2b0_ballIntf;
    std::unique_ptr<CHudBossEnergyInterface> x2b4_bossEnergyIntf;
    EHudState x2b8_curState = EHudState::None;
    EHudState x2bc_nextState = EHudState::None;
    EHudState x2c0_setState = EHudState::None;
    ETransitionState x2c4_activeTransState = ETransitionState::NotTransitioning;
    float x2c8_transT = 1.f;
    u32 x2cc_preLoadCountdown = 0;
    float x2d0_ = 0.f;
    u32 x2d4_totalEnergyTanks = 0;
    u32 x2d8_missileAmount = 0;
    u32 x2dc_missileCapacity = 0;
    rstl::reserved_vector<SCachedHudLight, 3> x340_lights;

    union
    {
        struct
        {
            bool x2e0_24_ : 1;
            bool x2e0_25_ : 1;
            bool x2e0_26_latestFirstPerson : 1;
            bool x2e0_27_energyLow : 1;
        };
        u16 _dummy = 0;
    };

    u32 x2e4_ = 0;
    u32 x2e8_ = 0;
    u32 x2ec_missilesActive = 0;
    float x2f0_visorBeamMenuAlpha = 1.f;
    zeus::CVector3f x2f8_fpCamDir;
    zeus::CVector3f x304_basewidgetIdlePos;
    zeus::CVector3f x310_cameraPos;
    zeus::CQuaternion x31c_;
    zeus::CQuaternion x32c_;
    std::unique_ptr<CActorLights> x33c_lights;
    rstl::reserved_vector<SCachedHudLight, 3> x340_hudLights;
    CSfxHandle x3a4_damageSfx;
    CCameraFilterPass x3a8_camFilter;
    CGuiWidget* x3d4_selected_basewidget_pivot = nullptr;
    std::vector<zeus::CTransform> x3d8_lightTransforms;
    float x3e8_ = 0.f;
    float x3ec_ = 0.f;
    float x3f0_ = 1.f;
    float x3f4_ = 0.f;
    float x3f8_ = 0.f;
    float x3fc_ = 0.f;
    float x400_ = 0.f;
    float x404_ = 0.f;
    zeus::CVector3f x408_;
    float x414_ = 0.f;
    float x418_ = 0.f;
    zeus::CVector3f x41c_;
    zeus::CMatrix3f x428_;
    zeus::CQuaternion x44c_;
    float x45c_ = 0.f;
    float x460_ = 0.f;
    float x464_ = 0.f;
    rstl::reserved_vector<zeus::CTransform, 3> x46c_;
    zeus::CVector2f x500_viewportScale = {1.f, 1.f};
    u32 x508_ = 0;
    u32 x50c_ = 0;
    float x510_ = 0.f;
    float x514_ = 0.f;
    float x518_ = 0.f;
    CCameraFilterPass x51c_camFilter2;
    CHUDMemoParms x548_hudMemoParms;
    TLockedToken<CStringTable> x550_hudMemoString;
    u32 x554_hudMemoIdx = 0;
    float x558_messageTextAlpha = 0.f;
    float x55c_lastSfxChars = 0.f;
    float x560_messageTextScale = 0.f;
    u32 x564_ = 0;
    zeus::CVector3f x568_fpCamDir;
    float x574_ = 1.f;
    float x578_ = 0.f;
    float x57c_energyLowTimer = 0.f;
    float x580_energyLowPulse = 0.f;
    float x584_abuttonPulse = 0.f;
    CGuiWidget* x588_base_basewidget_pivot;
    CGuiWidget* x58c_helmet_BaseWidget_Pivot;
    CGuiModel* x590_base_Model_AutoMapper;
    CGuiTextPane* x594_base_textpane_counter;
    CGuiWidget* x598_base_basewidget_message;
    CGuiTextPane* x59c_base_textpane_message;
    CGuiModel* x5a0_base_model_abutton;
    rstl::reserved_vector<SVideoBand, 4> x5a4_videoBands;
    rstl::reserved_vector<CGuiLight*, 4> x5d8_guiLights;
    float x5ec_camFovTweaks[16];
    float x62c_camYTweaks[64];
    float x72c_camZTweaks[32];
    rstl::reserved_vector<SProfileInfo, 15> x7ac_;

    static CSamusHud* g_SamusHud;
    static rstl::prereserved_vector<bool, 4> BuildPlayerHasVisors(const CStateManager& mgr);
    static rstl::prereserved_vector<bool, 4> BuildPlayerHasBeams(const CStateManager& mgr);
    void InitializeFrameGluePermanent(const CStateManager& mgr);
    void InitializeFrameGlueMutable(const CStateManager& mgr);
    void UninitializeFrameGlueMutable();
    void UpdateEnergy(float dt, const CStateManager& mgr, bool init);
    void UpdateFreeLook(float dt, const CStateManager& mgr);
    void UpdateMissile(float dt, const CStateManager& mgr, bool init);
    void UpdateVideoBands(float dt, const CStateManager& mgr);
    void UpdateBallMode(const CStateManager& mgr, bool init);
    void UpdateThreatAssessment(float dt, const CStateManager& mgr);
    void UpdateVisorAndBeamMenus(float dt, const CStateManager& mgr);
    void UpdateCameraDebugSettings();
    void UpdateEnergyLow(float dt, const CStateManager& mgr);
    void UpdateHudLag(float dt, const CStateManager& mgr);
    void UpdateHudDynamicLights(float dt, const CStateManager& mgr);
    void UpdateHudDamage(float dt, const CStateManager& mgr,
                         DataSpec::ITweakGui::EHelmetVisMode helmetVis);
    void UpdateStaticInterference(float dt, const CStateManager& mgr);
    void ShowDamage(const zeus::CVector3f& position, float dam, float prevDam, const CStateManager& mgr);
    void EnterFirstPerson(const CStateManager& mgr);
    void LeaveFirstPerson(const CStateManager& mgr);
    static EHudState GetDesiredHudState(const CStateManager& mgr);

public:
    CSamusHud(CStateManager& stateMgr);
    void Update(float dt, const CStateManager& mgr,
                DataSpec::ITweakGui::EHelmetVisMode helmetVis,
                bool hudVis, bool targetingManager);
    void UpdateStateTransition(float time, const CStateManager& mgr);
    bool CheckLoadComplete(CStateManager& stateMgr);
    void OnNewInGameGuiState(EInGameGuiState state, CStateManager& stateMgr);
    void RefreshHudOptions();
    void Touch();
    static zeus::CTransform BuildFinalCameraTransform(const zeus::CQuaternion& rot,
                                                      const zeus::CVector3f& pos,
                                                      const zeus::CVector3f& camPos);
    static void DisplayHudMemo(const std::u16string& text, const CHUDMemoParms& info)
    {
        if (g_SamusHud)
            g_SamusHud->InternalDisplayHudMemo(text, info);
    }
    void InternalDisplayHudMemo(const std::u16string& text, const CHUDMemoParms& info)
    {
        SetMessage(text, info);
    }
    void SetMessage(const std::u16string& text, const CHUDMemoParms& info);
    static void DeferHintMemo(ResId strg, u32 timePeriods, const CHUDMemoParms& info)
    {
        if (g_SamusHud)
            g_SamusHud->InternalDeferHintMemo(strg, timePeriods, info);
    }
    void InternalDeferHintMemo(ResId strg, u32 timePeriods, const CHUDMemoParms& info);
};

}
}

#endif // __URDE_CSAMUSHUD_HPP__
