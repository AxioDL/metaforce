#pragma once

#include <memory>
#include <vector>

#include "Runtime/CIOWin.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/MP1/CMemoryCardDriver.hpp"
#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class CGuiTableGroup;
class CGuiTextPane;
class CWorldSaveGameInfo;
class CStringTable;
class CTexture;
struct CFinalInput;

namespace MP1 {

enum class ESaveContext { FrontEnd, InGame };

class CSaveGameScreen {
public:
  enum class EUIType {
    Empty = 0,
    BusyReading = 1,
    BusyWriting = 2,
    NoCardFound = 3,
    NeedsFormatBroken = 4,
    NeedsFormatEncoding = 5,
    CardDamaged = 6,
    WrongDevice = 7,
    InsufficientSpaceBadCheck = 8,
    InsufficientSpaceOKCheck = 9,
    IncompatibleCard = 10,
    SaveCorrupt = 11,
    StillInsufficientSpace = 12,
    ProgressWillBeLost = 13,
    NotOriginalCard = 14,
    AllDataWillBeLost = 15,
    SaveReady = 16,
    // Metaforce Addition
    CreateDolphinCardFailed
  };

  bool IsHiddenFromFrontEnd() const {
    switch (x10_uiType) {
    case EUIType::SaveReady:
    case EUIType::Empty:
    case EUIType::BusyReading:
    case EUIType::BusyWriting:
      return false;
    default:
      return true;
    }
  }

private:
  ESaveContext x0_saveCtx;
  u64 x8_serial;
  EUIType x10_uiType = EUIType::Empty;
  TLockedToken<CTexture> x14_txtrSaveBanner;
  TLockedToken<CTexture> x20_txtrSaveIcon0;
  TLockedToken<CTexture> x2c_txtrSaveIcon1;
  TLockedToken<CStringTable> x38_strgMemoryCard;
  TLockedToken<CGuiFrame> x44_frmeGenericMenu;
  CGuiFrame* x50_loadedFrame = nullptr;
  CGuiTextPane* x54_textpane_message{};
  CGuiTableGroup* x58_tablegroup_choices{};
  CGuiTextPane* x5c_textpane_choice0{};
  CGuiTextPane* x60_textpane_choice1{};
  CGuiTextPane* x64_textpane_choice2{};
  CGuiTextPane* x68_textpane_choice3{};
  std::unique_ptr<CMemoryCardDriver> x6c_cardDriver;
  std::vector<TLockedToken<CWorldSaveGameInfo>> x70_saveWorlds;
  CIOWin::EMessageReturn x80_iowRet = CIOWin::EMessageReturn::Normal;
  u32 x84_navConfirmSfx = SFXui_frontend_save_confirm;
  u32 x88_navMoveSfx = SFXui_frontend_save_move;
  u32 x8c_navBackSfx = SFXui_frontend_save_back;
  bool x90_needsDriverReset = false;
  bool x91_uiTextDirty = false;
  bool x92_savingDisabled = false;
  bool x93_inGame;

  void ContinueWithoutSaving();

public:
  static std::unique_ptr<CMemoryCardDriver> ConstructCardDriver(bool inGame);
  void ResetCardDriver();
  CIOWin::EMessageReturn Update(float dt);
  void SetInGame(bool v) { x93_inGame = v; }
  bool PumpLoad();
  [[nodiscard]] EUIType SelectUIType() const;
  void SetUIText();
  void SetUIColors();
  void Draw() const;

  void DoAdvance(CGuiTableGroup* caller);
  void DoSelectionChange(CGuiTableGroup* caller, int oldSel);

  void ProcessUserInput(const CFinalInput& input);
  void StartGame(int idx);
  void SaveNESState();
  void EraseGame(int idx);
  [[nodiscard]] const CGameState::GameFileStateInfo* GetGameData(int idx) const;
  [[nodiscard]] EUIType GetUIType() const { return x10_uiType; }
  [[nodiscard]] bool IsSavingDisabled() const { return x92_savingDisabled; }
  explicit CSaveGameScreen(ESaveContext saveCtx, u64 serial);
};

} // namespace MP1
} // namespace metaforce
