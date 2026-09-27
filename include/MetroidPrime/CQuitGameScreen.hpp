#ifndef _CQUITGAMESCREEN
#define _CQUITGAMESCREEN

#include "GameVersions.h"
#include "Kyoto/TToken.hpp"

class CGuiFrame;
class CGuiTableGroup;
class CGuiTextPane;
class CFinalInput;

enum EQuitType {
  kQT_QuitGame,
  kQT_ContinueFromLastSave,
  kQT_SaveProgress,
  kQT_QuitNESMetroid,
  kQT_ContinuePlaying
};

enum EQuitAction { kQA_None, kQA_Yes, kQA_No };

class CQuitGameScreen {
public:
  CQuitGameScreen(EQuitType type);
#if VERSION >= VERSION_GM8P_00
  ~CQuitGameScreen();
#endif

  void SetColors();
  void FinishedLoading();
  void DoSelectionChange(CGuiTableGroup* caller, int oldSel);
  void DoAdvance(CGuiTableGroup* caller);
  EQuitAction Update(float dt);
#if VERSION == VERSION_GM8J_00
  void Draw();
#else
  void Draw() const;
#endif
  void ProcessUserInput(const CFinalInput& input);

private:
  EQuitType mType;
  TCachedToken< CGuiFrame > mFrame;
  CGuiFrame* mLoadedFrame;
  CGuiTableGroup* mTablegroup_quitgame;
  EQuitAction mAction;
#if VERSION >= VERSION_GM8P_00
  CGuiTextPane* mTextpane_title;
  CGuiTextPane* mTextpane_yes;
  CGuiTextPane* mTextpane_no;
#endif
};
#if VERSION >= VERSION_GM8P_00
CHECK_SIZEOF(CQuitGameScreen, 0x28)
#else
CHECK_SIZEOF(CQuitGameScreen, 0x1c)
#endif

#endif // _CQUITGAMESCREEN
