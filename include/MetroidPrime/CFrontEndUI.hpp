#ifndef _CFRONTENDUI
#define _CFRONTENDUI

#include "MetroidPrime/CIOWin.hpp"

#include "Kyoto/CToken.hpp"
#include "Kyoto/TToken.hpp"

#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CArchitectureQueue;
class CAudioGrpSetLoc;
class CDependencyGroup;
class CGuiFrame;
class CGuiModel;
class CGuiSliderGroup;
class CGuiTableGroup;
class CGuiTextPane;
class CGuiWidget;
class CMoviePlayer;
class CStaticAudioPlayer;
class CTexture;
class CGBASupport;
class CNESEmulator;
class CSaveGameScreen;
class CQuitGameScreen;
class CGuiTextSupport;
class CFinalInput;
class CRumbleGenerator;
class CStringTable;
class SOptionsFrontEndFrame;

class CFrontEndUI : public CIOWin {
public:
  CFrontEndUI();
  ~CFrontEndUI();

  EMessageReturn OnMessage(const CArchitectureMessage& message, CArchitectureQueue& queue) override;
  void Draw() const override;

  enum EPhase {
    kP_LoadDepsGroup = 0,
    kP_LoadDeps,
    kP_LoadFrames,
    kP_LoadMovies,
    kP_DisplayFrontEnd,
    kP_ToPlayGame,
    kP_ExitFrontEnd
  };

  enum EScreen {
    kS_OpenCredits = 0,
    kS_Title,
    kS_AttractMovie,
    kS_FileSelect,
    kS_FusionBonus,
    kS_ToPlayGame
  };

  enum EMenuMovie {
    kMM_Stopped = -1,
    kMM_FirstStart = 0,
    kMM_StartLoop,
    kMM_StartFileSelectA,
    kMM_FileSelectLoop,
    kMM_FileSelectPlayGameA,
    kMM_FileSelectGBA,
    kMM_GBALoop,
    kMM_GBAFileSelectA,
#if VERSION >= VERSION_GM8P_00
    kMM_BackToTitle
#else
    kMM_GBAFileSelectB
#endif
  };

  struct SMenuMovieData {
    rstl::auto_ptr< CMoviePlayer > mMovie;
  };

  struct SGuiTextPair {
    CGuiTextPane* mTextPane;
    CGuiTextPane* mTextPaneB;

    SGuiTextPair();
    SGuiTextPair(const CGuiFrame* frame, const char* name);
    void SetPairText(const rstl::wstring& str);
    void SetPairText(const wchar_t* str);
  };

  struct SFileSelectOption {
    CGuiWidget* mBase;
    rstl::reserved_vector< SGuiTextPair, 4 > mTextpanes;
    int mCurField;
    float mChRate;

    SFileSelectOption(CGuiFrame* frame, int idx);
    static float ComputeRandom();
  };

  struct SNewFileSelectFrame {
    enum ESubMenu { kSM_Root = 0, kSM_EraseGame, kSM_EraseGamePopup, kSM_NewGamePopup };

    enum EAction {
      kA_None = 0,
      kA_GameOptions,
      kA_FusionBonus,
      kA_SlideShow,
#if VERSION >= VERSION_GM8P_00
      kA_Language,
      kA_ReturnToTitle,
#endif
    };

    uint mRnd;
    CSaveGameScreen* mSaveUI;
    ESubMenu mSubMenu;
    EAction mAction;
    TCachedToken< CGuiFrame > mFrme;
    CGuiFrame* mLoadedFrame;
    CGuiTableGroup* mTablegroup_fileselect;
    CGuiModel* mModel_erase;
    SGuiTextPair mTextpane_erase;
#if VERSION >= VERSION_GM8P_00
    SGuiTextPair mTextpane_language;
#endif
    SGuiTextPair mTextpane_cheats;
    SGuiTextPair mTextpane_gba;
    CGuiTableGroup* mTablegroup_popup;
    CGuiModel* mModel_dash7;
    SGuiTextPair mTextpane_popupadvance;
    SGuiTextPair mTextpane_popupcancel;
    SGuiTextPair mTextpane_popupextra;
#if VERSION < VERSION_GM8P_00
    CGuiTextPane* mTextpane_cancel;
#endif
    rstl::reserved_vector< SFileSelectOption, 3 > mFileSelections;
    CVector3f mModel_erase_position;
    float mRowPitch;
    float mCurTime;
    bool mSaveReady;
    bool mNeedsEraseToggle;
    bool mNeedsNewToggle;

    SNewFileSelectFrame(CSaveGameScreen* saveUI, uint rnd);
    ~SNewFileSelectFrame();
    bool PumpLoad();
    void FinishedLoading();
#if VERSION >= VERSION_GM8P_00
    void SetStrings();
    void ReapplyStrings();
#endif
    uint GetUserFileSelection() const;
    void Update(float dt);
    EAction ProcessUserInput(const CFinalInput& input);
    void Draw() const;
    void HandleActiveChange(CGuiWidget* active);
    void DoFileselectAdvance(CGuiTableGroup* caller);
    void SetupFrameContents();
    void ClearFrameContents();
    void EnterErase();
    void ResetFrame();
    void ActivateNewGamePopup();
    void DeactivateNewGamePopup();
    void ActivateExistingGamePopup();
    void DeactivateExistingGamePopup();
    bool IsTextDoneAnimating();
    void DoSelectionChange(CGuiTableGroup* caller, int oldSelection);
    void DoFileselectCancel(CGuiTableGroup* caller);
    void DoPopupAdvance(CGuiTableGroup* caller);
    void DoPopupCancel(CGuiTableGroup* caller);

    static void StartTextAnimating(CGuiTextPane* text, const rstl::wstring& str, float chRate);
  };

  struct SFusionBonusFrame {
    struct SGBALinkFrame {
      enum EUIType {
        kUIT_Empty = -1,
        kUIT_InsertPak = 0,
        kUIT_ConnectSocket = 1,
        kUIT_PressStartAndSelect = 2,
        kUIT_BeginLink = 3,
        kUIT_Linking = 4,
        kUIT_LinkFailed = 5,
        kUIT_LinkCompleteOrLinking = 6,
        kUIT_TurnOffGBA = 7,
        kUIT_Complete = 8,
        kUIT_Cancelled = 9
      };

      enum EAction { kGA_None = 0, kGA_Complete = 1, kGA_Cancelled = 2 };

      EUIType mUiType;
      CGBASupport* mGbaSupport;
      CGuiFrame* mFrme;
      SGuiTextPair mTextpane_instructions;
      CGuiTextPane* mTextpane_yes;
      CGuiTextPane* mTextpane_no;
      CGuiModel* mModel_gc;
      CGuiModel* mModel_gba;
      CGuiModel* mModel_cable;
      CGuiModel* mModel_circlegcport;
      CGuiModel* mModel_circlegbaport;
      CGuiModel* mModel_circlestartselect;
      CGuiModel* mModel_pakout;
      CGuiModel* mModel_gbascreen;
      CGuiModel* mModel_connect;
      bool mLinkInProgress;

      SGBALinkFrame(CGuiFrame* frme, CGBASupport* support, bool linkInProgress);
      ~SGBALinkFrame();
      void Draw();
      void FinishedLoading();
      void Update(float dt);
      EAction ProcessUserInput(const CFinalInput& input, bool linkInProgress);
      void SetUIText(EUIType tp);
    };

    enum EAction { kFA_None = 0, kFA_GoBack = 1, kFA_PlayNESMetroid = 2 };

    rstl::single_ptr< SGBALinkFrame > mGbaLinkFrame;
    rstl::single_ptr< CGBASupport > mGbaSupport;
    EAction mAction;
    TCachedToken< CGuiFrame > mGbaScreen;
    TCachedToken< CGuiFrame > mGbaLink;
    CGuiFrame* mLoadedFrame;
    CGuiTableGroup* mTablegroup_options;
    CGuiTableGroup* mTablegroup_fusionsuit;
    SGuiTextPair mTextpane_instructions;
    bool mLastDoDraw;
    bool mFusionNotComplete;
    bool mMpNotComplete;

    SFusionBonusFrame();
    ~SFusionBonusFrame();
    void Draw();
    void Update(float dt, CSaveGameScreen* saveUI);
    EAction ProcessUserInput(const CFinalInput& input, CSaveGameScreen* saveUI);
    bool PumpLoad();
    void FinishedLoading();
#if VERSION >= VERSION_GM8P_00
    void SetStrings();
    void ReapplyStrings();
#endif
    void ResetCompletionFlags();
    void SetTableColors(CGuiTableGroup* tbgp);
    void DoOptionsAdvance(CGuiTableGroup* caller);
    void DoSelectionChange(CGuiTableGroup* caller, int oldSelection);
    void DoOptionsCancel(CGuiTableGroup* caller);
  };

  struct SFrontEndFrame {
    enum EAction {
      kEA_None = 0,
      kEA_StartGame,
      kEA_FusionBonus,
      kEA_GameOptions,
      kEA_SlideShow
#if VERSION >= VERSION_GM8P_00
      ,
      kEA_Language,
      kEA_ReturnToTitle
#endif
    };

    uint mRnd;
    EAction mAction;
    TCachedToken< CGuiFrame > mFrme;
    CGuiFrame* mLoadedFrme;
    CGuiTableGroup* mTablegroup_mainmenu;
#if VERSION >= VERSION_GM8P_00
    SGuiTextPair mLanguagePair;
#endif
    SGuiTextPair mGbaPair;
    SGuiTextPair mCheatPair;

    SFrontEndFrame(uint rnd);
    ~SFrontEndFrame();
    bool PumpLoad();
    void FinishedLoading();
#if VERSION >= VERSION_GM8P_00
    void SetStrings();
    void ReapplyStrings();
#endif
    void Update(float dt);
    EAction ProcessUserInput(const CFinalInput& input);
    void Draw();
    void DoAdvance(CGuiTableGroup* caller);
    void DoSelectionChange(CGuiTableGroup* caller, int oldSelection);
    void HandleActiveChange(CGuiTableGroup* caller);
    void DoCancel(CGuiTableGroup* caller);

    static void FindAndSetPairText(CGuiFrame& frame, const char* name, const wchar_t* str);
  };

  struct SNesEmulatorFrame {
    enum EMode { kEM_Emulator = 0, kEM_SaveProgress, kEM_ContinuePlaying, kEM_QuitNESMetroid };

    EMode mMode;
    rstl::single_ptr< CNESEmulator > mNesEmu;
    rstl::single_ptr< CQuitGameScreen > mQuitScreen;
    rstl::single_ptr< CGuiTextSupport > mTextSupport;
    float mRemTime;
#if VERSION >= VERSION_GM8P_00
    float x14_;
#endif
    bool mEmulationSuspended;
    bool mEnableFiltering;

    SNesEmulatorFrame();
    ~SNesEmulatorFrame();
    void SetMode(EMode mode);
    int Update(float dt, CSaveGameScreen* saveUi);
    void ProcessUserInput(const CFinalInput& input, CSaveGameScreen* sui);
    void Draw(CSaveGameScreen* saveUi) const;
  };

#if VERSION >= VERSION_GM8P_00
  struct SLanguageSelectFrame {
    TLockedToken< CGuiFrame > mFrame;
    TLockedToken< CStringTable > mStrings;
    int mInitialLanguage;
    CGuiFrame* mLoadedFrame;
    CGuiTableGroup* mTablegroup_menu;
    bool mActive : 1;
    bool mCanDraw : 1;

    SLanguageSelectFrame();
    ~SLanguageSelectFrame();
    void Update(float dt, CSaveGameScreen* saveUI);
    bool ProcessUserInput(const CFinalInput& input, CSaveGameScreen* saveUI);
    void Draw() const;
    void DoSelectionChange(CGuiTableGroup* caller, int oldSelection);
    void DoCancel(CGuiTableGroup* caller);
    void SetTableColors();
  };

#endif
  static void PlayAdvanceSfx();
#if VERSION >= VERSION_GM8P_00
  static void SetTitlePosition(CGuiFrame& frame);
#endif

private:
  void TransitionToFive();
#if VERSION >= VERSION_GM8P_00
  void ReapplyStrings();
#endif
  void UpdateMusicVol();
  void FinishedLoadingDepsGroup();
  bool PumpLoad();
  EMessageReturn Update(float dt, CArchitectureQueue& queue);
  bool PumpMovieLoad();
  void UpdateMovies(float dt);
  void ProcessUserInput(const CFinalInput& input, CArchitectureQueue& queue);
  bool IsInScreenNotTransitioning(EScreen screen) const {
    return mCurScreen == screen && mNextScreen == screen;
  }
  bool GetHasAttractMovies() const { return mAttractCount > 0; }
  bool CanShowSaveUI() const;
  void StartStateTransition(EScreen screen);
  void CompleteStateTransition();
  void StartAttractMovie();
  void StopAttractMovie();
  void SetCurrentMovie(EMenuMovie movie);
  const char* GetNextAttractMovieFileName();
  static const char* GetAttractMovieFileName(int idx);
  void SetFadeBlackTimer(float seconds);
  void SetFadeBlackWithMovie();
  void StartSlideShow(CArchitectureQueue& queue);

  EPhase mPhase;
  uint mRndA;
  uint mRndB;
  TToken< CDependencyGroup > mDepsGroup;
  rstl::vector< CToken > mDeps;
  TCachedToken< CTexture > mPressStart;
  TCachedToken< CAudioGrpSetLoc > mFrontendAudioGrp;
  EScreen mCurScreen;
  EScreen mNextScreen;
  float mFadeBlackTimer;
  bool mFadeBlackWithMovie;
  float mPressStartTime;
  float mPressStartAlpha;
  float mMusicVol;
  rstl::reserved_vector< SMenuMovieData, 9 > mMenuMovies;
  EMenuMovie mCurMovie;
  int mNextAttract;
  int mAttractCount;
  rstl::auto_ptr< CMoviePlayer > mAttractMovie;
  CMoviePlayer* mCurMoviePtr;
#if VERSION >= VERSION_GM8P_00
  bool mPlayerSkipToTitle : 1;
  bool mMoviesLoaded : 1;
  bool mDeferSlideShow : 1;
  bool mStringsReloading : 1;
  bool mNoCardFrame : 1;
  bool mNoSaveUI : 1;
#else
  bool mPlayerSkipToTitle;
  bool mMoviesLoaded;
  bool mDeferSlideShow;
#endif
  rstl::single_ptr< CStaticAudioPlayer > mAudio1;
  rstl::single_ptr< CStaticAudioPlayer > mAudio2;
  rstl::single_ptr< CSaveGameScreen > mSaveUI;
  rstl::single_ptr< SNewFileSelectFrame > mFrontendCardFrme;
  rstl::single_ptr< SFusionBonusFrame > mFusionBonusFrme;
  rstl::single_ptr< SFrontEndFrame > mFrontendNoCardFrme;
  rstl::single_ptr< SNesEmulatorFrame > mEmuFrme;
  rstl::single_ptr< SOptionsFrontEndFrame > mOptionsFrme;
#if VERSION >= VERSION_GM8P_00
  rstl::single_ptr< SLanguageSelectFrame > mLanguageFrme;
#endif
  CStaticAudioPlayer* mCurAudio;
};

NESTED_CHECK_SIZEOF(CFrontEndUI, SMenuMovieData, 0x8)
NESTED_CHECK_SIZEOF(CFrontEndUI, SGuiTextPair, 0x8)
NESTED_CHECK_SIZEOF(CFrontEndUI, SFileSelectOption, 0x30)
NESTED_CHECK_SIZEOF(CFrontEndUI, SNewFileSelectFrame,
                    (VERSION >= VERSION_GM8P_00 ? 0x114 : 0x110))
NESTED_CHECK_SIZEOF(CFrontEndUI::SFusionBonusFrame, SGBALinkFrame, 0x44)
NESTED_CHECK_SIZEOF(CFrontEndUI, SFusionBonusFrame, 0x3c)
NESTED_CHECK_SIZEOF(CFrontEndUI, SFrontEndFrame,
                    (VERSION >= VERSION_GM8P_00 ? 0x34 : 0x2c))
#if VERSION >= VERSION_GM8P_00
NESTED_CHECK_SIZEOF(CFrontEndUI, SNesEmulatorFrame, 0x1c)
#else
NESTED_CHECK_SIZEOF(CFrontEndUI, SNesEmulatorFrame, 0x18)
#endif
CHECK_SIZEOF(CFrontEndUI, (VERSION >= VERSION_GM8P_00 ? 0xfc : 0xf8))
#if VERSION >= VERSION_GM8P_00
NESTED_CHECK_SIZEOF(CFrontEndUI, SLanguageSelectFrame, 0x28)
#endif

#endif // _CFRONTENDUI
