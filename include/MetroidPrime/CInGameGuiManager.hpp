#ifndef _CINGAMEGUIMANAGER
#define _CINGAMEGUIMANAGER

#include "types.h"

#include "MetroidPrime/CInGameGuiManagerCommon.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Player/CFaceplateDecoration.hpp"
#include "MetroidPrime/SOnScreenTex.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/list.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CDependencyGroup;
class CPlayerVisor;
class CSamusHud;
class CAutoMapper;
class CPauseScreenBlur;
class CSamusFaceReflection;
class CMessageScreen;
class CPauseScreen;
class CSaveGameScreen;
class CGuiWidget;
class CGuiModel;
class CGuiCamera;

class CStateManager;
class CArchitectureQueue;
class CFinalInput;

class CInGameGuiManager {
public:
  CInGameGuiManager(const CStateManager& mgr, CArchitectureQueue& queue);
  ~CInGameGuiManager();

  bool GetIsGameDraw() const;

  void PreDraw(CStateManager& mgr, bool isCameraActive);
  void Draw(const CStateManager& mgr) const;
  void PauseGame(const CStateManager& mgr, EInGameGuiState state);
  void ShowPauseGameHudMessage(const CStateManager& mgr, CAssetId message, float time);
  void StartFadeIn();
  void Update(const CStateManager& mgr, float dt, CArchitectureQueue& queue, bool cameraActive);
  void ProcessControllerInput(const CStateManager& mgr, const CFinalInput& input,
                              CArchitectureQueue& queue);
  bool CheckLoadComplete(const CStateManager& mgr);
  bool IsInGameplayStateNotTransitioning() const {
    const bool ret = InGameGuiStates::IsGameplayState(mPrevState) &&
                     InGameGuiStates::IsGameplayState(mNextState);
    return ret;
  }
  const bool IsInOrTransitioningToOrFromState(EInGameGuiState state) const {
    return mPrevState == state || mNextState == state;
  }
  bool IsStateTransitioning() const { return mPrevState != mNextState; }
  bool IsInStateNotTransitioning(EInGameGuiState state) const {
    return mPrevState == state && mNextState == state;
  }
  bool IsInSaveUI() const { return mInSaveUI; }

private:
  enum ELoadPhase { kLP_LoadDepsGroup, kLP_PreLoadDeps, kLP_LoadDeps, kLP_Done };
  typedef rstl::reserved_vector< TToken< CDependencyGroup >, 13 > TPauseScreenDGRPs;
  typedef rstl::pair< CAssetId, TToken< CTexture > > TDumpedTexture;

  static TPauseScreenDGRPs LockPauseScreenDependencies();
  void InitializeDumpableARAMTextures();
  bool CheckDGRPLoadComplete();
  void BeginStateTransition(EInGameGuiState state, const CStateManager& mgr);
  void DoStateTransition(const CStateManager& mgr);
  void EnsureStates(const CStateManager& mgr);
  bool IsTextureInPauseScreen(CAssetId id) const;
  void DestroyAreaTextures(const CStateManager& mgr);
  const bool TryReloadAreaTextures();
  void UpdateAutoMapper(const CStateManager& mgr, float dt);
  void TryCompleteStateTransition(CArchitectureQueue& queue);
  void RefreshHudOptions();
  bool IsInPausedStateNotTransitioning() const {
    return InGameGuiStates::IsPausedState(mPrevState) &&
           InGameGuiStates::IsPausedState(mNextState);
  }

  TToken< CDependencyGroup > mIggmPreLoad;
  rstl::vector< CToken > mPreLoadDeps;
  ELoadPhase mLoadPhase;
  CRandom16 mRand;
  CFaceplateDecoration mFaceplateDecor;
  rstl::single_ptr< CPlayerVisor > mPlayerVisor;
  rstl::single_ptr< CSamusHud > mSamusHud;
  rstl::single_ptr< CAutoMapper > mAutoMapper;
  rstl::single_ptr< CPauseScreenBlur > mPauseScreenBlur;
  rstl::single_ptr< CSamusFaceReflection > mSamusReflection;
  rstl::single_ptr< CMessageScreen > mMessageScreen;
  rstl::single_ptr< CPauseScreen > mPauseScreen;
  rstl::single_ptr< CSaveGameScreen > mSaveUI;
  TCachedToken< CTexture > mDeathDot;
  TPauseScreenDGRPs mPauseScreenDGRPs;
  rstl::vector< TToken< CDependencyGroup > > mInGameGuiDGRPs;
  rstl::vector< CAssetId > mInGameTextureIDs;
  rstl::vector< CToken > mPauseResources;
  CCameraFilterPass mCamFilter;
  CAssetId mPauseGameHudMessage;
  float mPauseGameHudTime;
  rstl::list< TDumpedTexture > mDumpedTextures;
  CGuiWidget* mBasewidget_automapper;
  CGuiModel* mModel_automapper;
  CGuiCamera* mBasehud_camera;
  CGuiWidget* mBasewidget_functional;
  CQuaternion mAutomapperRotate;
  CVector3f mAutomapperOffset;
  CQuaternion mCamRotate;
  CVector3f mCamOffset;
  CTransform4f mMapCamXf;
  EInGameGuiState mPrevState;
  EInGameGuiState mNextState;
  SOnScreenTex mOnScreenTex;
  float mOnScreenTexAlpha;
  rstl::single_ptr< TCachedToken< CTexture > > mOnScreenTexTok;
  CTweakGui::EHelmetVisMode mHelmetVisMode;
  uint mEnableTargetingManager;
  uint mEnableAutoMapper;
  CTweakGui::EHudVisMode mHudVisMode;
  uint mEnablePlayerVisor;
  float mVisorStaticAlpha;
  bool x1f8_24_ : 1;
  bool mPlayerAlive : 1;
  bool mDeferTransition : 1;
  bool mInSaveUI : 1;
};
CHECK_SIZEOF(CInGameGuiManager, 0x1fc)
#endif
