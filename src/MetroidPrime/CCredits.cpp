#include "MetroidPrime/CCredits.hpp"
#include "MetroidPrime/CAutoSave.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CSaveGameScreen.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStaticAudioPlayer.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "MetroidPrime/CArchitectureMessage.hpp"
#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/CPlayMovie.hpp"
#include "MetroidPrime/CQuitGameScreen.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"

#include "GuiSys/CGuiTextSupport.hpp"
#include "GuiSys/CGuiWidget.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CMoviePlayer.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "Kyoto/Text/CStringTable.hpp"

#include "MetaRender/CCubeRenderer.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"

#include "dolphin/gx.h"

static const char* const skMovieNames[] = {"Video/wingame.thp",       "Video/wingame_best.thp",
                                           "Video/wingame_best.thp",  "Video/losegame.thp",
                                           "Video/05_tallonText.thp", "Video/AfterCredits.thp",
                                           "Video/SpecialEnding.thp", "Video/creditBG.thp"};
static const char* const skCompletionMovieNames[] = {
    "Video/win_bad_begin.thp",  "Video/win_bad_loop.thp",  "Video/win_bad_end.thp",
    "Video/win_good_begin.thp", "Video/win_good_loop.thp", "Video/win_good_end.thp"};
static const char* const skCreditsAudio = "Audio/ending3.rsf";
static const char* const skCompletionAudio = "Audio/samusjak.rsf";

bool CPlayMovie::GetIsContinueDraw() const { return false; }
bool CCredits::GetIsContinueDraw() const { return false; }
bool CMoviePlayer::DrawVideo() const {
  if (!CanDrawVideo()) {
    return false;
  }
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  const int left = CGraphics::GetViewport().mLeft;
  const int top = CGraphics::GetViewport().mTop;
  const int width = CGraphics::GetViewport().mWidth;
  const int height = CGraphics::GetViewport().mHeight;
  const uint movieWidth = GetWidth();
  const uint movieHeight = GetHeight();
  const int xMargin = (movieWidth - width) / 2;
  const int yMargin = (movieHeight - height) / 2;
  const int xmin = left - xMargin;
  const int xmax = left + width + xMargin;
  const int ymin = top - yMargin;
  const int ymax = top + height + yMargin;
  const CVector3f topLeft(xmin, 0.f, ymax);
  const CVector3f topRight(xmax, 0.f, ymax);
  const CVector3f bottomLeft(xmin, 0.f, ymin);
  const CVector3f bottomRight(xmax, 0.f, ymin);
  const_cast< CMoviePlayer* >(this)->DrawFrame(topLeft, topRight, bottomLeft, bottomRight);
  return true;
}

static void SetStaticAudioPlayerVolume(const rstl::single_ptr< CStaticAudioPlayer >& player,
                                       float volume) {
  if (!player.null()) {
    player->SetVolume(
        CCast::ToUint8(0.7421875f * volume * gpGameState->GameOptions().GetMusicVolume()));
    player->StartMixOut();
  }
}

void CCredits::DrawText(CGuiTextSupport& text, const CVector3f& translation) {
  CGraphics::SetCullMode(kCM_None);
  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  gpRender->SetModelMatrix(CTransform4f::Translate(translation));
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetBlendMode_AdditiveAlpha();
  text.Render();
}

static void QueueIOWin(CArchitectureQueue& queue, CIOWin* win) {
  queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, 12, 11, win));
}

static void QueueAutoSave(CArchitectureQueue& queue) { QueueIOWin(queue, rs_new CAutoSave()); }

CPlayMovie::CPlayMovie(EWhichMovie movie)
: CIOWin(rstl::string_l("PlayMovie"))
, x14_state(0)
, x18_which(movie)
, x38_moviePlayer(nullptr)
, x3c_movieIndex(-1)
, x48_completionScreenStrings(static_cast< CStringTable* >(nullptr))
, x50_largeFont(static_cast< CRasterFont* >(nullptr))
, x68_textDelay(gpTweakGui->GetCompletionScreenTextDelay())
, x6c_resultsTime(0.f)
, x70_pulseTime(gpTweakGui->GetCompletionScreenPulseTime())
, x74_printedCharacters(0.f)
, x78_24_finished(false)
, x78_25_exit(false)
, x78_26_resultsScreen(false)
, x78_27_quitScreen(false) {
  if (IsGameWon(x18_which)) {
    x78_26_resultsScreen = true;
  } else if (movie == kWM_LoseGame) {
    x40_quitScreen = rs_new CQuitGameScreen(kQT_ContinueFromLastSave);
    x78_27_quitScreen = true;
  }
  CGraphics::SetIsBeginSceneClearFb(true);
  if (x78_26_resultsScreen) {
    x48_completionScreenStrings =
        gpSimplePool->GetObj(gpTweakGui->GetCompletionScreenTable().data());
    x48_completionScreenStrings.Lock();
    x50_largeFont = gpSimplePool->GetObj(gpTweakGui->GetCompletionScreenTitleFont().data());
    x50_largeFont.Lock();
    int start = 0;
    switch (x18_which) {
    case kWM_WinGameBad:
    case kWM_WinGameGood:
      start = 0;
      break;
    case kWM_WinGameBest:
      start = 3;
      break;
    default:
      break;
    }
    const int end = start + 3;
    for (int i = start; i < end; ++i) {
      const bool loop = i % 3 == 1;
      x1c_movies.push_back(rstl::auto_ptr< CMoviePlayer >(
          rs_new CMoviePlayer(skCompletionMovieNames[i], 0.05f, loop, false)));
    }
  } else {
    x1c_movies.push_back(rstl::auto_ptr< CMoviePlayer >(
        rs_new CMoviePlayer(skMovieNames[movie], 0.f, false, movie == kWM_LoseGame)));
  }
}

CPlayMovie::~CPlayMovie() {}

bool CPlayMovie::IsGameWon(EWhichMovie movie) {
  return movie == kWM_WinGameBad || movie == kWM_WinGameGood || movie == kWM_WinGameBest;
}

CIOWin::EMessageReturn CPlayMovie::OnMessage(const CArchitectureMessage& msg,
                                             CArchitectureQueue& queue) {
  switch (msg.GetType()) {
  case kAM_TimerTick: {
    const float dt = MakeMsg::GetParmTimerTick(msg).GetReal();
    switch (x14_state) {
    case 0: {
      if (x78_26_resultsScreen) {
        if (!x48_completionScreenStrings.IsLoaded() || !x50_largeFont.IsLoaded()) {
          return kMR_Exit;
        }
        const int width = CGraphics::GetViewport().mWidth;
        const int height = CGraphics::GetViewport().mHeight;
        const SObjectTag* font = gpResourceFactory->GetResourceIdByName(
            gpTweakGui->GetCompletionScreenTitleFont().data());
        x58_titleText = rs_new CGuiTextSupport(
            font->GetId(),
            CGuiTextProperties(false, true, kJustification_Right, kVerticalJustification_Bottom),
            gpTweakGui->x344_completionTitleColor, gpTweakGui->x348_completionTitleOutlineColor,
            CColor::White(), width, height, gpSimplePool);
        x58_titleText->SetTypeWriteEffectOptions(true, 1.f, 10.f);
        x58_titleText->SetText(x48_completionScreenStrings->GetString(0));
        font = gpResourceFactory->GetResourceIdByName(
            gpTweakGui->GetCompletionScreenBodyFont().data());
        x5c_resultsText = rs_new CGuiTextSupport(
            font->GetId(),
            CGuiTextProperties(false, true, kJustification_Right, kVerticalJustification_Top),
            gpTweakGui->x34c_completionBodyColor, gpTweakGui->x350_completionBodyOutlineColor,
            CColor::White(), width, height, gpSimplePool);
        x5c_resultsText->SetTypeWriteEffectOptions(true, 1.f, 15.f);
        const int percent = gpGameState->GetPlayerState()->CalculateItemCollectionRate() * 100 /
                            gpGameState->GetPlayerState()->GetTotalPickupCount();
        x5c_resultsText->AddText(x48_completionScreenStrings->GetString(1));
        x5c_resultsText->AddText(
            CStringExtras::ConvertToUNICODE(CBasics::Stringize(" %d%%\n", percent)));
        int minutes;
        int remainingMinutes;
        int hours;
        minutes = gpGameState->GetTotalPlayTime() / 60.0;
        hours = minutes / 60.f;
        remainingMinutes = minutes - hours * 60;
        x5c_resultsText->AddText(x48_completionScreenStrings->GetString(2));
        x5c_resultsText->AddText(CStringExtras::ConvertToUNICODE(
            CBasics::Stringize(" %02d:%02d\n", hours, remainingMinutes)));
        x60_unlockText = rs_new CGuiTextSupport(
            font->GetId(),
            CGuiTextProperties(false, true, kJustification_Right, kVerticalJustification_Top),
            gpTweakGui->x354_completionUnlockColor, gpTweakGui->x358_completionUnlockOutlineColor,
            CColor::White(), width, height, gpSimplePool);
        x60_unlockText->SetTypeWriteEffectOptions(true, 1.f, 15.f);
        if (!gpGameState->SystemState().GetNormalModeBeat()) {
          gpGameState->SystemState().SetHasHardMode(true);
          x60_unlockText->AddText(x48_completionScreenStrings->GetString(3));
        } else if (!gpGameState->SystemState().GetHardModeBeat() && gpGameState->GetHardMode()) {
          gpGameState->SystemState().SetHardModeBeat(true);
          x60_unlockText->AddText(x48_completionScreenStrings->GetString(4));
        }
        x64_continueText = rs_new CGuiTextSupport(
            font->GetId(),
            CGuiTextProperties(false, true, kJustification_Center, kVerticalJustification_Bottom),
            gpTweakGui->x354_completionUnlockColor, gpTweakGui->x358_completionUnlockOutlineColor,
            CColor::White(), width, height, gpSimplePool);
        x64_continueText->SetText(x48_completionScreenStrings->GetString(5));
      }
      x14_state = 1;
    }
    case 1:
      if (x78_26_resultsScreen) {
        bool loaded = true;
        for (int i = 0; i < x1c_movies.size(); ++i) {
          if (!x1c_movies[i]->PumpIndexLoad()) {
            x1c_movies[i]->Update(dt);
            if (!x1c_movies[i]->GetIsFullyCached()) {
              loaded = false;
            }
          } else {
            loaded = false;
          }
        }
        if (!loaded) {
          break;
        }
      }
      SetMovieIndex(0);
      x14_state = 2;
    case 2:
      if (x78_26_resultsScreen) {
        if (x44_audioPlayer.null()) {
          x44_audioPlayer =
              rs_new CStaticAudioPlayer(rstl::string_l(skCompletionAudio), 0x297988, 0x78cb60);
        }
        if (!x44_audioPlayer->IsReady()) {
          return kMR_Exit;
        }
        SetStaticAudioPlayerVolume(x44_audioPlayer, 0.91f);
      }
      x14_state = 3;
    case 3:
      if (x38_moviePlayer->PumpIndexLoad()) {
        break;
      }
      x38_moviePlayer->Update(dt);
      if (x78_26_resultsScreen) {
        UpdateText(dt);
        if (x38_moviePlayer->GetIsMovieFinishedPlaying()) {
          switch (x3c_movieIndex) {
          case 0:
            SetMovieIndex(1);
            break;
          case 1:
            SetMovieIndex(2);
            break;
          case 2:
            x78_24_finished = true;
            break;
          }
        }
        if (x3c_movieIndex == 2) {
          const float played = x38_moviePlayer->GetPlayedSeconds();
          const float remaining = x38_moviePlayer->GetTotalSeconds() - played;
          if (remaining <= 2.f) {
            SetStaticAudioPlayerVolume(x44_audioPlayer,
                                       0.91f * CMath::Clamp(0.f, remaining / 2.f, 1.f));
          }
        }
      } else {
        if (x18_which != kWM_LoseGame && x1c_movies.size() == 1 &&
            x38_moviePlayer->GetIsMovieFinishedPlaying()) {
          x78_24_finished = true;
        }
      }
      if (x78_27_quitScreen) {
        const EQuitAction action = x40_quitScreen->Update(dt);
        if (action == kQA_Yes) {
          gpMain->SetRestartMode(CMain::kRM_StateSetter);
          x78_24_finished = true;
        } else if (action == kQA_No) {
          x78_24_finished = true;
        }
      }
      if (x78_24_finished) {
        x78_25_exit = true;
      }
      if (x78_25_exit) {
        CFrameDelayedKiller::StallAndFlushAllAllocations();
        switch (x18_which) {
        case kWM_AfterCredits: {
          EWhichMovie next = kWM_WinGameBad;
          switch (gpMain->GetRestartMode()) {
          case CMain::kRM_WinBad:
            next = kWM_WinGameBad;
            break;
          case CMain::kRM_WinGood:
            next = kWM_WinGameGood;
            break;
          case CMain::kRM_WinBest:
            next = kWM_WinGameBest;
            break;
          }
          QueueIOWin(queue, rs_new CPlayMovie(next));
          break;
        }
        case kWM_WinGameBest:
          QueueIOWin(queue, rs_new CPlayMovie(kWM_SpecialEnding));
          break;
        case kWM_WinGameBad:
        case kWM_WinGameGood:
        case kWM_SpecialEnding:
          GXDrawDone();
          QueueAutoSave(queue);
          break;
        default:
          break;
        }
        return kMR_RemoveIOWinAndExit;
      }
      break;
    default:
      break;
    }
    break;
  }
  case kAM_UserInput: {
    const CArchMsgParmUserInput parm = MakeMsg::GetParmUserInput(msg);
    const CFinalInput input = parm.GetUserInput();
    if (x78_27_quitScreen) {
      x40_quitScreen->ProcessUserInput(input);
    } else {
      ProcessUserInput(input);
    }
    break;
  }
  default:
    break;
  }
  return kMR_Exit;
}

void CPlayMovie::Draw() const {
  if (x14_state == 3) {
    DrawVideo();
    if (x78_27_quitScreen) {
      x40_quitScreen->Draw();
    } else if (x78_26_resultsScreen) {
      DrawText();
    }
  }
}

void CPlayMovie::DrawVideo() const {
  if (x38_moviePlayer != nullptr) {
    x38_moviePlayer->DrawVideo();
  }
}

void CPlayMovie::DrawText() const {
  if (x3c_movieIndex == 2 && x38_moviePlayer->GetPlayedSeconds() > 1.6f) {
    return;
  }
  const int height = CGraphics::GetViewport().mHeight;
  const float y = 0.318f * height;
  if (!x58_titleText.null()) {
    const CVector3f position(-32.f, 0.f, height + y);
    CCredits::DrawText(*x58_titleText.get(), position);
  }
  if (!x5c_resultsText.null()) {
    const CVector3f position(-32.f, 0.f, y);
    CCredits::DrawText(*x5c_resultsText.get(), position);
  }
  if (!x60_unlockText.null()) {
    const rstl::pair< CVector2i, CVector2i >& bounds = x5c_resultsText->GetBounds();
    const int textHeight = bounds.second.GetY() - bounds.first.GetY() + 25;
    const CVector3f position(-32.f, 0.f, y - textHeight);
    CCredits::DrawText(*x60_unlockText.get(), position);
  }
  if (!x64_continueText.null() && x6c_resultsTime >= 2.f) {
    const CVector3f position(0.f, 0.f, height + 32);
    CCredits::DrawText(*x64_continueText.get(), position);
  }
}

void CPlayMovie::UpdateText(float dt) {
  if (x3c_movieIndex == 1) {
    x6c_resultsTime = rstl::min_val(x6c_resultsTime + dt, 2.5f);
  } else if (x3c_movieIndex == 2) {
    x6c_resultsTime = rstl::max_val(x6c_resultsTime - dt, 0.f);
  }
  x68_textDelay -= dt;
  if (x68_textDelay > 0.f) {
    return;
  }
  x70_pulseTime += dt;
  const float angle =
      M_PIF / 2.f * (2.f * (x70_pulseTime / gpTweakGui->GetCompletionScreenPulseTime()));
  if (!x58_titleText.null()) {
    x58_titleText->Update(dt);
  }
  if (!x5c_resultsText.null()) {
    x5c_resultsText->Update(dt);
    if (x5c_resultsText->GetNumCharactersPrinted() >= x74_printedCharacters + 0.3f) {
      x74_printedCharacters += 0.3f;
      CSfxManager::SfxStart(0x59e, 127, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    }
  }
  if (!x60_unlockText.null()) {
    const float alpha = 0.5f * CMath::FastSinR(angle) + 0.5f;
    const CColor fontColor =
        CColor::Lerp(gpTweakGui->x354_completionUnlockColor, CColor::Grey(), alpha);
    const CColor outlineColor =
        CColor::Lerp(gpTweakGui->x358_completionUnlockOutlineColor, CColor::Grey(), alpha);
    x60_unlockText->SetFontColor(fontColor);
    x60_unlockText->SetOutlineColor(outlineColor);
    x60_unlockText->Update(dt);
  }
  if (!x64_continueText.null() && x6c_resultsTime > 2.f) {
    x64_continueText->SetGeometryColor(
        CColor::White().WithAlphaModulatedBy((x6c_resultsTime - 2.f) / 0.5f));
    x64_continueText->Update(dt);
  }
}

CIOWin::EMessageReturn CPlayMovie::ProcessUserInput(const CFinalInput& input) {
  if (x3c_movieIndex == 1 && input.PA() && x6c_resultsTime >= 2.f) {
    x38_moviePlayer->DisableLoop();
    CSfxManager::SfxStart(0x58d, 127, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
  return kMR_Exit;
}

void CPlayMovie::SetMovieIndex(int index) {
  x3c_movieIndex = index;
  if (x3c_movieIndex != -1) {
    x38_moviePlayer = x1c_movies[x3c_movieIndex].get();
  } else {
    x38_moviePlayer = nullptr;
  }
}

CCredits::CCredits()
: CIOWin(rstl::string_l("Credits"))
, x14_state(0)
, x18_creditsTable(gpSimplePool->GetObj(gpTweakGui->GetCreditsTable().data()))
, x20_creditsFont(gpSimplePool->GetObj(gpTweakGui->GetJapaneseCreditsFont().data()))
, x48_scrollPosition(0.f)
, x4c_totalScrollDistance(0.f)
, x50_scrollSpeed(32.f)
, x54_textFadeRemaining(gpTweakGui->GetCredits_x30c())
, x58_videoFadeTime(0.f)
, x5c_24_finished(false)
, x5c_25_videoFaded(false)
, x5c_26_textFaded(false)
, x5c_27_fadingIn(true)
, x5c_28_fadingOut(false) {
  x18_creditsTable.Lock();
  x20_creditsFont.Lock();
}

CCredits::~CCredits() {}

CIOWin::EMessageReturn CCredits::OnMessage(const CArchitectureMessage& msg,
                                           CArchitectureQueue& queue) {
  switch (msg.GetType()) {
  case kAM_TimerTick:
    return Update(MakeMsg::GetParmTimerTick(msg).GetReal(), queue);
  case kAM_UserInput: {
    const CArchMsgParmUserInput parm = MakeMsg::GetParmUserInput(msg);
    const CFinalInput input = parm.GetUserInput();
    if (input.ControllerNumber() == 0) {
      return ProcessUserInput(input);
    }
    break;
  }
  default:
    break;
  }
  return kMR_Exit;
}

CIOWin::EMessageReturn CCredits::Update(float dt, CArchitectureQueue& queue) {
  switch (x14_state) {
  case 0: {
    if (!x18_creditsTable.IsLoaded() || !x20_creditsFont.IsLoaded()) {
      return kMR_Exit;
    }
    int height = CGraphics::GetViewport().mHeight;
    int width = CGraphics::GetViewport().mWidth - 64;
    if (x30_text.empty()) {
      const CStringTable& table = **x18_creditsTable;
      for (int i = 0; i < table.GetStringCount(); ++i) {
        rstl::ncrc_ptr< CGuiTextSupport > text = rs_new CGuiTextSupport(
            gpResourceFactory->GetResourceIdByName(gpTweakGui->GetCreditsFont().data())->GetId(),
            CGuiTextProperties(true, true, kJustification_Center, kVerticalJustification_Top),
            gpTweakGui->x300_creditsTextFontColor, gpTweakGui->x304_creditsTextBorderColor,
            CColor::White(), width, 0, gpSimplePool);
        text->SetText(rstl::wstring_l(table.GetString(i)));
        x30_text.push_back(
            rstl::pair< rstl::ncrc_ptr< CGuiTextSupport >, CVector2i >(text, CVector2i()));
      }
    }
    for (AUTO(it, x30_text.begin()); it != x30_text.end(); ++it) {
      if (!it->first->GetIsTextSupportFinishedLoading()) {
        return kMR_Exit;
      }
    }
    int totalHeight = 0;
    for (AUTO(it, x30_text.begin()); it != x30_text.end(); ++it) {
      const rstl::pair< CVector2i, CVector2i >& bounds = it->first->GetBounds();
      const int textHeight = bounds.second.GetY() - bounds.first.GetY();
      it->second.SetY(textHeight);
      it->second.SetX(totalHeight);
      it->first->SetExtentX(width);
      it->first->SetExtentY(textHeight);
      totalHeight += textHeight;
    }
    const float halfHeight = height / 2.f;
    x4c_totalScrollDistance = totalHeight + halfHeight;
    x50_scrollSpeed = x4c_totalScrollDistance /
                      (gpTweakGui->GetCredits_x308() -
                       rstl::max_val(gpTweakGui->GetCredits_x310(), gpTweakGui->GetCredits_x30c()));
    x14_state = 1;
  }
  case 1: {
    if (x28_moviePlayer.null()) {
      x28_moviePlayer =
          rs_new CMoviePlayer(skMovieNames[CPlayMovie::kWM_CreditBG], 0.f, true, true);
    }
    x14_state = 2;
  }
  case 2: {
    if (x2c_audioPlayer.null()) {
      x2c_audioPlayer = rs_new CStaticAudioPlayer(rstl::string_l(skCreditsAudio), 0, 0x5d7c00);
    }
    if (!x2c_audioPlayer->IsReady()) {
      return kMR_Exit;
    }
    SetStaticAudioPlayerVolume(x2c_audioPlayer, 1.1053f);
    x14_state = 3;
  }
  case 3: {
    if (x28_moviePlayer->PumpIndexLoad()) {
      break;
    }
    x28_moviePlayer->Update(dt);
    if (x5c_24_finished) {
      x5c_28_fadingOut = true;
      if (x5c_27_fadingIn) {
        x5c_27_fadingIn = false;
        x58_videoFadeTime = gpTweakGui->GetCredits_x310() - x58_videoFadeTime;
      }
    }
    if (x5c_27_fadingIn || x5c_28_fadingOut) {
      x58_videoFadeTime = CMath::Clamp(0.f, x58_videoFadeTime + dt, gpTweakGui->GetCredits_x310());
      if (x58_videoFadeTime == gpTweakGui->GetCredits_x310()) {
        if (x5c_27_fadingIn) {
          x5c_27_fadingIn = false;
          x58_videoFadeTime = 0.f;
        } else if (x5c_28_fadingOut) {
          x5c_25_videoFaded = true;
        }
      }

      if (x58_videoFadeTime != 0.f && x5c_28_fadingOut) {
        float volume = 1.f - x58_videoFadeTime / gpTweakGui->GetCredits_x310();
        volume = CMath::Clamp(0.f, volume, 1.f);
        SetStaticAudioPlayerVolume(x2c_audioPlayer, 1.1053f * volume);
      }
    }
    x48_scrollPosition =
        rstl::min_val(x4c_totalScrollDistance, (dt * x50_scrollSpeed) + x48_scrollPosition);

    if (x48_scrollPosition == x4c_totalScrollDistance || x5c_24_finished) {
      x5c_24_finished = true;
      x54_textFadeRemaining = rstl::max_val(0.f, x54_textFadeRemaining - dt);
      const float alpha = x54_textFadeRemaining / gpTweakGui->GetCredits_x30c();
      for (AUTO(it, x30_text.begin()); it != x30_text.end(); ++it) {
        it->first->SetGeometryColor(CColor::White().WithAlphaModulatedBy(alpha));
      }
      if (x54_textFadeRemaining <= 0.f) {
        x5c_26_textFaded = true;
      }
    }

    if (x5c_26_textFaded && x5c_25_videoFaded) {
      QueueIOWin(queue, rs_new CPlayMovie(CPlayMovie::kWM_AfterCredits));
      return kMR_RemoveIOWinAndExit;
    }
    break;
  }
  default:
    break;
  }
  return kMR_Exit;
}

CIOWin::EMessageReturn CCredits::ProcessUserInput(const CFinalInput& input) {
  if (input.DA()) {
    x48_scrollPosition = CMath::Clamp(0.f, x48_scrollPosition - ((x50_scrollSpeed * input.Time())),
                                      x4c_totalScrollDistance);
  } else {
    const float offset = input.ALAUp() - input.ALADown();
    x48_scrollPosition =
        CMath::Clamp(0.f, x48_scrollPosition - offset * (10.f * (x50_scrollSpeed * input.Time())),
                     x4c_totalScrollDistance);
  }
  return kMR_Exit;
}

void CCredits::Draw() const {
  if (x14_state != 3) {
    return;
  }
  DrawVideo();
  DrawText();
}

void CCredits::DrawText() const {
  const rstl::pair< CVector2f, CVector2f > region =
      gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  const float bottom = x48_scrollPosition - (region.second.GetY() - region.first.GetY());
  const float top = x48_scrollPosition;
  const float width = region.second.GetX();
  for (AUTO(it, x30_text.begin()); it != x30_text.end(); ++it) {
    const int offset = it->second.GetX();
    const int height = it->second.GetY();
    const int end = height + offset;
    if (!(bottom > end) && !(top < offset)) {
      DrawText(*it->first, CVector3f(0.5f * (width - it->first->GetTextBoundingWidth()), 0.f,
                                     x48_scrollPosition - it->second.GetX()));
    }
  }
  CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_CinemaBars,
                                CColor::Black(), nullptr, 1.f);
}

void CCredits::DrawVideo() const {
  /* Render movie */
  if (x28_moviePlayer.get() && x28_moviePlayer->DrawVideo() &&
      (x5c_27_fadingIn || x5c_28_fadingOut)) {
    float alpha = x58_videoFadeTime / gpTweakGui->GetCredits_x310();
    if (x5c_27_fadingIn) {
      alpha = 1.f - alpha;
    }

    alpha = CMath::Clamp(0.f, alpha, 1.f);
    CColor filterCol = CColor::Black().WithAlphaOf(alpha);
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                  filterCol, nullptr, 1.f);
  }
}
CAutoSave::CAutoSave()
: CIOWin(rstl::string_l("AutoSave"))
, x14_saveGameScreen(rs_new CSaveGameScreen(kSC_InGame, gpGameState->GetCardSerial())) {
  gpMain->RefreshGameState();
}

CAutoSave::~CAutoSave() {}

CIOWin::EMessageReturn CAutoSave::OnMessage(const CArchitectureMessage& msg,
                                            CArchitectureQueue& queue) {
  if (gpGameState->GetCardSerial() == 0) {
    return kMR_RemoveIOWinAndExit;
  }
  switch (msg.GetType()) {
  case kAM_TimerTick:
    if (x14_saveGameScreen->Update(MakeMsg::GetParmTimerTick(msg).GetReal()) != 0) {
      return kMR_RemoveIOWinAndExit;
    }
    break;
  case kAM_UserInput: {
    const CArchMsgParmUserInput parm = MakeMsg::GetParmUserInput(msg);
    const CFinalInput input = parm.GetUserInput();
    x14_saveGameScreen->ProcessUserInput(input);
    break;
  }
  default:
    break;
  }
  return kMR_Exit;
}

void CAutoSave::Draw() const { x14_saveGameScreen->Draw(); }

bool CAutoSave::GetIsContinueDraw() const { return false; }
