#include "CLogBookScreen.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "MP1.hpp"

namespace urde
{
namespace MP1
{

CLogBookScreen::CLogBookScreen(const CStateManager& mgr, CGuiFrame& frame,
                               const CStringTable& pauseStrg)
: CPauseScreenBase(mgr, frame, pauseStrg)
{
    x258_artifactDoll = std::make_unique<CArtifactDoll>();
    CMain::EnsureWorldPaksReady();
    InitializeLogBook();
}

CLogBookScreen::~CLogBookScreen()
{
    CArtifactDoll::CompleteArtifactHeadScan(x4_mgr);
    for (CGuiModel* model : x144_model_titles)
        model->SetLocalTransform(model->GetTransform());
    CMain::EnsureWorldPakReady(g_GameState->CurrentWorldAssetId());
}

void CLogBookScreen::InitializeLogBook()
{

}

bool CLogBookScreen::IsArtifactCategorySelected() const
{
    return x70_tablegroup_leftlog->GetUserSelection() == 4;
}

bool CLogBookScreen::VReady() const
{
    return true;
}

void CLogBookScreen::VActivate()
{

}

u32 CLogBookScreen::GetRightTableCount() const
{
    return 0;
}

}
}
