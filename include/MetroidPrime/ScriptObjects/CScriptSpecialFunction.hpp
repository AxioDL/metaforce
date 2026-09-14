#ifndef _CSCRIPTSPECIALFUNCTION
#define _CSCRIPTSPECIALFUNCTION

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TAverage.hpp"

#include "MetroidPrime/Player/CPlayerState.hpp"

class CScriptSpecialFunction : public CActor {
public:
  enum ESpecialFunction {
    kSF_What = 0,
    kSF_PlayerFollowLocator = 1,
    kSF_SpinnerController = 2,
    kSF_ObjectFollowLocator = 3,
    kSF_ChaffTarget = 4,
    kSF_InventoryActivator = 5,
    kSF_MapStation = 6,
    kSF_SaveStation = 7,
    kSF_IntroBossRingController = 8,
    kSF_ViewFrustumTester = 9,
    kSF_ShotSpinnerController = 10,
    kSF_EscapeSequence = 11,
    kSF_BossEnergyBar = 12,
    kSF_EndGame = 13,
    kSF_HUDFadeIn = 14,
    kSF_CinematicSkip = 15,
    kSF_ScriptLayerController = 16,
    kSF_RainSimulator = 17,
    kSF_AreaDamage = 18,
    kSF_ObjectFollowObject = 19,
    kSF_RedundantHintSystem = 20,
    kSF_DropBomb = 21,
    kSF_ScaleActor = 22,
    kSF_MissileStation = 23,
    kSF_Billboard = 24,
    kSF_PlayerInAreaRelay = 25,
    kSF_HUDTarget = 26,
    kSF_FogFader = 27,
    kSF_EnterLogbook = 28,
    kSF_PowerBombStation = 29,
    kSF_Ending = 30,
    kSF_FusionRelay = 31,
    kSF_WeaponSwitch = 32,
    kSF_FogVolume = 47,
    kSF_RadialDamage = 48,
    kSF_EnvFxDensityController = 49,
    kSF_RumbleEffect = 50
  };

  enum ESpinnerControllerMode {
    kSCM_Zero,
    kSCM_One,
  };

  enum ERingState { kRS_Scramble, kRS_Rotate, kRS_Stopped, kRS_Breakup };

  struct SRingController {
    TUniqueId x0_id;
    float x4_rotateSpeed;
    bool x8_reachedTarget;
    CVector3f xc_;

    SRingController(TUniqueId uid, float rotateSpeed, bool reachedTarget)
    : x0_id(uid)
    , x4_rotateSpeed(rotateSpeed)
    , x8_reachedTarget(reachedTarget)
    , xc_(CVector3f::Zero()) {}
  };

private:
  ESpecialFunction xe8_function;
  rstl::string xec_locatorName;
  float xfc_float1;
  float x100_float2;
  float x104_float3;
  float x108_float4;
  CVector3f x10c_vector3f;
  CColor x118_color;
  CDamageInfo x11c_damageInfo;
  float x138_;
  CTransform4f x13c_spinnerInitialXf;
  float x16c_;
  ushort x170_sfx1;
  ushort x172_sfx2;
  ushort x174_sfx3;
  CSfxHandle x178_sfxHandle;
  uint x17c_;
  float x180_;
  TAverage< float > x184_;
  float x194_;
  rstl::vector< SRingController > x198_ringControllers;
  ERingState x1a8_ringState;
  CVector3f x1ac_ringRotateTarget;
  bool x1b8_ringReverse;
  int x1bc_areaSaveId;
  int x1c0_layerIdx;
  CPlayerState::EItemType x1c4_item;
  rstl::optional_object< CAABox > x1c8_touchBounds;
  bool x1e4_24_spinnerInitializedXf : 1;
  bool x1e4_25_spinnerCanMove : 1;
  bool x1e4_26_sfx2Played : 1;
  bool x1e4_27_sfx3Played : 1;
  bool x1e4_28_frustumEntered : 1;
  bool x1e4_29_frustumExited : 1;
  bool x1e4_30_ : 1;
  bool x1e4_31_inAreaDamage : 1;
  bool x1e5_24_doSave : 1;
  bool x1e5_25_playerInArea : 1;
  bool x1e5_26_displayBillboard : 1;
  rstl::optional_object< CToken > x1e8_;

public:
  CScriptSpecialFunction(TUniqueId, const rstl::string&, const CEntityInfo&, const CTransform4f&,
                         ESpecialFunction, const rstl::string&, float, float, float, float,
                         const CVector3f&, const CColor&, const bool, const CDamageInfo&, int, int,
                         CPlayerState::EItemType, const ushort, const ushort, const ushort);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PreRender(CStateManager&, const CFrustumPlanes&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override { return x1c8_touchBounds; }

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

  void DeleteEmitter(CSfxHandle& handle);
  int GetSpecialEnding(const CStateManager&) const;
  void AddOrUpdateEmitter(float pitch, CSfxHandle& handle, ushort id, CVector3f pos, uchar vol);
};
CHECK_SIZEOF(CScriptSpecialFunction, (VERSION >= VERSION_GM8P_00 ? 0x208 : 0x1f8))

#endif // _CSCRIPTSPECIALFUNCTION
