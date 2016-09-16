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

namespace MP1
{

class CFrontEndUI : public CIOWin
{
public:
    enum class Phase
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six
    };
    struct SScreenData
    {

    };
private:
    Phase x14_phase = Phase::Zero;
    u32 x18_rndA;
    u32 x1c_rndB;
    TLockedToken<CDependencyGroup> x20_depsGroup;
    TLockedToken<CTexture> x38_pressStart;
    TLockedToken<CAudioGroupSet> x44_frontendAudioGrp;
    u32 x50_ = 0;
    u32 x54_ = 0;
    float x58_ = 0.f;
    bool x5c_ = false;
    float x60_ = 0.f;
    float x64_ = 0.f;
    float x68_ = 1.f;
    std::unique_ptr<u32> x6c_[9];
    int xb8_nextAttract = -1;
    int xbc_nextAttract = 0;
    int xc0_attractCount = 0;
    std::unique_ptr<u32> xc4_;
    u32 xc8_ = 0;
    u32 xcc_ = 0;
    bool xd0_ = 0;
    bool xd1_ = 0;
    bool xd2_ = 0;
    u32 xd4_ = 0;
    u32 xd8_ = 0;
    std::unique_ptr<SScreenData> xdc_scrData;
    u32 xe0_ = 0;
    u32 xe4_ = 0;
    u32 xec_ = 0;
    u32 xf0_ = 0;
    u32 xf4_ = 0;
public:
    enum class EMenuMovie
    {
    };
    enum class EScreen
    {
    };

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
    void StartAttractMovie(int idx);
    void UpdateMenuHighlights(CGuiTableGroup* grp);
    void CompleteStateTransition();
    bool CanBuild(const SObjectTag& tag);
    void StartStateTransition(EScreen screen);
    void HandleDebugMenuReturnValue(CGameDebug::EReturnValue val, CArchitectureQueue& queue);
    void Draw() const;
    void UpdateMovies(float dt);
    void ProcessUserInput(const CFinalInput& input, CArchitectureQueue& queue);
    EMessageReturn Update(float dt, CArchitectureQueue& queue);
    EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
    void StartGame();
    void InitializeFrame();
};

}
}

#endif // __URDE_CFRONTENDUI_HPP__
