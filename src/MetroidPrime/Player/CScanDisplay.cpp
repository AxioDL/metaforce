#include "MetroidPrime/Player/CScanDisplay.hpp"
#include "GuiSys/CAuiImagePane.hpp"
#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiWidget.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include <float.h>
#include <stdlib.h>

#include <Kyoto/CSimplePool.hpp>

#include <Kyoto/Graphics/CGraphics.hpp>

#include <MetaRender/CCubeRenderer.hpp>

#include <GuiSys/CGuiTextPane.hpp>

#include <rstl/math.hpp>

#if defined(TARGET_PC)
#include "Metaforce/Display.hpp"
#endif

void CScanDisplay::SetScanMessageTypeEffect(CGuiTextPane* pane, bool type) {
  if (type) {
    pane->TextSupport().SetTypeWriteEffectOptions(true, 0.1f, 60.f);
  } else {
    pane->TextSupport().SetTypeWriteEffectOptions(false, 0.f, 0.f);
  }
}

CScanDisplay::CDataDot::CDataDot()
: mState(kDS_Hidden)
, mStartPos(CVector2f::Zero())
, mCurPos(mStartPos)
, mDestPos(mStartPos)
, mTransitionDuration(0.f)
, mRemainingTime(0.f)
, mAlpha(0.f)
, mDesiredAlpha(0.f) {}

void CScanDisplay::CDataDot::Draw(CColor color, float radius) const {
  if (mAlpha == 0.f || mState == kDS_Hidden) {
    return;
  }

#if defined(TARGET_PC)
  gpRender->SetModelMatrix(CTransform4f::Translate(mCurPos.GetX(), 0.f, mCurPos.GetY()) *
                           CTransform4f::Scale(1.f / metaforce::GetDisplayAspectScale(), 1.f, 1.f));
#else
  gpRender->SetModelMatrix(CTransform4f::Translate(mCurPos.GetX(), 0.f, mCurPos.GetY()));
#endif
  CGraphics::StreamBegin(kP_TriangleStrip);
  CGraphics::StreamColor(color.WithAlphaModulatedBy(mAlpha));
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(CVector3f(-radius, 0.f, radius));
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(CVector3f(-radius, 0.f, -radius));
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(CVector3f(radius, 0.f, radius));
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(CVector3f(radius, 0.f, -radius));
  CGraphics::StreamEnd();
}

void CScanDisplay::CDataDot::StartTransitionTo(const CVector2f& target, float duration) {
  mRemainingTime = duration;
  mTransitionDuration = duration;
  mStartPos = mCurPos;
  mDestPos = target;
}

void CScanDisplay::CDataDot::Update(float dt) {
  if (mRemainingTime > 0.f) {
    mRemainingTime = rstl::max_val(0.f, mRemainingTime - dt);
    mCurPos =
        CVector2f::Lerp(mDestPos, mStartPos,
                        mTransitionDuration > 0.f ? mRemainingTime / mTransitionDuration : 0.f);
  }

  if (mAlpha < mDesiredAlpha) {
    mAlpha = rstl::min_val(mDesiredAlpha, mAlpha + 2.f * dt);
  } else if (mAlpha > mDesiredAlpha) {
    mAlpha = rstl::max_val(mDesiredAlpha, mAlpha - 2.f * dt);
  }
}

void CScanDisplay::CDataDot::SetDestPosition(const CVector2f& dest) {
  if (mRemainingTime > 0.f) {
    mDestPos = dest;
    return;
  }

  mCurPos = dest;
}

CScanDisplay::CScanDisplay(const CGuiFrame* selHud)
: mDataDotTexture(gpSimplePool->GetObj("TXTR_DataDot"))
, mState(kSS_Inactive)
, mObject(kInvalidUniqueId)
, mSelHud(selHud)
, mMessage(nullptr)
, mScrollMessage(nullptr)
, mTextGroup(nullptr)
, mXMark(nullptr)
, mAButton(nullptr)
, mDash(nullptr)
, mDataDots(CDataDot())
, mPaneStates(SBucketPane())
, mXAlpha(0.f)
, mBodyAlpha(0.f)
, mPageCounter(0)
, mAPulse(1.f)
, mScanComplete(false) {}

static inline int find_bucket_that_uses_panel(const CScannableObjectInfo& info, int panel) {
  for (int i = 0; i < 4; ++i) {
    if (info.GetBucket(i).GetImagePosition() == panel) {
      return i;
    }
  }
  return -1;
}

void CScanDisplay::StartScan(TUniqueId uid, const CScannableObjectInfo& info, CGuiTextPane* message,
                             CGuiTextPane* scrollMessage, CGuiWidget* textGroup, CGuiModel* xMark,
                             CGuiModel* aButton, CGuiModel* dash, float scanTime) {
  mScanComplete = scanTime >= info.GetTotalDownloadTime();
  mObject = uid;
  mScannableInfo = info;
  mState = kSS_Downloading;
  mPageCounter = 0;
  mXAlpha = 0.f;
  mMessage = message;
  mScrollMessage = scrollMessage;
  mTextGroup = textGroup;
  mXMark = xMark;
  mAButton = aButton;
  mDash = dash;

  mTextGroup->SetVisibility(true, kTM_Children);
  mTextGroup->SetColor(CColor::White().WithAlphaOf(0.f));

  for (int i = 0; i < 20; ++i) {
    const rstl::string name(CScannableObjectInfo::GetImagePaneName(i));
    CAuiImagePane* pane = static_cast< CAuiImagePane* >(mSelHud->FindWidget(name));
    pane->SetColor(gpTweakGuiColors->GetScanDisplayImagePaneColor().WithAlphaOf(0.f));
    pane->SetTextureID0(kInvalidAssetId, gpSimplePool);
    pane->SetAnimationParms(CVector2f::Zero(), 0.f, 0.f);
    const int bucket = find_bucket_that_uses_panel(*mScannableInfo, i);
    if (bucket >= 0) {
      mPaneStates[bucket].mImagePane = pane;
    }
  }

  for (int i = 0; i < mPaneStates.size(); ++i) {
    SBucketPane& state = mPaneStates[i];
    CAuiImagePane* pane = state.mImagePane;
    if (pane) {
      const CScannableObjectInfo::SBucket& bucket = mScannableInfo->GetBucket(i);
      if (bucket.x14_interval > 0.f) {
        pane->SetAnimationParms(CVector2f(bucket.xc_size.GetX(), bucket.xc_size.GetY()),
                                bucket.x14_interval, bucket.x18_fadeDuration);
      }
      pane->SetTextureID0(bucket.GetTextureId(), gpSimplePool);
      pane->SetFlashFactor(0.f);
      const float startTime = GetDownloadStartTime(i);
      if (scanTime >= startTime) {
        state.mAlpha = 0.f;
      } else {
        state.mAlpha = -1.f;
      }
    }
  }

  const CAssetId stringId = mScannableInfo->GetStringTableId();
  if (stringId != kInvalidAssetId) {
    mScanString = TCachedToken< CStringTable >(gpSimplePool->GetObj(SObjectTag('STRG', stringId)));
    mScanString->Lock();
  }
  for (int i = 0; i < 4; ++i) {
    CDataDot& dot = mDataDots[i];
    if (mScannableInfo->GetBucket(i).GetImagePosition() != CScannableObjectInfo::kPT_Invalid) {
      const float startTime = GetDownloadStartTime(i);
      if (startTime - gpTweakGui->GetScanAppearanceDuration() < scanTime) {
        dot.SetAlpha(0.f);
        dot.SetDotState(CDataDot::kDS_Done);
      } else {
        dot.SetDesiredAlpha(1.f);
        dot.SetDotState(CDataDot::kDS_Seek);
        dot.StartTransitionTo(CVector2f::Zero(), FLT_EPSILON);
        dot.Update(FLT_EPSILON);
      }
    } else {
      dot.SetDotState(CDataDot::kDS_Hidden);
    }
  }
}

void CScanDisplay::StopScan() {
  switch (mState) {
  case CScanDisplay::kSS_Inactive:
  case CScanDisplay::kSS_5:
    break;
  case CScanDisplay::kSS_Downloading:
  case CScanDisplay::kSS_DownloadComplete:
  case CScanDisplay::kSS_ViewingScan:
    mState = kSS_Done;
    for (int i = 0; i < mDataDots.capacity(); ++i) {
      mDataDots[i].SetDesiredAlpha(0.f);
    }
    break;
  default:
    break;
  }
}

void CScanDisplay::Update(float dt, float scanningTime) {
  if (mState == kSS_Inactive) {
    mDataDotTexture.Unlock();
    return;
  }
  mDataDotTexture.Lock();
  mDataDotTexture.TryCache();

  bool active = false;
  if (mState == kSS_Done) {
    mBodyAlpha = rstl::max_val(0.f, mBodyAlpha - 2.f * dt);
    if (mBodyAlpha > 0.f) {
      active = true;
    }
  } else {
    active = true;
    mBodyAlpha = rstl::min_val(1.f, mBodyAlpha + 2.f * dt);
    if (mState == kSS_DownloadComplete) {
      if (mScrollMessage->TextSupport().GetIsTextSupportFinishedLoading()) {
        mXAlpha = rstl::max_val(0.f, mXAlpha - dt);
      }
      if (mXAlpha < 0.5f) {
        mAPulse += 2.f * dt;
        if (mAPulse > 1.f) {
          mAPulse -= 2.f;
        }
      }
    } else if (mState == kSS_ViewingScan) {
      mAPulse += 2.f * dt;
      if (mAPulse > 1.f) {
        mAPulse -= 2.f;
      }
      if (mXAlpha == 1.f) {
        mMessage->TextSupport().SetText(mScanString->GetObject()->GetString(0));
        SetScanMessageTypeEffect(mMessage, !mScanComplete);
      }
    } else if (mState == kSS_Downloading &&
               scanningTime >= mScannableInfo->GetTotalDownloadTime() && mScanString &&
               mScanString->TryCache()) {
      if (mScanComplete || mScannableInfo->GetCategory() == 0) {
        mState = kSS_ViewingScan;
        mXAlpha = 1.f;
        mAPulse = 1.f;
        CSfxManager::SfxStart(0x589);
      } else {
        mState = kSS_DownloadComplete;
        mXAlpha = 1.f;
        mAPulse = 1.f;
        rstl::wstring message(gpStringTable->GetString(29));
        message.append(gpStringTable->GetString(mScannableInfo->GetCategory() + 30), -1);
        message.append(gpStringTable->GetString(30), -1);
        mMessage->TextSupport().SetText(message);
        SetScanMessageTypeEffect(mMessage, true);
        CSfxManager::SfxStart(0x591);
      }
      if (mScanString->GetObject()->GetStringCount() > 2) {
        mScrollMessage->TextSupport().SetText(mScanString->GetObject()->GetString(2), true);
        SetScanMessageTypeEffect(mScrollMessage, !mScanComplete);
      }
      mScrollMessage->SetIsVisible(false);
    }
  }

  for (int i = 0; i < 4; ++i) {
    SBucketPane& state = mPaneStates[i];
    CAuiImagePane* pane = state.mImagePane;
    if (pane) {
      const float download = mState == kSS_Downloading ? GetDownloadFraction(i, scanningTime) : 1.f;
      if (state.mAlpha > 0.f) {
        const float fadeIn = gpTweakGui->GetScanPaneFadeInTime();
        const float fadeOut = gpTweakGui->GetScanPaneFadeOutTime();
        const float flashFactor = gpTweakGui->GetScanPaneFlashFactor();
        state.mAlpha = rstl::max_val(0.f, state.mAlpha - dt);
        const float flash = state.mAlpha > fadeOut ? 1.f - (state.mAlpha - fadeOut) / fadeIn
                                                   : state.mAlpha / fadeOut;
        pane->SetFlashFactor(mBodyAlpha * (flash * flashFactor));
      }
      const float alpha = download * mBodyAlpha;
      pane->SetColor(gpTweakGuiColors->GetScanDisplayImagePaneColor().WithAlphaModulatedBy(alpha));
      pane->SetDeResFactor(1.f - alpha);
      const float startTime = GetDownloadStartTime(i);
      if (startTime - gpTweakGui->GetScanAppearanceDuration() < scanningTime) {
        CDataDot& dot = mDataDots[i];
        switch (dot.GetDotState()) {
        case CDataDot::kDS_Hidden:
          break;
        case CDataDot::kDS_Seek:
        case CDataDot::kDS_Hold:
          dot.SetDotState(CDataDot::kDS_RevealPane);
          dot.StartTransitionTo(CVector2f::Zero(), gpTweakGui->GetScanAppearanceDuration());
          break;
        case CDataDot::kDS_RevealPane:
          if (dot.GetTransitionFactor() == 0.f) {
            dot.SetDotState(CDataDot::kDS_Done);
            dot.SetDesiredAlpha(0.f);
            CSfxManager::SfxStart(0x586);
            const float fadeOut = gpTweakGui->GetScanPaneFadeOutTime();
            state.mAlpha = fadeOut + gpTweakGui->GetScanPaneFadeInTime();
          }
          break;
        case CDataDot::kDS_Done:
          break;
        default:
          break;
        }
      }
    }
  }

  for (int i = 0; i < 4; ++i) {
    CDataDot& dot = mDataDots[i];
    switch (dot.GetDotState()) {
    case CDataDot::kDS_Hidden:
      continue;
    case CDataDot::kDS_Seek:
    case CDataDot::kDS_Hold:
      if (dot.GetTransitionFactor() == 0.f) {
        const bool hold = dot.GetDotState() == CDataDot::kDS_Hold;
        const float magnitude = gpTweakGui->GetScanDataDotPosRandMagnitude();
        const float durationMin = hold ? gpTweakGui->GetScanDataDotHoldDurationMin()
                                       : gpTweakGui->GetScanDataDotSeekDurationMin();
        const float durationMax = hold ? gpTweakGui->GetScanDataDotHoldDurationMax()
                                       : gpTweakGui->GetScanDataDotSeekDurationMax();
        const float x =
            hold ? dot.GetCurrPosition().GetX()
                 : magnitude * (CCast::ToReal32(rand()) / float(RAND_MAX)) - 0.5f * magnitude;
        const float y =
            hold ? dot.GetCurrPosition().GetY()
                 : magnitude * (CCast::ToReal32(rand()) / float(RAND_MAX)) - 0.5f * magnitude;
        const int random = rand();
        dot.StartTransitionTo(CVector2f(x, y), (durationMax - durationMin) *
                                                       (CCast::ToReal32(random) / float(RAND_MAX)) +
                                                   durationMin);
        dot.SetDotState(hold ? CDataDot::kDS_Seek : CDataDot::kDS_Hold);
      }
      break;
    case CDataDot::kDS_RevealPane:
    case CDataDot::kDS_Done: {
      const CGuiCamera* const camera = mSelHud->GetFrameCamera();
      const CVector3f position = mPaneStates[i].mImagePane->GetWorldTransform().GetTranslation();
      const CVector3f screen = camera->ConvertToScreenSpace(position);
      const CVector2f viewport(0.5f * (screen.GetX() * CGraphics::GetViewportWidth()),
                               0.5f * (screen.GetY() * CGraphics::GetViewportHeight()));
      dot.SetDestPosition(viewport);
      break;
    }
    default:
      break;
    }
    dot.Update(dt);
  }

  if (!active) {
    mState = kSS_Inactive;
    mObject = kInvalidUniqueId;
    mScannableInfo = rstl::optional_object_null();
    mMessage->TextSupport().SetText(rstl::wstring_l(L""));
    mScrollMessage->TextSupport().SetText(rstl::wstring_l(L""));
    mTextGroup->SetVisibility(false, kTM_Children);
    mXMark->SetVisibility(false, kTM_Children);
    mAButton->SetVisibility(false, kTM_Children);
    mDash->SetVisibility(false, kTM_Children);
    mMessage = nullptr;
    mScrollMessage = nullptr;
    mTextGroup = nullptr;
    mXMark = nullptr;
    mAButton = nullptr;
    mDash = nullptr;
    mPaneStates = rstl::reserved_vector< SBucketPane, 4 >(SBucketPane());
    mScanString = rstl::optional_object_null();
    mPageCounter = 0;
    mScanComplete = false;
  } else {
    const CColor color = CColor::White().WithAlphaOf(mBodyAlpha);
    mTextGroup->SetColor(color);
  }
}

void CScanDisplay::ProcessInput(const CFinalInput& input) {
  const bool inactive = GetScanState() == kSS_Inactive || GetScanState() == kSS_Done;
  if (inactive) {
    return;
  }
  if (mState == kSS_DownloadComplete && mXAlpha == 0.f) {
    if (input.PA()) {
      CGuiTextSupport& support = mMessage->TextSupport();
      if (support.GetCurTime() < support.GetTotalAnimationTime()) {
        support.SetCurTime(support.GetTotalAnimationTime());
      } else {
        mState = kSS_ViewingScan;
        mXAlpha = 1.f;
        CSfxManager::SfxStart(0x59f);
      }
    }
  } else if (mState == kSS_ViewingScan) {
    const int oldCounter = mPageCounter;
    const int totalPages = mScrollMessage->TextSupport().GetTotalPageCount();
    if (input.PA() && totalPages != -1) {
      CGuiTextSupport& support =
          oldCounter == 0 ? mMessage->TextSupport() : mScrollMessage->TextSupport();
      if (support.GetCurTime() < support.GetTotalAnimationTime()) {
        support.SetCurTime(support.GetTotalAnimationTime());
      } else {
        mPageCounter = rstl::min_val(mPageCounter + 1, int(totalPages));
      }
    }
    if (mPageCounter != oldCounter) {
      CSfxManager::SfxStart(0x59f);
      if (mPageCounter == 0) {
        mMessage->SetIsVisible(true);
        mScrollMessage->SetIsVisible(false);
      } else {
        if (oldCounter == 0) {
          mMessage->SetIsVisible(false);
          mScrollMessage->SetIsVisible(true);
        }
        mScrollMessage->TextSupport().SetPage(mPageCounter - 1);
        SetScanMessageTypeEffect(mScrollMessage, !mScanComplete);
      }
    }
  }

  float xAlpha = 0.f;
  float aAlpha = 0.f;
  float dashAlpha = 0.f;
  if (mState == kSS_DownloadComplete) {
    xAlpha = rstl::min_val(1.f, 2.f * mXAlpha);
    aAlpha = (1.f - xAlpha) * CMath::AbsF(mAPulse);
  } else if (mState == kSS_ViewingScan) {
    if (mPageCounter < mScrollMessage->TextSupport().GetTotalPageCount()) {
      aAlpha = CMath::AbsF(mAPulse);
    } else {
      dashAlpha = 1.f;
    }
  }
  mXMark->SetVisibility(xAlpha > 0.f, kTM_Children);
  mAButton->SetVisibility(aAlpha > 0.f, kTM_Children);
  mDash->SetVisibility(dashAlpha > 0.f, kTM_Children);
  mXMark->SetColor(CColor::White().WithAlphaOf(xAlpha));
  mAButton->SetColor(CColor::White().WithAlphaOf(aAlpha));
  mDash->SetColor(CColor(uchar(137), uchar(214), uchar(255), uchar(255)).WithAlphaOf(dashAlpha));
}

void CScanDisplay::Draw() const {
  const CTexture* const texture = mDataDotTexture.GetObject();
  if (!texture) {
    return;
  }
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  gpRender->SetBlendMode_AdditiveAlpha();
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  const float radius = gpTweakGui->GetScanDataDotRadius();
  const CColor& color = gpTweakGuiColors->GetScanDataDotColor();
  for (int i = 0; i < 4; ++i) {
    mDataDots[i].Draw(color, radius);
  }
}

float CScanDisplay::GetDownloadStartTime(const int bucket) const {
  if (!mScannableInfo) {
    return 0.f;
  }
  const CScannableObjectInfo::SBucket& b = mScannableInfo->GetBucket(bucket);
  float appearance = b.x4_appearanceRange;
  float ret = 0.f;
  for (int i = 0; i < 4; ++i) {
    const float tmp = mScannableInfo->GetBucket(i).x4_appearanceRange;
    if (tmp < appearance) {
      ret = rstl::max_val(ret, tmp);
    }
  }

  ret += gpTweakGui->GetScanAppearanceDuration();
  return ret;
}

float CScanDisplay::GetDownloadFraction(const int bucket, float time) const {
  if (!mScannableInfo) {
    return 0.f;
  }

  float appearance = mScannableInfo->GetBucket(bucket).GetAppearanceRange();
  float startTime = GetDownloadStartTime(bucket);

  if (startTime == appearance) {
    return 1.f;
  }

  return CMath::Clamp(0.f, (time - startTime) / (appearance - startTime), 1.f);
}
