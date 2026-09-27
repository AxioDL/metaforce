#include "GuiSys/CGuiTextSupport.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "Kyoto/Text/CTextExecuteBuffer.hpp"
#include "Kyoto/Text/CTextParser.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
#include <math.h>

CTextExecuteBuffer* CGuiTextSupport::gpExecBuf;
CTextParser* CGuiTextSupport::gpTextParser;

CGuiTextProperties::CGuiTextProperties(
    const bool wordWrap, const bool horizontal, const EJustification justification,
    const EVerticalJustification vertJustification,
    const rstl::vector< rstl::pair< CAssetId, CAssetId > >* txtrMap)
: mWordWrap(wordWrap)
, mHorizontal(horizontal)
, mJustification(justification)
, mVertJustification(vertJustification)
, mTxtrMap(txtrMap) {}

CGuiTextSupport::CGuiTextSupport(CAssetId fontId, const CGuiTextProperties& props,
                                 const CColor& fontCol, const CColor& outlineCol,
                                 const CColor& geomCol, int extX, int extY, CSimplePool* store)
: mCurTimeMod900(0.f)
, mProps(props)
, mFontColor(fontCol)
, mOutlineColor(outlineCol)
, mGeometryColor(geomCol)
, mImageBaseline(false)
, mExtentX(extX)
, mExtentY(extY)
, mCurTime(0.f)
, mTypeEnable(false)
, mChFadeTime(0.1f)
, mChRate(10.f)
, mFontId(fontId)
, mOneBufBounds(CVector2i(0, 0), CVector2i(0, 0))
, mPageCounter(0)
, mMultipageFlag(false) {
  if (mFontId != kInvalidAssetId) {
    mFont = TLockedToken< CRasterFont >(store->GetObj(SObjectTag('FONT', mFontId)));
  }
}

CGuiTextSupport::~CGuiTextSupport() {}

bool CGuiTextSupport::GetIsTextSupportFinishedLoading() const {
  CheckAndRebuildRenderBuffer();
  return _GetIsTextSupportFinishedLoading();
}

bool CGuiTextSupport::_GetIsTextSupportFinishedLoading() const {
  for (int i = 0; i < mAssets.size(); ++i) {
    if (!mAssets[i].IsLocked()) {
      mAssets[i].Lock();
    }
    if (!mAssets[i].IsLoaded()) {
      return false;
    }
  }
  if (mFont.valid()) {
    TLockedToken< CRasterFont > font = *mFont;
    return font->IsFinishedLoading();
  }
  if (!mFont.valid() && mAssets.empty()) {
    return false;
  }
  return true;
}

void CGuiTextSupport::SetText(const rstl::string& str, bool multipage) {
  rstl::wstring converted = CStringExtras::ConvertToUNICODE(str);
  SetText(converted, multipage);
}

void CGuiTextSupport::SetText(const rstl::wstring& str, bool multipage) {
  if (mString != str) {
    mPrimStartTimes.clear();
    mCurTime = 0.f;
    mString = str;
    ClearRenderBuffer();
    mMultipageFlag = multipage;
    mPageCounter = 0;
  }
}

void CGuiTextSupport::AddText(const rstl::wstring& str) {
  if (mRenderBuf) {
    mPrimStartTimes.reserve(mPrimStartTimes.size() + 1);
    mPrimStartTimes.push_back(
        rstl::pair< float, int >(rstl::max_val(GetCurrentAnimationOverAge(), GetCurTime()),
                                 mRenderBuf->GetNumPrimitives()));
  }
  mString.append(str);
  ClearRenderBuffer();
}

void CGuiTextSupport::SetWordWrap(bool wordWrap) {
  if (wordWrap != mProps.mWordWrap) {
    mProps.mWordWrap = wordWrap;
    ClearRenderBuffer();
  }
}

void CGuiTextSupport::SetImageBaseline(bool baseline) {
  if (mImageBaseline != baseline) {
    mImageBaseline = baseline;
    ClearRenderBuffer();
  }
}

void CGuiTextSupport::SetJustification(EJustification just) {
  if (just != mProps.mJustification) {
    mProps.mJustification = just;
    ClearRenderBuffer();
  }
}

void CGuiTextSupport::SetVerticalJustification(EVerticalJustification just) {
  if (just != mProps.mVertJustification) {
    mProps.mVertJustification = just;
    ClearRenderBuffer();
  }
}

void CGuiTextSupport::SetFontColor(const CColor& col) {
  if (!(mFontColor == col)) {
    ClearRenderBuffer();
    mFontColor = col;
  }
}

void CGuiTextSupport::SetOutlineColor(const CColor& col) {
  if (!(mOutlineColor == col)) {
    ClearRenderBuffer();
    mOutlineColor = col;
  }
}

void CGuiTextSupport::SetGeometryColor(const CColor& col) { mGeometryColor = col; }

void CGuiTextSupport::SetControlTXTRMap(
    const rstl::vector< rstl::pair< CAssetId, CAssetId > >* txtrMap) {
  if (mProps.mTxtrMap != txtrMap) {
    mProps.mTxtrMap = txtrMap;
    ClearRenderBuffer();
  }
}

void CGuiTextSupport::Render() const {
  CheckAndRebuildRenderBuffer();
  CTransform4f oldModel = CGraphics::GetModelMatrix();
  CGraphics::SetModelMatrix(oldModel * CTransform4f::Scale(CVector3f(1.f, 1.f, -1.f)));
  if (const CTextRenderBuffer* buf = GetCurrentPageRenderBuffer()) {
    buf->Render(mGeometryColor, mCurTimeMod900);
  }
  CGraphics::SetModelMatrix(oldModel);
}

void CGuiTextSupport::CheckAndRebuildTextBuffer() const {
  gpExecBuf->Clear();
  gpExecBuf->AddWordWrapping(mProps.GetIsWrap());
  gpExecBuf->BeginBlock(0, 0, mExtentX, mExtentY, mImageBaseline,
                        mProps.GetIsLeftToRight() ? kTD_Horizontal : kTD_Vertical,
                        mProps.GetJustification(), mProps.GetVerticalJustification());
  gpExecBuf->AddColor(kCT_Main, mFontColor.GetRed(), mFontColor.GetGreen(),
                      mFontColor.GetBlue(), mFontColor.GetAlpha());
  gpExecBuf->AddColor(kCT_Outline, mOutlineColor.GetRed(), mOutlineColor.GetGreen(),
                      mOutlineColor.GetBlue(), mOutlineColor.GetAlpha());
  rstl::wstring str;
  if (mFontId != kInvalidAssetId) {
    str = rstl::wstring(CStringExtras::ConvertToUNICODE(
        rstl::string(CBasics::Stringize("&font=%8.8X;", mFontId))));
  }
  str.append(mString);
  gpTextParser->ParseText(*gpExecBuf, str.c_str(), str.size(), mProps.mTxtrMap);
  gpExecBuf->EndBlock();
}

bool CGuiTextSupport::CheckAndRebuildRenderBuffer() const {
  if ((!mMultipageFlag && !mRenderBuf) ||
      (mMultipageFlag && mRenderBufferPages.empty())) {
    CheckAndRebuildTextBuffer();
    mAssets = gpExecBuf->GetAssets();
    if (_GetIsTextSupportFinishedLoading()) {
      CheckAndRebuildTextBuffer();
      if (mMultipageFlag) {
        mRenderBufferPages =
            gpExecBuf->BuildRenderBufferPages(CVector2i(mExtentX, mExtentY));
      } else {
        mRenderBuf = gpExecBuf->BuildRenderBuffer();
        mOneBufBounds = mRenderBuf->AccumulateTextBounds();
      }
      gpExecBuf->Clear();
    } else {
      return false;
    }
    const_cast< CGuiTextSupport* >(this)->Update(0.f);
  }
  return true;
}

void CGuiTextSupport::ClearRenderBuffer() {
  mRenderBuf.clear();
  mRenderBufferPages = rstl::list< CTextRenderBuffer >();
}

void CGuiTextSupport::Update(float dt) {
  if (mTypeEnable) {
    CTextRenderBuffer* buf = GetCurrentPageRenderBuffer();
    if (buf != nullptr) {
      float chStartTime = 0.f;
      for (int i = 0; i < buf->GetNumPrimitives(); ++i) {
        for (int j = 0; j < mPrimStartTimes.size(); ++j) {
          const rstl::pair< float, int >& p = mPrimStartTimes[j];
          if (p.second < i) {
            continue;
          }
          if (p.second != i) {
            break;
          }
          chStartTime = p.first;
          break;
        }
        CTextRenderBuffer::Primitive prim = buf->GetPrimitive(i);
        float alpha =
            rstl::min_val(1.f, rstl::max_val(0.f, (mCurTime - chStartTime) / mChFadeTime));
        chStartTime += 1.f / mChRate;
        CColor col(prim.mColor);
        col.SetAlpha(alpha);
        prim.mColor = col.GetColor_u32();
        buf->SetPrimitive(prim, i);
      }
    }
    mCurTime += dt;
  }
  mCurTimeMod900 = fmod(mCurTimeMod900 + dt, 900.0);
}

void CGuiTextSupport::SetTypeWriteEffectOptions(bool enable, float fadeTime, float rate) {
  mTypeEnable = enable;
  mChFadeTime = rstl::max_val(fadeTime, 0.0001f);
  mChRate = rstl::max_val(rate, 1.f);
}

float CGuiTextSupport::GetTotalAnimationTime() const {
  if (const CTextRenderBuffer* buf = GetCurrentPageRenderBuffer()) {
    if (mTypeEnable) {
      return buf->GetNumPrimitives() / mChRate;
    }
  }
  return 0.f;
}

float CGuiTextSupport::GetNumCharactersPrinted() const {
  if (const CTextRenderBuffer* buf = GetCurrentPageRenderBuffer()) {
    if (mTypeEnable) {
      return rstl::min_val(static_cast< float >(buf->GetNumPrimitives()), mCurTime * mChRate);
    }
  }
  return 0.f;
}

float CGuiTextSupport::GetNumCharsTotal() const {
  if (const CTextRenderBuffer* buf = GetCurrentPageRenderBuffer()) {
    if (mTypeEnable) {
      return buf->GetNumPrimitives();
    }
  }
  return 0.f;
}

float CGuiTextSupport::GetCurrentAnimationOverAge() const {
  float ret = 0.f;
  if (const CTextRenderBuffer* buf = GetCurrentPageRenderBuffer()) {
    if (mTypeEnable) {
      if (!mPrimStartTimes.empty()) {
        const rstl::pair< float, int >& last = mPrimStartTimes.back();
        ret = rstl::max_val(ret, (buf->GetNumPrimitives() - last.second) / mChRate + last.first);
      } else {
        ret = rstl::max_val(ret, buf->GetNumPrimitives() / mChRate);
      }
    }
  }
  return ret;
}

int CGuiTextSupport::GetTotalPageCount() {
  if (CheckAndRebuildRenderBuffer()) {
    return mRenderBufferPages.size();
  }
  return -1;
}

void CGuiTextSupport::SetPage(int page) {
  mPageCounter = page;
  mPrimStartTimes.clear();
  mCurTime = 0.f;
}

const CTextRenderBuffer* CGuiTextSupport::GetCurrentPageRenderBuffer() const {
  if (mRenderBuf && !mMultipageFlag) {
    return mRenderBuf.get_ptr();
  }
  if (mMultipageFlag && mRenderBufferPages.size() > mPageCounter) {
    int i = 0;
    for (AUTO(it, mRenderBufferPages.begin());; ++it, ++i) {
      if (i == mPageCounter) {
        return &*it;
      }
    }
  }
  return nullptr;
}

const rstl::pair< CVector2i, CVector2i >& CGuiTextSupport::GetBounds() {
  CheckAndRebuildRenderBuffer();
  return mOneBufBounds;
}
