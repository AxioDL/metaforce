#pragma once

#include "Runtime/CToken.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/Shaders/CTexturedQuadFilter.hpp"
#include "Runtime/Graphics/Shaders/CScanLinesFilter.hpp"
#include "Runtime/MP1/CInGameGuiManagerCommon.hpp"

namespace urde {
class CStateManager;

namespace MP1 {

class CPauseScreenBlur {
  enum class EState { InGame, MapScreen, SaveGame, HUDMessage, Pause };

  TLockedToken<CTexture> x4_mapLightQuarter;
  EState x10_prevState = EState::InGame;
  EState x14_nextState = EState::InGame;
  float x18_blurAmt = 0.f;
  CCameraBlurPass x1c_camBlur;
  bool x50_24_blurring : 1 = false;
  bool x50_25_gameDraw : 1 = true;

  CTexturedQuadFilter m_quarterFilter{EFilterType::Multiply, x4_mapLightQuarter};
  CScanLinesFilterEven m_linesFilter{EFilterType::Multiply};

  void OnBlurComplete(bool);
  void SetState(EState state);

public:
  CPauseScreenBlur();
  void OnNewInGameGuiState(EInGameGuiState state, CStateManager& stateMgr);
  bool IsGameDraw() const { return x50_25_gameDraw; }
  void Update(float dt, const CStateManager& stateMgr, bool);
  void Draw(const CStateManager& stateMgr);
  float GetBlurAmt() const { return std::fabs(x18_blurAmt); }
  bool IsNotTransitioning() const { return x10_prevState == x14_nextState; }
};

} // namespace MP1
} // namespace urde
