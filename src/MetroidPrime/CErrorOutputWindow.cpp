#include "MetroidPrime/CErrorOutputWindow.hpp"

#include "MetroidPrime/CArchitectureMessage.hpp"

#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CMoviePlayer.hpp"
#include "Kyoto/Input/IController.hpp"
#include "Kyoto/Text/CTextRenderBuffer.hpp"
#include "Kyoto/Text/CTextExecuteBuffer.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"

#include "MetroidPrime/CIOWin.hpp"
#include "MetroidPrime/CMemoryCardDriver.hpp"
#include "dolphin/dvd.h"

#if defined(TARGET_PC)
#include "Metaforce/Display.hpp"
#endif

CErrorOutputWindow::CErrorOutputWindow(EFlag flag)
: CIOWin(rstl::string_l("Error output window"))
, x14_state(kS_Zero)
, x18_24_(false)
, x18_25_(true)
, x18_26_(true)
, x18_27_(true)
, x18_28_(flag == kF_Zero)
, x1c_msg(nullptr) {}

bool CErrorOutputWindow::GetIsContinueDraw() const { return x14_state < kS_Two; }

CIOWin::EMessageReturn CErrorOutputWindow::OnMessage(const CArchitectureMessage& msg,
                                                     CArchitectureQueue&) {
  switch (msg.GetType()) {
  case kAM_UserInput:
    if (x14_state != kS_Zero) {
      return kMR_Exit;
    }
    return kMR_Normal;

  case kAM_FrameBegin:
    UpdateWindow();
    // fallthrough

  case kAM_TimerTick:
  case kAM_FrameEnd:
    return x14_state > kS_One ? kMR_Exit : kMR_Normal;
  default:
    break;
  }
  return kMR_Normal;
}

void CErrorOutputWindow::UpdateWindow() {
  if (x14_state == 1) {
    SetState(kS_Two);
    return;
  }
  s32 driveStatus = DVDGetDriveStatus();
  const wchar_t* errMsg = nullptr;
  bool flagThing = x14_state != kS_Zero;
  static s32 sLastDvdStatus = 0;
  if (driveStatus != sLastDvdStatus) {
    sLastDvdStatus = driveStatus;
  }
  switch (driveStatus) {
  case 5:
    errMsg = L"The Disc Cover is open.\nIf you want to continue the game,\nplease close the Disc "
             L"Cover.";
    break;
  case 4:
    errMsg = L"Please insert the\nMetroid Prime Game Disc.";
    break;
  case 6:
    errMsg = L"This is not the\nMetroid Prime Game Disc.\n \nPlease insert the\nMetroid Prime "
             L"Game Disc.";
    break;
  case -1:
    errMsg = L"An error has occurred. Turn the\npower off and refer to the\nNintendo "
             L"GameCube\nInstruction Booklet\nfor further instructions.";
    break;
  case 0xb:
    errMsg = L"The Game Disc could not be read.\nPlease read the Nintendo GameCube\nInstruction "
             L"Booklet\nfor more information.";
    break;
  default:
    break;
  }
  bool flag = false;
  if (driveStatus != 2 && driveStatus != 1) {
    flagThing = errMsg != nullptr;
    if (errMsg != nullptr) {
      if (x1c_msg != nullptr && errMsg != x1c_msg) {
        flag = true;
      }
      x1c_msg = errMsg;
    }
  }
  if (!flagThing) {
    if (x14_state != kS_Zero) {
      if (x18_28_) {
        CGraphics::SetIsBeginSceneClearFb(x18_24_);
      }
      SetState(kS_Zero);
    }
  } else if (x14_state != kS_Three) {
    if (x14_state == kS_Zero) {
      x18_24_ = CGraphics::IsBeginSceneClearFb();
    }
    SetState(EState(x14_state + 1));
  } else {
    if (flag) {
      if (x18_28_) {
        CGraphics::SetIsBeginSceneClearFb(x18_24_);
      }
      SetState(kS_Two);
    }
  }
}

void CErrorOutputWindow::Draw() const {
  switch (x14_state) {
  case 0:
    break;

  case 2:
    DrawError();

  case 1:
  case 3:
    CGraphics::SetIsBeginSceneClearFb(false);
    if (gpRender != NULL) {
      gpRender->SetRequestRGBA6(true);
    }
    break;
  }
}

void CErrorOutputWindow::DrawError() const {
  if (!x1c_msg) {
    return;
  }

  CViewport viewport = CGraphics::GetViewport();
  CTextExecuteBuffer execBuffer;
  execBuffer.AddWordWrapping(true);
  execBuffer.BeginBlock(0, 0, viewport.mWidth, viewport.mHeight, false, kTD_Horizontal,
                        kJustification_Center, kVerticalJustification_Center);
  execBuffer.AddFont(TToken< CRasterFont >(*gpDefaultFont));
  execBuffer.AddString(rstl::wstring_l(x1c_msg));
  execBuffer.EndBlock();

  if (x18_28_) {
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                  CColor::Black().WithAlphaOf(1.f), nullptr, 1.f);
  }

  const float top = CCast::ToReal32(viewport.mTop);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetOrtho(viewport.mLeft, viewport.mLeft + viewport.mWidth,
                      viewport.mTop + viewport.mHeight, top, -4096.f, 4096.f);
#if defined(TARGET_PC)
  metaforce::AdjustUiProjection();
#endif
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  CGraphics::SetCullMode(kCM_None);
  CGraphics::SetDepthWriteMode(true, kE_Always, false);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  const CTransform4f xf = CTransform4f::FromColumns(CVector3f::Right(), CVector3f::Forward(),
                                             CVector3f::Down(), CVector3f(0.f, 0.f, viewport.mHeight));
  CGraphics::SetModelMatrix(xf);
  execBuffer.BuildRenderBuffer().Render(CColor::White(), 0.f);
  CGraphics::SetCullMode(kCM_Front);
}

void CErrorOutputWindow::SetState(EState state) {
  if (state != kS_Zero && gpController != nullptr) {
    for (int i = 0; i < 4; ++i) {
      gpController->SetMotorState(static_cast< EIOPort >(i), kMS_Stop);
    }
  }

  if (state != x14_state) {
    if (x14_state == kS_Zero) {
      if (gpRender != nullptr) {
        gpRender->SetRequestRGBA6(true);
      }
      if (x18_28_) {
        x18_26_ = CStreamAudioManager::GetMusicUnmute();
        x18_27_ = CStreamAudioManager::GetSfxUnmute();
        x18_25_ = CMoviePlayer::GetAudioEnabled();
        CStreamAudioManager::SetMusicUnmute(false);
        CStreamAudioManager::SetSfxUnmute(false);
        CMoviePlayer::SetAudioEnabled(false);
      }
    } else if (state == kS_Zero && x18_28_) {
      CStreamAudioManager::SetMusicUnmute(x18_26_);
      CStreamAudioManager::SetSfxUnmute(x18_27_);
      CMoviePlayer::SetAudioEnabled(x18_25_);
    }
    x14_state = state;
  }
}

void CErrorOutputWindow::Update() { UpdateWindow(); }

void CErrorOutputWindow::ShowMessage() const {
  if (!x1c_msg) {
    return;
  }

  DrawError();
}
