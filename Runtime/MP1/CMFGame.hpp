#pragma once

#include "CMFGameBase.hpp"
#include "CInGameGuiManager.hpp"
#include "Graphics/Shaders/CColoredQuadFilter.hpp"

namespace urde {
class CStateManager;
class CInGameGuiManager;
class CToken;

namespace MP1 {

enum class EGameFlowState { InGame = 0, Paused, SamusDied, CinematicSkip };

class CMFGame : public CMFGameBase {
  std::shared_ptr<CStateManager> x14_stateManager;
  std::shared_ptr<CInGameGuiManager> x18_guiManager;
  EGameFlowState x1c_flowState = EGameFlowState::InGame;
  float x20_cineSkipTime;
  u32 x24_ = 0;
  TUniqueId x28_skippedCineCam = kInvalidUniqueId;
  union {
    struct {
      bool x2a_24_initialized : 1;
      bool x2a_25_samusAlive : 1;
    };
    u8 _dummy = 0;
  };

  CColoredQuadFilter m_fadeToBlack = {EFilterType::Multiply};

  bool IsCameraActiveFlow() const {
    return (x1c_flowState == EGameFlowState::InGame || x1c_flowState == EGameFlowState::SamusDied);
  }

public:
  CMFGame(const std::weak_ptr<CStateManager>& stateMgr, const std::weak_ptr<CInGameGuiManager>& guiMgr,
          const CArchitectureQueue&);
  ~CMFGame();
  CIOWin::EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
  void Touch();
  void Draw() const;
  void PlayerDied();
  void UnpauseGame();
  void EnterMessageScreen(float time);
  void SaveGame();
  void EnterLogBook();
  void PauseGame();
  void EnterMapScreen();
};

class CMFGameLoader : public CMFGameLoaderBase {
  std::shared_ptr<CStateManager> x14_stateMgr;
  std::shared_ptr<CInGameGuiManager> x18_guiMgr;
  std::vector<CToken> x1c_loadList;

  union {
    struct {
      bool x2c_24_initialized : 1;
      bool x2c_25_transitionFinished : 1;
    };
    u8 _dummy = 0;
  };

  void MakeLoadDependencyList();

public:
  CMFGameLoader();
  ~CMFGameLoader();
  EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
  void Draw() const;
};

} // namespace MP1
} // namespace urde
