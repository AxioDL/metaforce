#ifndef _CGRAPPLEARM
#define _CGRAPPLEARM

#include "types.h"

#include "Kyoto/Animation/CAnimCharacterSet.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include <rstl/optional_object.hpp>
#include <rstl/pair.hpp>
#include <rstl/single_ptr.hpp>

class CVector3f;
class CModelData;
class CStateManager;
class CFrustumPlanes;
class CGunController;
class CGenDescription;
class CSwooshDescription;
class CElementGen;
class CParticleSwoosh;
class CRainSplashGenerator;
class CModelFlags;
class CActorLights;
class CInt32POINode;

class CGrappleArm {
public:
  enum EArmState {
    kAS_IntoGrapple,
    kAS_IntoGrappleIdle,
    kAS_FireGrapple,
    kAS_Three,
    kAS_ConnectGrapple,
    kAS_Five,
    kAS_Connected,
    kAS_Seven,
    kAS_OutOfGrapple,
    kAS_GunControllerAnimation,
    kAS_Done
  };

  CGrappleArm(const CVector3f& scale);
  ~CGrappleArm();

  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum, const CVector3f& camPos);
  void Update(float swingT, float dt, CStateManager& mgr);
  void EnterFidget(CStateManager& mgr, int a, int b, int c);
  void EnterFreeLook(int gunId, int setId, CStateManager& mgr);
  void EnterComboFire(int a, CStateManager& mgr);
  void ReturnToDefault(CStateManager& mgr, float dt, bool setState);
  void EnterIdle(CStateManager& mgr);
  void EnterStruck(CStateManager&, float, bool, bool);
  void DisconnectGrappleBeam();
  void ResetAuxParams(bool resetGunController);
  static void PointGenerator(void* context, const CVector3f* vertices, const CVector3f* normals,
                             int count);
  void GrappleBeamConnected();
  void GrappleBeamDisconnected();
  void Activate(bool active);
  void RenderGrappleBeam(const CStateManager&, const CVector3f&) const;
  void AsyncLoadSuit(CStateManager& mgr);
  void TouchModel(const CStateManager&) const;
  void Render(const CStateManager&, const CVector3f&, const CModelFlags&,
              const CActorLights*) const;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);

  void SetTransform(const CTransform4f& xf) { mXf = xf; }
  CTransform4f GetTransform() const { return mXf; }
  CTransform4f& AuxTransform() { return mAuxXf; }
  CGunController* GunController() { return mGunController.get(); }
  const CGunController* GetGunController() const { return mGunController.get(); }

  EArmState GetAnimState() const { return mAnimState; }
  void SetAnimState(EArmState state);
  bool CanRender() const { return mActive && !mSuitLoading; }
  const bool GetActive() const { return mActive; }
  void SetActive(const bool active) { mActive = active; }
  const bool BeamActive() const { return mBeamActive; }
  void SetBeamActive(const bool active) { mBeamActive = active; }
  const bool IsArmMoving() const { return mArmMoving; }
  void SetIsArmMoving(const bool active) { mArmMoving = active; }
  const bool IsGrappling() const { return mIsGrappling; }
  void SetIsGrappling(const bool active) { mIsGrappling = active; }
  const bool IsSuitLoading() const { return mSuitLoading; }
  void SetIsSuitLoading(const bool active) { mSuitLoading = active; }

  void BuildSuitDependencyList();
  void FillTokenVector(const rstl::vector< SObjectTag >& tags, rstl::vector< CToken >& objects);
  void LoadSuitPoll();
  void BuildXRayModel();
  void RenderXRayModel(const CStateManager& mgr, const CTransform4f& xf,
                       const CModelFlags& flags) const;
  void LoadAnimations();
  void UpdateArmMovement(float dt, CStateManager& mgr);
  void UpdateSwingAction(float swingT, float dt, CStateManager& mgr);
  const bool UpdateGrappleBeam(float dt, const CTransform4f& beamLoc, CStateManager& mgr);
  void UpdateGrappleBeamFX(const CVector3f& beamGunPos, const CVector3f& beamAirPos,
                           CStateManager& mgr);
  void DoUserAnimEvents(CStateManager& mgr);
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type);

private:
  static float kPhaseDelta;
  static float kAmplitudeX;
  static float kAmplitudeZ;
  static float kMetresPerSecond;
  rstl::optional_object< CModelData > mGrappleArmModel;
  rstl::optional_object< CModelData > mGrappleArmSkeletonModel;
  CModelData mGrappleGearModel;
  CModelData mGrapNoz1Model;
  CModelData mGrapNoz2Model;
  TToken< CAnimCharacterSet > mGrappleArm;
  rstl::vector< CToken > mAnims;
  rstl::reserved_vector< rstl::vector< CToken >, 8 > mSuitDeps;
  CTransform4f mXf;
  CTransform4f mGrapLocatorXf;
  CTransform4f mGrapNozLoc1Xf;
  CTransform4f mGrapNozLoc2Xf;
  CTransform4f mAuxXf;
  CVector3f mGrapplePointPos;
  CVector3f mScale;
  rstl::single_ptr< CGunController > mGunController;
  CSfxHandle mGrappleLoopSfx;
  CSfxHandle mSwooshSfx;
  EArmState mAnimState;
  float mBeamT;
  float mBeamDist;
  float mAnglePhase;
  float mXAmplitude;
  float mZAmplitude;
  rstl::pair< ushort, CSfxHandle > mAnimSfx;
  TLockedToken< CGenDescription > mGrappleSegmentDesc;
  TLockedToken< CGenDescription > mGrappleClawDesc;
  TLockedToken< CGenDescription > mGrappleHitDesc;
  TLockedToken< CGenDescription > mGrappleMuzzleDesc;
  TLockedToken< CSwooshDescription > mGrappleSwooshDesc;
  rstl::single_ptr< CElementGen > mGrappleSegmentGen;
  rstl::single_ptr< CElementGen > mGrappleClawGen;
  rstl::single_ptr< CElementGen > mGrappleHitGen;
  rstl::single_ptr< CElementGen > mGrappleMuzzleGen;
  rstl::single_ptr< CParticleSwoosh > mGrappleSwooshGen;
  rstl::single_ptr< CRainSplashGenerator > mRainSplashGenerator;
  CPlayerState::EPlayerSuit mLoadedSuit;
  int mPitchBend;
  short mRumbleHandle;
  bool mActive : 1;
  bool mBeamActive : 1;
  bool mGrappleHit : 1;
  bool mArmMoving : 1;
  bool mIsGrappling : 1;
  bool mSuitLoading : 1;
};
CHECK_SIZEOF(CGrappleArm, 0x3b4)

#endif // _CGRAPPLEARM
