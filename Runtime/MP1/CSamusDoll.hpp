#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "Runtime/CPlayerState.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/Character/CActorLights.hpp"
#include "Runtime/Character/CAnimCharacterSet.hpp"
#include "Runtime/Character/CModelData.hpp"
#include "Runtime/Particle/CElementGen.hpp"

#include <zeus/CRelAngle.hpp>
#include <zeus/CQuaternion.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CDependencyGroup;
namespace MP1 {

class CSamusDoll {
  std::vector<CToken> x0_depToks;
  zeus::CTransform x10_ballXf;
  float x40_alphaIn = 0.f;
  CPlayerState::EPlayerSuit x44_suit;
  CPlayerState::EBeamId x48_beam;
  bool x4c_completedMorphball = false;
  bool x4d_selectedMorphball = false;
  float x50_totalTransitionTime = 1.f;
  float x54_remTransitionTime = 0.f;
  float x58_suitPulseFactor = 0.f;
  float x5c_beamPulseFactor = 0.f;
  float x60_grapplePulseFactor = 0.f;
  float x64_bootsPulseFactor = 0.f;
  float x68_visorPulseFactor = 0.f;
  float x6c_ballPulseFactor = 0.f;
  zeus::CQuaternion x70_fixedRot;
  float x80_fixedZoom = -3.6f;
  zeus::CVector3f x84_interpStartOffset = skInitialOffset;
  zeus::CQuaternion x90_userInterpRot;
  float xa0_userInterpZoom = -3.6f;
  zeus::CVector3f xa4_offset = skInitialOffset;
  zeus::CQuaternion xb0_userRot;
  float xc0_userZoom = -3.6f;
  float xc4_viewInterp = 0.f;
  std::optional<CModelData> xc8_suitModel0;
  rstl::reserved_vector<TCachedToken<CSkinnedModel>, 2> x118_suitModel1and2;
  std::optional<CModelData> x134_suitModelBoots;
  std::optional<CModelData> x184_ballModelData;
  TLockedToken<CModel> x1d4_spiderBallGlass;
  u32 x1e0_ballMatIdx;
  u32 x1e4_glassMatIdx;
  u32 x1e8_ballGlowColorIdx;
  TLockedToken<CAnimCharacterSet> x1ec_itemScreenSamus;
  TLockedToken<CModel> x1f4_invBeam;
  TLockedToken<CModel> x200_invVisor;
  TLockedToken<CModel> x20c_invGrappleBeam;
  TLockedToken<CModel> x218_invFins;
  TLockedToken<CGenDescription> x224_ballInnerGlow;
  std::unique_ptr<CElementGen> x22c_ballInnerGlowGen;
  TLockedToken<CGenDescription> x230_ballTransitionFlash;
  std::unique_ptr<CElementGen> x238_ballTransitionFlashGen;
  std::vector<CLight> x23c_lights;
  std::unique_ptr<CActorLights> x24c_actorLights;
  TLockedToken<CTexture> x250_phazonIndirectTexture; // Used to be optional
  zeus::CRelAngle x260_phazonOffsetAngle;
  CSfxHandle x264_offsetSfx;
  CSfxHandle x268_rotateSfx;
  CSfxHandle x26c_zoomSfx;
  bool x270_24_hasSpiderBall : 1;
  bool x270_25_hasGrappleBeam : 1;
  bool x270_26_pulseSuit : 1 = false;
  bool x270_27_pulseBeam : 1 = false;
  bool x270_28_pulseGrapple : 1 = false;
  bool x270_29_pulseBoots : 1 = false;
  bool x270_30_pulseVisor : 1 = false;
  bool x270_31_loaded : 1 = false;

  static constexpr zeus::CVector3f skInitialOffset{0.0f, 0.0f, 0.8f};
  static CModelData BuildSuitModelData1(CPlayerState::EPlayerSuit suit);
  static CModelData BuildSuitModelDataBoots(CPlayerState::EPlayerSuit suit);
  void SetupLights();
  void SetTransitionAnimation();
  void SetRotationSfxPlaying(bool playing);
  void SetOffsetSfxPlaying(bool playing);
  void SetZoomSfxPlaying(bool playing);

public:
  CSamusDoll(const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp, CPlayerState::EPlayerSuit suit,
             CPlayerState::EBeamId beam, bool hasSpiderBall, bool hasGrappleBeam);
  bool IsLoaded() const;
  bool CheckLoadComplete();
  void Update(float dt, CRandom16& rand);
  void Draw(const CStateManager& mgr, float alpha);
  void Touch();
  void SetInMorphball(bool morphballComplete);
  void SetRotation(float xDelta, float zDelta, float dt);
  void SetOffset(const zeus::CVector3f& offset, float dt);
  void BeginViewInterpolate(bool zoomOut);
  void SetPulseSuit(bool b) { x270_26_pulseSuit = b; }
  void SetPulseVisor(bool b) { x270_30_pulseVisor = b; }
  void SetPulseBoots(bool b) { x270_29_pulseBoots = b; }
  void SetPulseGrapple(bool b) { x270_28_pulseGrapple = b; }
  void SetPulseBeam(bool b) { x270_27_pulseBeam = b; }
  float GetViewInterpolation() const { return xc4_viewInterp; }
  bool IsZoomedOut() const { return xc0_userZoom == -4.f; }
};

} // namespace MP1
} // namespace urde
