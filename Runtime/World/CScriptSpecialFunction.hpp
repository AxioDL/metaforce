#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

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

  enum class ERingState { Scramble, Rotate, Stopped, Breakup };

  struct SRingController {
    TUniqueId x0_id;
    float x4_rotateSpeed;
    bool x8_reachedTarget;
    zeus::CVector3f xc_;
    SRingController(TUniqueId uid, float rotateSpeed, bool reachedTarget);
  };

private:
  ESpecialFunction xe8_function;
  std::string xec_locatorName;
  float xfc_float1;
  float x100_float2;
  float x104_float3;
  float x108_float4;
  zeus::CVector3f x10c_vector3f;
  zeus::CColor x118_color;
  CDamageInfo x11c_damageInfo;
  float x138_ = 0.f;
  zeus::CTransform x13c_ = zeus::CTransform();
  float x16c_ = 0.f;
  u16 x170_sfx1;
  u16 x172_sfx2;
  u16 x174_sfx3;
  CSfxHandle x178_sfxHandle;
  u32 x17c_;
  float x180_ = 0.f;
  TReservedAverage<float, 6> x184_;
  float x194_ = 0.f;
  std::vector<SRingController> x198_ringControllers;
  ERingState x1a8_ringState = ERingState::Stopped;
  zeus::CVector3f x1ac_ringRotateTarget = zeus::skZero3f;
  bool x1b8_ringReverse = true;
  s32 x1bc_areaSaveId;
  s32 x1c0_layerIdx;
  CPlayerState::EItemType x1c4_item;
  std::optional<zeus::CAABox> x1c8_touchBounds;
  bool x1e4_24_ : 1 = false;
  bool x1e4_25_spinnerCanMove : 1 = false;
  bool x1e4_26_sfx2Played : 1 = true;
  bool x1e4_27_sfx3Played : 1 = false;
  bool x1e4_28_frustumEntered : 1 = false;
  bool x1e4_29_frustumExited : 1 = false;
  bool x1e4_30_ : 1 = false;
  bool x1e4_31_inAreaDamage : 1 = false;
  bool x1e5_24_doSave : 1 = false;
  bool x1e5_25_playerInArea : 1 = false;
  bool x1e5_26_displayBillboard : 1 = false;
  TLockedToken<CTexture> x1e8_; // Used to be optional
public:
  CScriptSpecialFunction(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, ESpecialFunction,
                         std::string_view, float, float, float, float, const zeus::CVector3f&, const zeus::CColor&,
                         bool, const CDamageInfo&, s32, s32, CPlayerState::EItemType, s16, s16, s16);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override;
  void Render(CStateManager&) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override { return x1c8_touchBounds; }

  void SkipCinematic(CStateManager&);
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
  void AddOrUpdateEmitter(float pitch, CSfxHandle& handle, u16 id, const zeus::CVector3f& pos, float vol);
};
} // namespace urde
