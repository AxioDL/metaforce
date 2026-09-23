#ifndef _CAUTOMAPPER
#define _CAUTOMAPPER

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

#include "MetroidPrime/CInGameGuiManagerCommon.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CGuiFrame;
class CGuiTextPane;
class CGuiWidget;
class CMapUniverse;
class CMapWorldInfo;
class CModel;
class CStateManager;
class CStringTable;
class CTexture;
class CUnitVector3f;
class IWorld;
class CFinalInput;

class CAutoMapper {
public:
  enum ELoadPhase {
    kLP_LoadResources,
    kLP_LoadUniverse,
    kLP_Done,
  };

  enum EAutoMapperState {
    kAMS_MiniMap,
    kAMS_MapScreen,
    kAMS_MapScreenUniverse,
  };

  enum EZoomState {
    kZS_None,
    kZS_In,
    kZS_Out,
  };

  struct SAutoMapperRenderState {
    enum Ease {
      kE_None,
      kE_Linear,
      kE_Out,
      kE_In,
      kE_InOut,
    };

    CVector2i x0_viewportSize;
    CQuaternion x8_camOrientation;
    float x18_camDist;
    float x1c_camAngle;
    CVector3f x20_areaPoint;
    float x2c_drawDepth1;
    float x30_drawDepth2;
    float x34_alphaSurfaceVisited;
    float x38_alphaOutlineVisited;
    float x3c_alphaSurfaceUnvisited;
    float x40_alphaOutlineUnvisited;
    Ease x44_viewportEase;
    Ease x48_camEase;
    Ease x4c_pointEase;
    Ease x50_depth1Ease;
    Ease x54_depth2Ease;
    Ease x58_alphaEase;

    SAutoMapperRenderState(const SAutoMapperRenderState& other);
    SAutoMapperRenderState(const CVector2i& viewportSize, const CQuaternion& camOrientation,
                           float camDist, float camAngle, const CVector3f& areaPoint,
                           float drawDepth1, float drawDepth2, float alphaSurfaceVisited,
                           float alphaOutlineVisited, float alphaSurfaceUnvisited,
                           float alphaOutlineUnvisited);

    void ResetInterpolation();
    static void InterpolateWithClamp(const SAutoMapperRenderState& a, SAutoMapperRenderState& out,
                                     const SAutoMapperRenderState& b, float t);
  };

  struct SAutoMapperHintStep {
    enum Type {
      kHST_PanToArea,
      kHST_PanToWorld,
      kHST_SwitchToUniverse,
      kHST_SwitchToWorld,
      kHST_ShowBeacon,
      kHST_ZoomIn,
      kHST_ZoomOut,
    };

    union Data {
      CAssetId x0_worldId;
      int x0_areaId;
      float x0_float;

      Data(int value) : x0_areaId(value) {}

      Data(float value) : x0_float(value) {}
    };

    Type x0_type;
    Data x4_data;
    bool x8_processing;

    SAutoMapperHintStep(Type type, int data) : x0_type(type), x4_data(data), x8_processing(false) {}

    SAutoMapperHintStep(Type type, const float& data)
    : x0_type(type), x4_data(data), x8_processing(false) {}
  };

  struct SAutoMapperHintLocation {
    int x0_showBeacon;
    float x4_beaconAlpha;
    CAssetId x8_worldId;
    TAreaId xc_areaId;

    SAutoMapperHintLocation(uint showBeacon, float beaconAlpha, CAssetId worldId, int areaId);
  };

  // Virtuals
  virtual ~CAutoMapper();

  // Public methods
  explicit CAutoMapper(const CStateManager& stateMgr);
  bool CheckLoadComplete();
  bool CanLeaveMapScreen(const CStateManager& mgr) const;
  float GetMapRotationX() const { return xa8_renderState0.x1c_camAngle; }
  TAreaId GetFocusAreaIndex() const { return xa0_curAreaId; }
  void SetCurWorldAssetId(int mlvlId);
  void MuteAllLoopedSounds();
  void UnmuteAllLoopedSounds();
  void ProcessControllerInput(const CFinalInput& input, const CStateManager& mgr);
  bool IsInPlayerControlState() const {
    return IsInMapperState(kAMS_MapScreen) || IsInMapperState(kAMS_MapScreenUniverse);
  }
  void Update(float dt, const CStateManager& mgr);
  void Draw(const CStateManager& mgr, const CTransform4f& xf, float alpha) const;
  float GetTimeIntoInterpolation() const { return x1c8_interpTime; }
  void BeginMapperStateTransition(EAutoMapperState state, const CStateManager& mgr);
  void CompleteMapperStateTransition(const CStateManager& mgr);
  void ResetInterpolationTimer(float duration);
  SAutoMapperRenderState BuildMiniMapWorldRenderState(const CStateManager& stateMgr,
                                                      const CQuaternion& rot, int area) const;
  SAutoMapperRenderState BuildMapScreenWorldRenderState(const CStateManager& mgr,
                                                        const CQuaternion& rot, int area,
                                                        bool doingHint) const;
  SAutoMapperRenderState BuildMapScreenUniverseRenderState(const CStateManager& mgr,
                                                           const CQuaternion& rot, int area) const;
  void LeaveMapScreenState();
  void ProcessMapScreenInput(const CFinalInput& input, const CStateManager& mgr);
  static CQuaternion GetMiniMapCameraOrientation(const CStateManager& stateMgr);
  CVector3f GetAreaPointOfInterest(const CStateManager& mgr, int aid) const;

  int FindClosestVisibleArea(const CVector3f& point, const CUnitVector3f& camDir,
                             const CStateManager& mgr, const IWorld& wld,
                             const CMapWorldInfo& mwInfo) const;
  rstl::pair< int, int > FindClosestVisibleWorld(const CVector3f& point,
                                                 const CUnitVector3f& camDir,
                                                 const CStateManager& mgr) const;
  EAutoMapperState GetCurrentState() const { return x1bc_state; }
  EAutoMapperState GetNextState() const { return x1c0_nextState; }
  bool IsInMapperState(EAutoMapperState state) const;
  bool IsInMapperStateTransition() const;
  bool IsRenderStateInterpolating() const;
  bool IsFullyInMiniMapState() const { return IsInMapperState(kAMS_MiniMap); }
  bool IsFullyOutOfMiniMapState() const {
    return x1bc_state != kAMS_MiniMap && x1c0_nextState != kAMS_MiniMap;
  }
  void OnNewInGameGuiState(EInGameGuiState state, const CStateManager& mgr);
  float GetInterp() const {
    if (x1c4_interpDur > 0.f)
      return x1c8_interpTime / x1c4_interpDur;
    return 0.f;
  }

private:
  bool NotHintNavigating(const CStateManager& mgr) const;
  bool CanLeaveMapScreenInternal(const CStateManager& mgr) const;
  void LeaveMapScreen(const CStateManager& mgr);
  void SetupMiniMapWorld(CStateManager& mgr);
  bool HasCurrentMapUniverseWorld(const CStateManager& mgr);
  bool CheckDummyWorldLoad(const CStateManager& mgr);
  void UpdateHintNavigation(float dt, const CStateManager& mgr);
  static CVector2i GetMiniMapViewportSize();
  static CVector2i GetMapScreenViewportSize();
  static float GetMapAreaMiniMapDrawDepth();
  float GetMapAreaMaxDrawDepth(const CStateManager& mgr, int aid) const;
  static float GetMapAreaMiniMapDrawAlphaSurfaceVisited(const CStateManager& mgr);
  static float GetMapAreaMiniMapDrawAlphaOutlineVisited(const CStateManager& mgr);
  static float GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(const CStateManager& mgr);
  static float GetMapAreaMiniMapDrawAlphaOutlineUnvisited(const CStateManager& mgr);
  float GetDesiredMiniMapCameraDistance(const CStateManager& mgr) const;
  float GetBaseMapScreenCameraMoveSpeed() const;
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
  void TransformRenderStateWorldToUniverse(SAutoMapperRenderState& state);
  void SetupHintNavigation();
  CAssetId GetAreaHintDescriptionString(CAssetId mreaId);

  template < class T >
  void SetResLockState(T& list, bool lock);

  ELoadPhase x4_loadPhase;
  TCachedToken< CMapUniverse > x8_mapu;
  rstl::vector< rstl::auto_ptr< IWorld > > x14_dummyWorlds;
  IWorld* x24_world;
  rstl::single_ptr< TCachedToken< CGuiFrame > > x28_frmeMapScreen;
  CGuiFrame* x2c_frmeInitialized;
  TCachedToken< CModel > x30_miniMapSamus;
  TCachedToken< CTexture > x3c_hintBeacon;
  rstl::reserved_vector< CToken, 5 > x48_mapIcons;
  CAssetId x74_areaHintDescId;
  rstl::optional_object< TCachedToken< CStringTable > > x78_areaHintDesc;
  CAssetId x88_mapAreaStringId;
  rstl::optional_object< TCachedToken< CStringTable > > x8c_mapAreaString;
  int x9c_worldIdx;
  TAreaId xa0_curAreaId;
  TAreaId xa4_otherAreaId;
  SAutoMapperRenderState xa8_renderState0;
  SAutoMapperRenderState x104_renderState1;
  SAutoMapperRenderState x160_renderState2;
  EAutoMapperState x1bc_state;
  EAutoMapperState x1c0_nextState;
  float x1c4_interpDur;
  float x1c8_interpTime;
  CSfxHandle x1cc_panningSfx;
  CSfxHandle x1d0_rotatingSfx;
  CSfxHandle x1d4_zoomingSfx;
  float x1d8_flashTimer;
  float x1dc_playerFlashPulse;
  rstl::list< SAutoMapperHintStep > x1e0_hintSteps;
  rstl::list< SAutoMapperHintLocation > x1f8_hintLocations;
  rstl::reserved_vector< CToken, 9 > x210_lstick;
  rstl::reserved_vector< CToken, 9 > x25c_cstick;
  rstl::reserved_vector< CToken, 2 > x2a8_ltrigger;
  rstl::reserved_vector< CToken, 2 > x2bc_rtrigger;
  rstl::reserved_vector< CToken, 2 > x2d0_abutton;
  uint x2e4_lStickPos;
  uint x2e8_rStickPos;
  uint x2ec_lTriggerPos;
  uint x2f0_rTriggerPos;
  uint x2f4_aButtonPos;
  CGuiTextPane* x2f8_textpane_areaname;
  CGuiTextPane* x2fc_textpane_hint;
  CGuiTextPane* x300_textpane_instructions;
  CGuiTextPane* x304_textpane_instructions1;
  CGuiTextPane* x308_textpane_instructions2;
  CGuiWidget* x30c_basewidget_leftPane;
  CGuiWidget* x310_basewidget_yButtonPane;
  CGuiWidget* x314_basewidget_bottomPane;
  float x318_leftPanePos;
  float x31c_yButtonPanePos;
  float x320_bottomPanePos;
  EZoomState x324_zoomState;
  int x328_;
  bool x32c_loadingDummyWorld;
};
NESTED_CHECK_SIZEOF(CAutoMapper, SAutoMapperHintStep, 0xc)
NESTED_CHECK_SIZEOF(CAutoMapper, SAutoMapperHintLocation, 0x10)
CHECK_SIZEOF(CAutoMapper, 0x330)

#endif // _CAUTOMAPPER
