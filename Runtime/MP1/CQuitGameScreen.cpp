#include "CQuitGameScreen.hpp"
#include "Input/CFinalInput.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CStringTable.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Audio/CSfxManager.hpp"

namespace urde::MP1
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

void CQuitGameScreen::SetColors()
{
    x14_tablegroup_quitgame->SetColors(zeus::CColor{0.784313f, 0.784313f, 0.784313f, 1.f},
                                       zeus::CColor{0.196078f, 0.196078f, 0.196078f, 1.f});
}

void CQuitGameScreen::FinishedLoading()
{
    x10_loadedFrame = x4_frame.GetObj();
    x10_loadedFrame->SetMaxAspect(1.33f);

    x14_tablegroup_quitgame = static_cast<CGuiTableGroup*>(
        x10_loadedFrame->FindWidget("tablegroup_quitgame"));
    x14_tablegroup_quitgame->SetVertical(false);
    x14_tablegroup_quitgame->SetMenuAdvanceCallback(
        std::bind(&CQuitGameScreen::DoAdvance, this, std::placeholders::_1));
    x14_tablegroup_quitgame->SetMenuSelectionChangeCallback(
        std::bind(&CQuitGameScreen::DoSelectionChange, this, std::placeholders::_1, std::placeholders::_2));

    static_cast<CGuiTextPane*>(x10_loadedFrame->FindWidget("textpane_title"))->TextSupport().
        SetText(g_MainStringTable->GetString(Titles[int(x0_type)]));

    static_cast<CGuiTextPane*>(x10_loadedFrame->FindWidget("textpane_yes"))->TextSupport().
        SetText(g_MainStringTable->GetString(22));
    static_cast<CGuiTextPane*>(x10_loadedFrame->FindWidget("textpane_no"))->TextSupport().
        SetText(g_MainStringTable->GetString(23));

    x14_tablegroup_quitgame->SetUserSelection(DefaultSelections[int(x0_type)]);
    SetColors();
}

void CQuitGameScreen::DoSelectionChange(CGuiTableGroup* caller, int oldSel)
{
    SetColors();
    CSfxManager::SfxStart(SFXui_quit_change, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
}

void CQuitGameScreen::DoAdvance(CGuiTableGroup* caller)
{
    if (caller->GetUserSelection() == 0)
    {
        /* Yes */
        CSfxManager::SfxStart(SFXui_advance, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        x18_action = EQuitAction::Yes;
    }
    else
    {
        /* No */
        CSfxManager::SfxStart(SFXui_table_change_mode, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        x18_action = EQuitAction::No;
    }
}

EQuitAction CQuitGameScreen::Update(float dt)
{
    if (!x10_loadedFrame && x4_frame.IsLoaded())
        FinishedLoading();
    return x18_action;
}

void CQuitGameScreen::Draw()
{
    if (x0_type == EQuitType::QuitGame)
        m_blackScreen->draw(zeus::CColor(0.f, 0.5f));

    if (x10_loadedFrame)
        x10_loadedFrame->Draw(CGuiWidgetDrawParms{1.f,
        zeus::CVector3f{0.f, 0.f, VerticalOffsets[int(x0_type)]}});
}

void CQuitGameScreen::ProcessUserInput(const CFinalInput& input)
{
    if (input.ControllerIdx() != 0)
        return;
    if (!x10_loadedFrame)
        return;
    x10_loadedFrame->ProcessUserInput(input);
    if (input.PB() && x0_type != EQuitType::ContinueFromLastSave)
        x18_action = EQuitAction::No;
}

CQuitGameScreen::CQuitGameScreen(EQuitType tp)
: x0_type(tp)
{
    x4_frame = g_SimplePool->GetObj("FRME_QuitScreen");
    if (tp == EQuitType::QuitGame)
        m_blackScreen.emplace(EFilterType::Blend);
}

}
