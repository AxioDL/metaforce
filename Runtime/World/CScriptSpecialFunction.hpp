#pragma once

#include "CActor.hpp"
#include "CDamageInfo.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CColor.hpp"

namespace urde {
class CScriptSpecialFunction : public CActor {
public:
  enum class ESpecialFunction {
    What = 0,
    PlayerFollowLocator = 1,
    SpinnerController = 2,
    ObjectFollowLocator = 3,
    ChaffTarget = 4,
    InventoryActivator = 5,
    MapStation = 6,
    SaveStation = 7,
    IntroBossRingController = 8,
    ViewFrustumTester = 9,
    ShotSpinnerController = 10,
    EscapeSequence = 11,
    BossEnergyBar = 12,
    EndGame = 13,
    HUDFadeIn = 14,
    CinematicSkip = 15,
    ScriptLayerController = 16,
    RainSimulator = 17,
    AreaDamage = 18,
    ObjectFollowObject = 19,
    RedundantHintSystem = 20,
    DropBomb = 21,
    ScaleActor = 22,
    MissileStation = 23,
    Billboard = 24,
    PlayerInAreaRelay = 25,
    HUDTarget = 26,
    FogFader = 27,
    EnterLogbook = 28,
    PowerBombStation = 29,
    Ending = 30,
    FusionRelay = 31,
    WeaponSwitch = 32,
    FogVolume = 47,
    RadialDamage = 48,
    EnvFxDensityController = 49,
    RumbleEffect = 50
  };

  enum class ESpinnerControllerMode {
    Zero,
    One,
  };

  struct SRingController {
    TUniqueId x0_id;
    float x4_;
    bool x8_;
    zeus::CVector3f xc_;
    SRingController(TUniqueId uid, float f, bool b);
  };

private:
  ESpecialFunction xe8_function;
  std::string xec_locatorName;
  float xfc_;
  float x100_;
  float x104_;
  float x108_;
  zeus::CVector3f x10c_;
  zeus::CColor x118_;
  CDamageInfo x11c_damageInfo;
  float x138_ = 0.f;
  zeus::CTransform x13c_ = zeus::CTransform::Identity();
  float x16c_ = 0.f;
  s16 x170_;
  s16 x172_;
  s16 x174_;
  CSfxHandle x178_sfxHandle;
  u32 x17c_;
  float x180_ = 0.f;
  std::vector<float> x184_;
  float x194_ = 0.f;
  std::vector<SRingController> x198_ringControllers;
  u32 x1a8_ = 2;
  zeus::CVector3f x1ac_ = zeus::CVector3f::skZero;
  bool x1b8_ = true;
  s32 x1bc_areaSaveId;
  s32 x1c0_layerIdx;
  CPlayerState::EItemType x1c4_item;
  rstl::optional<zeus::CAABox> x1c8_;
  union {
    struct {
      bool x1e4_24_ : 1;
      bool x1e4_25_spinnerCanMove : 1;
      bool x1e4_26_ : 1;
      bool x1e4_27_ : 1;
      bool x1e4_28_frustumEntered : 1;
      bool x1e4_29_frustumExited : 1;
      bool x1e4_30_ : 1;
      bool x1e4_31_ : 1;
      bool x1e5_24_doSave : 1;
      bool x1e5_25_playerInArea : 1;
      bool x1e5_26_displayBillboard : 1;
    };
    u32 x1e4_dummy = 0;
  };
  TLockedToken<CTexture> x1e8_; // Used to be optional
public:
  CScriptSpecialFunction(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, ESpecialFunction,
                         std::string_view, float, float, float, float, const zeus::CVector3f&, const zeus::CColor&,
                         bool, const CDamageInfo&, s32, s32, CPlayerState::EItemType, s16, s16, s16);

  void Accept(IVisitor& visitor);
  void Think(float, CStateManager&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void PreRender(CStateManager&, const zeus::CFrustum&);
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
  void Render(const CStateManager&) const;

  void SkipCinematic(CStateManager&);
  void RingMoveCloser(CStateManager&, float);
  void RingMoveAway(CStateManager&, float);
  void ThinkRingPuller(float, CStateManager&);
  void RingScramble(CStateManager&);
  void ThinkIntroBossRingController(float, CStateManager&);
  void ThinkPlayerFollowLocator(float, CStateManager&);
  void ThinkSpinnerController(float, CStateManager&, ESpinnerControllerMode);
  void ThinkObjectFollowLocator(float, CStateManager&);
  void ThinkObjectFollowObject(float, CStateManager&);
  void ThinkChaffTarget(float, CStateManager&);
  void ThinkActorScale(float, CStateManager&);
  void ThinkSaveStation(float, CStateManager&);
  void ThinkRainSimulator(float, CStateManager&);
  void ThinkAreaDamage(float, CStateManager&);
  void ThinkPlayerInArea(float, CStateManager&);

  bool ShouldSkipCinematic(CStateManager& stateMgr) const;

  void DeleteEmitter(const CSfxHandle& handle);
  u32 GetSpecialEnding(const CStateManager&) const;
};
} // namespace urde
