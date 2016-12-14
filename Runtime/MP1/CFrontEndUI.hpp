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
class CSaveUI;
class CGuiTextPane;
class CGuiWidget;
class CGuiTableGroup;

namespace MP1
{
class CNESEmulator;

class CFrontEndUI : public CIOWin
{
public:
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

    struct SGuiTextPair
    {
        CGuiTextPane* x0_panes[2] = {};
        void SetPairText(const std::wstring& str);
    };

    struct SNewFileSelectFrame
    {
        u32 x0_rnd;
        CSaveUI* x4_saveUI;
        TLockedToken<CGuiFrame> x10_frme;
        CGuiFrame* x1c_ = nullptr;
        CGuiWidget* x20_ = nullptr;
        CGuiWidget* x24_ = nullptr;
        SGuiTextPair x28_;
        SGuiTextPair x30_;
        SGuiTextPair x38_;
        CGuiWidget* x40_ = nullptr;
        CGuiWidget* x44_ = nullptr;
        SGuiTextPair x48_;
        SGuiTextPair x50_;
        SGuiTextPair x58_;
        CGuiWidget* x60_ = nullptr;
        CGuiWidget* x64_ = nullptr;
        zeus::CVector3f xf8_;
        float x104_ = 0.f;
        float x108_ = 0.f;
        bool x10c_ = false;
        bool x10d_ = false;
        bool x10e_ = false;
        SNewFileSelectFrame(CSaveUI* sui, u32 rnd);
        bool PumpLoad();
    };

    struct SFrontEndFrame
    {
        u32 x0_rnd;
        TLockedToken<CGuiFrame> x8_frme;
        CGuiFrame* x14_loadedFrme = nullptr;
        CGuiTableGroup* x18_tablegroup_mainmenu = nullptr;
        SGuiTextPair x1c_gbaPair;
        SGuiTextPair x24_cheatPair;
        SFrontEndFrame(u32 rnd);
        void FinishedLoading();
        bool PumpLoad();
        static SGuiTextPair FindTextPanePair(CGuiFrame* frame, const char* name);
        static void FindAndSetPairText(CGuiFrame* frame, const char* name, const std::wstring& str);

        void DoMenuSelectionChange(const CGuiTableGroup* caller);
        void DoMenuAdvance(const CGuiTableGroup* caller);
    };

    struct SFusionBonusFrame
    {
        u32 x0_ = 0;
        std::unique_ptr<CNESEmulator> x4_nesEmu;
        std::unique_ptr<CGuiTextSupport> xc_textSupport;
        float x10_ = 8.f;
        bool x14_ = false;
        bool x15_ = true;

        SFusionBonusFrame();
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
    };

private:
    EScreen x14_screen = EScreen::Zero;
    u32 x18_rndA;
    u32 x1c_rndB;
    TLockedToken<CDependencyGroup> x20_depsGroup;
    TLockedToken<CTexture> x38_pressStart;
    TLockedToken<CAudioGroupSet> x44_frontendAudioGrp;
    EScreen x50_curScreen = EScreen::Zero;
    EScreen x54_nextScreen = EScreen::Zero;
    float x58_movieSeconds = 0.f;
    bool x5c_movieSecondsNeeded = false;
    float x60_ = 0.f;
    float x64_ = 0.f;
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
    std::unique_ptr<CGBASupport> xe4_gbaSupport;
    std::unique_ptr<SFrontEndFrame> xe8_frontendFrme;
    u32 xec_ = 0;
    u32 xf0_ = 0;
    CStaticAudioPlayer* xf4_curAudio = nullptr;

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
