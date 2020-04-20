#pragma once

#include <array>
#include <list>
#include <memory>
#include <optional>
#include <vector>

#include "Runtime/rstl.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/AutoMapper/CMapUniverse.hpp"
#include "Runtime/MP1/CInGameGuiManager.hpp"

#include <zeus/CQuaternion.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector2i.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CMapWorldInfo;
class CStateManager;
class IWorld;

struct CFinalInput;

class CAutoMapper {
public:
  using EInGameGuiState = MP1::EInGameGuiState;
  enum class ELoadPhase { LoadResources, LoadUniverse, Done };
  enum class EAutoMapperState { MiniMap, MapScreen, MapScreenUniverse };
  struct SAutoMapperRenderState {
    enum class Ease { None, Linear, Out, In, InOut };
    using FGetViewportSize = zeus::CVector2i (*)();

    FGetViewportSize m_getViewportSize = nullptr;
    zeus::CVector2i x0_viewportSize;
    zeus::CQuaternion x8_camOrientation;
    float x18_camDist = 0.0f;
    float x1c_camAngle = 0.0f;
    zeus::CVector3f x20_areaPoint;
    float x2c_drawDepth1 = 0.0f;
    float x30_drawDepth2 = 0.0f;
    float x34_alphaSurfaceVisited = 0.0f;
    float x38_alphaOutlineVisited = 0.0f;
    float x3c_alphaSurfaceUnvisited = 0.0f;
    float x40_alphaOutlineUnvisited = 0.0f;
    Ease x44_viewportEase = Ease::None;
    Ease x48_camEase = Ease::None;
    Ease x4c_pointEase = Ease::None;
    Ease x50_depth1Ease = Ease::None;
    Ease x54_depth2Ease = Ease::None;
    Ease x58_alphaEase = Ease::None;
    SAutoMapperRenderState() = default;
    SAutoMapperRenderState(FGetViewportSize getViewportSize, const zeus::CQuaternion& camOrientation, float camDist,
                           float camAngle, const zeus::CVector3f& areaPoint, float drawDepth1, float drawDepth2,
                           float alphaSurfaceVisited, float alphaOutlineVisited, float alphaSurfaceUnvisited,
                           float alphaOutlineUnvisited)
    : m_getViewportSize(getViewportSize)
    , x0_viewportSize(getViewportSize())
    , x8_camOrientation(camOrientation)
    , x18_camDist(camDist)
    , x1c_camAngle(camAngle)
    , x20_areaPoint(areaPoint)
    , x2c_drawDepth1(drawDepth1)
    , x30_drawDepth2(drawDepth2)
    , x34_alphaSurfaceVisited(alphaSurfaceVisited)
    , x38_alphaOutlineVisited(alphaOutlineVisited)
    , x3c_alphaSurfaceUnvisited(alphaSurfaceUnvisited)
    , x40_alphaOutlineUnvisited(alphaOutlineUnvisited) {}

    static void InterpolateWithClamp(const SAutoMapperRenderState& a, SAutoMapperRenderState& out,
                                     const SAutoMapperRenderState& b, float t);
    void ResetInterpolation() {
      x44_viewportEase = Ease::None;
      x48_camEase = Ease::None;
      x4c_pointEase = Ease::None;
      x50_depth1Ease = Ease::None;
      x54_depth2Ease = Ease::None;
      x58_alphaEase = Ease::None;
    }

    zeus::CVector2i GetViewportSize() const {
      if (m_getViewportSize)
        return m_getViewportSize();
      else
        return x0_viewportSize;
    }
  };

  struct SAutoMapperHintStep {
    enum class Type { PanToArea, PanToWorld, SwitchToUniverse, SwitchToWorld, ShowBeacon, ZoomIn, ZoomOut };
    struct PanToArea {};
    struct PanToWorld {};
    struct SwitchToUniverse {};
    struct SwitchToWorld {};
    struct ShowBeacon {};
    struct ZoomIn {};
    struct ZoomOut {};

    Type x0_type;
    union {
      CAssetId x4_worldId;
      TAreaId x4_areaId;
      float x4_float;
    };
    bool x8_processing = false;

    SAutoMapperHintStep(PanToArea, TAreaId areaId) : x0_type(Type::PanToArea), x4_areaId(areaId) {}
    SAutoMapperHintStep(PanToWorld, CAssetId worldId) : x0_type(Type::PanToWorld), x4_worldId(worldId) {}
    SAutoMapperHintStep(SwitchToUniverse) : x0_type(Type::SwitchToUniverse), x4_worldId(0) {}
    SAutoMapperHintStep(SwitchToWorld, CAssetId worldId) : x0_type(Type::SwitchToWorld), x4_worldId(worldId) {}
    SAutoMapperHintStep(ShowBeacon, float val) : x0_type(Type::ShowBeacon), x4_float(val) {}
    SAutoMapperHintStep(ZoomIn) : x0_type(Type::ZoomIn), x4_worldId(0) {}
    SAutoMapperHintStep(ZoomOut) : x0_type(Type::ZoomOut), x4_worldId(0) {}
  };

  struct SAutoMapperHintLocation {
    u32 x0_showBeacon;
    float x4_beaconAlpha;
    CAssetId x8_worldId;
    TAreaId xc_areaId;
  };

private:
  enum class EZoomState { None, In, Out };

  ELoadPhase x4_loadPhase = ELoadPhase::LoadResources;
  TLockedToken<CMapUniverse> x8_mapu;
  std::vector<std::unique_ptr<IWorld>> x14_dummyWorlds;
  CWorld* x24_world;
  TLockedToken<CGuiFrame> x28_frmeMapScreen; // Used to be ptr
  bool m_frmeInitialized = false;
  TLockedToken<CModel> x30_miniMapSamus;
  TLockedToken<CTexture> x3c_hintBeacon;
  std::vector<CTexturedQuadFilter> m_hintBeaconFilters;
  rstl::reserved_vector<TLockedToken<CTexture>, 5> x48_mapIcons;
  CAssetId x74_areaHintDescId;
  TLockedToken<CStringTable> x78_areaHintDesc;
  CAssetId x88_mapAreaStringId;
  TLockedToken<CStringTable> x8c_mapAreaString; // Used to be optional
  u32 x9c_worldIdx = 0;
  TAreaId xa0_curAreaId;
  TAreaId xa4_otherAreaId;
  std::array<SAutoMapperRenderState, 3> xa8_renderStates; // xa8, x104, x160; current, next, prev
  EAutoMapperState x1bc_state = EAutoMapperState::MiniMap;
  EAutoMapperState x1c0_nextState = EAutoMapperState::MiniMap;
  float x1c4_interpDur = 0.f;
  float x1c8_interpTime = 0.f;
  CSfxHandle x1cc_panningSfx;
  CSfxHandle x1d0_rotatingSfx;
  CSfxHandle x1d4_zoomingSfx;
  float x1d8_flashTimer = 0.f;
  float x1dc_playerFlashPulse = 0.f;
  std::list<SAutoMapperHintStep> x1e0_hintSteps;
  std::list<SAutoMapperHintLocation> x1f8_hintLocations;
  rstl::reserved_vector<TLockedToken<CTexture>, 9> x210_lstick;
  rstl::reserved_vector<TLockedToken<CTexture>, 9> x25c_cstick;
  rstl::reserved_vector<TLockedToken<CTexture>, 2> x2a8_ltrigger;
  rstl::reserved_vector<TLockedToken<CTexture>, 2> x2bc_rtrigger;
  rstl::reserved_vector<TLockedToken<CTexture>, 2> x2d0_abutton;
  u32 x2e4_lStickPos = 0;
  u32 x2e8_rStickPos = 0;
  u32 x2ec_lTriggerPos = 0;
  u32 x2f0_rTriggerPos = 0;
  u32 x2f4_aButtonPos = 0;
  CGuiTextPane* x2f8_textpane_areaname = nullptr;
  CGuiTextPane* x2fc_textpane_hint = nullptr;
  CGuiTextPane* x300_textpane_instructions = nullptr;
  CGuiTextPane* x304_textpane_instructions1 = nullptr;
  CGuiTextPane* x308_textpane_instructions2 = nullptr;
  CGuiWidget* x30c_basewidget_leftPane = nullptr;
  CGuiWidget* x310_basewidget_yButtonPane = nullptr;
  CGuiWidget* x314_basewidget_bottomPane = nullptr;
  float x318_leftPanePos = 0.f;
  float x31c_yButtonPanePos = 0.f;
  float x320_bottomPanePos = 0.f;
  EZoomState x324_zoomState = EZoomState::None;
  u32 x328_ = 0;
  bool x32c_loadingDummyWorld = false;

  std::optional<zeus::CVector2f> m_lastMouseCoord;
  zeus::CVector2f m_mouseDelta;
  boo::SScrollDelta m_lastAccumScroll;
  boo::SScrollDelta m_mapScroll;

  template <class T>
  static void SetResLockState(T& list, bool lock) {
    for (auto& res : list)
      if (lock)
        res.Lock();
      else
        res.Unlock();
  }
  bool NotHintNavigating() const;
  bool CanLeaveMapScreenInternal(const CStateManager& mgr) const;
  void LeaveMapScreen(CStateManager& mgr);
  void SetupMiniMapWorld(CStateManager& mgr);
  bool HasCurrentMapUniverseWorld() const;
  bool CheckDummyWorldLoad(CStateManager& mgr);
  void UpdateHintNavigation(float dt, CStateManager& mgr);
  static zeus::CVector2i GetMiniMapViewportSize();
  static zeus::CVector2i GetMapScreenViewportSize();
  static float GetMapAreaMiniMapDrawDepth() { return 2.f; }
  float GetMapAreaMaxDrawDepth(const CStateManager& mgr, TAreaId aid) const;
  static float GetMapAreaMiniMapDrawAlphaSurfaceVisited(const CStateManager& mgr);
  static float GetMapAreaMiniMapDrawAlphaOutlineVisited(const CStateManager& mgr);
  static float GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(const CStateManager& mgr);
  static float GetMapAreaMiniMapDrawAlphaOutlineUnvisited(const CStateManager& mgr);
  float GetDesiredMiniMapCameraDistance(const CStateManager& mgr) const;
  static float GetBaseMapScreenCameraMoveSpeed();
  float GetClampedMapScreenCameraDistance(float value) const;
  float GetFinalMapScreenCameraMoveSpeed() const;
  void ProcessMapRotateInput(const CFinalInput& input, const CStateManager& mgr);
  void ProcessMapZoomInput(const CFinalInput& input, const CStateManager& mgr);
  void ProcessMapPanInput(const CFinalInput& input, const CStateManager& mgr);
  void SetShouldPanningSoundBePlaying(bool shouldBePlaying);
  void SetShouldZoomingSoundBePlaying(bool shouldBePlaying);
  void SetShouldRotatingSoundBePlaying(bool shouldBePlaying);
  void TransformRenderStatesWorldToUniverse();
  void TransformRenderStatesUniverseToWorld();
  void TransformRenderStateWorldToUniverse(SAutoMapperRenderState&);
  void SetupHintNavigation();
  CAssetId GetAreaHintDescriptionString(CAssetId mreaId);

public:
  explicit CAutoMapper(CStateManager& stateMgr);
  bool CheckLoadComplete();
  bool CanLeaveMapScreen(const CStateManager& mgr) const;
  float GetMapRotationX() const { return xa8_renderStates[0].x1c_camAngle; }
  float GetMapRotationZ() const { return xa8_renderStates[0].x8_camOrientation.yaw(); }
  TAreaId GetFocusAreaIndex() const { return xa0_curAreaId; }
  CAssetId GetCurrWorldAssetId() const { return x24_world->IGetWorldAssetId(); }
  void SetCurWorldAssetId(CAssetId mlvlId);
  void MuteAllLoopedSounds();
  void UnmuteAllLoopedSounds();
  void ProcessControllerInput(const CFinalInput& input, CStateManager& mgr);
  bool IsInPlayerControlState() const {
    return IsInMapperState(EAutoMapperState::MapScreen) || IsInMapperState(EAutoMapperState::MapScreenUniverse);
  }
  void Update(float dt, CStateManager& mgr);
  void Draw(const CStateManager& mgr, const zeus::CTransform& xf, float alpha);
  float GetTimeIntoInterpolation() const { return x1c8_interpTime; }
  void BeginMapperStateTransition(EAutoMapperState state, CStateManager& mgr);
  void CompleteMapperStateTransition(CStateManager& mgr);
  void ResetInterpolationTimer(float duration);
  SAutoMapperRenderState BuildMiniMapWorldRenderState(const CStateManager& stateMgr, const zeus::CQuaternion& rot,
                                                      TAreaId area) const;
  SAutoMapperRenderState BuildMapScreenWorldRenderState(const CStateManager& mgr, const zeus::CQuaternion& rot,
                                                        TAreaId area, bool doingHint) const;
  SAutoMapperRenderState BuildMapScreenUniverseRenderState(const CStateManager& mgr, const zeus::CQuaternion& rot,
                                                           TAreaId area) const;
  void LeaveMapScreenState();
  void ProcessMapScreenInput(const CFinalInput& input, CStateManager& mgr);
  zeus::CQuaternion GetMiniMapCameraOrientation(const CStateManager& stateMgr) const;
  zeus::CVector3f GetAreaPointOfInterest(const CStateManager& mgr, TAreaId aid) const;
  TAreaId FindClosestVisibleArea(const zeus::CVector3f& point, const zeus::CUnitVector3f& camDir,
                                 const CStateManager& mgr, const IWorld& wld, const CMapWorldInfo& mwInfo) const;
  std::pair<int, int> FindClosestVisibleWorld(const zeus::CVector3f& point, const zeus::CUnitVector3f& camDir,
                                              const CStateManager& mgr) const;

  EAutoMapperState GetNextState() const { return x1c0_nextState; }
  bool IsInMapperState(EAutoMapperState state) const { return state == x1bc_state && state == x1c0_nextState; }
  bool IsInMapperStateTransition() const { return x1c0_nextState != x1bc_state; }
  bool IsRenderStateInterpolating() const { return x1c8_interpTime < x1c4_interpDur; }
  bool IsStateTransitioning() const { return x1bc_state != x1c0_nextState; }
  bool IsFullyInMiniMapState() const { return IsInMapperState(EAutoMapperState::MiniMap); }
  bool IsFullyOutOfMiniMapState() const {
    return x1bc_state != EAutoMapperState::MiniMap && x1c0_nextState != EAutoMapperState::MiniMap;
  }
  void OnNewInGameGuiState(EInGameGuiState state, CStateManager& mgr);
  float GetInterp() const {
    if (x1c4_interpDur > 0.f)
      return x1c8_interpTime / x1c4_interpDur;
    return 0.f;
  }
};
} // namespace urde
