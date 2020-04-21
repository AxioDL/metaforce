#pragma once

#include <array>
#include <memory>
#include <string_view>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/Shaders/CRandomStaticFilter.hpp"
#include "Runtime/GuiSys/CHudBallInterface.hpp"
#include "Runtime/GuiSys/CHudBossEnergyInterface.hpp"
#include "Runtime/GuiSys/CHudDecoInterface.hpp"
#include "Runtime/GuiSys/CHudEnergyInterface.hpp"
#include "Runtime/GuiSys/CHudFreeLookInterface.hpp"
#include "Runtime/GuiSys/CHudHelmetInterface.hpp"
#include "Runtime/GuiSys/CHudMissileInterface.hpp"
#include "Runtime/GuiSys/CHudRadarInterface.hpp"
#include "Runtime/GuiSys/CHudThreatInterface.hpp"
#include "Runtime/GuiSys/CHudVisorBeamMenu.hpp"
#include "Runtime/GuiSys/CTargetingManager.hpp"
#include "Runtime/MP1/CInGameGuiManager.hpp"
#include "Runtime/World/CHUDMemoParms.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CMatrix3f.hpp>
#include <zeus/CQuaternion.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CGuiFrame;
class CGuiLight;
class CStateManager;

enum class EHudState { Combat, XRay, Thermal, Scan, Ball, None };

namespace MP1 {

class CSamusHud {
  enum class ELoadPhase { Zero, One, Two, Three };

  enum class ETransitionState { NotTransitioning, Countdown, Loading, Transitioning };

  struct SCachedHudLight {
    zeus::CVector3f x0_pos;
    zeus::CColor xc_color;
    float x10_distC = 0.f;
    float x14_distL = 0.f;
    float x18_distQ = 0.f;
    float x1c_fader = 0.f;
    SCachedHudLight(const zeus::CVector3f& pos, const zeus::CColor& color, float f1, float f2, float f3, float f4)
    : x0_pos(pos), xc_color(color), x10_distC(f1), x14_distL(f2), x18_distQ(f3), x1c_fader(f4) {}
  };

  struct SVideoBand {
    CGuiModel* x0_videoband = nullptr;
    float x4_randA = 0.f;
    float x8_randB = 0.f;
  };

  struct SProfileInfo {
    u32 x0_ = 0;
    u32 x4_ = 0;
    u64 x8_profUsec = 0;
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
  std::unique_ptr<CHudEnergyInterface> x28c_energyIntf;
  std::unique_ptr<CHudThreatInterface> x290_threatIntf;
  std::unique_ptr<CHudMissileInterface> x294_missileIntf;
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
  float x2d0_playerHealth = 0.f;
  u32 x2d4_totalEnergyTanks = 0;
  u32 x2d8_missileAmount = 0;
  u32 x2dc_missileCapacity = 0;
  bool x2e0_24_inFreeLook : 1 = false;
  bool x2e0_25_lookControlHeld : 1 = false;
  bool x2e0_26_latestFirstPerson : 1 = true;
  bool x2e0_27_energyLow : 1;
  u32 x2e4_ = 0;
  u32 x2e8_ = 0;
  CPlayerGun::EMissileMode x2ec_missileMode = CPlayerGun::EMissileMode::Inactive;
  float x2f0_visorBeamMenuAlpha = 1.f;
  zeus::CVector3f x2f8_fpCamDir;
  zeus::CVector3f x304_basewidgetIdlePos;
  zeus::CVector3f x310_cameraPos;
  zeus::CQuaternion x31c_hudLag;
  zeus::CQuaternion x32c_invHudLag;
  std::unique_ptr<CActorLights> x33c_lights;
  rstl::reserved_vector<SCachedHudLight, 3> x340_hudLights;
  CSfxHandle x3a4_damageSfx;
  CCameraFilterPass<CColoredQuadFilter> x3a8_camFilter;
  CGuiLight* x3d4_damageLight = nullptr;
  std::vector<zeus::CTransform> x3d8_lightTransforms;
  float x3e8_damageTIme = 0.f;
  float x3ec_damageLightPulser = 0.f;
  float x3f0_damageFilterAmtInit = 1.f;
  float x3f4_damageFilterAmt = 0.f;
  float x3f8_damageFilterAmtGain = 0.f;
  float x3fc_hudDamagePracticalsInit = 0.f;
  float x400_hudDamagePracticals = 0.f;
  float x404_hudDamagePracticalsGain = 0.f;
  zeus::CVector3f x408_damagerToPlayerNorm;
  float x414_decoShakeTranslateAmt = 0.f;
  float x418_decoShakeTranslateAmtVel = 0.f;
  zeus::CVector3f x41c_decoShakeTranslate;
  zeus::CMatrix3f x428_decoShakeRotate;
  zeus::CQuaternion x44c_hudLagShakeRot;
  float x45c_decoShakeAmtInit = 0.f;
  float x460_decoShakeAmt = 0.f;
  float x464_decoShakeAmtGain = 0.f;
  rstl::reserved_vector<zeus::CTransform, 3> x46c_;
  zeus::CVector2f x500_viewportScale = {1.f, 1.f};
  CSfxHandle x508_staticSfxHi;
  CSfxHandle x50c_staticSfxLo;
  float x510_staticInterp = 0.f;
  float x514_staticCycleTimerHi = 0.f;
  float x518_staticCycleTimerLo = 0.f;
  CCameraFilterPass<CRandomStaticFilter> x51c_camFilter2;
  CHUDMemoParms x548_hudMemoParms;
  TLockedToken<CStringTable> x550_hudMemoString;
  u32 x554_hudMemoIdx = 0;
  float x558_messageTextTime = 0.f;
  float x55c_lastSfxChars = 0.f;
  float x560_messageTextScale = 0.f;
  CSfxHandle x564_freeLookSfx;
  zeus::CVector3f x568_fpCamDir;
  float x574_lookDeltaDot = 1.f;
  float x578_freeLookSfxCycleTimer = 0.f;
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
  std::array<float, 16> x5ec_camFovTweaks;
  std::array<float, 64> x62c_camYTweaks;
  std::array<float, 32> x72c_camZTweaks;
  rstl::reserved_vector<SProfileInfo, 15> x7ac_;

  CColoredQuadFilter m_energyDrainFilter;
  CCookieCutterDepthRandomStaticFilter m_cookieCutterStatic{EFilterType::NoColor};

  static CSamusHud* g_SamusHud;
  static rstl::reserved_vector<bool, 4> BuildPlayerHasVisors(const CStateManager& mgr);
  static rstl::reserved_vector<bool, 4> BuildPlayerHasBeams(const CStateManager& mgr);
  void InitializeFrameGluePermanent(const CStateManager& mgr);
  void InitializeFrameGlueMutable(const CStateManager& mgr);
  void UninitializeFrameGlueMutable();
  void InitializeDamageLight();
  void UpdateEnergy(float dt, const CStateManager& mgr, bool init);
  void UpdateFreeLook(float dt, const CStateManager& mgr);
  void UpdateMissile(float dt, const CStateManager& mgr, bool init);
  void UpdateVideoBands(float dt, const CStateManager& mgr);
  void UpdateBallMode(const CStateManager& mgr, bool init);
  void UpdateThreatAssessment(float dt, const CStateManager& mgr);
  void UpdateVisorAndBeamMenus(float dt, const CStateManager& mgr);
  void UpdateCameraDebugSettings();
  void UpdateEnergyLow(float dt, const CStateManager& mgr);
  void ApplyClassicLag(const zeus::CUnitVector3f& lookDir, zeus::CQuaternion& rot, const CStateManager& mgr, float dt,
                       bool invert);
  void UpdateHudLag(float dt, const CStateManager& mgr);
  bool IsCachedLightInAreaLights(const SCachedHudLight& light, const CActorLights& areaLights) const;
  bool IsAreaLightInCachedLights(const CLight& light) const;
  int FindEmptyHudLightSlot(const CLight& light) const;
  zeus::CColor GetVisorHudLightColor(const zeus::CColor& color, const CStateManager& mgr) const;
  void UpdateHudDynamicLights(float dt, const CStateManager& mgr);
  void UpdateHudDamage(float dt, const CStateManager& mgr, DataSpec::ITweakGui::EHelmetVisMode helmetVis);
  void UpdateStaticSfx(CSfxHandle& handle, float& cycleTimer, u16 sfxId, float dt, float oldStaticInterp,
                       float staticThreshold);
  void UpdateStaticInterference(float dt, const CStateManager& mgr);
  int GetRelativeDirection(const zeus::CVector3f& position, const CStateManager& mgr);
  void ShowDamage(const zeus::CVector3f& position, float dam, float prevDam, const CStateManager& mgr);
  void EnterFirstPerson(const CStateManager& mgr);
  void LeaveFirstPerson(const CStateManager& mgr);
  void DrawAttachedEnemyEffect(const CStateManager& mgr);
  static EHudState GetDesiredHudState(const CStateManager& mgr);

public:
  explicit CSamusHud(CStateManager& stateMgr);
  ~CSamusHud();
  void Update(float dt, const CStateManager& mgr, CInGameGuiManager::EHelmetVisMode helmetVis, bool hudVis,
              bool targetingManager);
  void Draw(const CStateManager& mgr, float alpha, CInGameGuiManager::EHelmetVisMode helmetVis, bool hudVis,
            bool targetingManager);
  void DrawHelmet(const CStateManager& mgr, float camYOff);
  void ProcessControllerInput(const CFinalInput& input);
  void UpdateStateTransition(float time, const CStateManager& mgr);
  bool CheckLoadComplete(CStateManager& stateMgr);
  void OnNewInGameGuiState(EInGameGuiState state, CStateManager& stateMgr);
  void RefreshHudOptions();
  void Touch();
  CTargetingManager& GetTargetingManager() { return x8_targetingMgr; }
  const zeus::CVector2f& GetViewportScale() const { return x500_viewportScale; }
  static zeus::CTransform BuildFinalCameraTransform(const zeus::CQuaternion& rot, const zeus::CVector3f& pos,
                                                    const zeus::CVector3f& camPos);
  static void DisplayHudMemo(std::u16string_view text, const CHUDMemoParms& info) {
    if (g_SamusHud)
      g_SamusHud->InternalDisplayHudMemo(text, info);
  }
  void InternalDisplayHudMemo(std::u16string_view text, const CHUDMemoParms& info) { SetMessage(text, info); }
  void SetMessage(std::u16string_view text, const CHUDMemoParms& info);
  static void DeferHintMemo(CAssetId strg, u32 strgIdx, const CHUDMemoParms& info) {
    if (g_SamusHud)
      g_SamusHud->InternalDeferHintMemo(strg, strgIdx, info);
  }
  void InternalDeferHintMemo(CAssetId strg, u32 strgIdx, const CHUDMemoParms& info);
  CGuiFrame* GetBaseHudFrame() const { return x274_loadedFrmeBaseHud; }
};

} // namespace MP1
} // namespace urde
