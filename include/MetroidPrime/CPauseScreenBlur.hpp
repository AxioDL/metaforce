#ifndef _CPAUSESCREENBLUR
#define _CPAUSESCREENBLUR

#include "MetroidPrime/Cameras/CCameraBlurPass.hpp"
#include "MetroidPrime/CInGameGuiManagerCommon.hpp"

#include "Kyoto/TToken.hpp"

class CTexture;
class CStateManager;

class CPauseScreenBlur {
public:
  enum EState { kS_InGame, kS_MapScreen, kS_SaveGame, kS_HUDMessage, kS_Pause };

  CPauseScreenBlur();
  virtual ~CPauseScreenBlur();

  void OnNewInGameGuiState(EInGameGuiState state, const CStateManager& stateMgr);
  bool IsGameDraw() const { return mGameDraw; }
  void Update(float dt, const CStateManager& stateMgr, bool);
  void Draw(const CStateManager& stateMgr);
  float GetBlurAmt() const;
  bool IsNotTransitioning() const { return mPrevState == mNextState; }

private:
  TLockedToken< CTexture > mMapLightQuarter;
  EState mPrevState;
  EState mNextState;
  float mBlurAmt;
  CCameraBlurPass mCamBlur;
  bool mBlurring : 1;
  bool mGameDraw : 1;

  void OnBlurComplete(bool);
  void SetState(EState state);
  float GetBlurAmtInline() const { return fabs(mBlurAmt); }
};

#endif // _CPAUSESCREENBLUR
