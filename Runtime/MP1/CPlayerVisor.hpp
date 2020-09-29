#pragma once

#include "Runtime/CPlayerState.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Graphics/Shaders/CColoredQuadFilter.hpp"
#include "Runtime/Graphics/Shaders/CTexturedQuadFilter.hpp"

#include <zeus/CVector2f.hpp>

namespace urde {
class CStateManager;
class CTargetingManager;

namespace MP1 {

class CPlayerVisor {
  struct SScanTarget {
    TUniqueId x0_objId = kInvalidUniqueId;
    float x4_timer = 0.f;
    float x8_inRangeTimer = 0.f;
    bool xc_inBox = false;
  };

  enum class EScanWindowState { NotInScanVisor, Idle, Scan };

  rstl::reserved_vector<zeus::CVector2f, 3> x0_scanWindowSizes;
  CPlayerState::EPlayerVisor x1c_curVisor = CPlayerState::EPlayerVisor::Combat;
  CPlayerState::EPlayerVisor x20_nextVisor = CPlayerState::EPlayerVisor::Combat;
  float x24_visorSfxVol = 1.f; // used to be u8
  bool x25_24_visorTransitioning : 1 = false;
  bool x25_25_ : 1 = false;
  float x28_ = 0.f;
  float x2c_scanDimInterp = 1.f;
  EScanWindowState x30_prevState = EScanWindowState::NotInScanVisor;
  EScanWindowState x34_nextState = EScanWindowState::NotInScanVisor;
  float x38_windowInterpDuration = 0.f;
  float x3c_windowInterpTimer = 0.f;
  zeus::CVector2f x40_prevWindowDims;
  zeus::CVector2f x48_interpWindowDims;
  zeus::CVector2f x50_nextWindowDims;
  float x58_scanMagInterp = 1.f;
  CSfxHandle x5c_visorLoopSfx;
  CSfxHandle x60_scanningLoopSfx;
  CCameraFilterPass<CColoredQuadFilter> x64_scanDim;
  CCameraBlurPass x90_xrayBlur;
  float xc4_vpScaleX = 1.f;
  float xc8_vpScaleY = 1.f;
  TLockedToken<CModel> xcc_scanFrameCorner;
  TLockedToken<CModel> xd8_scanFrameCenterSide;
  TLockedToken<CModel> xe4_scanFrameCenterTop;
  TLockedToken<CModel> xf0_scanFrameStretchSide;
  TLockedToken<CModel> xfc_scanFrameStretchTop;
  // TCachedToken<CModel> x108_newScanPane;
  CTexturedQuadFilter x108_newScanPane;
  TLockedToken<CModel> x114_scanShield;
  int x120_assetLockCountdown = 0;
  TLockedToken<CModel> x124_scanIconNoncritical;
  TLockedToken<CModel> x130_scanIconCritical;
  rstl::reserved_vector<SScanTarget, 64> x13c_scanTargets;
  TLockedToken<CTexture> x540_xrayPalette;
  float x54c_scanFrameColorInterp = 0.f;
  float x550_scanFrameColorImpulseInterp = 0.f;

  int FindEmptyInactiveScanTarget() const;
  int FindCachedInactiveScanTarget(TUniqueId uid) const;
  bool DrawScanObjectIndicators(const CStateManager& mgr) const;
  void UpdateScanObjectIndicators(const CStateManager& mgr, float dt);
  void UpdateScanWindow(float dt, const CStateManager& mgr);
  EScanWindowState GetDesiredScanWindowState(const CStateManager& mgr) const;
  void LockUnlockAssets();
  void DrawScanEffect(const CStateManager& mgr, CTargetingManager* tgtMgr);
  void DrawXRayEffect(const CStateManager& mgr);
  void DrawThermalEffect(const CStateManager& mgr);
  void UpdateCurrentVisor(float transFactor);
  void FinishTransitionIn();
  void BeginTransitionIn(const CStateManager& mgr);
  void FinishTransitionOut(const CStateManager& mgr);
  void BeginTransitionOut();

public:
  explicit CPlayerVisor(CStateManager& stateMgr);
  ~CPlayerVisor();
  void Update(float dt, const CStateManager& stateMgr);
  void Draw(const CStateManager& stateMgr, CTargetingManager* tgtManager);
  void Touch();
  float GetDesiredViewportScaleX(const CStateManager& stateMgr) const;
  float GetDesiredViewportScaleY(const CStateManager& stateMgr) const;
};

} // namespace MP1
} // namespace urde
