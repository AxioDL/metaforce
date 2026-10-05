#include "MetroidPrime/CFrontEndUI.hpp"

#include "GameVersions.h"

#include "Kyoto/Audio/CAudioGroupSet.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStaticAudioPlayer.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CMoviePlayer.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Text/CStringTable.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiTextSupport.hpp"
#include "GuiSys/CGuiWidget.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"

#include "MetroidPrime/CGBASupport.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CNESEmulator.hpp"
#include "MetroidPrime/CQuitGameScreen.hpp"
#include "MetroidPrime/CSaveGameScreen.hpp"
#include "MetroidPrime/CSaveWorldMemory.hpp"
#include "MetroidPrime/CSlideShow.hpp"
#include "MetroidPrime/CStateSetterFlow.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/SOptionsFrontEndFrame.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include "MetroidPrime/SFX/FrontEnd.h"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"

#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct FEMovie {
  const char* path;
  bool loop;
};

static const FEMovie FEMovies[] = {
    {"Video/00_first_start.thp", false},
    {"Video/01_startloop.thp", true},
    {"Video/02_start_fileselect_A.thp", false},
    {"Video/03_fileselectloop.thp", true},
    {"Video/04_fileselect_playgame_A.thp", false},
    {"Video/06_fileselect_GBA.thp", false},
    {"Video/07_GBAloop.thp", true},
    {"Video/08_GBA_fileselect.thp", false},
#if VERSION >= VERSION_GM8P_00
    {"Video/back.thp", false},
#else
    {"Video/08_GBA_fileselect.thp", false},
#endif
};
#if VERSION >= VERSION_GM8P_00
static const char* const skPressStartTextures[] = {
    "TXTR_PressStart", "TXTR_GerStart",   "TXTR_FreStart",   "TXTR_SpaStart",
    "TXTR_ItaStart",   "TXTR_PressStart", "TXTR_PressStart",
};
#endif

// TODO: set this to the default value and bake version string as a post-build step
#if VERSION == VERSION_GM8E_00
const char MetroidBuildInfo[] = BUILD_INFO_TAG "Build v1.088 10/29/2002 2:21:25\0PAD";
#elif VERSION == VERSION_GM8E_01
const char MetroidBuildInfo[] = BUILD_INFO_TAG "Build v1.093 11/5/2002 19:50:01\0PAD";
#elif VERSION == VERSION_GM8E_02
const char MetroidBuildInfo[] = BUILD_INFO_TAG "Build v1.111 3/10/2003 17:56:21\0PAD";
#elif VERSION == VERSION_GM8P_00
const char MetroidBuildInfo[] = BUILD_INFO_TAG "Build v1.110 2/4/2003 22:16:07\0_PAD";
#else
const char MetroidBuildInfo[] = BUILD_INFO;
#endif
const char* const BuildTime = MetroidBuildInfo + BUILD_INFO_TAG_SIZE;

static const s16 FETransitionBackSFX[3][2] = {
    {SFXfnt_transfore_00L, SFXfnt_transfore_00R},
    {SFXfnt_transfore_01L, SFXfnt_transfore_01R},
    {SFXfnt_transfore_02L, SFXfnt_transfore_02R},
};

static const s16 FETransitionForwardSFX[3][2] = {
    {SFXfnt_transback_00L, SFXfnt_transback_00R},
    {SFXfnt_transback_01L, SFXfnt_transback_01R},
    {SFXfnt_transback_02L, SFXfnt_transback_02R},
};

static const char* const kAudioFrontend1Path = "Audio/frontend_1.rsf";
static const char* const kAudioFrontend2Path = "Audio/frontend_2.rsf";
static const char* const kFrontEndAGSCName = "FrontEnd_AGSC";

static const float AudioFadeTimeB[3] = {
    4.2f,
    6.1f,
    6.1f,
};

static const float AudioFadeTimeA[3] = {
    0.43999997f,
    5.4100003f,
    3.4100003f,
};

#if VERSION >= VERSION_GM8P_00
static float AudioFadeTimeBack = AudioFadeTimeA[0];
#endif

#include "MetroidPrime/DefaultWorld.hpp"

static const CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType NextLinkUI[10] = {
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_ConnectSocket,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_PressStartAndSelect,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_BeginLink,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Linking,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Empty,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_TurnOffGBA,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Complete,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_InsertPak,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Empty,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Empty,
};

static const CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType PrevLinkUI[10] = {
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Cancelled,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Cancelled,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Cancelled,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Cancelled,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Empty,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Cancelled,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Empty,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Cancelled,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Empty,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::kUIT_Empty,
};

void CFrontEndUI::PlayAdvanceSfx() {
  CSfxManager::SfxStart(SFXui_x_start_00);
  CSfxManager::SfxStart(SFXui_x_start_00r);
}

#if VERSION >= VERSION_GM8P_00
extern bool sIs50Hz;

void CFrontEndUI::SetTitlePosition(CGuiFrame& frame) {
  if (sIs50Hz) {
    CGuiWidget* title = frame.FindWidget("textpane_title");
    title->LocalTransform() = title->GetTransform() * CTransform4f::Translate(0.f, 0.f, -0.5f);
    title->RecalculateTransforms();
    CGuiWidget* titleB = frame.FindWidget("textpane_titleb");
    titleB->LocalTransform() = titleB->GetTransform() * CTransform4f::Translate(0.f, 0.f, -0.5f);
    titleB->RecalculateTransforms();
  }
}

#endif

void CFrontEndUI::SFrontEndFrame::FindAndSetPairText(CGuiFrame& frame, const char* name,
                                                     const wchar_t* str) {
  CGuiTextPane* w1 = static_cast< CGuiTextPane* >(frame.FindWidget(name));
  w1->TextSupport().SetText(rstl::wstring(str));
  CGuiTextPane* w2 =
      static_cast< CGuiTextPane* >(frame.FindWidget(CBasics::Stringize("%sb", name)));
  w2->TextSupport().SetText(rstl::wstring(str));
}

CFrontEndUI::SGuiTextPair::SGuiTextPair() {
  mTextPane = nullptr;
  mTextPaneB = nullptr;
}

CFrontEndUI::SGuiTextPair::SGuiTextPair(const CGuiFrame* frame, const char* name) {
  mTextPane = static_cast< CGuiTextPane* >(frame->FindWidget(name));
  mTextPaneB = static_cast< CGuiTextPane* >(frame->FindWidget(CBasics::Stringize("%sb", name)));
}

void CFrontEndUI::SGuiTextPair::SetPairText(const wchar_t* str) {
  mTextPane->TextSupport().SetText(rstl::wstring(str));
  mTextPaneB->TextSupport().SetText(rstl::wstring(str));
}

void CFrontEndUI::SGuiTextPair::SetPairText(const rstl::wstring& str) {
  mTextPane->TextSupport().SetText(str);
  mTextPaneB->TextSupport().SetText(str);
}

#if VERSION >= VERSION_GM8P_00
CFrontEndUI::SLanguageSelectFrame::SLanguageSelectFrame()
: mFrame(gpSimplePool->GetObj("FRME_LanguageSelect"))
, mStrings(gpSimplePool->GetObj("STRG_LanguageSelect"))
, mInitialLanguage(gpGameState->SystemState().GetLanguage())
, mLoadedFrame(*mFrame)
, mActive(true)
, mCanDraw(true) {
  mTablegroup_menu =
      static_cast< CGuiTableGroup* >(mLoadedFrame->FindWidget("tablegroup_menu"));
  mTablegroup_menu->SetUserSelection(gpGameState->SystemState().GetLanguage());
  SetTableColors();
  mTablegroup_menu->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< SLanguageSelectFrame, CGuiTableGroup* const, const int >::Make(
          *this, &SLanguageSelectFrame::DoSelectionChange));
  mTablegroup_menu->SetMenuCancelCallback(
      TFunctor1FromMethod< SLanguageSelectFrame, CGuiTableGroup* const >::Make(
          *this, &SLanguageSelectFrame::DoCancel));
}

CFrontEndUI::SLanguageSelectFrame::~SLanguageSelectFrame() {}

void CFrontEndUI::SLanguageSelectFrame::Update(float dt, CSaveGameScreen* saveUI) {
  mCanDraw = saveUI == nullptr || saveUI->GetUIType() == CSaveGameScreen::kUIT_SaveReady;
  mLoadedFrame->Update(dt);
  CGuiTextPane* title = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_title"));
  title->TextSupport().SetText(rstl::wstring(mStrings->GetString(0)));

  for (int i = 0; i < 5; ++i) {
    char name[32];
    sprintf(name, "%s%d", "textpane_choice", i);
    CGuiTextPane* choice = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget(name));
    choice->TextSupport().SetText(rstl::wstring(mStrings->GetString(i + 1)));
  }

  CGuiTextPane* proceed =
      static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_proceed"));
  if (proceed != nullptr) {
    proceed->TextSupport().SetText(rstl::wstring(gpStringTable->GetString(0x4c)));
  }
}

bool CFrontEndUI::SLanguageSelectFrame::ProcessUserInput(const CFinalInput& input,
                                                         CSaveGameScreen* saveUI) {
  mActive = true;
  if (saveUI != nullptr) {
    saveUI->ProcessUserInput(input);
  }
  if (mCanDraw) {
    mLoadedFrame->ProcessUserInput(input);
  }
  if (!mActive && mInitialLanguage != gpGameState->SystemState().GetLanguage() &&
      saveUI != nullptr) {
    saveUI->SaveNESState();
  }
  return mActive;
}

void CFrontEndUI::SLanguageSelectFrame::Draw() const {
  if (mCanDraw) {
    mLoadedFrame->Draw(CGuiWidgetDrawParms::sDefaultDrawParms);
  }
}

void CFrontEndUI::SLanguageSelectFrame::DoSelectionChange(CGuiTableGroup* caller,
                                                          int oldSelection) {
  CSfxManager::SfxStart(SFXfnt_selection_change, 0x7f, 0x40, false, CSfxManager::kMedPriority,
                        false, CSfxManager::kAllAreas);
  SetTableColors();
  const int language = mTablegroup_menu->GetUserSelection();
  if (language != gpGameState->SystemState().GetLanguage()) {
    gpGameState->SystemState().SetLanguage(language);
    CStringTable::SetLanguage(language);
    CMain::ReloadStringTables();
  }
}

void CFrontEndUI::SLanguageSelectFrame::DoCancel(CGuiTableGroup* caller) {
  CSfxManager::SfxStart(SFXfnt_back, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                        CSfxManager::kAllAreas);
  mActive = false;
}

void CFrontEndUI::SLanguageSelectFrame::SetTableColors() {
  const CColor selected(static_cast< uchar >(0xff), static_cast< uchar >(0xff), static_cast< uchar >(0xff),
                        static_cast< uchar >(0xff));
  const CColor unselected(static_cast< uchar >(0xa0), static_cast< uchar >(0xa0), static_cast< uchar >(0xa0),
                          static_cast< uchar >(0xc8));
  mTablegroup_menu->SetColors(selected, unselected);
}

#endif

CFrontEndUI::SNesEmulatorFrame::SNesEmulatorFrame()
: mMode(kEM_Emulator)
, mNesEmu(rs_new CNESEmulator())
, mQuitScreen(nullptr)
, mTextSupport(nullptr)
, mRemTime(8.f)
#if VERSION >= VERSION_GM8P_00
, x14_(0.f)
#endif
, mEmulationSuspended(false)
, mEnableFiltering(true) {
  mTextSupport = rs_new CGuiTextSupport(
      gpResourceFactory->GetResourceIdByName("FONT_Deface14B")->GetId(),
#if VERSION >= VERSION_GM8P_00
      0, 0,
#endif
      CGuiTextProperties(false, true, kJustification_Left, kVerticalJustification_Center, nullptr),
      CColor(0xFFFFFFFFu), CColor::Black(), CColor(0xFFFFFFFFu),
#if VERSION < VERSION_GM8P_00
      0, 0,
#endif
      gpSimplePool);
#if VERSION >= VERSION_GM8P_00
  mTextSupport->SetText(rstl::wstring(gpStringTable->GetString(0x61)));
#else
  mTextSupport->SetText(rstl::wstring_l(gpStringTable->GetString(0x67)));
#endif
  mTextSupport->SetExtentX(mTextSupport->GetBounds().second.GetX());
  mTextSupport->SetExtentY(mTextSupport->GetBounds().second.GetY());
}

CFrontEndUI::SNesEmulatorFrame::~SNesEmulatorFrame() {}

void CFrontEndUI::SNesEmulatorFrame::SetMode(EMode mode) {
  switch (mode) {
  case kEM_Emulator:
    mQuitScreen = nullptr;
    break;
  case kEM_SaveProgress:
    mQuitScreen = nullptr;
    mQuitScreen = rs_new CQuitGameScreen(kQT_SaveProgress);
    break;
  case kEM_ContinuePlaying:
    mQuitScreen = nullptr;
    mQuitScreen = rs_new CQuitGameScreen(kQT_ContinuePlaying);
    break;
  case kEM_QuitNESMetroid:
    mQuitScreen = nullptr;
    mQuitScreen = rs_new CQuitGameScreen(kQT_QuitNESMetroid);
    break;
  default:
    break;
  }
  mMode = mode;
}

int CFrontEndUI::SNesEmulatorFrame::Update(float dt, CSaveGameScreen* saveUi) {
  const bool doUpdate = saveUi == nullptr || saveUi->GetUIType() == CSaveGameScreen::kUIT_SaveReady;
  mRemTime = rstl::max_val(0.f, mRemTime - dt);

  mTextSupport->SetGeometryColor(CColor::White().WithAlphaOf(rstl::min_val(mRemTime, 1.f)));

  if (mTextSupport->GetIsTextSupportFinishedLoading()) {
    mTextSupport->SetExtentX(mTextSupport->GetBounds().second.GetX());
    mTextSupport->SetExtentY(mTextSupport->GetBounds().second.GetY());
  }

  if (doUpdate) {
    switch (mMode) {
    case kEM_Emulator: {
      mNesEmu->Update();
      if (!mNesEmu->IsGameOver()) {
        mEmulationSuspended = false;
      }
      if (mNesEmu->IsGameOver() && !mEmulationSuspended) {
        mEmulationSuspended = true;
        if (saveUi != nullptr && !saveUi->IsSavingDisabled()) {
          SetMode(kEM_SaveProgress);
          break;
        }
        SetMode(kEM_ContinuePlaying);
        break;
      }
      if (mNesEmu->GetPasswordEntryState() == CNESEmulator::kPES_NotEntered) {
        if (saveUi != nullptr) {
          mNesEmu->LoadPassword(gpGameState->SystemState().GetNESState());
        }
      }
      break;
    }
    case kEM_SaveProgress: {
      if (saveUi != nullptr) {
        EQuitAction action = mQuitScreen->Update(dt);
        if (action == kQA_Yes) {
          memcpy(gpGameState->SystemState().GetNESState(), mNesEmu->GetPassword(), 0x12);
          saveUi->SaveNESState();
          SetMode(kEM_ContinuePlaying);
        } else if (action == kQA_No) {
          SetMode(kEM_ContinuePlaying);
        }
      } else {
        SetMode(kEM_ContinuePlaying);
      }
      break;
    }
    case kEM_ContinuePlaying: {
      EQuitAction action = mQuitScreen->Update(dt);
      if (action == kQA_Yes) {
        SetMode(kEM_Emulator);
      } else if (action == kQA_No) {
        return 1;
      }
      break;
    }
    case kEM_QuitNESMetroid: {
      EQuitAction action = mQuitScreen->Update(dt);
      if (action == kQA_Yes) {
        return 1;
      } else if (action == kQA_No) {
        SetMode(kEM_Emulator);
      }
      break;
    }
    default:
      break;
    }
  }

  return 0;
}

void CFrontEndUI::SNesEmulatorFrame::ProcessUserInput(const CFinalInput& input,
                                                      CSaveGameScreen* sui) {
  bool doInput = true;
  if (sui != nullptr && sui->GetUIType() != CSaveGameScreen::kUIT_SaveReady) {
    doInput = false;
  }

  if (sui != nullptr) {
    sui->ProcessUserInput(input);
  }

  if (!doInput) {
    return;
  }

  switch (mMode) {
  case kEM_Emulator:
    mNesEmu->ProcessUserInput(input, 4);
    if (input.ControllerNumber() == 0 && input.PL()) {
      SetMode(kEM_QuitNESMetroid);
    }
    break;
  case kEM_SaveProgress:
  case kEM_ContinuePlaying:
  case kEM_QuitNESMetroid:
    mQuitScreen->ProcessUserInput(input);
    break;
  default:
    break;
  }
}

void CFrontEndUI::SNesEmulatorFrame::Draw(CSaveGameScreen* saveUi) const {
  CColor mulColor = CColor::White();
  CColor dimColor(static_cast< uchar >(0x60), static_cast< uchar >(0x60),
                  static_cast< uchar >(0x60), static_cast< uchar >(0xFF));

  bool blackout = false;
  if (saveUi != nullptr && saveUi->GetUIType() != CSaveGameScreen::kUIT_SaveReady) {
    blackout = true;
  }

  if (blackout) {
    mulColor = CColor(0u);
  } else if (mQuitScreen.get() != nullptr) {
    mulColor = dimColor;
  }

  mNesEmu->Draw(mulColor, mEnableFiltering);

  if (!blackout && mQuitScreen.get() != nullptr) {
    mQuitScreen->Draw();
  } else {
    CGraphics::SetUseVideoFilter(mEnableFiltering);
  }

  if (mRemTime < 7.5f && mRemTime > 0.f) {
    if (mTextSupport->GetIsTextSupportFinishedLoading()) {
      CGraphics::SetCullMode(kCM_None);
      gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
      gpRender->SetBlendMode_AlphaBlended();
      gpRender->SetDepthReadWrite(false, false);
      CGraphics::SetModelMatrix(CTransform4f::Translate(-280.f, 0.f, -160.f));
      mTextSupport->Render();
    }
  }
}

CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::SGBALinkFrame(CGuiFrame* frme, CGBASupport* support,
                                                             bool linkInProgress)
: mUiType(kUIT_Empty)
, mGbaSupport(support)
, mFrme(frme)
, mTextpane_instructions()
, mTextpane_yes(nullptr)
, mTextpane_no(nullptr)
, mModel_gc(nullptr)
, mModel_gba(nullptr)
, mModel_cable(nullptr)
, mModel_circlegcport(nullptr)
, mModel_circlegbaport(nullptr)
, mModel_circlestartselect(nullptr)
, mModel_pakout(nullptr)
, mModel_gbascreen(nullptr)
, mModel_connect(nullptr)
, mLinkInProgress(linkInProgress) {
  support->InitializeSupport();
  FinishedLoading();
}

CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::~SGBALinkFrame() {}

void CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::Draw() {
  mFrme->Draw(CGuiWidgetDrawParms::sDefaultDrawParms);
}

void CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::FinishedLoading() {
  mTextpane_instructions = SGuiTextPair(mFrme, "textpane_instructions");
  mTextpane_yes = static_cast< CGuiTextPane* >(mFrme->FindWidget("textpane_yes"));
  mTextpane_no = static_cast< CGuiTextPane* >(mFrme->FindWidget("textpane_no"));
  mModel_gc = static_cast< CGuiModel* >(mFrme->FindWidget("model_gc"));
  mModel_gba = static_cast< CGuiModel* >(mFrme->FindWidget("model_gba"));
  mModel_cable = static_cast< CGuiModel* >(mFrme->FindWidget("model_cable"));
  mModel_circlegcport = static_cast< CGuiModel* >(mFrme->FindWidget("model_circlegcport"));
  mModel_circlegbaport = static_cast< CGuiModel* >(mFrme->FindWidget("model_circlegbaport"));
  mModel_circlestartselect =
      static_cast< CGuiModel* >(mFrme->FindWidget("model_circlestartselect"));
  mModel_pakout = static_cast< CGuiModel* >(mFrme->FindWidget("model_pakout"));
  mModel_gbascreen = static_cast< CGuiModel* >(mFrme->FindWidget("model_gbascreen"));
  mModel_connect = static_cast< CGuiModel* >(mFrme->FindWidget("model_connect"));
  SetUIText(kUIT_InsertPak);
}

void CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::Update(float dt) {
  mGbaSupport->Update(dt);
  mFrme->Update(dt);
}

CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EAction
CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::ProcessUserInput(const CFinalInput& input,
                                                                bool linkInProgress) {
  if (linkInProgress != mLinkInProgress) {
    mLinkInProgress = linkInProgress;
    SetUIText(mUiType);
  }

  switch (mUiType) {
  case kUIT_InsertPak:
  case kUIT_ConnectSocket:
  case kUIT_PressStartAndSelect:
  case kUIT_BeginLink:
  case kUIT_LinkFailed:
  case kUIT_LinkCompleteOrLinking:
  case kUIT_TurnOffGBA:
    if (input.PA()) {
      PlayAdvanceSfx();
      SetUIText(NextLinkUI[mUiType]);
    } else if (input.PB()) {
      if (PrevLinkUI[mUiType] == kUIT_Empty)
        break;
      CSfxManager::SfxStart(SFXfnt_back, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
      SetUIText(PrevLinkUI[mUiType]);
    }
    break;
  case kUIT_Linking: {
    if (mGbaSupport->GetPhase() == CGBASupport::kP_Complete) {
      if (mGbaSupport->IsFusionLinked())
        gpGameState->SystemState().SetFusionLinked(true);
      if (mGbaSupport->IsFusionBeat())
        gpGameState->SystemState().SetFusionBeat(true);
      if (mGbaSupport->IsFusionLinked()) {
        PlayAdvanceSfx();
        SetUIText(kUIT_LinkCompleteOrLinking);
      } else {
        CSfxManager::SfxStart(SFXfnt_back, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
        SetUIText(kUIT_LinkFailed);
      }
    } else if (mGbaSupport->GetPhase() == CGBASupport::kP_Failed) {
      CSfxManager::SfxStart(SFXfnt_back, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
      SetUIText(kUIT_LinkFailed);
    }
    break;
  }
  case kUIT_Complete:
    return kGA_Complete;
  case kUIT_Cancelled:
    return kGA_Cancelled;
  default:
    break;
  }

  return kGA_None;
}

void CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::SetUIText(EUIType tp) {
  enum EStringIndex {
#if VERSION >= VERSION_GM8P_00
    kSI_ConnectSocket = 0x3e,
#else
    kSI_ConnectSocket = 0x44,
#endif
    kSI_LinkFailed,
    kSI_LinkComplete,
    kSI_LinkCompleteUnsaved,
    kSI_Linking,
    kSI_InsertPak,
    kSI_PressStartAndSelect,
    kSI_BeginLink,
    kSI_TurnOffGBA,
#if VERSION >= VERSION_GM8P_00
    kSI_Cancel = 0x4c,
#else
    kSI_Cancel = 0x52,
#endif
    kSI_Continue,
    kSI_Retry
  };

  int instructions = -1;
  int yes = -1;
  int no = -1;
  bool cableVisible = false;
  bool circleGcVisible = false;
  bool circleGbaVisible = false;
  bool circleStartVisible = false;
  bool pakoutVisible = false;
  bool gbaScreenVisible = false;
  bool connectVisible = false;

  switch (tp) {
  case kUIT_InsertPak:
    instructions = kSI_InsertPak;
    no = kSI_Cancel;
    yes = kSI_Continue;
    pakoutVisible = true;
    circleGbaVisible = true;
    break;
  case kUIT_ConnectSocket:
    instructions = kSI_ConnectSocket;
    no = kSI_Cancel;
    yes = kSI_Continue;
    cableVisible = true;
    circleGcVisible = true;
    circleGbaVisible = true;
    break;
  case kUIT_PressStartAndSelect:
    instructions = kSI_PressStartAndSelect;
    no = kSI_Cancel;
    yes = kSI_Continue;
    cableVisible = true;
    circleStartVisible = true;
    gbaScreenVisible = true;
    break;
  case kUIT_BeginLink:
    instructions = kSI_BeginLink;
    no = kSI_Cancel;
    yes = kSI_Continue;
    cableVisible = true;
    gbaScreenVisible = true;
    break;
  case kUIT_TurnOffGBA:
    instructions = kSI_TurnOffGBA;
    no = kSI_Cancel;
    yes = kSI_Continue;
    cableVisible = true;
    gbaScreenVisible = true;
    circleStartVisible = true;
    break;
  case kUIT_Linking:
    mGbaSupport->StartLink();
    instructions = kSI_Linking;
    cableVisible = true;
    gbaScreenVisible = true;
    connectVisible = true;
    break;
  case kUIT_LinkFailed:
    instructions = kSI_LinkFailed;
    no = kSI_Cancel;
    yes = kSI_Retry;
    cableVisible = true;
    circleGcVisible = true;
    circleGbaVisible = true;
    circleStartVisible = true;
    gbaScreenVisible = true;
    break;
  case kUIT_LinkCompleteOrLinking:
    yes = kSI_Continue;
    cableVisible = true;
    gbaScreenVisible = true;
    instructions = mLinkInProgress ? int(kSI_LinkComplete) : int(kSI_LinkCompleteUnsaved);
    break;
  case kUIT_Complete:
  case kUIT_Cancelled:
  default:
    break;
  }

  rstl::wstring emptyStr = rstl::wstring_l(L"");

#if VERSION >= VERSION_GM8P_00
  mTextpane_instructions.SetPairText(
      instructions == -1 ? emptyStr : rstl::wstring(gpStringTable->GetString(instructions)));

  mTextpane_no->TextSupport().SetText(
      yes == -1 ? emptyStr : rstl::wstring(gpStringTable->GetString(yes)), false);

  mTextpane_yes->TextSupport().SetText(
      no == -1 ? emptyStr : rstl::wstring(gpStringTable->GetString(no)), false);

#else
  mTextpane_instructions.SetPairText(
      instructions == -1 ? emptyStr : rstl::wstring_l(gpStringTable->GetString(instructions)));

  mTextpane_yes->TextSupport().SetText(
      yes == -1 ? emptyStr : rstl::wstring_l(gpStringTable->GetString(yes)), false);

  mTextpane_no->TextSupport().SetText(
      no == -1 ? emptyStr : rstl::wstring_l(gpStringTable->GetString(no)), false);

#endif

  mModel_gc->SetVisibility(true, kTM_Children);
  mModel_gba->SetVisibility(true, kTM_Children);
  mModel_cable->SetVisibility(cableVisible, kTM_Children);
  mModel_circlegcport->SetVisibility(circleGcVisible, kTM_Children);
  mModel_circlegbaport->SetVisibility(circleGbaVisible, kTM_Children);
  mModel_circlestartselect->SetVisibility(circleStartVisible, kTM_Children);
  mModel_pakout->SetVisibility(pakoutVisible, kTM_Children);
  mModel_gbascreen->SetVisibility(gbaScreenVisible, kTM_Children);
  mModel_connect->SetVisibility(connectVisible, kTM_Children);

  mUiType = tp;
}

CStateSetterFlow::CStateSetterFlow() : CIOWin(rstl::string_l("")) {}

CStateSetterFlow::~CStateSetterFlow() {}

CIOWin::EMessageReturn CStateSetterFlow::OnMessage(const CArchitectureMessage& message,
                                                   CArchitectureQueue& queue) {
  switch (message.GetType()) {
  case kAM_TimerTick:
    gpMain->RefreshGameState();
#if VERSION >= VERSION_GM8E_02
    gpGameState->HintOptions().EnsureHintNextTime();
#endif
    return kMR_RemoveIOWinAndExit;
  default:
    return kMR_Exit;
  }
}

CFrontEndUI::SFrontEndFrame::SFrontEndFrame(uint rnd)
: mRnd(rnd)
, mAction(kEA_None)
, mFrme(gpSimplePool->GetObj("FRME_FrontEndPL"))
, mLoadedFrme(nullptr)
, mTablegroup_mainmenu(nullptr)
#if VERSION >= VERSION_GM8P_00
, mLanguagePair()
#endif
, mGbaPair()
, mCheatPair() {
  mFrme.Lock();
}

CFrontEndUI::SFrontEndFrame::~SFrontEndFrame() {}

bool CFrontEndUI::SFrontEndFrame::PumpLoad() {
  if (mLoadedFrme) {
    return true;
  }
  if (mFrme.TryCache()) {
    CGuiFrame* frme = mFrme.GetObject();
    if (frme->GetIsFinishedLoading()) {
      mLoadedFrme = frme;
      FinishedLoading();
      return true;
    }
  }
  return false;
}

void CFrontEndUI::SFrontEndFrame::FinishedLoading() {
  mTablegroup_mainmenu =
      static_cast< CGuiTableGroup* >(mLoadedFrme->FindWidget("tablegroup_mainmenu"));

#if VERSION >= VERSION_GM8P_00
  mLanguagePair = SGuiTextPair(mLoadedFrme, "textpane_lang");
  mGbaPair = SGuiTextPair(mLoadedFrme, "textpane_gba");
  mCheatPair = SGuiTextPair(mLoadedFrme, "textpane_cheats");
#else
  mGbaPair = SGuiTextPair(mLoadedFrme, "textpane_gba");
  mGbaPair.SetPairText(gpStringTable->GetString(0x25));

  mCheatPair = SGuiTextPair(mLoadedFrme, "textpane_cheats");
  mCheatPair.SetPairText(gpStringTable->GetString(0x60));

  FindAndSetPairText(*mLoadedFrme, "textpane_start", gpStringTable->GetString(0x43));
  FindAndSetPairText(*mLoadedFrme, "textpane_options", gpStringTable->GetString(0x5e));
  FindAndSetPairText(*mLoadedFrme, "textpane_title", gpStringTable->GetString(0x62));

  CGuiTextPane* proceed =
      static_cast< CGuiTextPane* >(mLoadedFrme->FindWidget("textpane_proceed"));
  if (proceed) {
    proceed->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(0x55)));
  }

#endif

  mTablegroup_mainmenu->SetMenuAdvanceCallback(
      TFunctor1FromMethod< SFrontEndFrame, CGuiTableGroup* const >::Make(
          *this, &SFrontEndFrame::DoAdvance));
  mTablegroup_mainmenu->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< SFrontEndFrame, CGuiTableGroup* const, const int >::Make(
          *this, &SFrontEndFrame::DoSelectionChange));
  mTablegroup_mainmenu->SetMenuCancelCallback(
      TFunctor1FromMethod< SFrontEndFrame, CGuiTableGroup* const >::Make(
          *this, &SFrontEndFrame::DoCancel));

  HandleActiveChange(mTablegroup_mainmenu);
#if VERSION >= VERSION_GM8P_00
  SetStrings();
#endif
}

#if VERSION >= VERSION_GM8P_00
void CFrontEndUI::SFrontEndFrame::ReapplyStrings() { SetStrings(); }

void CFrontEndUI::SFrontEndFrame::SetStrings() {
  mLanguagePair.SetPairText(gpStringTable->GetString(0x27));
  mGbaPair.SetPairText(gpStringTable->GetString(0x25));
  mCheatPair.SetPairText(gpStringTable->GetString(0x5a));
  FindAndSetPairText(*mLoadedFrme, "textpane_start", gpStringTable->GetString(0x3d));
  FindAndSetPairText(*mLoadedFrme, "textpane_options", gpStringTable->GetString(0x58));
  FindAndSetPairText(*mLoadedFrme, "textpane_title", gpStringTable->GetString(0x5c));
  SetTitlePosition(*mLoadedFrme);

  CGuiTextPane* proceed =
      static_cast< CGuiTextPane* >(mLoadedFrme->FindWidget("textpane_proceed"));
  if (proceed != nullptr) {
    proceed->TextSupport().SetText(rstl::wstring(gpStringTable->GetString(0x4c)));
  }
  CGuiTextPane* cancel =
      static_cast< CGuiTextPane* >(mLoadedFrme->FindWidget("textpane_cancel"));
  if (cancel != nullptr) {
    cancel->TextSupport().SetText(rstl::wstring(gpStringTable->GetString(0x4f)));
  }
}

#endif

void CFrontEndUI::SFrontEndFrame::Update(float dt) {
  CGuiWidget* imageGallery = mTablegroup_mainmenu->GetWorkerWidget(
#if VERSION >= VERSION_GM8P_00
      4
#else
      3
#endif
  );

  if (CSlideShow::SlideShowGalleryFlags()) {
    imageGallery->SetIsSelectable(true);
    mCheatPair.mTextPane->TextSupport().SetFontColor(CColor::White());
  } else {
    imageGallery->SetIsSelectable(false);
    CGuiTextSupport& textSupport = mCheatPair.mTextPane->TextSupport();
    CColor color = CColor::Grey().WithAlphaOf(0.5f);
    textSupport.SetFontColor(color);
  }

  mLoadedFrme->Update(dt);
}

CFrontEndUI::SFrontEndFrame::EAction
CFrontEndUI::SFrontEndFrame::ProcessUserInput(const CFinalInput& input) {
  mAction = kEA_None;
  mLoadedFrme->ProcessUserInput(input);
  return mAction;
}

void CFrontEndUI::SFrontEndFrame::Draw() {
  mLoadedFrme->Draw(CGuiWidgetDrawParms::sDefaultDrawParms);
}

void CFrontEndUI::SFrontEndFrame::DoAdvance(CGuiTableGroup* caller) {
  switch (mTablegroup_mainmenu->GetUserSelection()) {
  case 0:
    CSfxManager::SfxStart(FETransitionForwardSFX[mRnd][0], 0x7f, 0x40, false,
                          CSfxManager::kMedPriority, false, CSfxManager::kAllAreas);
    CSfxManager::SfxStart(FETransitionForwardSFX[mRnd][1], 0x7f, 0x40, false,
                          CSfxManager::kMedPriority, false, CSfxManager::kAllAreas);
    mAction = kEA_StartGame;
    break;
#if VERSION >= VERSION_GM8P_00
  case 1:
    PlayAdvanceSfx();
    mAction = kEA_Language;
    break;
  case 2:
#else
  case 1:
#endif
    mAction = kEA_FusionBonus;
    break;
#if VERSION >= VERSION_GM8P_00
  case 3:
#else
  case 2:
#endif
    PlayAdvanceSfx();
    mAction = kEA_GameOptions;
    break;
#if VERSION >= VERSION_GM8P_00
  case 4:
#else
  case 3:
#endif
    PlayAdvanceSfx();
    mAction = kEA_SlideShow;
    break;
  default:
    break;
  }
}

void CFrontEndUI::SFrontEndFrame::DoSelectionChange(CGuiTableGroup* caller, int oldSelection) {
  CSfxManager::SfxStart(SFXfnt_selection_change, 0x7f, 0x40, false, CSfxManager::kMedPriority,
                        false, CSfxManager::kAllAreas);
  HandleActiveChange(caller);
}

void CFrontEndUI::SFrontEndFrame::HandleActiveChange(CGuiTableGroup* caller) {
  CColor selected((uchar)0xFF, (uchar)0xFF, (uchar)0xFF, (uchar)0xFF);
  CColor unselected((uchar)0xA0, (uchar)0xA0, (uchar)0xA0, (uchar)0xC8);
  caller->SetColors(selected, unselected);
}

void CFrontEndUI::SFrontEndFrame::DoCancel(CGuiTableGroup* caller) {
#if VERSION >= VERSION_GM8P_00
  mAction = kEA_ReturnToTitle;
#endif
}

float CFrontEndUI::SFileSelectOption::ComputeRandom() {
  return rand() / static_cast< float >(RAND_MAX) * 30.f + 30.f;
}

void CFrontEndUI::SNewFileSelectFrame::StartTextAnimating(CGuiTextPane* text,
                                                          const rstl::wstring& str, float chRate) {
  text->TextSupport().SetText(rstl::wstring_l(L""));
  text->TextSupport().SetText(str);
  text->TextSupport().SetTypeWriteEffectOptions(true, 0.1f, chRate);
}

CFrontEndUI::SFileSelectOption::SFileSelectOption(CGuiFrame* frame, int idx)
: mBase(frame->FindWidget(CBasics::Stringize("basewidget_file%d", idx)))
, mCurField(0)
, mChRate(ComputeRandom()) {
  char buf[32];
#if NONMATCHING
  snprintf(buf, sizeof(buf), "textpane_filename%d", idx);
#else
  sprintf(buf, "textpane_filename%d", idx);
#endif
  mTextpanes.push_back(SGuiTextPair(frame, buf));

#if NONMATCHING
  snprintf(buf, sizeof(buf), "textpane_world%d", idx);
#else
  sprintf(buf, "textpane_world%d", idx);
#endif
  mTextpanes.push_back(SGuiTextPair(frame, buf));

#if NONMATCHING
  snprintf(buf, sizeof(buf), "textpane_playtime%d", idx);
#else
  sprintf(buf, "textpane_playtime%d", idx);
#endif
  mTextpanes.push_back(SGuiTextPair(frame, buf));

#if NONMATCHING
  snprintf(buf, sizeof(buf), "textpane_date%d", idx);
#else
  sprintf(buf, "textpane_date%d", idx);
#endif
  mTextpanes.push_back(SGuiTextPair(frame, buf));
}

CFrontEndUI::SNewFileSelectFrame::SNewFileSelectFrame(CSaveGameScreen* saveUI, uint rnd)
: mRnd(rnd)
, mSaveUI(saveUI)
, mSubMenu(kSM_Root)
, mAction(kA_None)
, mFrme(gpSimplePool->GetObj("FRME_NewFileSelect"))
, mLoadedFrame(nullptr)
, mTablegroup_fileselect(nullptr)
, mModel_erase(nullptr)
, mTextpane_erase()
#if VERSION >= VERSION_GM8P_00
, mTextpane_language()
#endif
, mTextpane_cheats()
, mTextpane_gba()
, mTablegroup_popup(nullptr)
, mModel_dash7(nullptr)
, mTextpane_popupadvance()
, mTextpane_popupcancel()
, mTextpane_popupextra()
#if VERSION < VERSION_GM8P_00
, mTextpane_cancel(nullptr)
#endif
, mFileSelections()
, mModel_erase_position(CVector3f::Zero())
, mRowPitch(0.f)
, mCurTime(0.f)
, mSaveReady(false)
, mNeedsEraseToggle(false)
, mNeedsNewToggle(false) {
  mFrme.Lock();
}

CFrontEndUI::SNewFileSelectFrame::~SNewFileSelectFrame() {}

bool CFrontEndUI::SNewFileSelectFrame::PumpLoad() {
  if (mLoadedFrame) {
    return true;
  }
  if (mFrme.TryCache()) {
    CGuiFrame* frme = mFrme.GetObject();
    if (frme->GetIsFinishedLoading()) {
      mLoadedFrame = frme;
      FinishedLoading();
      return true;
    }
  }
  return false;
}

void CFrontEndUI::SNewFileSelectFrame::FinishedLoading() {
  mTablegroup_fileselect =
      static_cast< CGuiTableGroup* >(mLoadedFrame->FindWidget("tablegroup_fileselect"));
  mModel_erase = static_cast< CGuiModel* >(mLoadedFrame->FindWidget("model_erase"));
  mModel_erase_position = mModel_erase->GetLocalPosition();

  mTextpane_erase = SGuiTextPair(mLoadedFrame, "textpane_erase");
  mTextpane_gba = SGuiTextPair(mLoadedFrame, "textpane_gba");
#if VERSION >= VERSION_GM8P_00
  mTextpane_language = SGuiTextPair(mLoadedFrame, "textpane_lang");
#endif
  mTextpane_cheats = SGuiTextPair(mLoadedFrame, "textpane_cheats");
  mTextpane_popupadvance = SGuiTextPair(mLoadedFrame, "textpane_popupadvance");
  mTextpane_popupcancel = SGuiTextPair(mLoadedFrame, "textpane_popupcancel");
  mTextpane_popupextra = SGuiTextPair(mLoadedFrame, "textpane_popupextra");

  mTablegroup_popup =
      static_cast< CGuiTableGroup* >(mLoadedFrame->FindWidget("tablegroup_popup"));
  mModel_dash7 = static_cast< CGuiModel* >(mLoadedFrame->FindWidget("model_dash7"));
#if VERSION < VERSION_GM8P_00
  mTextpane_cancel =
      static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_cancel"));
#endif

#if VERSION < VERSION_GM8P_00
  SFrontEndFrame::FindAndSetPairText(*mLoadedFrame, "textpane_title",
                                     gpStringTable->GetString(0x61));

  CGuiTextPane* proceed =
      static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_proceed"));
  if (proceed) {
    proceed->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(0x55)));
  }

#endif

  mTablegroup_popup->SetIsVisible(false);
  mTablegroup_popup->SetIsActive(false);
  mTablegroup_popup->SetVertical(false);
  mTablegroup_popup->GetWorkerWidget(2)->SetIsSelectable(false);
  mTablegroup_popup->GetWorkerWidget(2)->SetVisibility(false, kTM_Children);

  mTablegroup_fileselect->SetMenuAdvanceCallback(
      TFunctor1FromMethod< SNewFileSelectFrame, CGuiTableGroup* const >::Make(
          *this, &SNewFileSelectFrame::DoFileselectAdvance));
  mTablegroup_fileselect->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< SNewFileSelectFrame, CGuiTableGroup* const, const int >::Make(
          *this, &SNewFileSelectFrame::DoSelectionChange));
  mTablegroup_fileselect->SetMenuCancelCallback(
      TFunctor1FromMethod< SNewFileSelectFrame, CGuiTableGroup* const >::Make(
          *this, &SNewFileSelectFrame::DoFileselectCancel));

  mTablegroup_popup->SetMenuAdvanceCallback(
      TFunctor1FromMethod< SNewFileSelectFrame, CGuiTableGroup* const >::Make(
          *this, &SNewFileSelectFrame::DoPopupAdvance));
  mTablegroup_popup->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< SNewFileSelectFrame, CGuiTableGroup* const, const int >::Make(
          *this, &SNewFileSelectFrame::DoSelectionChange));
  mTablegroup_popup->SetMenuCancelCallback(
      TFunctor1FromMethod< SNewFileSelectFrame, CGuiTableGroup* const >::Make(
          *this, &SNewFileSelectFrame::DoPopupCancel));

  for (int i = 0; i < 3; ++i) {
    mFileSelections.push_back(SFileSelectOption(mLoadedFrame, i));
  }

  mRowPitch = (mFileSelections[1].mBase->GetLocalPosition() -
                   mFileSelections[0].mBase->GetLocalPosition())
                      .GetZ();
#if VERSION >= VERSION_GM8P_00
  SetStrings();
#endif
}

#if VERSION >= VERSION_GM8P_00
void CFrontEndUI::SNewFileSelectFrame::SetStrings() {
  SFrontEndFrame::FindAndSetPairText(*mLoadedFrame, "textpane_title",
                                     gpStringTable->GetString(0x5b));
  SetTitlePosition(*mLoadedFrame);

  CGuiTextPane* proceed =
      static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_proceed"));
  if (proceed != nullptr) {
    proceed->TextSupport().SetText(rstl::wstring(gpStringTable->GetString(0x4c)));
  }
  CGuiTextPane* cancel =
      static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_cancel"));
  if (cancel != nullptr) {
    cancel->TextSupport().SetText(rstl::wstring(gpStringTable->GetString(0x4f)));
  }
}

void CFrontEndUI::SNewFileSelectFrame::ReapplyStrings() {
  SetStrings();
  ClearFrameContents();
}

#endif

uint CFrontEndUI::SNewFileSelectFrame::GetUserFileSelection() const {
  int sel = mTablegroup_fileselect->GetUserSelection();
  return sel < 3 ? sel : 0;
}

void CFrontEndUI::SNewFileSelectFrame::Update(float dt) {
  bool saveReady = mSaveUI->GetUIType() == CSaveGameScreen::kUIT_SaveReady;
  if (saveReady != mSaveReady) {
    if (saveReady) {
      ClearFrameContents();
    } else {
      if (mSubMenu != kSM_Root) {
        ResetFrame();
        DeactivateExistingGamePopup();
        DeactivateNewGamePopup();
        mSubMenu = kSM_Root;
      }
    }
    mSaveReady = saveReady;
  }
  if (mSaveReady) {
    SetupFrameContents();
  }
  mLoadedFrame->Update(dt);
}

bool CGuiWidget::GetIsActive() const { return mIsActive; }

CFrontEndUI::SNewFileSelectFrame::EAction
CFrontEndUI::SNewFileSelectFrame::ProcessUserInput(const CFinalInput& input) {
  mAction = kA_None;

  if (mSubMenu != kSM_EraseGamePopup) {
    mSaveUI->ProcessUserInput(input);
  }

  if (IsTextDoneAnimating()) {
    const float maxTime = 0.5f;
    mCurTime = rstl::min_val(maxTime, mCurTime + input.Time());
  }

  if (mCurTime < 0.5f) {
    return mAction;
  }

  if (mSaveReady) {
    mLoadedFrame->ProcessUserInput(input);
  }

  if (mNeedsEraseToggle) {
    if (mTablegroup_popup->GetIsActive()) {
      DeactivateExistingGamePopup();
    } else {
      ActivateExistingGamePopup();
    }
    mNeedsEraseToggle = false;
  }

  if (mNeedsNewToggle) {
    if (mTablegroup_popup->GetIsActive()) {
      DeactivateNewGamePopup();
    } else {
      ActivateNewGamePopup();
    }
    mNeedsNewToggle = false;
  }

  return mAction;
}

void CFrontEndUI::SNewFileSelectFrame::Draw() const {
  if (mLoadedFrame != nullptr && mSaveReady) {
    mLoadedFrame->Draw(CGuiWidgetDrawParms::sDefaultDrawParms);
  }
}

void CFrontEndUI::SNewFileSelectFrame::HandleActiveChange(CGuiWidget* active) {
  if (active != nullptr) {
    int sel = static_cast< CGuiTableGroup* >(active)->GetUserSelection();
    CColor selected((uchar)0xFF, (uchar)0xFF, (uchar)0xFF, (uchar)0xFF);
    CColor unselected((uchar)0xA0, (uchar)0xA0, (uchar)0xA0, (uchar)0xC8);
    static_cast< CGuiTableGroup* >(active)->SetColors(selected, unselected);
    if (active == mTablegroup_fileselect) {
      CVector3f pos = mModel_erase_position + CVector3f(0.f, 0.f, sel * mRowPitch);
      const CTransform4f& xf = CTransform4f::Translate(pos);
      CGuiModel* erase = mModel_erase;
      erase->LocalTransform() = xf;
      erase->RecalculateTransforms();
    }
    bool shouldHide = (mSubMenu == kSM_Root || mSubMenu == kSM_NewGamePopup);
    if (shouldHide) {
      mModel_erase->SetIsVisible(false);
    } else {
      mModel_erase->SetIsVisible(true);
    }
  }
}

void CFrontEndUI::SNewFileSelectFrame::DoFileselectAdvance(CGuiTableGroup* caller) {
  int userSel = mTablegroup_fileselect->GetUserSelection();
  if (userSel < 3) {
    if (mSubMenu == kSM_EraseGame) {
      if (mSaveUI->GetGameData(userSel) != nullptr) {
        PlayAdvanceSfx();
        mNeedsEraseToggle = true;
      }
    } else {
      if (mSaveUI->GetGameData(userSel) != nullptr) {
        mSaveUI->StartGame(userSel);
      } else {
        mNeedsNewToggle = true;
      }
    }
  } else if (userSel == 3) {
    PlayAdvanceSfx();
    EnterErase();
#if VERSION >= VERSION_GM8P_00
  } else if (userSel == 4) {
    PlayAdvanceSfx();
    mAction = kA_Language;
  } else if (userSel == 5) {
    mAction = kA_FusionBonus;
  } else if (userSel == 6) {
#else
  } else if (userSel == 4) {
    mAction = kA_FusionBonus;
  } else if (userSel == 5) {
#endif
    mAction = kA_SlideShow;
  }
}

void CFrontEndUI::SNewFileSelectFrame::SetupFrameContents() {
  for (int i = 0; i < 3; ++i) {
    SFileSelectOption& option = mFileSelections[i];
    if (option.mCurField == 4)
      continue;

    CGuiTextPane* curPane;
    if (option.mCurField == -1) {
      curPane = nullptr;
    } else {
      curPane = option.mTextpanes[option.mCurField].mTextPane;
    }

    if (curPane != nullptr) {
      float total = curPane->TextSupport().GetNumCharsTotal();
      float printed = curPane->TextSupport().GetNumCharactersPrinted();
      if (!(printed >= total))
        continue;
    }

    ++option.mCurField;
    if (option.mCurField >= 4)
      continue;

    const CGameState::GameFileStateInfo* data =
        static_cast< const CGameState::GameFileStateInfo* >(mSaveUI->GetGameData(i));
    SGuiTextPair& populatePair = option.mTextpanes[option.mCurField];
    CGuiTextPane* pane0 = populatePair.mTextPane;
    CGuiTextPane* pane1 = populatePair.mTextPaneB;
    rstl::wstring str;

    char timeBuf[32];
    switch (option.mCurField) {
    case 0:
      if (data != nullptr) {
        char buf[32];
#if NONMATCHING
        snprintf(buf, sizeof(buf), "  %02d%%", data->mItemPercent);
#else
        sprintf(buf, "  %02d%%", data->mItemPercent);
#endif
#if VERSION >= VERSION_GM8P_00
        int strIdx = data->mHardMode ? 0x64 : 0x28;
#else
        int strIdx = data->mHardMode ? 0x6a : 0x27;
#endif
        str = rstl::wstring_l(gpStringTable->GetString(strIdx + i)) +
              CStringExtras::ConvertToUNICODE(rstl::string_l(buf));
      } else {
        str = rstl::wstring(gpStringTable->GetString(0x24));
      }
      break;
    case 1:
      if (data != nullptr) {
        const wchar_t* worldName = nullptr;
        if (gpMemoryCard->HasSaveWorldMemory(data->mMlvlId)) {
          worldName = gpMemoryCard->GetSaveWorldMemory(data->mMlvlId).GetFrontEndName();
        }
        str = rstl::wstring_l(worldName != nullptr ? worldName : L"??????");
      } else {
#if VERSION >= VERSION_GM8P_00
        str = rstl::wstring(gpStringTable->GetString(0x34));
#else
        str = rstl::wstring_l(gpStringTable->GetString(0x33));
#endif
      }
      break;
#if VERSION >= VERSION_GM8P_00
    case 2:
      if (data != nullptr) {
        str = rstl::wstring(gpStringTable->GetString(0x37));
      } else {
        str = rstl::wstring(gpStringTable->GetString(0x36));
      }
      break;
    case 3:
      if (data != nullptr) {
#if NONMATCHING
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d",
                 static_cast< int >(data->mPlayTime) / 3600,
                 (static_cast< int >(data->mPlayTime) % 3600) / 60);
#else
        sprintf(timeBuf, "%02d:%02d", static_cast< int >(data->mPlayTime) / 3600,
                (static_cast< int >(data->mPlayTime) % 3600) / 60);
#endif
        str = CStringExtras::ConvertToUNICODE(rstl::string_l(timeBuf));
      } else {
        str = rstl::wstring(gpStringTable->GetString(0x35));
      }
      break;
#else
    case 2:
      if (data != nullptr) {
#if NONMATCHING
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d",
                 static_cast< int >(data->mPlayTime) / 3600,
                 (static_cast< int >(data->mPlayTime) % 3600) / 60);
#else
        sprintf(timeBuf, "%02d:%02d", static_cast< int >(data->mPlayTime) / 3600,
                (static_cast< int >(data->mPlayTime) % 3600) / 60);
#endif
        str = CStringExtras::ConvertToUNICODE(rstl::string_l(timeBuf));
      } else {
        str = rstl::wstring_l(gpStringTable->GetString(0x34));
      }
      break;
    case 3:
      if (data != nullptr) {
        str = rstl::wstring_l(gpStringTable->GetString(0x36));
      } else {
        str = rstl::wstring_l(gpStringTable->GetString(0x35));
      }
      break;
#endif
    }

    StartTextAnimating(pane0, str, option.mChRate);
    StartTextAnimating(pane1, str, option.mChRate);
  }
}

void CFrontEndUI::SNewFileSelectFrame::ClearFrameContents() {
  mCurTime = 0.f;
  bool hasSave = false;
  for (int i = 0; i < 3; ++i) {
    if (mSaveUI->GetGameData(i) != nullptr) {
      hasSave = true;
    }
    mFileSelections[i].mChRate = SFileSelectOption::ComputeRandom();
    mFileSelections[i].mCurField = -1;
    for (int j = 0; j < 4; ++j) {
      mFileSelections[i].mTextpanes[j].mTextPane->TextSupport().SetText(rstl::wstring_l(L""),
                                                                               false);
      mFileSelections[i].mTextpanes[j].mTextPaneB->TextSupport().SetText(
          rstl::wstring_l(L""), false);
    }
  }

#if VERSION >= VERSION_GM8P_00
  StartTextAnimating(mTextpane_erase.mTextPane, rstl::wstring(gpStringTable->GetString(0x26)),
                     60.f);
  StartTextAnimating(mTextpane_language.mTextPane,
                     rstl::wstring(gpStringTable->GetString(0x27)), 60.f);
  StartTextAnimating(mTextpane_gba.mTextPane, rstl::wstring(gpStringTable->GetString(0x25)),
                     60.f);
  StartTextAnimating(mTextpane_cheats.mTextPane, rstl::wstring(gpStringTable->GetString(0x5a)),
                     60.f);

  StartTextAnimating(mTextpane_erase.mTextPaneB, rstl::wstring(gpStringTable->GetString(0x26)),
                     60.f);
  StartTextAnimating(mTextpane_language.mTextPaneB,
                     rstl::wstring(gpStringTable->GetString(0x27)), 60.f);
  StartTextAnimating(mTextpane_gba.mTextPaneB, rstl::wstring(gpStringTable->GetString(0x25)),
                     60.f);
  StartTextAnimating(mTextpane_cheats.mTextPaneB,
                     rstl::wstring(gpStringTable->GetString(0x5a)), 60.f);

#else
  StartTextAnimating(mTextpane_erase.mTextPane,
                     rstl::wstring_l(gpStringTable->GetString(0x26)), 60.f);
  StartTextAnimating(mTextpane_gba.mTextPane, rstl::wstring_l(gpStringTable->GetString(0x25)),
                     60.f);
  StartTextAnimating(mTextpane_cheats.mTextPane,
                     rstl::wstring_l(gpStringTable->GetString(0x60)), 60.f);

  StartTextAnimating(mTextpane_erase.mTextPaneB,
                     rstl::wstring_l(gpStringTable->GetString(0x26)), 60.f);
  StartTextAnimating(mTextpane_gba.mTextPaneB, rstl::wstring_l(gpStringTable->GetString(0x25)),
                     60.f);
  StartTextAnimating(mTextpane_cheats.mTextPaneB,
                     rstl::wstring_l(gpStringTable->GetString(0x60)), 60.f);

#endif

  CGuiTextPane* erasePane = mTextpane_erase.mTextPane;
  if (hasSave) {
    erasePane->SetIsSelectable(true);
    erasePane->TextSupport().SetFontColor(CColor::White());
  } else {
    erasePane->SetIsSelectable(false);
    erasePane->TextSupport().SetFontColor(CColor::Grey().WithAlphaOf(0.5f));
  }

  mTablegroup_fileselect->SetUserSelection(0);
  CGuiTextPane* cheats = static_cast< CGuiTextPane* >(mTablegroup_fileselect->GetWorkerWidget(
#if VERSION >= VERSION_GM8P_00
      6
#else
      5
#endif
      ));
  if (CSlideShow::SlideShowGalleryFlags()) {
    cheats->SetIsSelectable(true);
    mTextpane_cheats.mTextPane->TextSupport().SetFontColor(CColor::White());
  } else {
    cheats->SetIsSelectable(false);
    mTextpane_cheats.mTextPane->TextSupport().SetFontColor(CColor::Grey().WithAlphaOf(0.5f));
  }

  HandleActiveChange(mTablegroup_fileselect);
}

void CFrontEndUI::SNewFileSelectFrame::EnterErase() {
  mSubMenu = kSM_EraseGame;

  mTextpane_erase.mTextPane->SetIsSelectable(false);
  CGuiTextSupport& eraseTS = mTextpane_erase.mTextPane->TextSupport();
  eraseTS.SetFontColor(CColor::Grey().WithAlphaOf(0.5f));

  mTextpane_gba.mTextPane->SetIsSelectable(false);
  CGuiTextSupport& gbaTS = mTextpane_gba.mTextPane->TextSupport();
  gbaTS.SetFontColor(CColor::Grey().WithAlphaOf(0.5f));

  mTextpane_cheats.mTextPane->SetIsSelectable(false);
  CGuiTextSupport& cheatsTS = mTextpane_cheats.mTextPane->TextSupport();
  cheatsTS.SetFontColor(CColor::Grey().WithAlphaOf(0.5f));

#if VERSION >= VERSION_GM8P_00
  mTextpane_language.mTextPane->SetIsSelectable(false);
  mTextpane_language.mTextPane->TextSupport().SetFontColor(CColor::Grey().WithAlphaOf(0.5f));
#endif

  for (int i = 2; i >= 0; --i) {
    if (mSaveUI->GetGameData(i)) {
      mFileSelections[i].mBase->SetIsSelectable(true);
      mTablegroup_fileselect->SetUserSelection(i);
    } else {
      mFileSelections[i].mBase->SetIsSelectable(false);
    }
  }

#if VERSION < VERSION_GM8P_00
  mTextpane_cancel->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(0x52)));
#endif
  HandleActiveChange(mTablegroup_fileselect);
}

void CFrontEndUI::SNewFileSelectFrame::ResetFrame() {
  mSubMenu = kSM_Root;

  mTextpane_gba.mTextPane->SetIsSelectable(true);
  mTextpane_gba.mTextPane->TextSupport().SetFontColor(CColor::White());

  mTextpane_cheats.mTextPane->SetIsSelectable(true);
  mTextpane_cheats.mTextPane->TextSupport().SetFontColor(CColor::White());

#if VERSION >= VERSION_GM8P_00
  mTextpane_language.mTextPane->SetIsSelectable(true);
  mTextpane_language.mTextPane->TextSupport().SetFontColor(CColor::White());
#endif

  ClearFrameContents();

  for (int i = 2; i >= 0; --i) {
    mTablegroup_fileselect->GetWorkerWidget(i)->SetIsSelectable(true);
  }

#if VERSION < VERSION_GM8P_00
  mTextpane_cancel->TextSupport().SetText(rstl::wstring_l(L""));
#endif
}

void CFrontEndUI::SNewFileSelectFrame::ActivateNewGamePopup() {
  mTablegroup_popup->SetIsActive(true);
  mTablegroup_popup->SetIsVisible(true);
  mTablegroup_popup->SetUserSelection(0);
  const CTransform4f& xf =
      CTransform4f::Translate(0.f, 0.f,
                              mRowPitch * mTablegroup_fileselect->GetUserSelection()) *
      mTablegroup_popup->GetTransform();
  CGuiTableGroup* popup = mTablegroup_popup;
  popup->LocalTransform() = xf;
  popup->RecalculateTransforms();
  mTablegroup_fileselect->SetIsActive(false);
  mSubMenu = kSM_NewGamePopup;
  HandleActiveChange(mTablegroup_popup);
  CColor col(static_cast< uchar >(0xff), static_cast< uchar >(0xff), static_cast< uchar >(0xff),
             static_cast< uchar >(0x00));
  mFileSelections[mTablegroup_fileselect->GetUserSelection()].mBase->SetColor(col);

  PlayAdvanceSfx();

  if (gpGameState->SystemState().GetNormalModeBeat()) {
#if VERSION >= VERSION_GM8P_00
    mTextpane_popupadvance.SetPairText(rstl::wstring(gpStringTable->GetString(0x60)));
    mTextpane_popupcancel.SetPairText(rstl::wstring(gpStringTable->GetString(0x58)));
    mTextpane_popupextra.SetPairText(rstl::wstring(gpStringTable->GetString(0x5f)));
#else
    mTextpane_popupadvance.SetPairText(rstl::wstring_l(gpStringTable->GetString(0x66)));
    mTextpane_popupcancel.SetPairText(rstl::wstring_l(gpStringTable->GetString(0x5e)));
    mTextpane_popupextra.SetPairText(rstl::wstring_l(gpStringTable->GetString(0x65)));
#endif

    mTablegroup_popup->GetWorkerWidget(2)->SetIsSelectable(true);
    mTablegroup_popup->GetWorkerWidget(2)->SetVisibility(true, kTM_Children);
    mModel_dash7->SetVisibility(true, kTM_Children);
  } else {
#if VERSION >= VERSION_GM8P_00
    mTextpane_popupadvance.SetPairText(rstl::wstring(gpStringTable->GetString(0x3d)));
    mTextpane_popupcancel.SetPairText(rstl::wstring(gpStringTable->GetString(0x58)));
#else
    mTextpane_popupadvance.SetPairText(rstl::wstring_l(gpStringTable->GetString(0x43)));
    mTextpane_popupcancel.SetPairText(rstl::wstring_l(gpStringTable->GetString(0x5e)));
#endif
    mModel_dash7->SetVisibility(false, kTM_Children);
  }
#if VERSION < VERSION_GM8P_00
  mTextpane_cancel->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(0x52)));
#endif
}

void CFrontEndUI::SNewFileSelectFrame::DeactivateNewGamePopup() {
  mTablegroup_popup->SetIsActive(false);
  mTablegroup_popup->SetIsVisible(false);
  mTablegroup_fileselect->SetIsActive(true);

  mTablegroup_popup->GetWorkerWidget(2)->SetIsSelectable(false);
  mTablegroup_popup->GetWorkerWidget(2)->SetVisibility(false, kTM_Children);

  mModel_dash7->SetVisibility(false, kTM_Children);

  mNeedsNewToggle = false;
  HandleActiveChange(mTablegroup_fileselect);
  CColor white(static_cast< uchar >(0xff), static_cast< uchar >(0xff), static_cast< uchar >(0xff),
               static_cast< uchar >(0xff));
  mFileSelections[mTablegroup_fileselect->GetUserSelection()].mBase->SetColor(white);
#if VERSION < VERSION_GM8P_00
  mTextpane_cancel->TextSupport().SetText(rstl::wstring_l(L""));
#endif
}

void CFrontEndUI::SNewFileSelectFrame::ActivateExistingGamePopup() {
  mTablegroup_popup->SetIsActive(true);
  mTablegroup_popup->SetIsVisible(true);
  mTablegroup_popup->SetUserSelection(0);
  const CTransform4f& xf =
      CTransform4f::Translate(0.f, 0.f,
                              mRowPitch * mTablegroup_fileselect->GetUserSelection()) *
      mTablegroup_popup->GetTransform();
  CGuiTableGroup* popup = mTablegroup_popup;
  popup->LocalTransform() = xf;
  popup->RecalculateTransforms();
  mTablegroup_fileselect->SetIsActive(false);
  mSubMenu = kSM_EraseGamePopup;
  HandleActiveChange(mTablegroup_popup);
#if VERSION >= VERSION_GM8P_00
  mTextpane_popupadvance.SetPairText(rstl::wstring(gpStringTable->GetString(0x59)));
  mTextpane_popupcancel.SetPairText(rstl::wstring(gpStringTable->GetString(0x26)));
#else
  mTextpane_popupadvance.SetPairText(rstl::wstring_l(gpStringTable->GetString(0x5f)));
  mTextpane_popupcancel.SetPairText(rstl::wstring_l(gpStringTable->GetString(0x26)));
#endif
  CColor col(static_cast< uchar >(0xff), static_cast< uchar >(0xff), static_cast< uchar >(0xff),
             static_cast< uchar >(0x00));
  mFileSelections[mTablegroup_fileselect->GetUserSelection()].mBase->SetColor(col);
  mModel_dash7->SetVisibility(false, kTM_Children);
}

void CFrontEndUI::SNewFileSelectFrame::DeactivateExistingGamePopup() {
  mTablegroup_popup->SetIsActive(false);
  mTablegroup_popup->SetIsVisible(false);
  mTablegroup_fileselect->SetIsActive(true);
  HandleActiveChange(mTablegroup_fileselect);
  CColor white(static_cast< uchar >(0xff), static_cast< uchar >(0xff), static_cast< uchar >(0xff),
               static_cast< uchar >(0xff));
  mFileSelections[mTablegroup_fileselect->GetUserSelection()].mBase->SetColor(white);
}

bool CFrontEndUI::SNewFileSelectFrame::IsTextDoneAnimating() {
  for (int i = 0; i < 3; ++i) {
    if (mFileSelections[i].mCurField != 4)
      return false;
  }

  const CGuiTextSupport& ts1 = mTextpane_erase.mTextPane->GetTextSupport();
  float curTime = ts1.GetCurTime();
  if (curTime < ts1.GetTotalAnimationTime())
    return false;

  const CGuiTextSupport& ts2 = mTextpane_cheats.mTextPane->GetTextSupport();
  curTime = ts2.GetCurTime();
  if (curTime < ts2.GetTotalAnimationTime())
    return false;

  const CGuiTextSupport& ts3 = mTextpane_gba.mTextPane->GetTextSupport();
  curTime = ts3.GetCurTime();
#if VERSION >= VERSION_GM8P_00
  if (curTime < ts3.GetTotalAnimationTime())
    return false;
  const CGuiTextSupport& language = mTextpane_language.mTextPane->GetTextSupport();
  curTime = language.GetCurTime();
  return !(curTime < language.GetTotalAnimationTime());
#else
  return !(curTime < ts3.GetTotalAnimationTime());
#endif
}

void CFrontEndUI::SNewFileSelectFrame::DoSelectionChange(CGuiTableGroup* caller, int oldSelection) {
  HandleActiveChange(caller);
  CSfxManager::SfxStart(SFXfnt_selection_change, 0x7f, 0x40, false, CSfxManager::kMedPriority,
                        false, CSfxManager::kAllAreas);
}

void CFrontEndUI::SNewFileSelectFrame::DoFileselectCancel(CGuiTableGroup* caller) {
  if (mSubMenu == kSM_EraseGame) {
    CSfxManager::SfxStart(SFXfnt_back, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    ResetFrame();
#if VERSION >= VERSION_GM8P_00
  } else {
    mAction = kA_ReturnToTitle;
#endif
  }
}

void CFrontEndUI::SNewFileSelectFrame::DoPopupAdvance(CGuiTableGroup* caller) {
  if (mSubMenu == kSM_EraseGamePopup) {
    if (mTablegroup_popup->GetUserSelection() == 1) {
      int fileSelection = mTablegroup_fileselect->GetUserSelection();
      PlayAdvanceSfx();
      mSaveUI->EraseGame(fileSelection);
      ResetFrame();
    } else {
      mSubMenu = kSM_EraseGame;
    }
    mNeedsEraseToggle = true;
  } else {
    if (gpGameState->SystemState().GetNormalModeBeat()) {
      if (mTablegroup_popup->GetUserSelection() == 1) {
        PlayAdvanceSfx();
        mAction = kA_GameOptions;
        return;
      }
      int fileSelection = GetUserFileSelection();
      gpGameState->SetHardMode(mTablegroup_popup->GetUserSelection() == 0);
      mSaveUI->StartGame(fileSelection);
    } else {
      if (mTablegroup_popup->GetUserSelection() == 1) {
        PlayAdvanceSfx();
        mAction = kA_GameOptions;
        return;
      }
      int fileSelection = GetUserFileSelection();
      mSaveUI->StartGame(fileSelection);
    }
  }
}

void CFrontEndUI::SNewFileSelectFrame::DoPopupCancel(CGuiTableGroup* caller) {
  if (mSubMenu == kSM_EraseGamePopup) {
    CSfxManager::SfxStart(SFXfnt_back, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    mSubMenu = kSM_EraseGame;
    mNeedsEraseToggle = true;
  } else {
    CSfxManager::SfxStart(SFXfnt_back, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    mSubMenu = kSM_Root;
    mNeedsNewToggle = true;
  }
}

CFrontEndUI::SFusionBonusFrame::SFusionBonusFrame()
: mGbaLinkFrame(nullptr)
, mGbaSupport(rs_new CGBASupport())
, mAction(kFA_None)
, mGbaScreen(gpSimplePool->GetObj("FRME_GBAScreen"))
, mGbaLink(gpSimplePool->GetObj("FRME_GBALink"))
, mLoadedFrame(nullptr)
, mTablegroup_options(nullptr)
, mTablegroup_fusionsuit(nullptr)
, mTextpane_instructions()
, mLastDoDraw(false)
, mFusionNotComplete(false)
, mMpNotComplete(false) {
  mGbaScreen.Lock();
  mGbaLink.Lock();
}

CFrontEndUI::SFusionBonusFrame::~SFusionBonusFrame() {}

void CFrontEndUI::SFusionBonusFrame::Draw() {
  if (!mLastDoDraw)
    return;
  if (mGbaLinkFrame.get() != nullptr) {
    mGbaLinkFrame->Draw();
  } else if (mLoadedFrame != nullptr) {
    mLoadedFrame->Draw(CGuiWidgetDrawParms::sDefaultDrawParms);
  }
}

void CFrontEndUI::SFusionBonusFrame::Update(float dt, CSaveGameScreen* saveUI) {
  const bool doDraw = saveUI == nullptr || saveUI->GetUIType() == CSaveGameScreen::kUIT_SaveReady;
  if (doDraw != mLastDoDraw) {
    mLastDoDraw = doDraw;
    if (mLastDoDraw) {
      ResetCompletionFlags();
    }
  }

  if (mGbaLinkFrame.get() != nullptr) {
    mGbaLinkFrame->Update(dt);
  } else if (mLoadedFrame != nullptr) {
    mLoadedFrame->Update(dt);
  }

  const int sel = mTablegroup_options->GetUserSelection();
  const bool showFusionSuit = gpGameState->SystemState().GetFusionLinked() &&
                              gpGameState->SystemState().GetNormalModeBeat();

  const bool fusionBeat = gpGameState->SystemState().GetFusionBeat();
  bool showProceed = sel == 1 && showFusionSuit;
  mTablegroup_fusionsuit->SetIsActive(showProceed);
  mTablegroup_fusionsuit->SetIsVisible(showProceed);
#if VERSION >= VERSION_GM8P_00
  mLoadedFrame->FindWidget("textpane_cancel")->SetIsVisible(!showProceed);
#else
  mLoadedFrame->FindWidget("textpane_proceed")->SetIsVisible(!showProceed);
#endif

#if VERSION >= VERSION_GM8P_00
  const wchar_t* text1 = mMpNotComplete ? gpStringTable->GetString(0x4a)
                         : showFusionSuit  ? L""
                                           : gpStringTable->GetString(0x48);
  const wchar_t* text0 = mFusionNotComplete ? gpStringTable->GetString(0x49)
                         : fusionBeat          ? L""
                                               : gpStringTable->GetString(0x47);

#else
  const wchar_t* text1 = mMpNotComplete ? gpStringTable->GetString(0x50)
                         : showFusionSuit  ? L""
                                           : gpStringTable->GetString(0x4e);
  const wchar_t* text0 = mFusionNotComplete ? gpStringTable->GetString(0x4f)
                         : fusionBeat          ? L""
                                               : gpStringTable->GetString(0x4d);

#endif

  mTextpane_instructions.SetPairText(sel == 1 ? text1 : text0);
}

CFrontEndUI::SFusionBonusFrame::EAction
CFrontEndUI::SFusionBonusFrame::ProcessUserInput(const CFinalInput& input,
                                                 CSaveGameScreen* saveUI) {
  mAction = kFA_None;

  if (saveUI != nullptr) {
    saveUI->ProcessUserInput(input);
  }

  if (mLastDoDraw) {
    if (mGbaLinkFrame.get() != nullptr) {
      SGBALinkFrame::EAction action = mGbaLinkFrame->ProcessUserInput(input, saveUI != nullptr);
      if (action != SGBALinkFrame::kGA_None) {
        mGbaLinkFrame = nullptr;
        if (action == SGBALinkFrame::kGA_Complete) {
          if (mTablegroup_options->GetUserSelection() == 0 &&
              !gpGameState->SystemState().GetFusionBeat()) {
            mFusionNotComplete = true;
          } else if (saveUI != nullptr) {
            saveUI->SaveNESState();
          }
        }
      }
    } else if (mLoadedFrame != nullptr) {
      mLoadedFrame->ProcessUserInput(input);
    }
  }

  return mAction;
}

bool CFrontEndUI::SFusionBonusFrame::PumpLoad() {
  if (mLoadedFrame) {
    return true;
  }
  if (mGbaScreen.TryCache()) {
    if (mGbaLink.TryCache()) {
      if (mGbaSupport->IsReady()) {
        CGuiFrame* frme = mGbaScreen.GetObject();
        if (frme->GetIsFinishedLoading()) {
          mLoadedFrame = frme;
          FinishedLoading();
          return true;
        }
      }
    }
  }
  return false;
}

void CFrontEndUI::SFusionBonusFrame::FinishedLoading() {
  mTablegroup_options =
      static_cast< CGuiTableGroup* >(mLoadedFrame->FindWidget("tablegroup_options"));
  mTablegroup_fusionsuit =
      static_cast< CGuiTableGroup* >(mLoadedFrame->FindWidget("tablegroup_fusionsuit"));
  mTextpane_instructions = SGuiTextPair(mLoadedFrame, "textpane_instructions");

#if VERSION < VERSION_GM8P_00
  SFrontEndFrame::FindAndSetPairText(*mLoadedFrame, "textpane_nes",
                                     gpStringTable->GetString(0x42));
  SFrontEndFrame::FindAndSetPairText(*mLoadedFrame, "textpane_fusionsuit",
                                     gpStringTable->GetString(0x3f));
  SFrontEndFrame::FindAndSetPairText(*mLoadedFrame, "textpane_fusionsuitno",
                                     gpStringTable->GetString(0x41));
  SFrontEndFrame::FindAndSetPairText(*mLoadedFrame, "textpane_fusionsuityes",
                                     gpStringTable->GetString(0x40));
  SFrontEndFrame::FindAndSetPairText(*mLoadedFrame, "textpane_title",
                                     gpStringTable->GetString(0x64));

  CGuiTextPane* proceed =
      static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_proceed"));
  proceed->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(0x55)));

  CGuiTextPane* cancel =
      static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_cancel"));
  cancel->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(0x52)));

#endif

  mTablegroup_fusionsuit->SetIsActive(false);
  mTablegroup_fusionsuit->SetIsVisible(false);
  mTablegroup_fusionsuit->SetVertical(false);
  mTablegroup_fusionsuit->SetUserSelection(gpGameState->SystemState().GetHasFusion() ? 1 : 0);

  SetTableColors(mTablegroup_options);
  SetTableColors(mTablegroup_fusionsuit);

  mTablegroup_options->SetMenuAdvanceCallback(
      TFunctor1FromMethod< SFusionBonusFrame, CGuiTableGroup* const >::Make(
          *this, &SFusionBonusFrame::DoOptionsAdvance));
  mTablegroup_options->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< SFusionBonusFrame, CGuiTableGroup* const, const int >::Make(
          *this, &SFusionBonusFrame::DoSelectionChange));
  mTablegroup_options->SetMenuCancelCallback(
      TFunctor1FromMethod< SFusionBonusFrame, CGuiTableGroup* const >::Make(
          *this, &SFusionBonusFrame::DoOptionsCancel));
  mTablegroup_fusionsuit->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< SFusionBonusFrame, CGuiTableGroup* const, const int >::Make(
          *this, &SFusionBonusFrame::DoSelectionChange));
#if VERSION >= VERSION_GM8P_00
  SetStrings();
#endif
}

#if VERSION >= VERSION_GM8P_00
void CFrontEndUI::SFusionBonusFrame::SetStrings() {
  SFrontEndFrame::FindAndSetPairText(*mLoadedFrame, "textpane_nes",
                                     gpStringTable->GetString(0x3c));
  SFrontEndFrame::FindAndSetPairText(*mLoadedFrame, "textpane_fusionsuit",
                                     gpStringTable->GetString(0x39));
  SFrontEndFrame::FindAndSetPairText(*mLoadedFrame, "textpane_fusionsuitno",
                                     gpStringTable->GetString(0x3b));
  SFrontEndFrame::FindAndSetPairText(*mLoadedFrame, "textpane_fusionsuityes",
                                     gpStringTable->GetString(0x3a));
  SFrontEndFrame::FindAndSetPairText(*mLoadedFrame, "textpane_title",
                                     gpStringTable->GetString(0x5e));
  SetTitlePosition(*mLoadedFrame);

  CGuiTextPane* proceed =
      static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_proceed"));
  proceed->TextSupport().SetText(rstl::wstring(gpStringTable->GetString(0x4c)));
  CGuiTextPane* cancel =
      static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_cancel"));
  cancel->TextSupport().SetText(rstl::wstring(gpStringTable->GetString(0x4f)));
}

void CFrontEndUI::SFusionBonusFrame::ReapplyStrings() { SetStrings(); }

#endif

void CFrontEndUI::SFusionBonusFrame::ResetCompletionFlags() {
  mMpNotComplete = false;
  mFusionNotComplete = false;
}

void CFrontEndUI::SFusionBonusFrame::SetTableColors(CGuiTableGroup* tbgp) {
  CColor selected(uchar(0xff), uchar(0xff), uchar(0xff), uchar(0xff));
  CColor unselected(uchar(0xa0), uchar(0xa0), uchar(0xa0), uchar(0xc8));
  tbgp->SetColors(selected, unselected);
}

void CFrontEndUI::SFusionBonusFrame::DoOptionsAdvance(CGuiTableGroup* caller) {
  int sel = mTablegroup_options->GetUserSelection();
  const CSystemState& systemState = gpGameState->SystemState();
  bool normalModeBeat = systemState.GetNormalModeBeat();
  bool fusionLinked = systemState.GetFusionLinked();
  bool fusionBeat = systemState.GetFusionBeat();

  switch (sel) {
  case 1:
    if (mMpNotComplete) {
      mMpNotComplete = false;
      PlayAdvanceSfx();
    } else if (normalModeBeat) {
      if (fusionLinked)
        break;
      mGbaLinkFrame = rs_new SGBALinkFrame(mGbaLink.GetObject(), mGbaSupport.get(), false);
      PlayAdvanceSfx();
    } else {
      mMpNotComplete = true;
      CSfxManager::SfxStart(SFXfnt_back, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    }
    break;
  case 0:
    if (mFusionNotComplete) {
      mFusionNotComplete = false;
      PlayAdvanceSfx();
    } else if (fusionBeat) {
      mAction = kFA_PlayNESMetroid;
    } else {
      mGbaLinkFrame = rs_new SGBALinkFrame(mGbaLink.GetObject(), mGbaSupport.get(), false);
      PlayAdvanceSfx();
    }
    break;
  default:
    break;
  }
}

void CFrontEndUI::SFusionBonusFrame::DoSelectionChange(CGuiTableGroup* caller, int oldSelection) {
  if (caller == mTablegroup_options) {
    CSfxManager::SfxStart(SFXfnt_selection_change, 0x7f, 0x40, false, CSfxManager::kMedPriority,
                          false, CSfxManager::kAllAreas);
    mMpNotComplete = false;
    mFusionNotComplete = false;
  } else {
    CSfxManager::SfxStart(SFXfnt_enum_change, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    gpGameState->SystemState().SetHasFusion(mTablegroup_fusionsuit->GetUserSelection() == 1);
    gpGameState->PlayerState()->SetIsFusionEnabled(mTablegroup_fusionsuit->GetUserSelection() ==
                                                   1);
  }
  SetTableColors(caller);
}

void CFrontEndUI::SFusionBonusFrame::DoOptionsCancel(CGuiTableGroup* caller) {
  if (mFusionNotComplete || mMpNotComplete) {
    mMpNotComplete = false;
    mFusionNotComplete = false;
    CSfxManager::SfxStart(SFXfnt_back, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  } else {
    mAction = kFA_GoBack;
    mTablegroup_options->SetUserSelection(0);
    mTablegroup_fusionsuit->SetIsActive(false);
    mTextpane_instructions.SetPairText(rstl::wstring_l(L""));
    SetTableColors(mTablegroup_options);
  }
}

CFrontEndUI::CFrontEndUI()
: CIOWin(rstl::string_l("FrontEndUI"))
, mPhase(kP_LoadDepsGroup)
, mRndA(rand() % 3)
, mRndB(rand() % 3)
, mDepsGroup(gpSimplePool->GetObj("FrontEnd_DGRP"))
, mDeps()
#if VERSION >= VERSION_GM8P_00
, mPressStart(
      gpSimplePool->GetObj(skPressStartTextures[gpGameState->SystemState().GetLanguage()]))
#else
, mPressStart(gpSimplePool->GetObj("TXTR_PressStart"))
#endif
, mFrontendAudioGrp(gpSimplePool->GetObj(kFrontEndAGSCName))
, mCurScreen(kS_OpenCredits)
, mNextScreen(kS_OpenCredits)
, mFadeBlackTimer(0.f)
, mFadeBlackWithMovie(false)
, mPressStartTime(0.f)
, mPressStartAlpha(0.f)
, mMusicVol(1.f)
, mMenuMovies(SMenuMovieData())
, mCurMovie(kMM_Stopped)
, mNextAttract(0)
, mAttractCount(0)
, mAttractMovie()
, mCurMoviePtr(nullptr)
, mPlayerSkipToTitle(false)
, mMoviesLoaded(false)
, mDeferSlideShow(false)
#if VERSION >= VERSION_GM8P_00
, mStringsReloading(false)
, mNoCardFrame(false)
, mNoSaveUI(false)
#endif
, mAudio1(nullptr)
, mAudio2(nullptr)
, mSaveUI(rs_new CSaveGameScreen(kSC_FrontEnd, gpGameState->GetCardSerial()))
, mFrontendCardFrme(nullptr)
, mFusionBonusFrme(nullptr)
, mFrontendNoCardFrme(nullptr)
, mEmuFrme(nullptr)
, mOptionsFrme(nullptr)
#if VERSION >= VERSION_GM8P_00
, mLanguageFrme(nullptr)
#endif
, mCurAudio(nullptr) {
  gpMain->ResetGameState();
  gpGameState->SetCurrentWorldId(skDefaultWorld.GetId());
  gpGameState->GameOptions().ResetToDefaults();
  gpGameState->WriteBackupBuf();
  gpResourceFactory->CanBuild(skDefaultWorld);
  mDepsGroup.Lock();
  for (int i = 0;; ++i) {
    if (!CDvdFile::FileExists(GetAttractMovieFileName(i))) {
      break;
    }
    ++mAttractCount;
  }
}

CFrontEndUI::~CFrontEndUI() {
  if (mPhase >= kP_DisplayFrontEnd) {
    CAudioSys::SysPopGroupFromARAM();
    const rstl::string groupName(mFrontendAudioGrp.GetObject()->GetGroupSetName());
    CAudioSys::SysUnloadGroupSet(groupName);
  }
  CStreamAudioManager::FadeBackIn(0.f);
}

void CFrontEndUI::TransitionToFive() {
  if (mPhase < kP_ToPlayGame) {
    CSfxManager::SfxStart(FETransitionForwardSFX[mRndB][0], 0x7f, 0x40, false,
                          CSfxManager::kMedPriority, false, CSfxManager::kAllAreas);
    CSfxManager::SfxStart(FETransitionForwardSFX[mRndB][1], 0x7f, 0x40, false,
                          CSfxManager::kMedPriority, false, CSfxManager::kAllAreas);
    mPhase = kP_ToPlayGame;
    StartStateTransition(kS_ToPlayGame);
  }
}

CIOWin::EMessageReturn CFrontEndUI::OnMessage(const CArchitectureMessage& message,
                                              CArchitectureQueue& queue) {
  switch (message.GetType()) {
  case kAM_TimerTick: {
    float dt = MakeMsg::GetParmTimerTick(message).GetReal();
    return Update(dt, queue);
  }
  case kAM_UserInput: {
    const CFinalInput& input = MakeMsg::GetParmUserInput(message).GetUserInput();
    ProcessUserInput(input, queue);
    break;
  }
  case kAM_QuitGameplay:
    mPhase = kP_ExitFrontEnd;
    break;
  default:
    break;
  }
  return kMR_Normal;
}

void CFrontEndUI::UpdateMusicVol() {
  float volMul = (mCurAudio == mAudio1.get()) ? 0.7421875f : 0.7421875f;
  if (mCurAudio != nullptr) {
    mCurAudio->SetVolume(CCast::ToUint8(
        volMul * mMusicVol * static_cast< float >(gpGameState->GameOptions().GetMusicVolume())));
  }
}

void CFrontEndUI::FinishedLoadingDepsGroup() {
  CDependencyGroup* dgrp = mDepsGroup.GetT();
  const rstl::vector< SObjectTag >& tags = dgrp->GetObjectTagVector();
  mDeps.reserve(tags.size());
  for (const SObjectTag* it = tags.data(); it != tags.data() + tags.size(); ++it) {
    CToken tok(gpSimplePool->GetObj(*it));
    tok.Lock();
    mDeps.push_back(tok);
  }
  mFrontendAudioGrp.Lock();
}

bool CFrontEndUI::PumpLoad() {
  for (int i = 0; i < mDeps.size(); ++i) {
    if (!mDeps[i].IsLoaded())
      return false;
  }
  return mFrontendAudioGrp.TryCache();
}

CIOWin::EMessageReturn CFrontEndUI::Update(float dt, CArchitectureQueue& queue) {
  // Update save UI if active and past file select phase
#if VERSION >= VERSION_GM8P_00
  if (!mNoSaveUI && mCurScreen >= kS_FileSelect && mNextScreen >= kS_FileSelect) {
#else
  if (mSaveUI.get() != nullptr && mCurScreen >= kS_FileSelect) {
#endif
    int saveResult = mSaveUI->Update(dt);
    if (saveResult == 1) {
      TransitionToFive();
    } else if (saveResult == 3 || saveResult == 2) {
#if VERSION >= VERSION_GM8P_00
      mNoCardFrame = true;
      mNoSaveUI = true;
#else
      mFrontendCardFrme = nullptr;
      mSaveUI = nullptr;
#endif
    }
  }

  UpdateMusicVol();

  switch (mPhase) {
  case kP_LoadDepsGroup:
    if (mDepsGroup.IsLoaded()) {
      FinishedLoadingDepsGroup();
      mDepsGroup.Unlock();
      mPhase = kP_LoadDeps;
    } else {
      return kMR_Exit;
    }
    // fallthrough
  case kP_LoadDeps:
    if (PumpLoad()) {
      mFrontendCardFrme = rs_new SNewFileSelectFrame(mSaveUI.get(), mRndB);
      mFusionBonusFrme = rs_new SFusionBonusFrame();
      mFrontendNoCardFrme = rs_new SFrontEndFrame(mRndB);

      mPressStart.Lock();
      mPressStart.TryCache();

      const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(kFrontEndAGSCName);
      CAudioGrpSetLoc* audioGrp = mFrontendAudioGrp.GetObject();
      const rstl::string& audioGrpName = audioGrp->GetGroupSetName();
      CAudioSys::SysLoadGroupSet(mFrontendAudioGrp, audioGrpName, tag->GetId());
      CAudioSys::SysPushGroupIntoARAM(audioGrpName, 0x26);
      CAudioSys::SysUnloadSampleData(audioGrpName);

      mAudio1 = rs_new CStaticAudioPlayer(rstl::string_l(kAudioFrontend1Path), 416480, 1973664);

      mAudio2 = rs_new CStaticAudioPlayer(rstl::string_l(kAudioFrontend2Path), 273556, 1636980);

      mPhase = kP_LoadFrames;
    }
    if (mPhase == kP_LoadDeps) {
      return kMR_Exit;
    }
    // fallthrough
  case kP_LoadFrames:
    if (!mAudio1->IsReady() || !mAudio2->IsReady() || !mFrontendCardFrme->PumpLoad() ||
        !mFusionBonusFrme->PumpLoad() || !mFrontendNoCardFrme->PumpLoad() ||
        !mSaveUI->PumpLoad()) {
      return kMR_Exit;
    }

    mCurAudio = mAudio1.get();
    mCurAudio->StartMixOut();
    mPhase = kP_LoadMovies;
    // fallthrough
  case kP_LoadMovies: {
    bool moviesReady = true;
    if (!PumpMovieLoad()) {
      moviesReady = false;
    } else {
      UpdateMovies(dt);
      for (int i = 0; i < 9; ++i) {
        if (!mMenuMovies[i].mMovie->GetIsFullyCached()) {
          moviesReady = false;
          break;
        }
      }
    }
    if (moviesReady) {
      mPhase = kP_DisplayFrontEnd;
      StartStateTransition(kS_Title);
    } else {
      return kMR_Exit;
    }
    // fallthrough
  }
  case kP_DisplayFrontEnd:
  case kP_ToPlayGame:
#if VERSION >= VERSION_GM8P_00
  {
    bool reloading = false;
    rstl::vector< SObjectTag > tags = gpSimplePool->GetReferencedTags();
    for (rstl::vector< SObjectTag >::const_iterator it = tags.begin(); it != tags.end(); ++it) {
      if (it->GetType() == 'STRG' && gpSimplePool->GetObj(*it).IsLoaded()) {
        TLockedToken< CStringTable > table = gpSimplePool->GetObj(*it);
        CStringTable& strings = **table;
        strings.TryFinishReload();
        if (strings.IsReloading()) {
          reloading = true;
          break;
        }
      }
    }
    if (reloading != mStringsReloading) {
      if (mStringsReloading) {
        printf("Reapplying strings.\n");
        ReapplyStrings();
      }
      mStringsReloading = reloading;
    }
  }
#endif
    if (mEmuFrme.get() != nullptr) {
      if (mEmuFrme->Update(dt,
#if VERSION >= VERSION_GM8P_00
                              mNoSaveUI ? nullptr : mSaveUI.get()
#else
                              mSaveUI.get()
#endif
                                  ) == 1) {
        mEmuFrme = nullptr;
#if VERSION >= VERSION_GM8P_00
        if (!mNoSaveUI) {
#else
        if (mSaveUI.get() != nullptr) {
#endif
          mSaveUI->SetInGame(false);
        }
        mCurAudio->StartMixOut();
      }
    } else {
      if (mDeferSlideShow) {
        mDeferSlideShow = false;
        mCurAudio->StartMixOut();
#if VERSION >= VERSION_GM8P_00
        if (!mNoSaveUI) {
#else
        if (mSaveUI.get() != nullptr) {
#endif
          mSaveUI->ResetCardDriver();
        }
      }

      if (IsInScreenNotTransitioning(kS_FileSelect)) {
#if VERSION >= VERSION_GM8P_00
        if (mOptionsFrme.get() != nullptr) {
          const bool active =
              mNoSaveUI || mSaveUI->GetUIType() == CSaveGameScreen::kUIT_SaveReady;
          if (active) {
            mOptionsFrme->Update(dt, mNoSaveUI ? nullptr : mSaveUI.get());
          } else {
            mOptionsFrme = nullptr;
          }
        } else if (mLanguageFrme.get() != nullptr) {
          const bool active =
              mNoSaveUI || mSaveUI->GetUIType() == CSaveGameScreen::kUIT_SaveReady;
          if (active) {
            mLanguageFrme->Update(dt, mNoSaveUI ? nullptr : mSaveUI.get());
          } else {
            mLanguageFrme = nullptr;
          }
        } else if (mNoCardFrame) {
          mFrontendNoCardFrme->Update(dt);
        } else {
          mFrontendCardFrme->Update(dt);
        }
#else
        if (mOptionsFrme.get() == nullptr) {
          if (mFrontendCardFrme.get() == nullptr) {
            mFrontendNoCardFrme->Update(dt);
          } else {
            mFrontendCardFrme->Update(dt);
          }
        } else {
          CSaveGameScreen* saveUI = mSaveUI.get();
          bool optionsActive = true;
          if (saveUI != nullptr) {
            CSaveGameScreen::EUIType type = saveUI->GetUIType();
            if (type != CSaveGameScreen::kUIT_SaveReady) {
              optionsActive = false;
            }
          }
          if (optionsActive) {
            mOptionsFrme->Update(dt, mSaveUI.get());
          } else {
            mOptionsFrme = nullptr;
          }
        }
#endif
      } else if (IsInScreenNotTransitioning(kS_FusionBonus)) {
        mFusionBonusFrme->Update(dt,
#if VERSION >= VERSION_GM8P_00
                                    mNoSaveUI ? nullptr : mSaveUI.get()
#else
                                    mSaveUI.get()
#endif
        );
      }

      // Check movie transition completion
      if (mCurScreen != mNextScreen) {
        if (mCurMoviePtr != nullptr && mCurMoviePtr->CanDrawVideo()) {
          if (mCurMoviePtr->GetIsMovieFinishedPlaying() || mCurMoviePtr->IsLooping()) {
            CompleteStateTransition();
          }
        }
      }

      // Handle fade timer
      if (mFadeBlackTimer > 0.f && !mFadeBlackWithMovie) {
        SetFadeBlackTimer(rstl::max_val(mFadeBlackTimer - dt, 0.f));
        if (mFadeBlackTimer == 0.f) {
          if (IsInScreenNotTransitioning(kS_Title)) {
            if (mAttractCount > 0) {
              StartStateTransition(kS_AttractMovie);
            }
          } else if (mNextScreen == kS_AttractMovie) {
            CompleteStateTransition();
          } else if (mCurScreen != mNextScreen) {
            CompleteStateTransition();
          }
        }
      }

      UpdateMovies(dt);

      // Press start pulsing
      if (mCurScreen == kS_Title && mNextScreen == kS_Title &&
          mFadeBlackTimer < 30.f - gpTweakGame->GetPressStartDelay()) {
        mPressStartTime = static_cast< float >(fmod(mPressStartTime + dt, 1.0));
        float halfTime = 0.5f;
        float alpha;
        if (mPressStartTime < halfTime) {
          alpha = mPressStartTime / halfTime;
        } else {
          alpha = (1.f - mPressStartTime) / halfTime;
        }
        mPressStartAlpha = alpha;
      } else {
        mPressStartTime = 0.f;
        mPressStartAlpha = 0.f;
      }
    }

    // Music volume fade
#if VERSION >= VERSION_GM8P_00
    if ((mCurScreen == kS_Title && mNextScreen == kS_FileSelect) ||
        (mCurScreen == kS_FileSelect && mNextScreen == kS_Title)) {
#else
    if (mCurScreen == kS_Title && mNextScreen == kS_FileSelect) {
#endif
      if (mCurMoviePtr->CanDrawVideo()) {
#if VERSION >= VERSION_GM8P_00
        float delay = mNextScreen == kS_Title ? AudioFadeTimeBack : AudioFadeTimeA[mRndA];
#else
        float delay = AudioFadeTimeA[mRndA];
#endif
        mMusicVol =
            1.f - CMath::Clamp(0.f, (mCurMoviePtr->GetPlayedSeconds() - delay) / 2.5f, 1.f);
      }
    } else if (mNextScreen == kS_ToPlayGame) {
      if (mCurMoviePtr->CanDrawVideo()) {
        float delay = AudioFadeTimeB[mRndB];
        float played = mCurMoviePtr->GetPlayedSeconds();
        float total = mCurMoviePtr->GetTotalSeconds();
        mMusicVol = 1.f - CMath::Clamp(0.f, (played - delay) / (total - delay), 1.f);
      }
    } else {
      mMusicVol = 1.f;
    }

    return kMR_Exit;

  case kP_ExitFrontEnd:
    return kMR_RemoveIOWin;

  default:
    break;
  }

  return kMR_Exit;
}

bool CFrontEndUI::PumpMovieLoad() {
  if (mMoviesLoaded) {
    return true;
  }

  for (int i = 0; i < 9; ++i) {
    if (mMenuMovies[i].mMovie.null()) {
      const FEMovie* movie = &FEMovies[static_cast< EMenuMovie >(i)];
      char path[256];
      strcpy(path, movie->path);

      if (i == 2) {
        path[strlen(path) - 5] = static_cast< char >(mRndA + 'A');
      } else if (i == 4) {
        path[strlen(path) - 5] = static_cast< char >(mRndB + 'A');
      }

      mMenuMovies[i].mMovie = rs_new CMoviePlayer(path, 0.05f, movie->loop, true);
      mMenuMovies[i].mMovie->SetPlayMode(CMoviePlayer::kPM_Stopped);
      return false;
    }

    if (mMenuMovies[i].mMovie->PumpIndexLoad()) {
      return false;
    }
  }

  mMoviesLoaded = true;
  return true;
}

void CFrontEndUI::UpdateMovies(float dt) {
  if (mCurMoviePtr != nullptr) {
    if (!mCurMoviePtr->PumpIndexLoad()) {
      if (mFadeBlackWithMovie) {
        mFadeBlackWithMovie = false;
        mFadeBlackTimer = mCurMoviePtr->GetTotalSeconds();
      }
    }
  }

  for (int i = 0; i < 9; ++i) {
    if (!mMenuMovies[i].mMovie.null()) {
      mMenuMovies[i].mMovie->Update(dt);
    }
  }

  CMoviePlayer* attractPlayer = mAttractMovie.get();
  if (attractPlayer != nullptr) {
    if (!attractPlayer->PumpIndexLoad()) {
      mAttractMovie->Update(dt);
    }
  }
}

void CFrontEndUI::ProcessUserInput(const CFinalInput& input, CArchitectureQueue& queue) {
  if (gpMain->GetCardBusy())
    return;
  if (input.ControllerNumber() > 1)
    return;

#if VERSION >= VERSION_GM8P_00
  if (mStringsReloading)
    return;
#endif

  if (mEmuFrme.get() != nullptr) {
    mEmuFrme->ProcessUserInput(input,
#if VERSION >= VERSION_GM8P_00
                                  mNoSaveUI ? nullptr : mSaveUI.get()
#else
                                  mSaveUI.get()
#endif
    );
    return;
  }

  if (mPhase != kP_DisplayFrontEnd)
    return;
  if (input.ControllerNumber() != 0)
    return;

  if (mCurScreen != mNextScreen) {
    if (mNextScreen == kS_AttractMovie) {
      if (input.PStart() || input.PA()) {
        const float maxFade = 1.f;
        SetFadeBlackTimer(rstl::min_val(mFadeBlackTimer, maxFade));
        PlayAdvanceSfx();
        return;
      }
    }
    if (input.PA() || input.PStart()) {
      if (mCurScreen == kS_OpenCredits && mNextScreen == kS_Title &&
          mFadeBlackTimer > 1.f) {
        mPlayerSkipToTitle = true;
        SetFadeBlackTimer(1.f);
        return;
      }
    }
  } else {
    if (mCurScreen == kS_Title) {
      if (input.PStart() || input.PA()) {
        if (mFadeBlackTimer < 30.f - gpTweakGame->GetPressStartDelay()) {
          CSfxManager::SfxStart(FETransitionBackSFX[mRndA][0], 0x7f, 0x40, false,
                                CSfxManager::kMedPriority, false, CSfxManager::kAllAreas);
          CSfxManager::SfxStart(FETransitionBackSFX[mRndA][1], 0x7f, 0x40, false,
                                CSfxManager::kMedPriority, false, CSfxManager::kAllAreas);
          StartStateTransition(kS_FileSelect);
          return;
        }
      }
    } else {
      bool isFileSelect = mCurScreen == kS_FileSelect && mNextScreen == kS_FileSelect;
      if (isFileSelect) {
#if VERSION >= VERSION_GM8P_00
        if (mOptionsFrme.get() != nullptr) {
          if (!mOptionsFrme->ProcessUserInput(input,
                                                 mNoSaveUI ? nullptr : mSaveUI.get())) {
            mOptionsFrme = nullptr;
          }
          return;
        }
#else
        if (mOptionsFrme.get() == nullptr) {
#endif
#if VERSION >= VERSION_GM8P_00
        if (mLanguageFrme.get() != nullptr) {
          if (!mLanguageFrme->ProcessUserInput(input,
                                                  mNoSaveUI ? nullptr : mSaveUI.get())) {
            mLanguageFrme = nullptr;
          }
          return;
        }
        if (mNoCardFrame) {
#else
          if (mFrontendCardFrme.get() == nullptr) {
#endif
          SFrontEndFrame::EAction action = mFrontendNoCardFrme->ProcessUserInput(input);
          if (action == SFrontEndFrame::kEA_FusionBonus) {
            StartStateTransition(kS_FusionBonus);
            return;
          } else if (action == SFrontEndFrame::kEA_GameOptions) {
            mOptionsFrme = rs_new SOptionsFrontEndFrame();
            return;
          } else if (action == SFrontEndFrame::kEA_StartGame) {
            TransitionToFive();
            return;
          } else if (action == SFrontEndFrame::kEA_SlideShow) {
            mDeferSlideShow = true;
            StartSlideShow(queue);
            return;
#if VERSION >= VERSION_GM8P_00
          } else if (action == SFrontEndFrame::kEA_Language) {
            mLanguageFrme = rs_new SLanguageSelectFrame();
            return;
          } else if (action == SFrontEndFrame::kEA_ReturnToTitle) {
            StartStateTransition(kS_Title);
            return;
#endif
          }
        } else {
          SNewFileSelectFrame::EAction action = mFrontendCardFrme->ProcessUserInput(input);
          if (action == SNewFileSelectFrame::kA_FusionBonus) {
            StartStateTransition(kS_FusionBonus);
            return;
          } else if (action == SNewFileSelectFrame::kA_GameOptions) {
            mOptionsFrme = rs_new SOptionsFrontEndFrame();
            return;
          } else if (action == SNewFileSelectFrame::kA_SlideShow) {
            mDeferSlideShow = true;
            StartSlideShow(queue);
            return;
#if VERSION >= VERSION_GM8P_00
          } else if (action == SNewFileSelectFrame::kA_Language) {
            mLanguageFrme = rs_new SLanguageSelectFrame();
            return;
          } else if (action == SNewFileSelectFrame::kA_ReturnToTitle) {
            StartStateTransition(kS_Title);
            return;
#endif
          }
        }
#if VERSION < VERSION_GM8P_00
        } else {
          if (mOptionsFrme->ProcessUserInput(input, mSaveUI.get()))
            return;
          mOptionsFrme = nullptr;
          return;
        }
#endif
      } else {
        bool isFusionBonus = mCurScreen == kS_FusionBonus && mNextScreen == kS_FusionBonus;
        if (isFusionBonus) {
          SFusionBonusFrame::EAction action =
              mFusionBonusFrme->ProcessUserInput(input,
#if VERSION >= VERSION_GM8P_00
                                                    mNoSaveUI ? nullptr : mSaveUI.get()
#else
                                                    mSaveUI.get()
#endif
              );
          if (action == SFusionBonusFrame::kFA_GoBack) {
            StartStateTransition(kS_FileSelect);
            return;
          } else if (action == SFusionBonusFrame::kFA_PlayNESMetroid) {
            mCurAudio->StopMixOut();
            mEmuFrme = rs_new SNesEmulatorFrame();
            if (
#if VERSION >= VERSION_GM8P_00
                !mNoSaveUI
#else
                mSaveUI.get() != nullptr
#endif
            ) {
              mSaveUI->SetInGame(true);
            }
            return;
          }
        }
      }
    }
  }
}

void CFrontEndUI::Draw() const {
  if (mPhase < kP_DisplayFrontEnd)
    return;

  if (mEmuFrme.get() != nullptr) {
#if VERSION >= VERSION_GM8P_00
    mEmuFrme->Draw(mNoSaveUI ? nullptr : mSaveUI.get());
#else
    mEmuFrme->Draw(mSaveUI.get());
#endif
  } else {
#if VERSION >= VERSION_GM8P_00
    if (mCurMoviePtr != nullptr) {
      mCurMoviePtr->DrawVideo();
    }
#else
    gpRender->SetDepthReadWrite(false, false);
    gpRender->SetViewportOrtho(false, -4096.f, 4096.f);

    int vpLeft = CGraphics::GetViewport().mLeft;
    int vpTop = CGraphics::GetViewport().mTop;
    int vpWidth = CGraphics::GetViewport().mWidth;
    int vpHeight = CGraphics::GetViewport().mHeight;

    if (mCurMoviePtr != nullptr && mCurMoviePtr->CanDrawVideo()) {
      uint vidWidth = mCurMoviePtr->GetWidth();
      uint vidHeight = mCurMoviePtr->GetHeight();
#if NONMATCHING
      int centerX = (static_cast< int >(vidWidth) - vpWidth) / 2;
      int centerY = (static_cast< int >(vidHeight) - vpHeight) / 2;
#else
      int centerX = (vidWidth - vpWidth) / 2;
      int centerY = (vidHeight - vpHeight) / 2;
#endif
      int vl = vpLeft - centerX;
      int vr = vpLeft + vpWidth + centerX;
      int vb = vpTop + vpHeight + centerY;
      int vt = vpTop - centerY;
      CVector3f v1(static_cast< float >(vl), 0.f, static_cast< float >(vb));
      CVector3f v2(static_cast< float >(vr), 0.f, static_cast< float >(vb));
      CVector3f v3(static_cast< float >(vl), 0.f, static_cast< float >(vt));
      CVector3f v4(static_cast< float >(vr), 0.f, static_cast< float >(vt));
      mCurMoviePtr->DrawFrame(v1, v2, v3, v4);
    }

#endif

    if (IsInScreenNotTransitioning(kS_FileSelect)) {
#if VERSION >= VERSION_GM8P_00
      if (mOptionsFrme.get() != nullptr) {
        mOptionsFrme->Draw();
      } else if (mLanguageFrme.get() != nullptr) {
        mLanguageFrme->Draw();
      } else if (!mStringsReloading) {
        if (mNoCardFrame)
          mFrontendNoCardFrme->Draw();
        else
          mFrontendCardFrme->Draw();
      }
#else
      if (mOptionsFrme.get() == nullptr) {
        if (mFrontendCardFrme.get() == nullptr)
          mFrontendNoCardFrme->Draw();
        else
          mFrontendCardFrme->Draw();
      } else {
        mOptionsFrme->Draw();
      }
#endif
    } else if (IsInScreenNotTransitioning(kS_FusionBonus)) {
      mFusionBonusFrme->Draw();
    }
  }
  if (mPressStartAlpha > 0.f && mPressStart.GetObject() != nullptr) {
    CTexture* tex = mPressStart.GetObject();
    short width = tex->GetWidth();
    short height = tex->GetHeight();
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    gpRender->SetBlendMode_AdditiveAlpha();
    gpRender->SetDepthReadWrite(false, false);
#if VERSION >= VERSION_GM8P_00
    const int y = sIs50Hz ? 96 : 72;
#endif
    const CColor& color = CColor::White().WithAlphaOf(mPressStartAlpha);
#if VERSION >= VERSION_GM8P_00
    CGraphics::Render2D(*tex, 320 - width / 2, y - height / 2, width, height, color);
#else
    CGraphics::Render2D(*tex, 320 - width / 2, 72 - height / 2, width, height, color);
#endif
  }

  if (GetHasAttractMovies()) {
    if (IsInScreenNotTransitioning(kS_Title) || mNextScreen == kS_AttractMovie) {
      if (mFadeBlackTimer < 1.f) {
        const CColor& color = CColor::Black().WithAlphaOf(1.f - mFadeBlackTimer);
        CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend,
                                      CCameraFilterPass::kFS_Fullscreen, color, nullptr, 1.f);
      }
    }
  }

  if (mPlayerSkipToTitle) {
    if (mNextScreen == kS_Title && mCurScreen == kS_OpenCredits) {
      float t = CMath::Clamp(0.f, mFadeBlackTimer, 1.f);
      const CColor& color = CColor::Black().WithAlphaOf(1.f - t);
      CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                    color, nullptr, 1.f);
    } else if (mNextScreen == kS_Title && mCurScreen == kS_Title) {
      float t = CMath::Clamp(0.f, 30.f - mFadeBlackTimer, 1.f);
      const CColor& color = CColor::Black().WithAlphaOf(1.f - t);
      CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                    color, nullptr, 1.f);
    }
  }

#if VERSION >= VERSION_GM8P_00
  if (!mNoSaveUI) {
#else
  if (mSaveUI.get() != nullptr) {
#endif
    CSaveGameScreen::EUIType uiType = mSaveUI->GetUIType();
    if ((CanShowSaveUI() && !CSaveGameScreen::IsHiddenFromFrontEnd(uiType)) ||
        IsInScreenNotTransitioning(kS_FileSelect) || IsInScreenNotTransitioning(kS_FusionBonus)) {
      mSaveUI->Draw();
    }
  }
}

bool CFrontEndUI::CanShowSaveUI() const {
  return (mCurScreen == kS_FileSelect || mCurScreen == kS_FusionBonus) &&
         (mNextScreen == kS_FileSelect || mNextScreen == kS_FusionBonus);
}

void CFrontEndUI::StartStateTransition(EScreen screen) {
  switch (mCurScreen) {
  case kS_OpenCredits:
    break;
  case kS_Title:
    if (screen == kS_FileSelect) {
      SetCurrentMovie(kMM_StartFileSelectA);
      SetFadeBlackTimer(mCurMoviePtr->GetTotalSeconds());
#if VERSION >= VERSION_GM8P_00
      mPlayerSkipToTitle = false;
#endif
    }
    break;
  case kS_AttractMovie:
    break;
  case kS_FileSelect:
    if (screen == kS_ToPlayGame) {
      SetCurrentMovie(kMM_FileSelectPlayGameA);
      SetFadeBlackTimer(mCurMoviePtr->GetTotalSeconds());
    } else if (screen == kS_FusionBonus) {
      SetCurrentMovie(kMM_FileSelectGBA);
      SetFadeBlackTimer(mCurMoviePtr->GetTotalSeconds());
      CSfxManager::SfxStart(SFXfnt_tofusion_L);
      CSfxManager::SfxStart(SFXfnt_tofusion_R);
#if VERSION >= VERSION_GM8P_00
    } else if (screen == kS_Title) {
      CSfxManager::SfxStart(0x458);
      CSfxManager::SfxStart(0x459);
      SetCurrentMovie(kMM_BackToTitle);
      SetFadeBlackTimer(mCurMoviePtr->GetTotalSeconds());
      if (mNoSaveUI) {
        mNoCardFrame = false;
        mNoSaveUI = false;
      }
      mPressStart =
          gpSimplePool->GetObj(skPressStartTextures[gpGameState->SystemState().GetLanguage()]);
      mPressStart.ForceCache();
#endif
    }
    break;
  case kS_FusionBonus:
#if VERSION < VERSION_GM8P_00
    if (screen == kS_ToPlayGame) {
      SetCurrentMovie(kMM_GBAFileSelectB);
      SetFadeBlackTimer(mCurMoviePtr->GetTotalSeconds());
    } else
#endif
        if (screen == kS_FileSelect) {
      SetCurrentMovie(kMM_GBAFileSelectA);
      SetFadeBlackTimer(mCurMoviePtr->GetTotalSeconds());
      CSfxManager::SfxStart(SFXfnt_fromfusion_L);
      CSfxManager::SfxStart(SFXfnt_fromfusion_R);
    }
    break;
  default:
    break;
  }

  switch (screen) {
  case kS_OpenCredits:
  case kS_Title:
#if VERSION >= VERSION_GM8P_00
    if (mCurScreen == kS_OpenCredits) {
#endif
      SetCurrentMovie(kMM_FirstStart);
      SetFadeBlackTimer(mCurMoviePtr->GetTotalSeconds());
#if VERSION >= VERSION_GM8P_00
    }
#endif
    break;
  case kS_AttractMovie:
    StartAttractMovie();
    SetFadeBlackWithMovie();
    break;
  case kS_FileSelect:
  default:
    break;
  }

  mNextScreen = screen;
}

void CFrontEndUI::CompleteStateTransition() {
  EScreen oldScreen = mCurScreen;
  mCurScreen = mNextScreen;

  switch (mCurScreen) {
  case kS_AttractMovie:
    mNextScreen = kS_OpenCredits;
    mCurScreen = kS_OpenCredits;
    mPlayerSkipToTitle = false;
    StartStateTransition(kS_Title);
    break;

  case kS_Title:
    SetCurrentMovie(kMM_StartLoop);
    SetFadeBlackTimer(30.f);
#if VERSION >= VERSION_GM8P_00
    if (oldScreen == kS_FileSelect) {
      mCurAudio->StopMixOut();
      mCurAudio = mAudio1.get();
      mCurAudio->StartMixOut();
    }
#endif
    break;

  case kS_FileSelect:
    SetCurrentMovie(kMM_FileSelectLoop);
    if (oldScreen == kS_Title) {
      mCurAudio->StopMixOut();
      mCurAudio = mAudio2.get();
      mCurAudio->StartMixOut();
    }
    if (
#if VERSION >= VERSION_GM8P_00
        !mNoSaveUI
#else
          mSaveUI.get() != nullptr
#endif
    ) {
      mSaveUI->ResetCardDriver();
    }
    break;

  case kS_FusionBonus:
    SetCurrentMovie(kMM_GBALoop);
    break;

  case kS_ToPlayGame:
    mPhase = kP_ExitFrontEnd;
    break;

  default:
    break;
  }
}

void CFrontEndUI::StartAttractMovie() {
  if (!mAttractMovie.null()) {
    return;
  }
  SetCurrentMovie(kMM_Stopped);
  const char* name = GetNextAttractMovieFileName();
  mAttractMovie = rstl::auto_ptr< CMoviePlayer >(rs_new CMoviePlayer(name, 0.f, false, false));
  mCurMoviePtr = mAttractMovie.get();
}

void CFrontEndUI::StopAttractMovie() {
  if (mAttractMovie.null()) {
    return;
  }
  mAttractMovie = rstl::auto_ptr< CMoviePlayer >();
  mCurMoviePtr = nullptr;
}

void CFrontEndUI::SetCurrentMovie(EMenuMovie movie) {
  if (movie == mCurMovie) {
    return;
  }
  StopAttractMovie();
  if (mCurMovie != kMM_Stopped) {
    mCurMoviePtr->SetPlayMode(CMoviePlayer::kPM_Stopped);
    mCurMoviePtr->Rewind();
  }
  mCurMovie = movie;
  if (mCurMovie != kMM_Stopped) {
    mCurMoviePtr = mMenuMovies[movie].mMovie.get();
    mCurMoviePtr->SetPlayMode(CMoviePlayer::kPM_Playing);
  } else {
    mCurMoviePtr = nullptr;
  }
}

const char* CFrontEndUI::GetNextAttractMovieFileName() {
  const char* ret = GetAttractMovieFileName(mNextAttract);
  mNextAttract = (mNextAttract + 1) % mAttractCount;
  return ret;
}

const char* CFrontEndUI::GetAttractMovieFileName(int idx) {
  return CBasics::Stringize("Video/attract%d.thp", idx);
}

void CFrontEndUI::SetFadeBlackTimer(float seconds) {
  mFadeBlackTimer = seconds;
  mFadeBlackWithMovie = false;
}

void CFrontEndUI::SetFadeBlackWithMovie() {
  mFadeBlackTimer = 1000000.f;
  mFadeBlackWithMovie = true;
}

void CFrontEndUI::StartSlideShow(CArchitectureQueue& queue) {
  mCurAudio->StopMixOut();
  queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, kFrontEndUIMsgPriority,
                                        kFrontEndUIDrawPriority, rs_new CSlideShow()));
}

#if VERSION >= VERSION_GM8P_00
void CFrontEndUI::ReapplyStrings() {
  if (mFrontendNoCardFrme.get() != nullptr) {
    mFrontendNoCardFrme->ReapplyStrings();
  }
  if (!mNoCardFrame) {
    mFrontendCardFrme->ReapplyStrings();
  }
  mFusionBonusFrme->ReapplyStrings();
}
#endif
