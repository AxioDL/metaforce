#pragma once

#include <list>
#include <memory>
#include <optional>
#include <vector>

#include "DataSpec/DNACommon/Tweaks/ITweakGui.hpp"

#include "Runtime/CDependencyGroup.hpp"
#include "Runtime/CRandom16.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/Shaders/CRandomStaticFilter.hpp"
#include "Runtime/MP1/CFaceplateDecoration.hpp"
#include "Runtime/MP1/CInGameGuiManagerCommon.hpp"
#include "Runtime/MP1/CInventoryScreen.hpp"
#include "Runtime/MP1/CMessageScreen.hpp"
#include "Runtime/MP1/CPauseScreen.hpp"
#include "Runtime/MP1/CPauseScreenBlur.hpp"
#include "Runtime/MP1/CPlayerVisor.hpp"
#include "Runtime/MP1/CSamusFaceReflection.hpp"
#include "Runtime/MP1/CSaveGameScreen.hpp"

#include <zeus/CQuaternion.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CActorLights;
class CArchitectureQueue;
class CAutoMapper;
class CGuiCamera;
class CGuiModel;
class CModelData;
class CStateManager;

namespace MP1 {
class CPauseScreen;
class CPauseScreenBlur;
class CSamusHud;

class CInGameGuiManager {
public:
  using EHelmetVisMode = DataSpec::ITweakGui::EHelmetVisMode;
  using EHudVisMode = DataSpec::ITweakGui::EHudVisMode;

private:
  enum class ELoadPhase { LoadDepsGroup = 0, PreLoadDeps, LoadDeps, Done };

  struct SGuiProfileInfo {};

  TLockedToken<CDependencyGroup> x0_iggmPreLoad;
  std::vector<CToken> x8_preLoadDeps;
  ELoadPhase x18_loadPhase = ELoadPhase::LoadDepsGroup;
  CRandom16 x1c_rand;
  CFaceplateDecoration x20_faceplateDecor;
  std::unique_ptr<CPlayerVisor> x30_playerVisor;
  std::unique_ptr<CSamusHud> x34_samusHud;
  std::unique_ptr<CAutoMapper> x38_autoMapper;
  std::unique_ptr<CPauseScreenBlur> x3c_pauseScreenBlur;
  std::unique_ptr<CSamusFaceReflection> x40_samusReflection;
  std::unique_ptr<CMessageScreen> x44_messageScreen;
  std::unique_ptr<CPauseScreen> x48_pauseScreen;
  std::unique_ptr<CSaveGameScreen> x4c_saveUI;
  TLockedToken<CTexture> x50_deathDot;
  std::vector<TLockedToken<CDependencyGroup>> x5c_pauseScreenDGRPs;
  std::vector<TLockedToken<CDependencyGroup>> xc8_inGameGuiDGRPs;
  std::vector<u32> xd8_;
  std::vector<CToken> xe8_pauseResources;
  CCameraFilterPass<CColoredQuadFilter> xf8_camFilter;
  CAssetId x124_pauseGameHudMessage;
  float x128_pauseGameHudTime = 0.f;
  std::list<CToken> x12c_;
  u32 x140_ = 0;
  CGuiWidget* x144_basewidget_automapper = nullptr;
  CGuiModel* x148_model_automapper = nullptr;
  CGuiCamera* x14c_basehud_camera = nullptr;
  CGuiWidget* x150_basewidget_functional = nullptr;
  zeus::CQuaternion x154_automapperRotate;
  zeus::CVector3f x164_automapperOffset;
  zeus::CQuaternion x170_camRotate;
  zeus::CVector3f x180_camOffset;
  zeus::CTransform x18c_mapCamXf;
  EInGameGuiState x1bc_prevState = EInGameGuiState::Zero;
  EInGameGuiState x1c0_nextState = EInGameGuiState::Zero;
  SOnScreenTex x1c4_onScreenTex;
  float x1d8_onScreenTexAlpha = 0.f;
  TLockedToken<CTexture> x1dc_onScreenTexTok; // Used to be heap-allocated
  std::optional<CTexturedQuadFilterAlpha> m_onScreenQuad;
  EHelmetVisMode x1e0_helmetVisMode;
  bool x1e4_enableTargetingManager;
  bool x1e8_enableAutoMapper;
  EHudVisMode x1ec_hudVisMode;
  u32 x1f0_enablePlayerVisor;
  float x1f4_visorStaticAlpha;
  bool x1f8_24_ : 1 = false;
  bool x1f8_25_playerAlive : 1 = true;
  bool x1f8_26_deferTransition : 1 = false;
  bool x1f8_27_exitSaveUI : 1 = true;

  std::optional<CTexturedQuadFilter> m_deathRenderTexQuad;
  std::optional<CTexturedQuadFilter> m_deathDotQuad;
  CRandomStaticFilter m_randomStatic{EFilterType::Blend};
  CColoredQuadFilter m_deathWhiteout{EFilterType::Blend};
  CColoredQuadFilter m_deathBlackout{EFilterType::Blend};

  static std::vector<TLockedToken<CDependencyGroup>> LockPauseScreenDependencies();
  bool CheckDGRPLoadComplete() const;
  void BeginStateTransition(EInGameGuiState state, CStateManager& stateMgr);
  void EnsureStates(CStateManager& stateMgr);
  void DoStateTransition(CStateManager& stateMgr);
  void DestroyAreaTextures(CStateManager& stateMgr);
  void TryReloadAreaTextures();
  bool IsInGameStateNotTransitioning() const;
  bool IsInPausedStateNotTransitioning() const;
  void UpdateAutoMapper(float dt, CStateManager& stateMgr);
  void OnNewPauseScreenState(CArchitectureQueue& archQueue);
  void RefreshHudOptions();

public:
  explicit CInGameGuiManager(CStateManager& stateMgr, CArchitectureQueue& archQueue);
  bool CheckLoadComplete(CStateManager& stateMgr);
  void Update(CStateManager& stateMgr, float dt, CArchitectureQueue& archQueue, bool useHud);
  void ProcessControllerInput(CStateManager& stateMgr, const CFinalInput& input, CArchitectureQueue& archQueue);
  void PreDraw(CStateManager& stateMgr, bool cameraActive);
  void Draw(CStateManager& stateMgr);
  void ShowPauseGameHudMessage(CStateManager& stateMgr, CAssetId pauseMsg, float time);
  void PauseGame(CStateManager& stateMgr, EInGameGuiState state);
  void StartFadeIn();
  bool WasInGame() const {
    return x1bc_prevState >= EInGameGuiState::Zero && x1bc_prevState <= EInGameGuiState::InGame;
  }
  bool IsInGame() const { return x1c0_nextState >= EInGameGuiState::Zero && x1c0_nextState <= EInGameGuiState::InGame; }
  bool IsInSaveUI() const { return x1f8_27_exitSaveUI; }
  bool GetIsGameDraw() const;
};

} // namespace MP1
} // namespace urde
