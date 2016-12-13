#ifndef __URDE_CFRONTENDUI_HPP__
#define __URDE_CFRONTENDUI_HPP__

#include "CIOWin.hpp"
#include "CGameDebug.hpp"
#include "RetroTypes.hpp"
#include "CToken.hpp"

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

namespace MP1
{

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
    struct SNewFileSelect
    {

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
    float x68_ = 1.f;
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
    u32 xd4_ = 0;
    u32 xd8_ = 0;
    std::unique_ptr<CSaveUI> xdc_saveUI;
    std::unique_ptr<SNewFileSelect> xe0_newFileSel;
    u32 xe4_ = 0;
    u32 xec_ = 0;
    u32 xf0_ = 0;
    u32 xf4_ = 0;

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
