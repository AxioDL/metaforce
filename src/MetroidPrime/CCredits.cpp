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

#if defined(TARGET_PC)
#include "Metaforce/Display.hpp"
#endif

#if VERSION == VERSION_GM8E_02
#pragma inline_max_size(250)
#endif

#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_R3ME_00
static const char* const skJapaneseBodyFont = "FONT_DFSoGei-W5_18";
#endif
static const char* const skMovieNames[] = {"Video/wingame.thp",
                                           "Video/wingame_best.thp",
                                           "Video/wingame_best.thp",
#if VERSION >= VERSION_GM8P_00
                                           "",
#else
                                           "Video/losegame.thp",
#endif
                                           "Video/05_tallonText.thp",
                                           "Video/AfterCredits.thp",
                                           "Video/SpecialEnding.thp",
                                           "Video/creditBG.thp"};
static const char* const skCompletionMovieNames[] = {
    "Video/win_bad_begin.thp",  "Video/win_bad_loop.thp",  "Video/win_bad_end.thp",
    "Video/win_good_begin.thp", "Video/win_good_loop.thp", "Video/win_good_end.thp"};
#if VERSION >= VERSION_GM8P_00
static const char* const skLoseGameMovieNames[] = {
    "Video/losegame.thp",         "Video/losegame_german.thp",  "Video/losegame_french.thp",
    "Video/losegame_spanish.thp", "Video/losegame_italian.thp", "",
    "Video/losegame.thp"};
#endif
static const char* const skCreditsAudio = "Audio/ending3.rsf";
static const char* const skCompletionAudio = "Audio/samusjak.rsf";
#if VERSION == VERSION_GM8J_00
static const CColor skPulseStartColor(1.f, 1.f, 0.392156869f, 1.f);
static const CColor skPulseEndColor(0.588235319f, 0.588235319f, 0.f, 1.f);
#elif VERSION >= VERSION_GM8P_00
static const CColor skPulseStartColor(1.f, 1.f, 1.f, 1.f);
static const CColor skPulseEndColor(0.5f, 0.5f, 0.5f, 1.f);
#endif
#if VERSION == VERSION_GM8J_00
static const CVector3f skTextOffset0(0.208333001f, 0.f, 0.f);
static const CVector3f skTextOffset1(-3.0277791f, 0.f, 1.91666698f);
static const CVector3f skTextOffset2(2.38898611f, 0.f, 11.0136547f);
static const CVector3f skTextOffset3(6.01348209f, 0.f, -3.6947031f);
#elif VERSION >= VERSION_GM8P_00
static const CVector3f skTextOffset0 = CVector3f::Zero();
static const CVector3f skTextOffset1 = CVector3f::Zero();
static const CVector3f skTextOffset2 = CVector3f::Zero();
static const CVector3f skTextOffset3 = CVector3f::Zero();
#endif

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
#if VERSION >= VERSION_GM8P_00
  const int movieWidth = GetWidth();
  const int movieHeight = GetHeight();
#else
  const uint movieWidth = GetWidth();
  const uint movieHeight = GetHeight();
#endif
  const int xMargin = (movieWidth - width) / 2;
  const int yMargin = (movieHeight - height) / 2;
  const int xmin = left - xMargin;
  const int xmax = left + width + xMargin;
  const int ymin = top - yMargin;
  const int ymax = top + height + yMargin;
#if VERSION >= VERSION_GM8P_00
  const_cast< CMoviePlayer* >(this)->DrawFrame(xmin, xmax, ymin, ymax);
#else
  const CVector3f topLeft(xmin, 0.f, ymax);
  const CVector3f topRight(xmax, 0.f, ymax);
  const CVector3f bottomLeft(xmin, 0.f, ymin);
  const CVector3f bottomRight(xmax, 0.f, ymin);
  const_cast< CMoviePlayer* >(this)->DrawFrame(topLeft, topRight, bottomLeft, bottomRight);
#endif
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

#if VERSION >= VERSION_GM8P_00
void CCredits::DrawText(CGuiTextSupport& text, const CTransform4f& transform) {
#else
void CCredits::DrawText(CGuiTextSupport& text, const CVector3f& translation) {
#endif
  CGraphics::SetCullMode(kCM_None);
  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
#if defined(TARGET_PC)
  metaforce::AdjustUiProjection();
#endif
#if VERSION >= VERSION_GM8P_00
  gpRender->SetModelMatrix(transform);
#else
  gpRender->SetModelMatrix(CTransform4f::Translate(translation));
#endif
#if VERSION == VERSION_GM8J_00
  CGraphics::SetDepthWriteMode(true, kE_Less, true);
#else
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetBlendMode_AdditiveAlpha();
#endif
  text.Render();
}

static void QueueIOWin(CArchitectureQueue& queue, CIOWin* win) {
  queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, 12, 11, win));
}

static void QueueAutoSave(CArchitectureQueue& queue) { QueueIOWin(queue, rs_new CAutoSave()); }

#if VERSION >= VERSION_GM8P_00
static const char* GetMovieName(int movie) {
  if (movie == CPlayMovie::kWM_LoseGame) {
    const int language = gpGameState->SystemState().GetLanguage();
    return skLoseGameMovieNames[language];
  }
  return skMovieNames[movie];
}
#endif

CPlayMovie::CPlayMovie(EWhichMovie movie)
: CIOWin(rstl::string_l("PlayMovie"))
, mState(0)
, mWhich(movie)
, mMoviePlayer(nullptr)
, mMovieIndex(-1)
, mCompletionScreenStrings(static_cast< CStringTable* >(nullptr))
, mLargeFont(static_cast< CRasterFont* >(nullptr))
, mTextDelay(gpTweakGui->GetCompletionScreenTextDelay())
, mResultsTime(0.f)
, mPulseTime(gpTweakGui->GetCompletionScreenPulseTime())
, mPrintedCharacters(0.f)
#if VERSION >= VERSION_GM8P_00
, mPulseStartColor(skPulseStartColor)
, mPulseEndColor(skPulseEndColor)
#endif
, mFinished(false)
, mExit(false)
, mResultsScreen(false)
, mQuitScreenFlag(false) {
  if (IsGameWon(mWhich)) {
    mResultsScreen = true;
  } else if (movie == kWM_LoseGame) {
    mQuitScreen = rs_new CQuitGameScreen(kQT_ContinueFromLastSave);
    mQuitScreenFlag = true;
  }
  CGraphics::SetIsBeginSceneClearFb(true);
  if (mResultsScreen) {
    mCompletionScreenStrings =
        gpSimplePool->GetObj(gpTweakGui->GetCompletionScreenTable().data());
    mCompletionScreenStrings.Lock();
    mLargeFont = gpSimplePool->GetObj(gpTweakGui->GetCompletionScreenTitleFont().data());
    mLargeFont.Lock();
    int start = 0;
    switch (mWhich) {
#if VERSION == VERSION_GM8E_48 || (VERSION >= VERSION_GM8P_00)
    case kWM_WinGameBad:
      start = 0;
      break;
    case kWM_WinGameGood:
    case kWM_WinGameBest:
      start = 3;
      break;
#else
    case kWM_WinGameBad:
    case kWM_WinGameGood:
      start = 0;
      break;
    case kWM_WinGameBest:
      start = 3;
      break;
#endif
    default:
      break;
    }
    const int end = start + 3;
    for (int i = start; i < end; ++i) {
      const bool loop = i % 3 == 1;
      mMovies.push_back(rstl::auto_ptr< CMoviePlayer >(
          rs_new CMoviePlayer(skCompletionMovieNames[i], 0.05f, loop, false)));
    }
  } else {
#if VERSION >= VERSION_GM8P_00
    const char* movieName = GetMovieName(movie);
    mMovies.push_back(rstl::auto_ptr< CMoviePlayer >(
        rs_new CMoviePlayer(movieName, 0.f, false, movie == kWM_LoseGame)));
#else
    mMovies.push_back(rstl::auto_ptr< CMoviePlayer >(
        rs_new CMoviePlayer(skMovieNames[movie], 0.f, false, movie == kWM_LoseGame)));
#endif
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
    switch (mState) {
    case 0: {
      if (mResultsScreen) {
        if (!mCompletionScreenStrings.IsLoaded() || !mLargeFont.IsLoaded()) {
          return kMR_Exit;
        }
        const int width = CGraphics::GetViewport().mWidth;
        const int height = CGraphics::GetViewport().mHeight;
        const SObjectTag* font = gpResourceFactory->GetResourceIdByName(
            gpTweakGui->GetCompletionScreenTitleFont().data());
        mTitleText = rs_new CGuiTextSupport(
            font->GetId(),
#if VERSION >= VERSION_GM8P_00
            width, height,
#endif
            CGuiTextProperties(false, true, kJustification_Right, kVerticalJustification_Bottom),
            gpTweakGui->mCompletionTitleColor, gpTweakGui->mCompletionTitleOutlineColor,
            CColor::White(),
#if VERSION < VERSION_GM8P_00
            width, height,
#endif
            gpSimplePool);
        mTitleText->SetTypeWriteEffectOptions(true, 1.f, 10.f);
        mTitleText->SetText(mCompletionScreenStrings->GetString(0));
#if VERSION == VERSION_GM8J_00
        font = gpResourceFactory->GetResourceIdByName(skJapaneseBodyFont);
#else
        font = gpResourceFactory->GetResourceIdByName(
            gpTweakGui->GetCompletionScreenBodyFont().data());
#endif
        mResultsText = rs_new CGuiTextSupport(
            font->GetId(),
#if VERSION >= VERSION_GM8P_00
            width, height,
#endif
            CGuiTextProperties(false, true, kJustification_Right, kVerticalJustification_Top),
            gpTweakGui->mCompletionBodyColor, gpTweakGui->mCompletionBodyOutlineColor,
            CColor::White(),
#if VERSION < VERSION_GM8P_00
            width, height,
#endif
            gpSimplePool);
        mResultsText->SetTypeWriteEffectOptions(true, 1.f, 15.f);
#if VERSION >= VERSION_GM8P_00
        const int percent = gpGameState->GetPlayerState()->CalculateItemCollectionPercentage();
#else
        const int percent = gpGameState->GetPlayerState()->CalculateItemCollectionRate() * 100 /
                            gpGameState->GetPlayerState()->GetTotalPickupCount();
#endif
        mResultsText->AddText(mCompletionScreenStrings->GetString(1));
        mResultsText->AddText(
            CStringExtras::ConvertToUNICODE(CBasics::Stringize(" %d%%\n", percent)));
        int minutes;
        int remainingMinutes;
        int hours;
        minutes = gpGameState->GetTotalPlayTime() / 60.0;
        hours = minutes / 60.f;
        remainingMinutes = minutes - hours * 60;
        mResultsText->AddText(mCompletionScreenStrings->GetString(2));
        mResultsText->AddText(CStringExtras::ConvertToUNICODE(
            CBasics::Stringize(" %02d:%02d\n", hours, remainingMinutes)));
#if VERSION == VERSION_GM8P_00
        mPulseStartColor = gpTweakGui->mCompletionUnlockColor;
        mPulseEndColor = skPulseEndColor;
#endif
        mUnlockText = rs_new CGuiTextSupport(
            font->GetId(),
#if VERSION >= VERSION_GM8P_00
            width, height,
#endif
            CGuiTextProperties(false, true, kJustification_Right, kVerticalJustification_Top),
#if VERSION >= VERSION_GM8P_00
            mPulseStartColor,
#else
            gpTweakGui->mCompletionUnlockColor,
#endif
            gpTweakGui->mCompletionUnlockOutlineColor, CColor::White(),
#if VERSION < VERSION_GM8P_00
            width, height,
#endif
            gpSimplePool);
        mUnlockText->SetTypeWriteEffectOptions(true, 1.f, 15.f);
        if (!gpGameState->SystemState().GetNormalModeBeat()) {
          gpGameState->SystemState().SetHasHardMode(true);
          mUnlockText->AddText(mCompletionScreenStrings->GetString(3));
        } else if (!gpGameState->SystemState().GetHardModeBeat() && gpGameState->GetHardMode()) {
          gpGameState->SystemState().SetHardModeBeat(true);
          mUnlockText->AddText(mCompletionScreenStrings->GetString(4));
        }
        mContinueText = rs_new CGuiTextSupport(
            font->GetId(),
#if VERSION >= VERSION_GM8P_00
            width, height,
#endif
            CGuiTextProperties(false, true, kJustification_Center, kVerticalJustification_Bottom),
            gpTweakGui->mCompletionUnlockColor, gpTweakGui->mCompletionUnlockOutlineColor,
            CColor::White(),
#if VERSION < VERSION_GM8P_00
            width, height,
#endif
            gpSimplePool);
        mContinueText->SetText(mCompletionScreenStrings->GetString(5));
      }
      mState = 1;
    }
    case 1:
      if (mResultsScreen) {
        bool loaded = true;
        for (int i = 0; i < mMovies.size(); ++i) {
          if (!mMovies[i]->PumpIndexLoad()) {
            mMovies[i]->Update(dt);
            if (!mMovies[i]->GetIsFullyCached()) {
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
      mState = 2;
    case 2:
      if (mResultsScreen) {
        if (mAudioPlayer.null()) {
          mAudioPlayer =
              rs_new CStaticAudioPlayer(rstl::string_l(skCompletionAudio), 0x297988, 0x78cb60);
        }
        if (!mAudioPlayer->IsReady()) {
          return kMR_Exit;
        }
        SetStaticAudioPlayerVolume(mAudioPlayer, 0.91f);
      }
      mState = 3;
    case 3:
      if (mMoviePlayer->PumpIndexLoad()) {
        break;
      }
      mMoviePlayer->Update(dt);
      if (mResultsScreen) {
        UpdateText(dt);
        if (mMoviePlayer->GetIsMovieFinishedPlaying()) {
          switch (mMovieIndex) {
          case 0:
            SetMovieIndex(1);
            break;
          case 1:
            SetMovieIndex(2);
            break;
          case 2:
            mFinished = true;
            break;
          }
        }
        if (mMovieIndex == 2) {
          const float played = mMoviePlayer->GetPlayedSeconds();
          const float remaining = mMoviePlayer->GetTotalSeconds() - played;
          if (remaining <= 2.f) {
            SetStaticAudioPlayerVolume(mAudioPlayer,
                                       0.91f * CMath::Clamp(0.f, remaining / 2.f, 1.f));
          }
        }
      } else {
        if (mWhich != kWM_LoseGame && mMovies.size() == 1 &&
            mMoviePlayer->GetIsMovieFinishedPlaying()) {
          mFinished = true;
        }
      }
      if (mQuitScreenFlag) {
        const EQuitAction action = mQuitScreen->Update(dt);
        if (action == kQA_Yes) {
          gpMain->SetRestartMode(CMain::kRM_StateSetter);
          mFinished = true;
        } else if (action == kQA_No) {
          mFinished = true;
        }
      }
      if (mFinished) {
        mExit = true;
      }
      if (mExit) {
        CFrameDelayedKiller::StallAndFlushAllAllocations();
        switch (mWhich) {
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
    if (mQuitScreenFlag) {
      mQuitScreen->ProcessUserInput(input);
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
  if (mState == 3) {
    DrawVideo();
    if (mQuitScreenFlag) {
      mQuitScreen->Draw();
    } else if (mResultsScreen) {
      DrawText();
    }
  }
}

void CPlayMovie::DrawVideo() const {
  if (mMoviePlayer != nullptr) {
    mMoviePlayer->DrawVideo();
  }
}

void CPlayMovie::DrawText() const {
#if VERSION == VERSION_GM8J_00
  if (mMovieIndex == 2 && mMoviePlayer->GetPlayedSeconds() > 0.65f) {
    return;
  }
  gpRender->SetBlendMode_AdditiveAlpha();
  const int width = CGraphics::GetViewport().mWidth;
  const int height = CGraphics::GetViewport().mHeight;
  const float y = 0.318f * height;
  if (!mTitleText.null()) {
    CTransform4f transform = CTransform4f::Scale(1.00887001f, 1.f, 1.00887001f);
    transform.SetTranslation(
        CVector3f(-32.f + -0.00887000561f * width, 0.f, height + y - -0.00887000561f * height) +
        skTextOffset0);
    CCredits::DrawText(*mTitleText, transform);
  }
  if (!mResultsText.null()) {
    CTransform4f transform = CTransform4f::Scale(0.875819981f, 1.f, 0.875819981f);
    transform.SetTranslation(CVector3f(-32.f + 0.124180019f * width, 0.f, y) + skTextOffset1);
    CCredits::DrawText(*mResultsText, transform);
  }
  if (!mUnlockText.null()) {
    const rstl::pair< CVector2i, CVector2i >& bounds = mResultsText->GetBounds();
    const int textHeight = bounds.second.GetY() - bounds.first.GetY() + 25;
    CTransform4f transform = CTransform4f::Scale(0.935598016f, 1.f, 0.935598016f);
    transform.SetTranslation(
        CVector3f(-32.f + 0.0644019842f * width, 0.f, y - 0.875819981f * textHeight) +
        skTextOffset2);
    CCredits::DrawText(*mUnlockText, transform);
  }
  if (!mContinueText.null() && mResultsTime >= 2.f) {
    CTransform4f transform = CTransform4f::Scale(0.869831026f, 1.f, 0.869831026f);
    transform.SetTranslation(
        CVector3f((0.130168974f * width) / 2.f, 0.f, height + 32 - 0.130168974f * height) +
        skTextOffset3);
    CCredits::DrawText(*mContinueText, transform);
  }
#else
  if (mMovieIndex == 2 && mMoviePlayer->GetPlayedSeconds() >
#if VERSION == VERSION_GM8P_00
                                 0.65f
#else
                                 1.6f
#endif
  ) {
    return;
  }
  const int height = CGraphics::GetViewport().mHeight;
  const float y = 0.318f * height;
  if (!mTitleText.null()) {
#if VERSION >= VERSION_GM8P_00
    CTransform4f transform = CTransform4f::Scale(1.f, 1.f, 1.f);
    transform.SetTranslation(CVector3f(-32.f, 0.f, height + y) + skTextOffset0);
    CCredits::DrawText(*mTitleText, transform);
#else
    const CVector3f position(-32.f, 0.f, height + y);
    CCredits::DrawText(*mTitleText, position);
#endif
  }
  if (!mResultsText.null()) {
#if VERSION >= VERSION_GM8P_00
    CTransform4f transform = CTransform4f::Scale(1.f, 1.f, 1.f);
    transform.SetTranslation(CVector3f(-32.f, 0.f, y) + skTextOffset1);
    CCredits::DrawText(*mResultsText, transform);
#else
    const CVector3f position(-32.f, 0.f, y);
    CCredits::DrawText(*mResultsText, position);
#endif
  }
  if (!mUnlockText.null()) {
    const rstl::pair< CVector2i, CVector2i >& bounds = mResultsText->GetBounds();
    const int textHeight = bounds.second.GetY() - bounds.first.GetY() + 25;
#if VERSION >= VERSION_GM8P_00
    CTransform4f transform = CTransform4f::Scale(1.f, 1.f, 1.f);
    transform.SetTranslation(CVector3f(-32.f, 0.f, y - textHeight) + skTextOffset2);
    CCredits::DrawText(*mUnlockText, transform);
#else
    const CVector3f position(-32.f, 0.f, y - textHeight);
    CCredits::DrawText(*mUnlockText, position);
#endif
  }
  if (!mContinueText.null() && mResultsTime >= 2.f) {
#if VERSION >= VERSION_GM8P_00
    CTransform4f transform = CTransform4f::Scale(1.f, 1.f, 1.f);
    transform.SetTranslation(CVector3f(0.f, 0.f, height + 32) + skTextOffset3);
    CCredits::DrawText(*mContinueText, transform);
#else
    const CVector3f position(0.f, 0.f, height + 32);
    CCredits::DrawText(*mContinueText, position);
#endif
  }
#endif
}

void CPlayMovie::UpdateText(float dt) {
  if (mMovieIndex == 1) {
    mResultsTime = rstl::min_val(mResultsTime + dt, 2.5f);
  } else if (mMovieIndex == 2) {
    mResultsTime = rstl::max_val(mResultsTime - dt, 0.f);
  }
  mTextDelay -= dt;
  if (mTextDelay > 0.f) {
    return;
  }
  mPulseTime += dt;
  const float angle =
      M_PIF / 2.f * (2.f * (mPulseTime / gpTweakGui->GetCompletionScreenPulseTime()));
  if (!mTitleText.null()) {
    mTitleText->Update(dt);
  }
  if (!mResultsText.null()) {
    mResultsText->Update(dt);
#if VERSION == VERSION_GM8P_00
    const float printStep = 0.9f;
#else
    const float printStep = 0.3f;
#endif
    if (mResultsText->GetNumCharactersPrinted() >= mPrintedCharacters + printStep) {
      mPrintedCharacters += printStep;
      CSfxManager::SfxStart(0x59e, 127, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    }
  }
  if (!mUnlockText.null()) {
    const float alpha = 0.5f * CMath::FastSinR(angle) + 0.5f;
#if VERSION >= VERSION_GM8P_00
    const CColor fontColor = CColor::Lerp(mPulseStartColor, mPulseEndColor, alpha);
#else
    const CColor fontColor =
        CColor::Lerp(gpTweakGui->mCompletionUnlockColor, CColor::Grey(), alpha);
#endif
    const CColor outlineColor =
        CColor::Lerp(gpTweakGui->mCompletionUnlockOutlineColor, CColor::Grey(), alpha);
    mUnlockText->SetFontColor(fontColor);
    mUnlockText->SetOutlineColor(outlineColor);
    mUnlockText->Update(dt);
  }
  if (!mContinueText.null() && mResultsTime > 2.f) {
    mContinueText->SetGeometryColor(
        CColor::White().WithAlphaModulatedBy((mResultsTime - 2.f) / 0.5f));
    mContinueText->Update(dt);
  }
}

CIOWin::EMessageReturn CPlayMovie::ProcessUserInput(const CFinalInput& input) {
  if (mMovieIndex == 1 && input.PA() && mResultsTime >= 2.f) {
    mMoviePlayer->DisableLoop();
    CSfxManager::SfxStart(0x58d, 127, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
  return kMR_Exit;
}

void CPlayMovie::SetMovieIndex(int index) {
  mMovieIndex = index;
  if (mMovieIndex != -1) {
    mMoviePlayer = mMovies[mMovieIndex].get();
  } else {
    mMoviePlayer = nullptr;
  }
}

CCredits::CCredits()
: CIOWin(rstl::string_l("Credits"))
, mState(0)
, mCreditsTable(gpSimplePool->GetObj(gpTweakGui->GetCreditsTable().data()))
#if VERSION < VERSION_GM8P_00
, mCreditsFont(gpSimplePool->GetObj(gpTweakGui->GetJapaneseCreditsFont().data()))
#endif
, mScrollPosition(0.f)
, mTotalScrollDistance(0.f)
, mScrollSpeed(32.f)
, mTextFadeRemaining(gpTweakGui->GetCredits_x30c())
, mVideoFadeTime(0.f)
, mFinished(false)
, mVideoFaded(false)
, mTextFaded(false)
, mFadingIn(true)
, mFadingOut(false) {
#if VERSION == VERSION_GM8J_00
  CGraphics::SetIsBeginSceneClearFb(true);
#endif
  mCreditsTable.Lock();
#if VERSION < VERSION_GM8P_00
  mCreditsFont.Lock();
#endif
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
#if VERSION >= VERSION_GM8P_00
    return ProcessUserInput(input);
#else
    if (input.ControllerNumber() == 0) {
      return ProcessUserInput(input);
    }
#endif
    break;
  }
  default:
    break;
  }
  return kMR_Exit;
}

CIOWin::EMessageReturn CCredits::Update(float dt, CArchitectureQueue& queue) {
  switch (mState) {
  case 0: {
    if (!mCreditsTable.IsLoaded()
#if VERSION < VERSION_GM8P_00
        || !mCreditsFont.IsLoaded()
#endif
    ) {
      return kMR_Exit;
    }
    int height = CGraphics::GetViewport().mHeight;
    int width = CGraphics::GetViewport().mWidth - 64;
    if (mText.empty()) {
      const CStringTable& table = **mCreditsTable;
      for (int i = 0; i < table.GetStringCount(); ++i) {
        rstl::ncrc_ptr< CGuiTextSupport > text = rs_new CGuiTextSupport(
            gpResourceFactory->GetResourceIdByName(gpTweakGui->GetCreditsFont().data())->GetId(),
#if VERSION >= VERSION_GM8P_00
            width, 0,
#endif
            CGuiTextProperties(true, true, kJustification_Center, kVerticalJustification_Top),
            gpTweakGui->mCreditsTextFontColor, gpTweakGui->mCreditsTextBorderColor,
            CColor::White(),
#if VERSION < VERSION_GM8P_00
            width, 0,
#endif
            gpSimplePool);
        text->SetText(rstl::wstring_l(table.GetString(i)));
        mText.push_back(
            rstl::pair< rstl::ncrc_ptr< CGuiTextSupport >, CVector2i >(text, CVector2i()));
      }
    }
    for (AUTO(it, mText.begin()); it != mText.end(); ++it) {
      if (!it->first->GetIsTextSupportFinishedLoading()) {
        return kMR_Exit;
      }
    }
    int totalHeight = 0;
    for (AUTO(it, mText.begin()); it != mText.end(); ++it) {
      const rstl::pair< CVector2i, CVector2i >& bounds = it->first->GetBounds();
      const int textHeight = bounds.second.GetY() - bounds.first.GetY();
      it->second.SetY(textHeight);
      it->second.SetX(totalHeight);
      it->first->SetExtentX(width);
      it->first->SetExtentY(textHeight);
      totalHeight += textHeight;
    }
    const float halfHeight = height / 2.f;
#if VERSION == VERSION_GM8J_00
    mTotalScrollDistance = 0.825084984f * totalHeight + halfHeight;
#else
    mTotalScrollDistance = totalHeight + halfHeight;
#endif
    mScrollSpeed = mTotalScrollDistance /
                      (gpTweakGui->GetCredits_x308() -
                       rstl::max_val(gpTweakGui->GetCredits_x310(), gpTweakGui->GetCredits_x30c()));
    mState = 1;
  }
  case 1: {
    if (mMoviePlayer.null()) {
      mMoviePlayer =
          rs_new CMoviePlayer(skMovieNames[CPlayMovie::kWM_CreditBG], 0.f, true, true);
    }
    mState = 2;
  }
  case 2: {
    if (mAudioPlayer.null()) {
      mAudioPlayer = rs_new CStaticAudioPlayer(rstl::string_l(skCreditsAudio), 0, 0x5d7c00);
    }
    if (!mAudioPlayer->IsReady()) {
      return kMR_Exit;
    }
    SetStaticAudioPlayerVolume(mAudioPlayer, 1.1053f);
    mState = 3;
  }
  case 3: {
    if (mMoviePlayer->PumpIndexLoad()) {
      break;
    }
    mMoviePlayer->Update(dt);
    if (mFinished) {
      mFadingOut = true;
      if (mFadingIn) {
        mFadingIn = false;
        mVideoFadeTime = gpTweakGui->GetCredits_x310() - mVideoFadeTime;
      }
    }
    if (mFadingIn || mFadingOut) {
      mVideoFadeTime = CMath::Clamp(0.f, mVideoFadeTime + dt, gpTweakGui->GetCredits_x310());
      if (mVideoFadeTime == gpTweakGui->GetCredits_x310()) {
        if (mFadingIn) {
          mFadingIn = false;
          mVideoFadeTime = 0.f;
        } else if (mFadingOut) {
          mVideoFaded = true;
        }
      }

      if (mVideoFadeTime != 0.f && mFadingOut) {
        float volume = 1.f - mVideoFadeTime / gpTweakGui->GetCredits_x310();
        volume = CMath::Clamp(0.f, volume, 1.f);
        SetStaticAudioPlayerVolume(mAudioPlayer, 1.1053f * volume);
      }
    }
    mScrollPosition =
        rstl::min_val(mTotalScrollDistance, (dt * mScrollSpeed) + mScrollPosition);

#if VERSION >= VERSION_GM8P_00
    if (mScrollPosition >= mTotalScrollDistance || mFinished) {
#else
    if (mScrollPosition == mTotalScrollDistance || mFinished) {
#endif
      mFinished = true;
      mTextFadeRemaining = rstl::max_val(0.f, mTextFadeRemaining - dt);
      const float alpha = mTextFadeRemaining / gpTweakGui->GetCredits_x30c();
      for (AUTO(it, mText.begin()); it != mText.end(); ++it) {
        it->first->SetGeometryColor(CColor::White().WithAlphaModulatedBy(alpha));
      }
      if (mTextFadeRemaining <= 0.f) {
        mTextFaded = true;
      }
    }

    if (mTextFaded && mVideoFaded) {
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
#if VERSION >= VERSION_GM8P_00
  return kMR_Exit;
#else
  if (input.DA()) {
    mScrollPosition = CMath::Clamp(0.f, mScrollPosition - ((mScrollSpeed * input.Time())),
                                      mTotalScrollDistance);
  } else {
    const float offset = input.ALAUp() - input.ALADown();
    mScrollPosition =
        CMath::Clamp(0.f, mScrollPosition - offset * (10.f * (mScrollSpeed * input.Time())),
                     mTotalScrollDistance);
  }
  return kMR_Exit;
#endif
}

void CCredits::Draw() const {
  if (mState != 3) {
    return;
  }
  DrawVideo();
  DrawText();
}

void CCredits::DrawText() const {
  float width, padding, top, bottom;
  const rstl::pair< CVector2f, CVector2f > region =
      gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  top = mScrollPosition;
  bottom = top - (region.second.GetY() - region.first.GetY());
  width = region.second.GetX();
  padding = 0.174915016f * (region.second.GetX() - region.first.GetX());
  for (AUTO(it, mText.begin()); it != mText.end(); ++it) {
    const int offset = it->second.GetX();
    const int height = it->second.GetY();
#if VERSION == VERSION_GM8J_00
    const int scaledOffset = static_cast< int >(0.825084984f * offset);
    const int end = static_cast< int >(0.825084984f * height + scaledOffset);
    if (!(bottom > end) && !(top < scaledOffset)) {
      const int textWidth = it->first->GetTextBoundingWidth();
      const float scrollOffset = mScrollPosition - offset;
      CTransform4f transform = CTransform4f::Scale(0.825084984f, 1.f, 0.825084984f);
      const float x = 0.5f * (width - textWidth) + padding / 2.f;
      const float z = scrollOffset + 0.174915016f * it->second.GetX();
      transform.SetTranslation(CVector3f(x, 0.f, z));
      CGuiTextSupport& text = *it->first;
      gpRender->SetBlendMode_AdditiveAlpha();
      DrawText(text, transform);
      gpRender->SetBlendMode_AlphaBlended();
      const CColor originalColor = text.GetGeometryColor();
      text.SetGeometryColor(CColor::Black());
      transform.AddTranslation(CVector3f(2.f, 0.f, -2.f));
      DrawText(text, transform);
      text.SetGeometryColor(originalColor);
    }
#elif VERSION == VERSION_GM8P_00
    const int scaledOffset = static_cast< int >(static_cast< float >(offset));
    const int end = static_cast< int >(static_cast< float >(height) + scaledOffset);
    if (!(bottom > end) && !(top < scaledOffset)) {
      const int textWidth = it->first->GetTextBoundingWidth();
      const float scrollOffset = mScrollPosition - offset;
      CTransform4f transform = CTransform4f::Scale(1.f, 1.f, 1.f);
      transform.SetTranslation(CVector3f(0.5f * (width - textWidth), 0.f, scrollOffset));
      DrawText(*it->first, transform);
    }
#else
    const int end = height + offset;
    if (!(bottom > end) && !(top < offset)) {
#if VERSION >= VERSION_GM8P_00
      CTransform4f transform = CTransform4f::Scale(1.f, 1.f, 1.f);
      transform.SetTranslation(CVector3f(0.5f * (width - it->first->GetTextBoundingWidth()), 0.f,
                                         mScrollPosition - it->second.GetX()));
      DrawText(*it->first, transform);
#else
      DrawText(*it->first, CVector3f(0.5f * (width - it->first->GetTextBoundingWidth()), 0.f,
                                     mScrollPosition - it->second.GetX()));
#endif
    }
#endif
  }
  CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_CinemaBars,
                                CColor::Black(), nullptr, 1.f);
}

void CCredits::DrawVideo() const {
  /* Render movie */
  if (mMoviePlayer.get() && mMoviePlayer->DrawVideo() &&
      (mFadingIn || mFadingOut)) {
    float alpha = mVideoFadeTime / gpTweakGui->GetCredits_x310();
    if (mFadingIn) {
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
, mSaveGameScreen(rs_new CSaveGameScreen(kSC_InGame, gpGameState->GetCardSerial())) {
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
    if (mSaveGameScreen->Update(MakeMsg::GetParmTimerTick(msg).GetReal()) != 0) {
      return kMR_RemoveIOWinAndExit;
    }
    break;
  case kAM_UserInput: {
    const CArchMsgParmUserInput parm = MakeMsg::GetParmUserInput(msg);
    const CFinalInput input = parm.GetUserInput();
    mSaveGameScreen->ProcessUserInput(input);
    break;
  }
  default:
    break;
  }
  return kMR_Exit;
}

void CAutoSave::Draw() const { mSaveGameScreen->Draw(); }

bool CAutoSave::GetIsContinueDraw() const { return false; }
