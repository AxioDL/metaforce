#ifndef _CSAVEGAMESCREEN
#define _CSAVEGAMESCREEN

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CIOWin.hpp"
#include "MetroidPrime/CMemoryCardDriver.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"
#include "types.h"

class CFinalInput;
class CTexture;
class CWorldSaveGameInfo;
class CStringTable;
class CGuiFrame;
class CGuiTextPane;
class CGuiTableGroup;

enum ESaveContext { kSC_FrontEnd, kSC_InGame };

class CSaveGameScreen {
public:
  enum EUIType {
    kUIT_Empty = 0,
    kUIT_BusyReading,
    kUIT_BusyWriting,
    kUIT_NoCardFound,
    kUIT_NeedsFormatBroken,
    kUIT_NeedsFormatEncoding,
    kUIT_CardDamaged,
    kUIT_WrongDevice,
#if VERSION < VERSION_GM8P_00
    kUIT_InsufficientSpaceBadCheck,
#endif
    kUIT_InsufficientSpaceOKCheck,
    kUIT_IncompatibleCard,
    kUIT_SaveCorrupt,
#if VERSION < VERSION_GM8P_00
    kUIT_StillInsufficientSpace,
#endif
    kUIT_ProgressWillBeLost,
    kUIT_NotOriginalCard,
    kUIT_AllDataWillBeLost,
    kUIT_SaveReady
  };

  CSaveGameScreen(ESaveContext saveContext, u64 cardSerial);
  ~CSaveGameScreen();
  CIOWin::EMessageReturn Update(float dt);
  bool PumpLoad();

  EUIType GetUIType() const { return mUiType; }
  bool IsSavingDisabled() const { return mSavingDisabled; }
  const CGameState::GameFileStateInfo* GetGameData(int idx) const;
  void EraseGame(int idx);
  void StartGame(int idx);
  void ResetCardDriver();
  void ProcessUserInput(const CFinalInput& input);
#if VERSION == VERSION_GM8J_00
  void Draw();
#else
  void Draw() const;
#endif
  static bool IsHiddenFromFrontEnd(EUIType type);
  void SetInGame(bool inGame);
  void SaveNESState();
  static CMemoryCardDriver* ConstructCardDriver(bool importPersistent);
  EUIType SelectUIType() const;
  void SetUIText();
  void SetUIColors();
  void DoAdvance(CGuiTableGroup* caller);
  void DoSelectionChange(CGuiTableGroup* caller, int oldSelection);

private:
  void ContinueWithoutSaving();
  ESaveContext mSaveCtx;
  u64 mSerial;
  EUIType mUiType;
  TCachedToken< CTexture > mTxtrSaveBanner;
  TCachedToken< CTexture > mTxtrSaveIcon0;
  TCachedToken< CTexture > mTxtrSaveIcon1;
  TCachedToken< CStringTable > mStrgMemoryCard;
  TCachedToken< CGuiFrame > mFrmeGenericMenu;
  CGuiFrame* mLoadedFrame;
  CGuiTextPane* mTextpane_message;
  CGuiTableGroup* mTablegroup_choices;
  CGuiTextPane* mTextpane_choice0;
  CGuiTextPane* mTextpane_choice1;
  CGuiTextPane* mTextpane_choice2;
  CGuiTextPane* mTextpane_choice3;
  rstl::single_ptr< CMemoryCardDriver > mCardDriver;
  rstl::vector< TToken< CWorldSaveGameInfo > > mSaveWorlds;
  CIOWin::EMessageReturn mIowRet;
  uint mNavConfirmSfx;
  uint mNavMoveSfx;
  uint mNavBackSfx;
  bool mNeedsDriverReset;
  bool mUiTextDirty;
  bool mSavingDisabled;
  bool mInGame;
};
CHECK_SIZEOF(CSaveGameScreen, 0x98)

#endif // _CSAVEGAMESCREEN
