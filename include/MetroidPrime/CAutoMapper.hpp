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

    CVector2i mViewportSize;
    CQuaternion mCamOrientation;
    float mCamDist;
    float mCamAngle;
    CVector3f mAreaPoint;
    float mDrawDepth1;
    float mDrawDepth2;
    float mAlphaSurfaceVisited;
    float mAlphaOutlineVisited;
    float mAlphaSurfaceUnvisited;
    float mAlphaOutlineUnvisited;
    Ease mViewportEase;
    Ease mCamEase;
    Ease mPointEase;
    Ease mDepth1Ease;
    Ease mDepth2Ease;
    Ease mAlphaEase;

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
      CAssetId mWorldId;
      int mAreaId;
      float mFloat;

      Data(int value) : mAreaId(value) {}

      Data(float value) : mFloat(value) {}
    };

    Type mType;
    Data mData;
    bool mProcessing;

    SAutoMapperHintStep(Type type, int data) : mType(type), mData(data), mProcessing(false) {}

    SAutoMapperHintStep(Type type, const float& data)
    : mType(type), mData(data), mProcessing(false) {}
  };

  struct SAutoMapperHintLocation {
    int mShowBeacon;
    float mBeaconAlpha;
    CAssetId mWorldId;
    TAreaId mAreaId;

    SAutoMapperHintLocation(uint showBeacon, float beaconAlpha, CAssetId worldId, int areaId);
  };

  // Virtuals
  virtual ~CAutoMapper();

  // Public methods
  explicit CAutoMapper(const CStateManager& stateMgr);
  bool CheckLoadComplete();
  bool CanLeaveMapScreen(const CStateManager& mgr) const;
  float GetMapRotationX() const { return mRenderState0.mCamAngle; }
  TAreaId GetFocusAreaIndex() const { return mCurAreaId; }
  void SetCurWorldAssetId(int mlvlId);
  void MuteAllLoopedSounds();
  void UnmuteAllLoopedSounds();
  void ProcessControllerInput(const CFinalInput& input, const CStateManager& mgr);
  bool IsInPlayerControlState() const {
    return IsInMapperState(kAMS_MapScreen) || IsInMapperState(kAMS_MapScreenUniverse);
  }
  void Update(float dt, const CStateManager& mgr);
  void Draw(const CStateManager& mgr, const CTransform4f& xf, float alpha) const;
  float GetTimeIntoInterpolation() const { return mInterpTime; }
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
  EAutoMapperState GetCurrentState() const { return mState; }
  EAutoMapperState GetNextState() const { return mNextState; }
  bool IsInMapperState(EAutoMapperState state) const;
  bool IsInMapperStateTransition() const;
  bool IsRenderStateInterpolating() const;
  bool IsFullyInMiniMapState() const { return IsInMapperState(kAMS_MiniMap); }
  bool IsFullyOutOfMiniMapState() const {
    return mState != kAMS_MiniMap && mNextState != kAMS_MiniMap;
  }
  void OnNewInGameGuiState(EInGameGuiState state, const CStateManager& mgr);
  float GetInterp() const {
    if (mInterpDur > 0.f)
      return mInterpTime / mInterpDur;
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

  ELoadPhase mLoadPhase;
  TCachedToken< CMapUniverse > mMapu;
  rstl::vector< rstl::auto_ptr< IWorld > > mDummyWorlds;
  IWorld* mWorld;
  rstl::single_ptr< TCachedToken< CGuiFrame > > mFrmeMapScreen;
  CGuiFrame* mFrmeInitialized;
  TCachedToken< CModel > mMiniMapSamus;
  TCachedToken< CTexture > mHintBeacon;
  rstl::reserved_vector< CToken, 5 > mMapIcons;
  CAssetId mAreaHintDescId;
  rstl::optional_object< TCachedToken< CStringTable > > mAreaHintDesc;
  CAssetId mMapAreaStringId;
  rstl::optional_object< TCachedToken< CStringTable > > mMapAreaString;
  int mWorldIdx;
  TAreaId mCurAreaId;
  TAreaId mOtherAreaId;
  SAutoMapperRenderState mRenderState0;
  SAutoMapperRenderState mRenderState1;
  SAutoMapperRenderState mRenderState2;
  EAutoMapperState mState;
  EAutoMapperState mNextState;
  float mInterpDur;
  float mInterpTime;
  CSfxHandle mPanningSfx;
  CSfxHandle mRotatingSfx;
  CSfxHandle mZoomingSfx;
  float mFlashTimer;
  float mPlayerFlashPulse;
  rstl::list< SAutoMapperHintStep > mHintSteps;
  rstl::list< SAutoMapperHintLocation > mHintLocations;
  rstl::reserved_vector< CToken, 9 > mLstick;
  rstl::reserved_vector< CToken, 9 > mCstick;
  rstl::reserved_vector< CToken, 2 > mLtrigger;
  rstl::reserved_vector< CToken, 2 > mRtrigger;
  rstl::reserved_vector< CToken, 2 > mAbutton;
  uint mLStickPos;
  uint mRStickPos;
  uint mLTriggerPos;
  uint mRTriggerPos;
  uint mAButtonPos;
  CGuiTextPane* mTextpane_areaname;
  CGuiTextPane* mTextpane_hint;
  CGuiTextPane* mTextpane_instructions;
  CGuiTextPane* mTextpane_instructions1;
  CGuiTextPane* mTextpane_instructions2;
  CGuiWidget* mBasewidget_leftPane;
  CGuiWidget* mBasewidget_yButtonPane;
  CGuiWidget* mBasewidget_bottomPane;
  float mLeftPanePos;
  float mYButtonPanePos;
  float mBottomPanePos;
  EZoomState mZoomState;
  int x328_;
  bool mLoadingDummyWorld;
};
NESTED_CHECK_SIZEOF(CAutoMapper, SAutoMapperHintStep, 0xc)
NESTED_CHECK_SIZEOF(CAutoMapper, SAutoMapperHintLocation, 0x10)
CHECK_SIZEOF(CAutoMapper, 0x330)

#endif // _CAUTOMAPPER
