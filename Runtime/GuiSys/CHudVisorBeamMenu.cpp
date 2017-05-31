#include "CHudVisorBeamMenu.hpp"
#include "GameGlobalObjects.hpp"
#include "CGameState.hpp"
#include "CGuiFrame.hpp"
#include "CGuiModel.hpp"
#include "CGuiTextPane.hpp"
#include "CStringTable.hpp"
#include "Audio/CSfxManager.hpp"

namespace urde
{

static const char* BaseMenuNames[] =
{
    "BaseWidget_VisorMenu",
    "BaseWidget_BeamMenu"
};

static const char* TextNames[] =
{
    "TextPane_VisorMenu",
    "TextPane_BeamMenu"
};

static const char* BaseTitleNames[] =
{
    "basewidget_visormenutitle",
    "basewidget_beammenutitle"
};

static const char* ModelNames[] =
{
    "model_visor",
    "model_beam"
};

static const char MenuItemOrders[2][4] =
{
    {'1', '0', '3', '2'},
    {'3', '2', '1', '0'}
};

static const int MenuStringIdx[2][4] =
{
    {0, 2, 1, 3}, // Combat, XRay, Scan, Thermal
    {4, 5, 6, 7}  // Power, Ice, Wave, Plasma
};

static const u16 SelectionSfxs[] =
{
    1395, 1398
};

CHudVisorBeamMenu::CHudVisorBeamMenu(CGuiFrame& baseHud, EHudVisorBeamMenu type,
                                     const rstl::prereserved_vector<bool, 4>& enables)
: x0_baseHud(baseHud), x4_type(type)
{
    x14_24_visibleDebug = true;
    x14_25_visibleGame = true;
    x14_26_dirty = true;

    x7c_animDur = g_tweakGui->GetBeamVisorMenuAnimTime();
    x80_24_swapBeamControls = g_GameState->GameOptions().GetSwapBeamControls();

    EHudVisorBeamMenu swappedType;
    if (x80_24_swapBeamControls)
        swappedType = EHudVisorBeamMenu(1 - int(x4_type));
    else
        swappedType = x4_type;

    x20_textpane_menu = static_cast<CGuiTextPane*>(x0_baseHud.FindWidget(TextNames[int(swappedType)]));
    x1c_basewidget_menutitle = x0_baseHud.FindWidget(BaseTitleNames[int(swappedType)]);
    x18_basewidget_menu = x0_baseHud.FindWidget(BaseMenuNames[int(swappedType)]);

    x24_model_ghost = static_cast<CGuiModel*>(x0_baseHud.FindWidget(hecl::Format("%sghost", ModelNames[int(x4_type)])));

    for (int i=0 ; i<4 ; ++i)
    {
        SMenuItem& item = x28_menuItems[i];
        item.x0_model_loz = static_cast<CGuiModel*>(x0_baseHud.FindWidget(hecl::Format("%sloz%c",
            ModelNames[int(x4_type)], MenuItemOrders[int(x4_type)][i])));
        item.x4_model_icon = static_cast<CGuiModel*>(x0_baseHud.FindWidget(hecl::Format("%sicon%c",
            ModelNames[int(x4_type)], MenuItemOrders[int(x4_type)][i])));
        item.xc_opacity = enables[i] ? 1.f : 0.f;
    }

    if (x4_type == EHudVisorBeamMenu::Visor)
    {
        x20_textpane_menu->TextSupport().SetFontColor(g_tweakGuiColors->GetVisorMenuTextFont());
        x20_textpane_menu->TextSupport().SetOutlineColor(g_tweakGuiColors->GetVisorMenuTextOutline());
    }
    else
    {
        x20_textpane_menu->TextSupport().SetFontColor(g_tweakGuiColors->GetBeamMenuTextFont());
        x20_textpane_menu->TextSupport().SetOutlineColor(g_tweakGuiColors->GetBeamMenuTextOutline());
    }

    zeus::CColor titleColor = zeus::CColor::skWhite;
    titleColor.a = 0.f;
    x1c_basewidget_menutitle->SetColor(titleColor);

    x20_textpane_menu->TextSupport().SetText(g_MainStringTable->GetString(MenuStringIdx[int(x4_type)][x8_selectedItem]));

    for (int i=0 ; i<4 ; ++i)
    {
        SMenuItem& item = x28_menuItems[i];
        item.x0_model_loz->SetColor(g_tweakGuiColors->GetVisorBeamMenuLozColor());
        UpdateMenuWidgetTransform(i, *item.x0_model_loz, 1.f);
    }

    Update(0.f, true);
}

void CHudVisorBeamMenu::UpdateMenuWidgetTransform(int idx, CGuiWidget& w, float t)
{
    float translate = t * g_tweakGui->GetVisorBeamMenuItemTranslate();
    float scale = t * g_tweakGui->GetVisorBeamMenuItemInactiveScale() +
                  (1.f - t) * g_tweakGui->GetVisorBeamMenuItemActiveScale();
    if (x4_type == EHudVisorBeamMenu::Visor)
    {
        if (idx == 2)
            idx = 3;
        else if (idx == 3)
            idx = 2;
    }
    else
    {
        if (idx == 1)
            idx = 2;
        else if (idx == 2)
            idx = 1;
    }

    switch (idx)
    {
    case 0:
        w.SetO2WTransform(x18_basewidget_menu->GetWorldTransform() *
                          zeus::CTransform::Translate(0.f, 0.f, translate) *
                          zeus::CTransform::Scale(scale));
    case 1:
        w.SetO2WTransform(x18_basewidget_menu->GetWorldTransform() *
                          zeus::CTransform::Translate(translate, 0.f, 0.f) *
                          zeus::CTransform::Scale(scale));
    case 2:
        w.SetO2WTransform(x18_basewidget_menu->GetWorldTransform() *
                          zeus::CTransform::Translate(0.f, 0.f, -translate) *
                          zeus::CTransform::Scale(scale));
    case 3:
        w.SetO2WTransform(x18_basewidget_menu->GetWorldTransform() *
                          zeus::CTransform::Translate(-translate, 0.f, 0.f) *
                          zeus::CTransform::Scale(scale));
    default: break;
    }
}

void CHudVisorBeamMenu::Update(float dt, bool init)
{
    bool curSwapBeamControls = g_GameState->GameOptions().GetSwapBeamControls();
    if (x80_24_swapBeamControls != curSwapBeamControls)
    {
        x80_24_swapBeamControls = curSwapBeamControls;
        EHudVisorBeamMenu swappedType;
        if (x80_24_swapBeamControls)
            swappedType = EHudVisorBeamMenu(1 - int(x4_type));
        else
            swappedType = x4_type;

        x18_basewidget_menu = x0_baseHud.FindWidget(BaseMenuNames[int(swappedType)]);
        x20_textpane_menu = static_cast<CGuiTextPane*>(x0_baseHud.FindWidget(TextNames[int(swappedType)]));
        x1c_basewidget_menutitle = x0_baseHud.FindWidget(BaseTitleNames[int(swappedType)]);

        for (int i=0 ; i<4 ; ++i)
        {
            SMenuItem& item = x28_menuItems[i];
            UpdateMenuWidgetTransform(i, *item.x4_model_icon, item.x8_positioner);
            UpdateMenuWidgetTransform(i, *item.x0_model_loz, 1.f);
        }

        UpdateMenuWidgetTransform(x8_selectedItem, *x24_model_ghost, x28_menuItems[x8_selectedItem].x8_positioner);
    }

    zeus::CColor activeColor = g_tweakGuiColors->GetVisorBeamMenuItemActive();
    zeus::CColor inactiveColor = g_tweakGuiColors->GetVisorBeamMenuItemInactive();
    zeus::CColor lozColor = g_tweakGuiColors->GetVisorBeamMenuLozColor();
    zeus::CColor tmpColors[4];

    for (int i=0 ; i<4 ; ++i)
    {
        SMenuItem& item = x28_menuItems[i];
        if (item.xc_opacity > 0.f)
            item.xc_opacity = std::min(item.xc_opacity + dt, 1.f);
        tmpColors[i] = zeus::CColor::lerp(activeColor, zeus::CColor::skClear, item.xc_opacity);
    }

    switch (x6c_animPhase)
    {
    case EAnimPhase::Steady:
        for (int i=0 ; i<4 ; ++i)
        {
            SMenuItem& item = x28_menuItems[i];
            zeus::CColor& color0 = (x8_selectedItem == i) ? activeColor : inactiveColor;
            zeus::CColor& color1 = (x8_selectedItem == i) ? lozColor : inactiveColor;
            zeus::CColor iconColor = (item.xc_opacity == 0.f) ? zeus::CColor::skClear : color0 + tmpColors[i];
            zeus::CColor lColor = (item.xc_opacity == 0.f) ? lozColor : color1 + tmpColors[i];
            item.x4_model_icon->SetColor(iconColor);
            item.x0_model_loz->SetColor(lColor);
            item.x8_positioner = (x8_selectedItem == i) ? 0.f : 1.f;
        }
        x24_model_ghost->SetColor(activeColor);
        break;
    case EAnimPhase::SelectFlash:
    {
        zeus::CColor color = zeus::CColor::skWhite;
        color.a = 0.f;
        x1c_basewidget_menutitle->SetColor(color);

        zeus::CColor& color0 = std::fmod(x10_interp, 0.1f) > 0.05f ? activeColor : inactiveColor;
        SMenuItem& item0 = x28_menuItems[xc_pendingSelection];
        color = color0 + tmpColors[xc_pendingSelection];
        item0.x4_model_icon->SetColor(color);
        item0.x0_model_loz->SetColor(color);

        SMenuItem& item1 = x28_menuItems[x8_selectedItem];
        color = zeus::CColor::lerp(inactiveColor, activeColor, x10_interp) + tmpColors[x8_selectedItem];
        item1.x4_model_icon->SetColor(color);
        item1.x0_model_loz->SetColor(lozColor);

        for (int i=0 ; i<4 ; ++i)
            x28_menuItems[i].x8_positioner = (x8_selectedItem == i) ? 1.f - x10_interp : 1.f;

        x24_model_ghost->SetColor(zeus::CColor::lerp(activeColor, inactiveColor, item1.x8_positioner));
        break;
    }
    case EAnimPhase::Animate:
        for (int i=0 ; i<4 ; ++i)
        {
            SMenuItem& item = x28_menuItems[i];
            zeus::CColor& color0 = (x8_selectedItem == i) ? activeColor : inactiveColor;
            zeus::CColor iconColor = (item.xc_opacity == 0.f) ? zeus::CColor::skClear : color0 + tmpColors[i];
            item.x4_model_icon->SetColor(iconColor);
            item.x0_model_loz->SetColor((item.xc_opacity == 0.f || x8_selectedItem == i) ? lozColor : inactiveColor);
            item.x8_positioner = (x8_selectedItem == i) ? 1.f - x10_interp : 1.f;
        }
        x24_model_ghost->SetColor(zeus::CColor::lerp(activeColor, inactiveColor, x28_menuItems[x8_selectedItem].x8_positioner));
        break;
    default: break;
    }

    if (x78_textFader > 0.f)
    {
        x78_textFader = std::max(0.f, x78_textFader - dt);
        zeus::CColor color = zeus::CColor::skWhite;
        color.a = x78_textFader / x7c_animDur;
        x1c_basewidget_menutitle->SetColor(color);
    }

    if (x14_26_dirty || init)
    {
        x14_26_dirty = false;
        for (int i=0 ; i<4 ; ++i)
        {
            SMenuItem& item = x28_menuItems[i];
            UpdateMenuWidgetTransform(i, *item.x4_model_icon, item.x8_positioner);
        }
        UpdateMenuWidgetTransform(x8_selectedItem, *x24_model_ghost, x28_menuItems[x8_selectedItem].x8_positioner);
    }

    if (x1c_basewidget_menutitle->GetGeometryColor().a)
        x1c_basewidget_menutitle->SetVisibility(true, ETraversalMode::Children);
    else
        x1c_basewidget_menutitle->SetVisibility(false, ETraversalMode::Children);

    for (int i=0 ; i<4 ; ++i)
    {
        SMenuItem& item = x28_menuItems[i];
        if (item.x4_model_icon->GetGeometryColor().a)
            item.x4_model_icon->SetIsVisible(true);
        else
            item.x4_model_icon->SetIsVisible(false);
    }
}

void CHudVisorBeamMenu::UpdateHudAlpha(float alpha)
{
    zeus::CColor color = zeus::CColor::skWhite;
    color.a = g_GameState->GameOptions().GetHUDAlpha() / 255.f * alpha;
    x18_basewidget_menu->SetColor(color);
}

void CHudVisorBeamMenu::SetIsVisibleGame(bool v)
{
    x14_25_visibleGame = v;
    bool vis = x14_24_visibleDebug && x14_25_visibleGame;
    x18_basewidget_menu->SetVisibility(vis, ETraversalMode::Children);
    if (vis)
        Update(0.f, true);
}

void CHudVisorBeamMenu::SetPlayerHas(const rstl::prereserved_vector<bool, 4>& enables)
{
    for (int i=0 ; i<4 ; ++i)
    {
        SMenuItem& item = x28_menuItems[i];
        if (item.xc_opacity == 0.f && enables[i])
            item.xc_opacity = FLT_EPSILON;
    }
}

void CHudVisorBeamMenu::SetSelection(int selection, int pending, float interp)
{
    if (x8_selectedItem == selection && xc_pendingSelection == pending && x10_interp == interp)
        return;

    if (pending != selection)
    {
        if (x6c_animPhase != EAnimPhase::SelectFlash)
            CSfxManager::SfxStart(SelectionSfxs[int(x4_type)], 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        x6c_animPhase = EAnimPhase::SelectFlash;
    }
    else if (interp < 1.f)
    {
        x6c_animPhase = EAnimPhase::Animate;
        x20_textpane_menu->TextSupport().SetText(g_MainStringTable->GetString(MenuStringIdx[int(x4_type)][x8_selectedItem]));
        x20_textpane_menu->TextSupport().SetTypeWriteEffectOptions(true, 0.1f, 16.f);
    }
    else
    {
        if (x6c_animPhase != EAnimPhase::Steady)
            x78_textFader = x7c_animDur;
        x6c_animPhase = EAnimPhase::Steady;
    }

    x14_26_dirty = true;
    x8_selectedItem = selection;
    xc_pendingSelection = pending;
    x10_interp = interp;
}

}
