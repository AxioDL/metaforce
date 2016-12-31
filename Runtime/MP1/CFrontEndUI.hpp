#ifndef __URDE_CFRONTENDUI_HPP__
#define __URDE_CFRONTENDUI_HPP__

#include "CIOWin.hpp"
#include "CGameDebug.hpp"
#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "Audio/CStaticAudioPlayer.hpp"
#include "CGBASupport.hpp"
#include "zeus/CVector3f.hpp"
#include "Input/CRumbleGenerator.hpp"
#include "GuiSys/CGuiTextSupport.hpp"
#include "Graphics/Shaders/CTexturedQuadFilter.hpp"
#include "Graphics/Shaders/CColoredQuadFilter.hpp"

namespace urde
{
class CGuiSliderGroup;
class CGuiTableGroup;
class CMoviePlayer;
struct SObjectTag;
class CDependencyGroup;
class CTexture;
class CAudioGroupSet;
class CSaveWorld;
class CStringTable;
class CGuiFrame;
class CGuiTextPane;
class CGuiWidget;
class CGuiTableGroup;
class CGuiModel;

namespace MP1
{
class CNESEmulator;
class CSaveUI;
class CQuitScreen;

class CFrontEndUI : public CIOWin
{
public:
    enum class EPhase
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six
    };
    enum class EScreen
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six
    };
    enum class EMenuMovie
    {
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

    struct SGuiTextPair
    {
        CGuiTextPane* x0_panes[2] = {};
        void SetPairText(const std::wstring& str);
    };
    static SGuiTextPair FindTextPanePair(CGuiFrame* frame, const char* name);
    static void FindAndSetPairText(CGuiFrame* frame, const char* name, const std::wstring& str);

    struct SFileMenuOption
    {
        CGuiWidget* x0_base;

        /* filename, world, playtime, date */
        SGuiTextPair x4_textpanes[4];

        u32 x28_ = 0;
        float x2c_ = ComputeRandom();

        static float ComputeRandom()
        {
            return rand() / float(RAND_MAX) * 30.f + 30.f;
        }
    };

    struct SNewFileSelectFrame
    {
        enum class ESubMenu
        {
            Zero,
            One,
            Two,
            Three
        };

        enum class EAction
        {
            Zero,
            One,
            Two,
            Three
        };

        u32 x0_rnd;
        CSaveUI* x4_saveUI;
        ESubMenu x8_subMenu = ESubMenu::Zero;
        EAction xc_action = EAction::Zero;
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
        SFileMenuOption x64_fileSelections[3];
        zeus::CVector3f xf8_model_erase_position;
        float x104_rowPitch = 0.f;
        float x108_curTime = 0.f;
        bool x10c_inputEnable = false;
        bool x10d_needsExistingToggle = false;
        bool x10e_needsNewToggle = false;

        SNewFileSelectFrame(CSaveUI* sui, u32 rnd);
        void FinishedLoading();
        bool PumpLoad();
        bool IsTextDoneAnimating() const;
        EAction ProcessUserInput(const CFinalInput& input);
        void Draw() const;

        void HandleActiveChange(CGuiTableGroup* active);
        void DeactivateExistingGamePopup();
        void ActivateExistingGamePopup();
        void DeactivateNewGamePopup();
        void ActivateNewGamePopup();

        void ResetFrame();
        void ClearFrameContents();
        void SetupFrameContents();

        void DoPopupCancel(CGuiTableGroup* caller);
        void DoPopupAdvance(CGuiTableGroup* caller);
        void DoFileMenuCancel(CGuiTableGroup* caller);
        void DoSelectionChange(CGuiTableGroup* caller);
        void DoFileMenuAdvance(CGuiTableGroup* caller);

        static SFileMenuOption FindFileSelectOption(CGuiFrame* frame, int idx);
        static void StartTextAnimating(CGuiTextPane* text, const std::wstring& str, float chRate);
    };

    struct SGBASupportFrame
    {
        struct SGBALinkFrame
        {
            enum class EUIType
            {
                Zero,
                One,
                Two,
                Three,
                Four,
                Five,
                Six,
                Seven
            };

            void SetUIText(EUIType tp);
            void ProcessUserInput(const CFinalInput &input, bool sui);
            void Update(float dt);
            void FinishedLoading();
            void Draw();
            SGBALinkFrame(const CGuiFrame* linkFrame, CGBASupport* support, bool);
        };

        enum class EAction
        {
            Zero,
            One,
            Two
        };

        std::unique_ptr<SGBALinkFrame> x0_gbaLinkFrame;
        std::unique_ptr<CGBASupport> x4_gbaSupport;
        TLockedToken<CGuiFrame> xc_gbaScreen;
        TLockedToken<CGuiFrame> x18_gbaLink;
        CGuiFrame* x24_loadedFrame = nullptr;
        CGuiTableGroup* x28_tablegroup_options = nullptr;
        CGuiTableGroup* x2c_tablegroup_fusionsuit = nullptr;
        SGuiTextPair x30_textpane_instructions;
        bool x38_ = false;
        bool x39_ = false;
        bool x3a_ = false;

        SGBASupportFrame();
        void FinishedLoading();
        bool PumpLoad();
        void SetTableColors(CGuiTableGroup* tbgp) const;
        EAction ProcessUserInput(const CFinalInput& input, CSaveUI* sui);
        void Draw() const;

        void DoOptionsCancel(CGuiTableGroup* caller);
        void DoSelectionChange(CGuiTableGroup* caller);
        void DoOptionsAdvance(CGuiTableGroup* caller);
    };

    struct SFrontEndFrame
    {
        enum class EAction
        {
            Zero,
            One,
            Two,
            Three,
            Four
        };

        u32 x0_rnd;
        EAction x4_action;
        TLockedToken<CGuiFrame> x8_frme;
        CGuiFrame* x14_loadedFrme = nullptr;
        CGuiTableGroup* x18_tablegroup_mainmenu = nullptr;
        SGuiTextPair x1c_gbaPair;
        SGuiTextPair x24_cheatPair;
        SFrontEndFrame(u32 rnd);
        void FinishedLoading();
        bool PumpLoad();
        EAction ProcessUserInput(const CFinalInput& input);
        void Draw() const;

        void DoCancel(CGuiTableGroup* caller);
        void DoSelectionChange(CGuiTableGroup* caller);
        void DoAdvance(CGuiTableGroup* caller);
    };

    struct SNesEmulatorFrame
    {
        enum class EMode
        {
            Emulator,
            SaveProgress,
            ContinuePlaying,
            QuitNESMetroid
        };

        EMode x0_mode = EMode::Emulator;
        std::unique_ptr<CNESEmulator> x4_nesEmu;
        std::unique_ptr<CQuitScreen> x8_quitScreen;
        std::unique_ptr<CGuiTextSupport> xc_textSupport;
        float x10_remTime = 8.f;
        bool x14_emulationSuspended = false;
        bool x15_enableFiltering = true;

        SNesEmulatorFrame();
        void SetMode(EMode mode);
        void ProcessUserInput(const CFinalInput& input, CSaveUI* sui);
        bool Update(float dt, CSaveUI* saveUi);
        void Draw(CSaveUI* saveUi) const;
    };

    struct SOptionsFrontEndFrame
    {
        float x0_ = 0.f;
        TLockedToken<CGuiFrame> x4_frme;
        TLockedToken<CStringTable> x10_pauseScreen;
        u32 x1c_ = 0;
        u32 x20_ = 0;
        u32 x24_ = 0;
        u32 x28_ = 0;
        u32 x2c_ = 0;
        u32 x30_ = 0;
        u32 x34_ = 0;
        float x38_ = 0.f;
        u32 x3c_ = 0;
        CRumbleGenerator x40_rumbleGen;
        union
        {
            u8 _dummy = 0;
            struct
            {
                bool x134_24_;
                bool x134_25_;
            };
        };
        SOptionsFrontEndFrame();
        bool ProcessUserInput(const CFinalInput& input, CSaveUI* sui);
        void Draw() const;
    };

    bool IsSaveUIConditional() const
    {
        if (x50_curScreen != EScreen::Three && x50_curScreen != EScreen::Four)
            return false;
        if (x54_nextScreen != EScreen::Three && x54_nextScreen != EScreen::Four)
            return false;
        return true;
    }

private:
    EPhase x14_phase = EPhase::Zero;
    u32 x18_rndA;
    u32 x1c_rndB;
    TLockedToken<CDependencyGroup> x20_depsGroup;
    std::vector<CToken> x2c_deps;
    TLockedToken<CTexture> x38_pressStart;
    TLockedToken<CAudioGroupSet> x44_frontendAudioGrp;
    EScreen x50_curScreen = EScreen::Zero;
    EScreen x54_nextScreen = EScreen::Zero;
    float x58_movieSeconds = 0.f;
    bool x5c_movieSecondsNeeded = false;
    float x60_ = 0.f;
    float x64_pressStartAlpha = 0.f;
    float x68_musicVol = 1.f;
    u32 x6c_;
    std::unique_ptr<CMoviePlayer> x70_menuMovies[9];
    EMenuMovie xb8_curMovie = EMenuMovie::Stopped;
    int xbc_nextAttract = 0;
    int xc0_attractCount = 0;
    std::unique_ptr<CMoviePlayer> xc4_attractMovie;
    CMoviePlayer* xcc_curMoviePtr = nullptr;
    bool xd0_ = false;
    bool xd1_moviesLoaded = false;
    bool xd2_ = false;
    std::unique_ptr<CStaticAudioPlayer> xd4_audio1;
    std::unique_ptr<CStaticAudioPlayer> xd8_audio2;
    std::unique_ptr<CSaveUI> xdc_saveUI;
    std::unique_ptr<SNewFileSelectFrame> xe0_newFileSel;
    std::unique_ptr<SGBASupportFrame> xe4_gbaSupportFrme;
    std::unique_ptr<SFrontEndFrame> xe8_frontendFrme;
    std::unique_ptr<SNesEmulatorFrame> xec_emuFrme;
    std::unique_ptr<SOptionsFrontEndFrame> xf0_optionsFrme;
    CStaticAudioPlayer* xf4_curAudio = nullptr;

    CColoredQuadFilter m_fadeToBlack = {CCameraFilterPass::EFilterType::Blend};
    std::experimental::optional<CTexturedQuadFilterAlpha> m_pressStartQuad;

    void SetMovieSecondsDeferred()
    {
        x58_movieSeconds = 1000000.f;
        x5c_movieSecondsNeeded = true;
    }

    void SetMovieSeconds(float seconds)
    {
        x58_movieSeconds = seconds;
        x5c_movieSecondsNeeded = false;
    }

    void TransitionToFive();
    void UpdateMusicVolume();
    void FinishedLoadingDepsGroup();
    bool PumpLoad();
public:

    CFrontEndUI(CArchitectureQueue& queue);
    void OnSliderSelectionChange(CGuiSliderGroup* grp, float);
    void OnCheckBoxSelectionChange(CGuiTableGroup* grp);
    void OnOptionSubMenuCancel(CGuiTableGroup* grp);
    void OnOptionsMenuCancel(CGuiTableGroup* grp);
    void OnNewGameMenuCancel(CGuiTableGroup* grp);
    void OnFileMenuCancel(CGuiTableGroup* grp);
    void OnGenericMenuSelectionChange(CGuiTableGroup* grp, int, int);
    void OnOptionsMenuAdvance(CGuiTableGroup* grp);
    void OnNewGameMenuAdvance(CGuiTableGroup* grp);
    void OnFileMenuAdvance(CGuiTableGroup* grp);
    void OnMainMenuAdvance(CGuiTableGroup* grp);
    void StartSlideShow(CArchitectureQueue& queue);
    std::string GetAttractMovieFileName(int idx);
    std::string GetNextAttractMovieFileName();
    void SetCurrentMovie(EMenuMovie movie);
    void StopAttractMovie();
    void StartAttractMovie();
    void UpdateMenuHighlights(CGuiTableGroup* grp);
    void CompleteStateTransition();
    bool CanBuild(const SObjectTag& tag);
    void StartStateTransition(EScreen screen);
    void HandleDebugMenuReturnValue(CGameDebug::EReturnValue val, CArchitectureQueue& queue);
    void Draw() const;
    void UpdateMovies(float dt);
    bool PumpMovieLoad();
    void ProcessUserInput(const CFinalInput& input, CArchitectureQueue& queue);
    EMessageReturn Update(float dt, CArchitectureQueue& queue);
    EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
    void StartGame();
    void InitializeFrame();
};

}
}

#endif // __URDE_CFRONTENDUI_HPP__
