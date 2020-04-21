#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "Runtime/CStateManager.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Character/CModelData.hpp"
#include "Runtime/Character/CAnimCharacterSet.hpp"
#include "Runtime/Weapon/CGunController.hpp"
#include "Runtime/Weapon/CGunMotion.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
class CActorLights;
class CStateManager;
struct CModelFlags;

class CGrappleArm {
public:
  enum class EArmState {
    IntoGrapple,
    IntoGrappleIdle,
    FireGrapple,
    Three,
    ConnectGrapple,
    Five,
    Connected,
    Seven,
    OutOfGrapple,
    GunControllerAnimation,
    Done
  };

private:
  std::optional<CModelData> x0_grappleArmModel;
  std::optional<CModelData> x50_grappleArmSkeletonModel;
  CModelData xa0_grappleGearModel;
  CModelData xec_grapNoz1Model;
  CModelData x138_grapNoz2Model;
  TCachedToken<CAnimCharacterSet> x184_grappleArm;
  std::vector<CToken> x18c_anims;
  rstl::reserved_vector<std::vector<CToken>, 8> x19c_suitDeps;
  zeus::CTransform x220_xf;
  zeus::CTransform x250_grapLocatorXf;
  zeus::CTransform x280_grapNozLoc1Xf;
  zeus::CTransform x2b0_grapNozLoc2Xf;
  zeus::CTransform x2e0_auxXf;
  zeus::CVector3f x310_grapplePointPos;
  zeus::CVector3f x31c_scale;
  std::unique_ptr<CGunController> x328_gunController;
  CSfxHandle x32c_grappleLoopSfx;
  CSfxHandle x330_swooshSfx;
  EArmState x334_animState = EArmState::Done;
  float x338_beamT = 0.f;
  float x33c_beamDist = 0.f;
  float x340_anglePhase = 0.f;
  float x344_xAmplitude = 0.f;
  float x348_zAmplitude = 0.f;
  std::pair<u16, CSfxHandle> x34c_animSfx = {0xffff, {}};
  TCachedToken<CGenDescription> x354_grappleSegmentDesc;
  TCachedToken<CGenDescription> x360_grappleClawDesc;
  TCachedToken<CGenDescription> x36c_grappleHitDesc;
  TCachedToken<CGenDescription> x378_grappleMuzzleDesc;
  TCachedToken<CSwooshDescription> x384_grappleSwooshDesc;
  std::unique_ptr<CElementGen> x390_grappleSegmentGen;
  std::unique_ptr<CElementGen> x394_grappleClawGen;
  std::unique_ptr<CElementGen> x398_grappleHitGen;
  std::unique_ptr<CElementGen> x39c_grappleMuzzleGen;
  std::unique_ptr<CParticleSwoosh> x3a0_grappleSwooshGen;
  std::unique_ptr<CRainSplashGenerator> x3a4_rainSplashGenerator;
  CPlayerState::EPlayerSuit x3a8_loadedSuit = CPlayerState::EPlayerSuit::Invalid;
  float x3ac_pitchBend = 0.f;
  s16 x3b0_rumbleHandle = -1;
  bool x3b2_24_active : 1 = false;
  bool x3b2_25_beamActive : 1 = false;
  bool x3b2_26_grappleHit : 1 = false;
  bool x3b2_27_armMoving : 1 = false;
  bool x3b2_28_isGrappling : 1 = false;
  bool x3b2_29_suitLoading : 1 = false;

  static float g_GrappleBeamAnglePhaseDelta;
  static float g_GrappleBeamXWaveAmplitude;
  static float g_GrappleBeamZWaveAmplitude;
  static float g_GrappleBeamSpeed;

  void FillTokenVector(const std::vector<SObjectTag>& tags, std::vector<CToken>& objects);
  void BuildSuitDependencyList();
  void LoadAnimations();
  void ResetAuxParams(bool resetGunController);
  void DisconnectGrappleBeam();
  void LoadSuitPoll();
  void BuildXRayModel();
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type);
  void DoUserAnimEvents(CStateManager& mgr);
  void UpdateArmMovement(float dt, CStateManager& mgr);
  void UpdateGrappleBeamFx(const zeus::CVector3f& beamGunPos, const zeus::CVector3f& beamAirPos, CStateManager& mgr);
  bool UpdateGrappleBeam(float dt, const zeus::CTransform& beamLoc, CStateManager& mgr);
  void UpdateSwingAction(float grappleSwingT, float dt, CStateManager& mgr);
  void RenderXRayModel(const CStateManager& mgr, const zeus::CTransform& modelXf, const CModelFlags& flags);

  static void PointGenerator(void* ctx, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn);

public:
  explicit CGrappleArm(const zeus::CVector3f& scale);
  void AsyncLoadSuit(CStateManager& mgr);
  void SetTransform(const zeus::CTransform& xf) { x220_xf = xf; }
  const zeus::CTransform& GetTransform() const { return x220_xf; }
  zeus::CTransform& AuxTransform() { return x2e0_auxXf; }
  void SetAnimState(EArmState state);
  EArmState GetAnimState() const { return x334_animState; }
  bool GetActive() const { return x3b2_24_active; }
  bool BeamActive() const { return x3b2_25_beamActive; }
  bool IsArmMoving() const { return x3b2_27_armMoving; }
  bool IsGrappling() const { return x3b2_28_isGrappling; }
  bool IsSuitLoading() const { return x3b2_29_suitLoading; }
  void Activate(bool);
  void GrappleBeamDisconnected();
  void GrappleBeamConnected();
  void RenderGrappleBeam(const CStateManager& mgr, const zeus::CVector3f& pos);
  void TouchModel(const CStateManager& mgr) const;
  void Update(float grappleSwingT, float dt, CStateManager& mgr);
  void PreRender(const CStateManager& mgr, const zeus::CFrustum& frustum, const zeus::CVector3f& camPos);
  void Render(const CStateManager& mgr, const zeus::CVector3f& pos, const CModelFlags& flags,
              const CActorLights* lights);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void EnterStruck(CStateManager& mgr, float angle, bool bigStrike, bool notInFreeLook);
  void EnterIdle(CStateManager& mgr);
  void EnterFidget(CStateManager& mgr, SamusGun::EFidgetType type, s32 gunId, s32 animSet);
  void EnterFreeLook(s32 gunId, s32 setId, CStateManager& mgr);
  void EnterComboFire(s32 gunId, CStateManager& mgr);
  void ReturnToDefault(CStateManager& mgr, float dt, bool setState);
  CGunController* GunController() { return x328_gunController.get(); }
  const CGunController* GunController() const { return x328_gunController.get(); }
};

} // namespace urde
