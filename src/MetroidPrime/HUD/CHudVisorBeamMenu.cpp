#include "MetroidPrime/HUD/CHudVisorBeamMenu.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/math.hpp"

#include <float.h>

static const char* const skBaseWidgetNames[] = {"BaseWidget_VisorMenu", "BaseWidget_BeamMenu"};
static const char* const skMenuTitleWidgetNames[] = {"TextPane_VisorMenu", "TextPane_BeamMenu"};
static const char* const skBaseTitleWidgetNames[] = {"basewidget_visormenutitle",
                                                     "basewidget_beammenutitle"};
static const char* const skVisorWidgetBaseName = "model_visor";
static const char* const skBeamWidgetBaseName = "model_beam";
static const char* const skLozSuffix = "loz";
static const char* const skIconSuffix = "icon";
static const char* const skGhostSuffix = "ghost";
static const char skVisorWidgetIndices[] = "1032";
static const char skBeamWidgetIndices[] = "3210";
static const ushort skSelectionSounds[] = {0x573, 0x576};
static const int skMenuStringIndices[2][4] = {{0, 2, 1, 3}, {4, 5, 6, 7}};

CHudVisorBeamMenu::CHudVisorBeamMenu(CGuiFrame& frame, EVisorBeamMenu type,
                                     const rstl::reserved_vector< bool, 4 >& enables)
: mBaseHud(frame)
, mType(type)
, mSelectedItem(0)
, mPendingSelection(0)
, mInterp(1.f)
, mVisibleDebug(true)
, mVisibleGame(true)
, mDirty(true)
, mMenuItems(4, SMenuItem())
, mAnimPhase(kAP_Steady)
, x70_(FLT_EPSILON)
, x74_(FLT_EPSILON)
, mTextFader(0.f) {
  mAnimDur = gpTweakGui->GetBeamVisorMenuAnimTime();
  mSwapBeamControls = gpGameState->GameOptions().GetSwapBeamControls();
  mTextpane_menu = static_cast< CGuiTextPane* >(
      frame.FindWidget(skMenuTitleWidgetNames[mSwapBeamControls ? 1 - mType : mType]));
  mBasewidget_menutitle =
      frame.FindWidget(skBaseTitleWidgetNames[mSwapBeamControls ? 1 - mType : mType]);
  mBasewidget_menu =
      frame.FindWidget(skBaseWidgetNames[mSwapBeamControls ? 1 - mType : mType]);
  const char* ghostName = CBasics::Stringize(
      "%s%s", mType == kVBM_Visor ? skVisorWidgetBaseName : skBeamWidgetBaseName, skGhostSuffix);
  mModel_ghost = static_cast< CGuiModel* >(frame.FindWidget(ghostName));
  for (int i = 0; i < 4; ++i) {
    const char* const baseName =
        mType == kVBM_Visor ? skVisorWidgetBaseName : skBeamWidgetBaseName;
    const char* const indices = mType == kVBM_Visor ? skVisorWidgetIndices : skBeamWidgetIndices;
    const char* lozName = CBasics::Stringize("%s%s%c", baseName, skLozSuffix, indices[i]);
    mMenuItems[i].mModel_loz = static_cast< CGuiModel* >(frame.FindWidget(lozName));
    const char* iconName = CBasics::Stringize("%s%s%c", baseName, skIconSuffix, indices[i]);
    mMenuItems[i].mModel_icon = static_cast< CGuiModel* >(frame.FindWidget(iconName));
    mMenuItems[i].mOpacity = enables[i] ? 1.f : 0.f;
  }
  if (mType == kVBM_Visor) {
    mTextpane_menu->TextSupport().SetFontColor(gpTweakGuiColors->GetVisorMenuTextFont());
    mTextpane_menu->TextSupport().SetOutlineColor(gpTweakGuiColors->GetVisorMenuTextOutline());
  } else {
    mTextpane_menu->TextSupport().SetFontColor(gpTweakGuiColors->GetBeamMenuTextFont());
    mTextpane_menu->TextSupport().SetOutlineColor(gpTweakGuiColors->GetBeamMenuTextOutline());
  }
  mBasewidget_menutitle->SetColor(CColor::White().WithAlphaOf(0.f));
  mTextpane_menu->TextSupport().SetText(
      rstl::wstring_l(gpStringTable->GetString(skMenuStringIndices[mType][mSelectedItem])));
  for (int i = 0; i < 4; ++i) {
    mMenuItems[i].mModel_loz->SetColor(gpTweakGuiColors->GetVisorBeamMenuLozColor());
    UpdateMenuWidgetTransform(i, *mMenuItems[i].mModel_loz, 1.f);
  }
  Update(0.f, true);
}

void CHudVisorBeamMenu::SetSelection(int selection, int pending, float interp) {
  if (mSelectedItem == selection && mPendingSelection == pending && interp == mInterp) {
    return;
  }
  if (pending != selection) {
    if (mAnimPhase != kAP_SelectFlash) {
      CSfxManager::SfxStart(skSelectionSounds[mType]);
    }
    mAnimPhase = kAP_SelectFlash;
  } else if (interp < 1.f) {
    mAnimPhase = kAP_Animate;
    mTextpane_menu->TextSupport().SetText(
        rstl::wstring_l(gpStringTable->GetString(skMenuStringIndices[mType][mSelectedItem])));
    mTextpane_menu->TextSupport().SetTypeWriteEffectOptions(true, 0.1f, 16.f);
  } else {
    if (mAnimPhase != kAP_Steady) {
      mTextFader = mAnimDur;
    }
    mAnimPhase = kAP_Steady;
  }
  mDirty = true;
  mSelectedItem = selection;
  mPendingSelection = pending;
  mInterp = interp;
}

void CHudVisorBeamMenu::SetPlayerHas(const rstl::reserved_vector< bool, 4 >& enables) {
  for (int i = 0; i < 4; ++i) {
    SMenuItem& item = mMenuItems[i];
    if (item.mOpacity == 0.f && enables[i]) {
      item.mOpacity = FLT_EPSILON;
    }
  }
}

void CHudVisorBeamMenu::UpdateHudAlpha(float alpha) {
  const float hudAlpha = gpGameState->GameOptions().GetHudAlpha();
  mBasewidget_menu->SetColor(CColor::White().WithAlphaOf(alpha * hudAlpha));
}

void CHudVisorBeamMenu::Update(float dt, const bool init) {
  const bool swapBeamControls = gpGameState->GameOptions().GetSwapBeamControls();
  if (swapBeamControls != mSwapBeamControls) {
    mSwapBeamControls = swapBeamControls;
    mBasewidget_menu = mBaseHud.FindWidget(skBaseWidgetNames[GetSwappedType()]);
    mTextpane_menu->TextSupport().SetText(rstl::wstring_l(L""));
    mTextpane_menu = static_cast< CGuiTextPane* >(
        mBaseHud.FindWidget(skMenuTitleWidgetNames[GetSwappedType()]));
    mBasewidget_menutitle = mBaseHud.FindWidget(skBaseTitleWidgetNames[GetSwappedType()]);
    for (int i = 0; i < 4; ++i) {
      SMenuItem& item = mMenuItems[i];
      UpdateMenuWidgetTransform(i, *item.mModel_icon, item.mPositioner);
      UpdateMenuWidgetTransform(i, *item.mModel_loz, 1.f);
    }
    UpdateMenuWidgetTransform(mSelectedItem, *mModel_ghost,
                              mMenuItems[mSelectedItem].mPositioner);
  }
  const CTweakGuiColors& guiColors = *gpTweakGuiColors;
  const CColor* const activeColor = &guiColors.GetVisorBeamMenuItemActive();
  const CColor* const inactiveColor = &guiColors.GetVisorBeamMenuItemInactive();
  const CColor* const lozColor = &guiColors.GetVisorBeamMenuLozColor();
  rstl::reserved_vector< CColor, 4 > colors;
  for (int i = 0; i < 4; ++i) {
    SMenuItem& item = mMenuItems[i];
    if (item.mOpacity > 0.f) {
      item.mOpacity = rstl::min_val(1.f, item.mOpacity + dt);
    }
    colors.push_back(CColor::Lerp(*activeColor, CColor(0), item.mOpacity));
  }
  switch (mAnimPhase) {
  case kAP_None:
    break;
  case kAP_Steady: {
    const CColor inactive = *inactiveColor;
    for (int i = 0; i < 4; ++i) {
      SMenuItem& item = mMenuItems[i];
      const CColor& iconBase = i == mSelectedItem ? *activeColor : inactive;
      const CColor& lozBase = i == mSelectedItem ? *lozColor : *inactiveColor;
      const CColor icon = item.mOpacity == 0.f ? CColor(0) : CColor::Add(iconBase, colors[i]);
      const CColor loz = item.mOpacity == 0.f ? *lozColor : CColor::Add(lozBase, colors[i]);
      item.mModel_icon->SetColor(icon);
      item.mModel_loz->SetColor(loz);
      item.mPositioner = i == mSelectedItem ? 0.f : 1.f;
    }
    mModel_ghost->SetColor(*activeColor);
    break;
  }
  case kAP_SelectFlash: {
    mBasewidget_menutitle->SetColor(CColor::White().WithAlphaOf(0.f));
    const CColor& flashBase = CMath::ModF(mInterp, 0.1f) > 0.05f ? *activeColor : *inactiveColor;
    const CColor flash = CColor::Add(flashBase, colors[mPendingSelection]);
    mMenuItems[mPendingSelection].mModel_icon->SetColor(flash);
    mMenuItems[mPendingSelection].mModel_loz->SetColor(flash);
    const CColor base = CColor::Lerp(*inactiveColor, *activeColor, mInterp);
    const CColor selectedColor = CColor::Add(base, colors[mSelectedItem]);
    mMenuItems[mSelectedItem].mModel_icon->SetColor(selectedColor);
    mMenuItems[mSelectedItem].mModel_loz->SetColor(*lozColor);
    for (int i = 0; i < 4; ++i) {
      mMenuItems[i].mPositioner = i == mSelectedItem ? 1.f - mInterp : 1.f;
    }
    mModel_ghost->SetColor(
        CColor::Lerp(*activeColor, *inactiveColor, mMenuItems[mSelectedItem].mPositioner));
    break;
  }
  case kAP_Animate:
    for (int i = 0; i < 4; ++i) {
      SMenuItem& item = mMenuItems[i];
      const CColor& iconBase = i == mSelectedItem ? *activeColor : *inactiveColor;
      CGuiModel* iconModel = item.mModel_icon;
      const CColor icon = item.mOpacity == 0.f ? CColor(0) : CColor::Add(iconBase, colors[i]);
      iconModel->SetColor(icon);
      const CColor loz =
          item.mOpacity == 0.f || i == mSelectedItem ? *lozColor : *inactiveColor;
      item.mModel_loz->SetColor(loz);
      item.mPositioner = i == mSelectedItem ? 1.f - mInterp : 1.f;
    }
    mModel_ghost->SetColor(
        CColor::Lerp(*activeColor, *inactiveColor, mMenuItems[mSelectedItem].mPositioner));
    break;
  default:
    break;
  }
  if (mTextFader > 0.f) {
    mTextFader = rstl::max_val(0.f, mTextFader - dt);
    const float alpha = mTextFader / mAnimDur;
    mBasewidget_menutitle->SetColor(CColor::White().WithAlphaOf(alpha));
  }
  if (mDirty || init) {
    mDirty = false;
    for (int i = 0; i < 4; ++i) {
      SMenuItem& item = mMenuItems[i];
      UpdateMenuWidgetTransform(i, *item.mModel_icon, item.mPositioner);
    }
    UpdateMenuWidgetTransform(mSelectedItem, *mModel_ghost,
                              mMenuItems[mSelectedItem].mPositioner);
  }
  if (!GetIsVisible()) {
    return;
  }
  if (mBasewidget_menutitle->GetModifiedColor().GetAlphau8()) {
    mBasewidget_menutitle->SetVisibility(true, kTM_Children);
  } else {
    mBasewidget_menutitle->SetVisibility(false, kTM_Children);
  }
  for (int i = 0; i < 4; ++i) {
    SMenuItem& item = mMenuItems[i];
    if (item.mModel_icon->GetModifiedColor().GetAlphau8()) {
      item.mModel_icon->SetIsVisible(true);
    } else {
      item.mModel_icon->SetIsVisible(false);
    }
  }
}

void CHudVisorBeamMenu::SetIsVisibleGame(const bool visible) {
  mVisibleGame = visible;
  const bool isVisible = GetIsVisible();
  mBasewidget_menu->SetVisibility(isVisible, kTM_Children);
  if (isVisible) {
    Update(0.f, true);
  }
}

void CHudVisorBeamMenu::UpdateMenuWidgetTransform(int idx, CGuiWidget& widget, float t) {
  t = CMath::AbsF(t);
  const float translate = t * gpTweakGui->GetVisorBeamMenuItemTranslate();
  const float scale = t * gpTweakGui->GetVisorBeamMenuItemInactiveScale() +
                      (1.f - t) * gpTweakGui->GetVisorBeamMenuItemActiveScale();
  if (mType == kVBM_Visor) {
    if (idx == 2) {
      idx = 3;
    } else if (idx == 3) {
      idx = 2;
    }
  } else {
    if (idx == 1) {
      idx = 2;
    } else if (idx == 2) {
      idx = 1;
    }
  }
  switch (idx) {
  case 0:
    widget.SetO2WTransform(mBasewidget_menu->GetWorldTransform() *
                           CTransform4f::Translate(0.f, 0.f, translate) *
                           CTransform4f::Scale(scale));
    break;
  case 1:
    widget.SetO2WTransform(mBasewidget_menu->GetWorldTransform() *
                           CTransform4f::Translate(translate, 0.f, 0.f) *
                           CTransform4f::Scale(scale));
    break;
  case 2:
    widget.SetO2WTransform(mBasewidget_menu->GetWorldTransform() *
                           CTransform4f::Translate(0.f, 0.f, -translate) *
                           CTransform4f::Scale(scale));
    break;
  case 3:
    widget.SetO2WTransform(mBasewidget_menu->GetWorldTransform() *
                           CTransform4f::Translate(-translate, 0.f, 0.f) *
                           CTransform4f::Scale(scale));
    break;
  default:
    break;
  }
}
