#pragma once

#include <array>
#include <memory>
#include <optional>

#include "Runtime/CGameDebug.hpp"
#include "Runtime/CGameOptionsTouchBar.hpp"
#include "Runtime/CIOWin.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/Audio/CStaticAudioPlayer.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/Shaders/CColoredQuadFilter.hpp"
#include "Runtime/Graphics/Shaders/CTexturedQuadFilter.hpp"
#include "Runtime/GuiSys/CGuiTextSupport.hpp"
#include "Runtime/Input/CRumbleGenerator.hpp"
#include "Runtime/MP1/CFrontEndUITouchBar.hpp"
#include "Runtime/MP1/CGBASupport.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
class CAudioGroupSet;
class CDependencyGroup;
class CGuiFrame;
class CGuiModel;
class CGuiSliderGroup;
class CGuiTableGroup;
class CGuiTableGroup;
class CGuiTextPane;
class CGuiWidget;
class CMoviePlayer;
class CSaveWorld;
class CStringTable;
class CTexture;
struct SObjectTag;

namespace MP1 {
class CNESEmulator;
class CSaveGameScreen;
class CQuitGameScreen;

class CFrontEndUI : public CIOWin {
public:
  enum class EPhase { LoadDepsGroup, LoadDeps, LoadFrames, LoadMovies, DisplayFrontEnd, ToPlayGame, ExitFrontEnd };
  enum class EScreen { OpenCredits, Title, AttractMovie, FileSelect, FusionBonus, ToPlayGame };
  enum class EMenuMovie {
    Stopped = -1,
    FirstStart = 0,
    StartLoop,
    StartFileSelectA,
    FileSelectLoop,
    FileSelectPlayGameA,
    FileSelectGBA,
    GBALoop,
    GBAFileSelectA,
    GBAFileSelectB
  };

  static void PlayAdvanceSfx();

  struct SGuiTextPair {
    std::array<CGuiTextPane*, 2> x0_panes{};
    void SetPairText(std::u16string_view str);
  };
  static SGuiTextPair FindTextPanePair(CGuiFrame* frame, std::string_view name);
  static void FindAndSetPairText(CGuiFrame* frame, std::string_view name, std::u16string_view str);

  struct SFileMenuOption {
    CGuiWidget* x0_base;

    /* filename, world, playtime, date */
    std::array<SGuiTextPair, 4> x4_textpanes;

    u32 x28_curField = 0;
    float x2c_chRate = ComputeRandom();

    static float ComputeRandom() { return rand() / float(RAND_MAX) * 30.f + 30.f; }
  };

  struct SNewFileSelectFrame {
    enum class ESubMenu { Root, EraseGame, EraseGamePopup, NewGamePopup };

    enum class EAction { None, GameOptions, FusionBonus, SlideShow };

    u32 x0_rnd;
    CSaveGameScreen* x4_saveUI;
    ESubMenu x8_subMenu = ESubMenu::Root;
    EAction xc_action = EAction::None;
    TLockedToken<CGuiFrame> x10_frme;
    CGuiFrame* x1c_loadedFrame = nullptr;
    CGuiTableGroup* x20_tablegroup_fileselect = nullptr;
    CGuiModel* x24_model_erase = nullptr;
    SGuiTextPair x28_textpane_erase;
    SGuiTextPair x30_textpane_cheats;
    SGuiTextPair x38_textpane_gba;
    CGuiTableGroup* x40_tablegroup_popup = nullptr;
    CGuiModel* x44_model_dash7 = nullptr;
    SGuiTextPair x48_textpane_popupadvance;
    SGuiTextPair x50_textpane_popupcancel;
    SGuiTextPair x58_textpane_popupextra;
    CGuiTextPane* x60_textpane_cancel = nullptr;
    std::array<SFileMenuOption, 3> x64_fileSelections;
    zeus::CVector3f xf8_model_erase_position;
    float x104_rowPitch = 0.f;
    float x108_curTime = 0.f;
    bool x10c_saveReady = false;
    bool x10d_needsEraseToggle = false;
    bool x10e_needsNewToggle = false;

    CFrontEndUITouchBar& m_touchBar;

    SNewFileSelectFrame(CSaveGameScreen* sui, u32 rnd, CFrontEndUITouchBar& touchBar);
    void FinishedLoading();
    bool PumpLoad();
    bool IsTextDoneAnimating() const;
    void Update(float dt);
    EAction ProcessUserInput(const CFinalInput& input, CFrontEndUITouchBar::EAction tbAction);
    void Draw() const;

    void HandleActiveChange(CGuiTableGroup* active);
    void DeactivateEraseGamePopup();
    void ActivateEraseGamePopup();
    void DeactivateNewGamePopup();
    void ActivateNewGamePopup();

    void ResetFrame();
    void ActivateErase();
    void ClearFrameContents();
    void SetupFrameContents();

    void DoPopupCancel(CGuiTableGroup* caller);
    void DoPopupAdvance(CGuiTableGroup* caller);
    void DoFileMenuCancel(CGuiTableGroup* caller);
    void DoSelectionChange(CGuiTableGroup* caller, int oldSel);
    void DoFileMenuAdvance(CGuiTableGroup* caller);

    static SFileMenuOption FindFileSelectOption(CGuiFrame* frame, int idx);
    static void StartTextAnimating(CGuiTextPane* text, std::u16string_view str, float chRate);
  };

  struct SFusionBonusFrame {
    struct SGBALinkFrame {
      enum class EUIType {
        Empty = -1,
        InsertPak = 0,
        ConnectSocket = 1,
        PressStartAndSelect = 2,
        BeginLink = 3,
        Linking = 4,
        LinkFailed = 5,
        LinkCompleteOrLinking = 6,
        TurnOffGBA = 7,
        Complete = 8,
        Cancelled = 9
      };

      enum class EAction { None = 0, Complete = 1, Cancelled = 2 };

      EUIType x0_uiType;
      CGBASupport* x4_gbaSupport;
      CGuiFrame* x8_frme;
      SGuiTextPair xc_textpane_instructions;
      CGuiTextPane* x14_textpane_yes = nullptr;
      CGuiTextPane* x18_textpane_no = nullptr;
      CGuiModel* x1c_model_gc = nullptr;
      CGuiModel* x20_model_gba = nullptr;
      CGuiModel* x24_model_cable = nullptr;
      CGuiModel* x28_model_circlegcport = nullptr;
      CGuiModel* x2c_model_circlegbaport = nullptr;
      CGuiModel* x30_model_circlestartselect = nullptr;
      CGuiModel* x34_model_pakout = nullptr;
      CGuiModel* x38_model_gbascreen = nullptr;
      CGuiModel* x3c_model_connect = nullptr;
      bool x40_linkInProgress;

      void SetUIText(EUIType tp);
      EAction ProcessUserInput(const CFinalInput& input, bool linkInProgress, CFrontEndUITouchBar::EAction tbAction);
      void Update(float dt);
      void FinishedLoading();
      void Draw();
      SGBALinkFrame(CGuiFrame* linkFrame, CGBASupport* support, bool linkInProgress);
    };

    enum class EAction { None, GoBack, PlayNESMetroid };

    std::unique_ptr<SGBALinkFrame> x0_gbaLinkFrame;
    std::unique_ptr<CGBASupport> x4_gbaSupport;
    EAction x8_action = EAction::None;
    TLockedToken<CGuiFrame> xc_gbaScreen;
    TLockedToken<CGuiFrame> x18_gbaLink;
    CGuiFrame* x24_loadedFrame = nullptr;
    CGuiTableGroup* x28_tablegroup_options = nullptr;
    CGuiTableGroup* x2c_tablegroup_fusionsuit = nullptr;
    SGuiTextPair x30_textpane_instructions;
    bool x38_lastDoDraw = false;
    bool x39_fusionNotComplete = false;
    bool x3a_mpNotComplete = false;

    CFrontEndUITouchBar& m_touchBar;

    bool m_gbaOverride = false;

    explicit SFusionBonusFrame(CFrontEndUITouchBar& touchBar);
    void FinishedLoading();
    bool PumpLoad();
    void SetTableColors(CGuiTableGroup* tbgp) const;
    void Update(float dt, CSaveGameScreen* saveUI);
    EAction ProcessUserInput(const CFinalInput& input, CSaveGameScreen* sui, CFrontEndUITouchBar::EAction tbAction);
    void Draw() const;

    void ResetCompletionFlags() {
      x39_fusionNotComplete = false;
      x3a_mpNotComplete = false;
    }

    void DoCancel(CGuiTableGroup* caller);
    void DoSelectionChange(CGuiTableGroup* caller, int oldSel);
    void DoAdvance(CGuiTableGroup* caller);
  };

  struct SFrontEndFrame {
    enum class EAction { None, StartGame, FusionBonus, GameOptions, SlideShow };

    u32 x0_rnd;
    EAction x4_action = EAction::None;
    TLockedToken<CGuiFrame> x8_frme;
    CGuiFrame* x14_loadedFrme = nullptr;
    CGuiTableGroup* x18_tablegroup_mainmenu = nullptr;
    SGuiTextPair x1c_gbaPair;
    SGuiTextPair x24_cheatPair;

    CFrontEndUITouchBar& m_touchBar;

    SFrontEndFrame(u32 rnd, CFrontEndUITouchBar& touchBar);
    void FinishedLoading();
    bool PumpLoad();
    void Update(float dt);
    EAction ProcessUserInput(const CFinalInput& input, CFrontEndUITouchBar::EAction tbAction);
    void Draw() const;
    void HandleActiveChange(CGuiTableGroup* active);

    void DoCancel(CGuiTableGroup* caller);
    void DoSelectionChange(CGuiTableGroup* caller, int oldSel);
    void DoAdvance(CGuiTableGroup* caller);
  };

  struct SNesEmulatorFrame {
    enum class EMode { Emulator, SaveProgress, ContinuePlaying, QuitNESMetroid };

    EMode x0_mode = EMode::Emulator;
    std::unique_ptr<CNESEmulator> x4_nesEmu;
    std::unique_ptr<CQuitGameScreen> x8_quitScreen;
    std::unique_ptr<CGuiTextSupport> xc_textSupport;
    float x10_remTime = 8.f;
    bool x14_emulationSuspended = false;
    bool x15_enableFiltering = true;

    SNesEmulatorFrame();
    void SetMode(EMode mode);
    void ProcessUserInput(const CFinalInput& input, CSaveGameScreen* sui);
    bool Update(float dt, CSaveGameScreen* saveUi);
    void Draw(CSaveGameScreen* saveUi) const;
  };

  struct SOptionsFrontEndFrame {
    float x0_uiAlpha = 0.f;
    TLockedToken<CGuiFrame> x4_frme;
    TLockedToken<CStringTable> x10_pauseScreen;
    CGuiFrame* x1c_loadedFrame = nullptr;
    CStringTable* x20_loadedPauseStrg = nullptr;
    CGuiTableGroup* x24_tablegroup_leftmenu = nullptr;
    CGuiTableGroup* x28_tablegroup_rightmenu = nullptr;
    CGuiTableGroup* x2c_tablegroup_double = nullptr;
    CGuiTableGroup* x30_tablegroup_triple = nullptr;
    CGuiSliderGroup* x34_slidergroup_slider = nullptr;
    float x38_rowPitch = 0.f;
    CSfxHandle x3c_sliderSfx;
    CRumbleGenerator x40_rumbleGen;
    bool x134_24_visible : 1 = true;
    bool x134_25_exitOptions : 1 = false;

    std::unique_ptr<CGameOptionsTouchBar> m_touchBar;
    bool m_touchBarInValue = false;
    bool m_touchBarValueDirty = false;

    SOptionsFrontEndFrame();

    void DoSliderChange(CGuiSliderGroup* caller, float value);
    void DoMenuCancel(CGuiTableGroup* caller);
    void DoMenuSelectionChange(CGuiTableGroup* caller, int oldSel);
    void DoLeftMenuAdvance(CGuiTableGroup* caller);

    void DeactivateRightMenu();
    void HandleRightSelectionChange();

    void SetRightUIText();
    void SetTableColors(CGuiTableGroup* tbgp) const;
    void FinishedLoading();
    bool PumpLoad();

    bool ProcessUserInput(const CFinalInput& input, CSaveGameScreen* sui);
    void Update(float dt, CSaveGameScreen* saveUi);
    void Draw() const;
  };

  bool CanShowSaveUI() const {
    if (x50_curScreen != EScreen::FileSelect && x50_curScreen != EScreen::FusionBonus)
      return false;
    if (x54_nextScreen != EScreen::FileSelect && x54_nextScreen != EScreen::FusionBonus)
      return false;
    return true;
  }

private:
  EPhase x14_phase = EPhase::LoadDepsGroup;
  u32 x18_rndA;
  u32 x1c_rndB;
  TLockedToken<CDependencyGroup> x20_depsGroup;
  std::vector<CToken> x2c_deps;
  TLockedToken<CTexture> x38_pressStart;
  TLockedToken<CAudioGroupSet> x44_frontendAudioGrp;
  EScreen x50_curScreen = EScreen::OpenCredits;
  EScreen x54_nextScreen = EScreen::OpenCredits;
  float x58_fadeBlackTimer = 0.f;
  bool x5c_fadeBlackWithMovie = false;
  float x60_pressStartTime = 0.f;
  float x64_pressStartAlpha = 0.f;
  float x68_musicVol = 1.f;
  u32 x6c_;
  std::array<std::unique_ptr<CMoviePlayer>, 9> x70_menuMovies;
  EMenuMovie xb8_curMovie = EMenuMovie::Stopped;
  int xbc_nextAttract = 0;
  int xc0_attractCount = 0;
  std::unique_ptr<CMoviePlayer> xc4_attractMovie;
  CMoviePlayer* xcc_curMoviePtr = nullptr;
  bool xd0_playerSkipToTitle = false;
  bool xd1_moviesLoaded = false;
  bool xd2_deferSlideShow = false;
  std::unique_ptr<CStaticAudioPlayer> xd4_audio1;
  std::unique_ptr<CStaticAudioPlayer> xd8_audio2;
  std::unique_ptr<CSaveGameScreen> xdc_saveUI;
  std::unique_ptr<SNewFileSelectFrame> xe0_frontendCardFrme;
  std::unique_ptr<SFusionBonusFrame> xe4_fusionBonusFrme;
  std::unique_ptr<SFrontEndFrame> xe8_frontendNoCardFrme;
  std::unique_ptr<SNesEmulatorFrame> xec_emuFrme;
  std::unique_ptr<SOptionsFrontEndFrame> xf0_optionsFrme;
  CStaticAudioPlayer* xf4_curAudio = nullptr;

  CColoredQuadFilter m_fadeToBlack{EFilterType::Blend};
  std::optional<CTexturedQuadFilterAlpha> m_pressStartQuad;

  std::unique_ptr<CFrontEndUITouchBar> m_touchBar;

  void SetFadeBlackWithMovie() {
    x58_fadeBlackTimer = 1000000.f;
    x5c_fadeBlackWithMovie = true;
  }

  void SetFadeBlackTimer(float seconds) {
    x58_fadeBlackTimer = seconds;
    x5c_fadeBlackWithMovie = false;
  }

  void TransitionToGame();
  void UpdateMusicVolume();
  void FinishedLoadingDepsGroup();
  bool PumpLoad();

public:
  CFrontEndUI();
  void StartSlideShow(CArchitectureQueue& queue);
  std::string GetAttractMovieFileName(int idx);
  std::string GetNextAttractMovieFileName();
  void SetCurrentMovie(EMenuMovie movie);
  void StopAttractMovie();
  void StartAttractMovie();
  void StartStateTransition(EScreen screen);
  void CompleteStateTransition();
  void HandleDebugMenuReturnValue(CGameDebug::EReturnValue val, CArchitectureQueue& queue);
  void Draw() override;
  void UpdateMovies(float dt);
  bool PumpMovieLoad();
  void ProcessUserInput(const CFinalInput& input, CArchitectureQueue& queue);
  EMessageReturn Update(float dt, CArchitectureQueue& queue);
  EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) override;
};

} // namespace MP1
} // namespace urde
