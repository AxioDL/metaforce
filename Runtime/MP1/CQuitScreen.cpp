#include "CQuitScreen.hpp"
#include "Input/CFinalInput.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CStringTable.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Audio/CSfxManager.hpp"

namespace urde
{
namespace MP1
{

static const int Titles[] =
{
    24, 25, 26, 27, 28
};

static const int DefaultSelections[] =
{
    1, 0, 1, 1, 0
};

static const float VerticalOffsets[] =
{
    0.f, 1.6f, 1.f, 0.f, 1.f
};

void CQuitScreen::SetColors()
{
    x14_tablegroup_quitgame->SetColors(zeus::CColor{0.784313f, 0.784313f, 0.784313f, 1.f},
                                       zeus::CColor{0.196078f, 0.196078f, 0.196078f, 1.f});
}

void CQuitScreen::FinishedLoading()
{
    x10_loadedFrame = x4_frame.GetObj();

    x14_tablegroup_quitgame = static_cast<CGuiTableGroup*>(
        x10_loadedFrame->FindWidget("tablegroup_quitgame"));
    x14_tablegroup_quitgame->SetMenuAdvanceCallback(
        std::bind(&CQuitScreen::DoAdvance, this, std::placeholders::_1));
    x14_tablegroup_quitgame->SetMenuSelectionChangeCallback(
        std::bind(&CQuitScreen::DoSelectionChange, this, std::placeholders::_1));

    static_cast<CGuiTextPane*>(x10_loadedFrame->FindWidget("textpane_title"))->TextSupport()->
        SetText(g_MainStringTable->GetString(Titles[int(x0_type)]));

    static_cast<CGuiTextPane*>(x10_loadedFrame->FindWidget("textpane_yes"))->TextSupport()->
        SetText(g_MainStringTable->GetString(22));
    static_cast<CGuiTextPane*>(x10_loadedFrame->FindWidget("textpane_no"))->TextSupport()->
        SetText(g_MainStringTable->GetString(23));

    x14_tablegroup_quitgame->SetUserSelection(DefaultSelections[int(x0_type)]);
    SetColors();
}

void CQuitScreen::DoSelectionChange(CGuiTableGroup* caller)
{
    SetColors();
    CSfxManager::SfxStart(1424, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
}

void CQuitScreen::DoAdvance(CGuiTableGroup* caller)
{
    if (caller->GetUserSelection() == 0)
    {
        /* Yes */
        CSfxManager::SfxStart(1432, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        x18_action = EQuitAction::Yes;
    }
    else
    {
        /* No */
        CSfxManager::SfxStart(1431, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        x18_action = EQuitAction::No;
    }
}

EQuitAction CQuitScreen::Update()
{
    if (!x10_loadedFrame && x4_frame.IsLoaded())
        FinishedLoading();
    return x18_action;
}

void CQuitScreen::Draw()
{
    if (x0_type == EQuitType::QuitGame)
        m_blackScreen->draw(zeus::CColor::skBlack);

    if (x10_loadedFrame)
        x10_loadedFrame->Draw(CGuiWidgetDrawParms{1.f,
        zeus::CVector3f{0.f, 0.f, VerticalOffsets[int(x0_type)]}});
}

void CQuitScreen::ProcessUserInput(const CFinalInput& input)
{
    if (input.ControllerIdx() != 0)
        return;
    if (!x10_loadedFrame)
        return;
    x10_loadedFrame->ProcessUserInput(input);
    if (input.PB() && x0_type != EQuitType::ContinueFromLastSave)
        x18_action = EQuitAction::No;
}

CQuitScreen::CQuitScreen(EQuitType tp)
: x0_type(tp)
{
    x4_frame = g_SimplePool->GetObj("FRME_QuitScreen");
    if (tp == EQuitType::QuitGame)
        m_blackScreen.emplace(CCameraFilterPass::EFilterType::Blend);
}

}
}
