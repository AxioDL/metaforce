#ifndef _CSAMUSDOLL
#define _CSAMUSDOLL

#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "rstl/single_ptr.hpp"

#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

class CActorLights;
class CAnimCharacterSet;
class CElementGen;
class CGenDescription;

class CSamusDoll {
public:
  CSamusDoll(const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp,
             const CPlayerState::EPlayerSuit suit, const CPlayerState::EBeamId beam,
             const bool hasSpiderBall, const bool hasGrappleBeam);
  ~CSamusDoll();
  bool CheckLoadComplete();
  bool IsLoaded() const;
  float GetViewInterpolation() const { return mViewInterp; }
  bool IsViewInterpolating() const { return mViewInterp != 0.f && mViewInterp != 1.f; }
  void Update(float dt, CRandom16& rand);
  void Draw(const CStateManager& mgr, float alpha);
  void Touch();
  void CheckTransition(const bool morphball);
  void SetRotation(float xDelta, float zDelta, float dt);
  void SetOffset(const CVector3f& offset, float dt);
  void BeginViewInterpolate(bool zoomIn);
  void SetPulseBeam(const bool pulse);
  void SetPulseGrapple(const bool pulse);
  void SetPulseBoots(const bool pulse);
  void SetPulseVisor(const bool pulse);
  void SetPulseSuit(const bool pulse);

private:
  static CModelData BuildSuitModelData1(CPlayerState::EPlayerSuit suit);
  static CModelData BuildSuitModelDataBoots(CPlayerState::EPlayerSuit suit);
  void SetupLights();
  void SetTransitionAnimation();
  void SetOffsetSfxPlaying(bool playing);
  void SetZoomSfxPlaying(const bool playing);
  void SetRotationSfxPlaying(bool playing);

  rstl::vector< CToken > mDepTokens;
  CTransform4f mBallTransform;
  float mAlphaIn;
  CPlayerState::EPlayerSuit mSuit;
  CPlayerState::EBeamId mBeam;
  bool mCompletedMorphball;
  bool mSelectedMorphball;
  float mTotalTransitionTime;
  float mRemTransitionTime;
  float mSuitPulseFactor;
  float mBeamPulseFactor;
  float mGrapplePulseFactor;
  float mBootsPulseFactor;
  float mVisorPulseFactor;
  float mBallPulseFactor;
  CQuaternion mFixedRot;
  float mFixedZoom;
  CVector3f mInterpStartOffset;
  CQuaternion mUserInterpRot;
  float mUserInterpZoom;
  CVector3f mOffset;
  CQuaternion mUserRot;
  float mUserZoom;

  float mViewInterp;
  rstl::optional_object< CModelData > mSuitModel0;
  rstl::reserved_vector< TLockedToken< CSkinnedModel >, 2 > mSuitModel1and2;
  rstl::optional_object< CModelData > mSuitModelBoots;
  rstl::optional_object< CModelData > mBallModelData;
  TCachedToken< CModel > mSpiderBallGlass;
  uint mBallMatIdx;
  uint mGlassMatIdx;
  uint mBallGlowColorIdx;
  CToken mItemScreenSamus;
  TCachedToken< CModel > mInvBeam;
  TCachedToken< CModel > mInvVisor;
  TCachedToken< CModel > mInvGrappleBeam;
  TCachedToken< CModel > mInvFins;
  TToken< CGenDescription > mBallInnerGlow;
  rstl::single_ptr< CElementGen > mBallInnerGlowGen;
  TToken< CGenDescription > mBallTransitionFlash;
  rstl::single_ptr< CElementGen > mBallTransitionFlashGen;
  rstl::vector< CLight > mLights;
  rstl::single_ptr< CActorLights > mActorLights;
  rstl::optional_object< TCachedToken< CTexture > > mPhazonIndirectTexture;
  float mPhazonOffsetAngle;
  CSfxHandle mOffsetSfx;
  CSfxHandle mRotateSfx;
  CSfxHandle mZoomSfx;
  bool mHasSpiderBall : 1;
  bool mHasGrappleBeam : 1;
  bool mPulseSuit : 1;
  bool mPulseBeam : 1;
  bool mPulseGrapple : 1;
  bool mPulseBoots : 1;
  bool mPulseVisor : 1;
  bool mLoaded : 1;
};
CHECK_SIZEOF(CSamusDoll, 0x274)

#endif // _CSAMUSDOLL
