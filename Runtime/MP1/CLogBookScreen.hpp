#ifndef __URDE_CLOGBOOKSCREEN_HPP__
#define __URDE_CLOGBOOKSCREEN_HPP__

#include "CInGameGuiManager.hpp"
#include "CPauseScreenBase.hpp"
#include "CArtifactDoll.hpp"

namespace urde::MP1
{

class CLogBookScreen : public CPauseScreenBase
{
    rstl::reserved_vector<std::vector<std::pair<CAssetId, bool>>, 5> x19c_scanCompletes;
    std::vector<std::pair<TCachedToken<CScannableObjectInfo>,
                          TCachedToken<CStringTable>>> x1f0_curViewScans;
    rstl::reserved_vector<std::vector<std::pair<TLockedToken<CScannableObjectInfo>,
                                                TLockedToken<CStringTable>>>, 5> x200_viewScans;
    float x254_viewInterp = 0.f;
    std::unique_ptr<CArtifactDoll> x258_artifactDoll;

    enum class ELeavePauseState
    {
        InPause = 0,
        LeavingPause = 1,
        LeftPause = 2
    };

    ELeavePauseState x25c_leavePauseState = ELeavePauseState::InPause;
    union
    {
        struct
        {
            bool x260_24_loaded : 1;
            bool x260_25_inTextScroll : 1;
            bool x260_26_exitTextScroll : 1;
        };
        s32 _dummy = 0;
    };

    void InitializeLogBook();
    void UpdateRightTitles();
    void PumpArticleLoad();
    bool IsScanCategoryReady(CSaveWorld::EScanCategory category) const;
    void UpdateBodyText();
    void UpdateBodyImagesAndText();
    int NextSurroundingArticleIndex(int cur) const;
    bool IsArtifactCategorySelected() const;
    int GetSelectedArtifactHeadScanIndex() const;
    static bool IsScanComplete(CSaveWorld::EScanCategory category, CAssetId scan, const CPlayerState& playerState);

public:
    CLogBookScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg);
    ~CLogBookScreen();


    bool InputDisabled() const;
    void TransitioningAway();
    void Update(float dt, CRandom16& rand, CArchitectureQueue& archQueue);
    void Touch();
    void ProcessControllerInput(const CFinalInput& input);
    void Draw(float transInterp, float totalAlpha, float yOff);
    bool VReady() const;
    void VActivate();
    void RightTableSelectionChanged(int selBegin, int selEnd);
    void ChangedMode(EMode oldMode);
    void UpdateRightTable();
    bool ShouldLeftTableAdvance() const;
    bool ShouldRightTableAdvance() const;
    u32 GetRightTableCount() const;
};

}

#endif // __URDE_CLOGBOOKSCREEN_HPP__
