#include "GuiSys/CGuiSliderGroup.hpp"

#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "rstl/math.hpp"

CGuiWidget* CGuiSliderGroup::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  // Retail consumes these values but uses a fixed slider range.
  in.ReadFloat();
  in.ReadFloat();
  in.ReadFloat();
  in.ReadFloat();
  CGuiWidget* widget = rs_new CGuiSliderGroup(parms, 0.f, 255.f, 0.f, 8.f);
  widget->ParseBaseInfo(frame, in, parms);
  return widget;
}

CGuiWidget* CGuiSliderGroup::GetWorkerWidget(int id) {
  if (id >= 0 && id <= 1) {
    return mSliderRangeWidgets[id];
  }
  return nullptr;
}

CGuiSliderGroup::CGuiSliderGroup(const CGuiWidgetParms& parms, float min, float max, float cur,
                                 float increment)
: CGuiCompoundWidget(parms)
, mMinVal(min)
, mMaxVal(max)
, mRoundedCurVal(cur)
, mCurVal(cur)
, mIncrement(increment)
, mSliderRangeWidgets(2, nullptr)
, mChangeCallback()
, mState(kS_None)
, mInputPending(false) {}

bool CGuiSliderGroup::AddWorkerWidget(CGuiWidget* worker) {
  const int id = worker->GetWorkerId();
  if (id >= 0 && id <= 1) {
    mSliderRangeWidgets[id] = worker;
  }
  return true;
}

void CGuiSliderGroup::ProcessUserInput(const CFinalInput& input) {
  if (input.DLALeft()) {
    MAF_Decrement(nullptr, nullptr);
    return;
  }
  if (input.DLARight()) {
    MAF_Increment(nullptr, nullptr);
    return;
  }
  if (input.PDPLeft()) {
    MAF_Decrement(nullptr, nullptr);
    return;
  }
  if (input.PDPRight()) {
    MAF_Increment(nullptr, nullptr);
    return;
  }
}

void CGuiSliderGroup::Update(float dt) {
  float delta = dt * (mMaxVal - mMinVal);
  float upper;
  for (upper = mMinVal; upper <= mCurVal; upper += mIncrement) {
  }
  upper = rstl::min_val(mMaxVal, upper);
  float lower = upper - mIncrement;
  float oldCur = mCurVal;
  if (mState == kS_Decreasing) {
    if (mInputPending) {
      mCurVal = rstl::max_val(mMinVal, oldCur - delta);
    } else {
      mCurVal = rstl::max_val(lower, oldCur - delta);
    }
  } else if (mState == kS_Increasing) {
    if (mInputPending) {
      mCurVal = rstl::min_val(mMaxVal, oldCur + delta);
    } else if (mCurVal != lower) {
      mCurVal = rstl::min_val(upper, oldCur + delta);
    }
  }
  if (oldCur == mCurVal) {
    mState = kS_None;
  }
  float oldRounded = mRoundedCurVal;
  mRoundedCurVal = upper - mCurVal > mCurVal - lower ? lower : upper;
  if (oldRounded != mRoundedCurVal && mChangeCallback) {
    mChangeCallback(this, oldRounded);
  }
  const float factor =
      mMaxVal == mMinVal ? 0.f : (mCurVal - mMinVal) / (mMaxVal - mMinVal);
  CGuiWidget* first = mSliderRangeWidgets[0];
  CGuiWidget* second = mSliderRangeWidgets[1];
  CVector3f a = first->GetIdlePosition();
  CVector3f b = second->GetIdlePosition();
  CVector3f position = CVector3f::Lerp(a, b, factor);
  first->SetLocalPosition(position);
  mInputPending = false;
}

void CGuiSliderGroup::SetCurVal(float cur) {
  mRoundedCurVal = CMath::Clamp(mMinVal, cur, mMaxVal);
  mCurVal = mRoundedCurVal;
}

void CGuiSliderGroup::SetMaxVal(float max) {
  mMaxVal = max;
  SetCurVal(mRoundedCurVal);
}

void CGuiSliderGroup::SetMinVal(float min) {
  mMinVal = min;
  SetCurVal(mRoundedCurVal);
}

void CGuiSliderGroup::SetIncrement(float increment) { mIncrement = increment; }

int CGuiSliderGroup::MAF_Increment(CGuiFunctionDef* func, CGuiControllerInfo* info) {
  mState = kS_Increasing;
  mInputPending = true;
  return 1;
}

int CGuiSliderGroup::MAF_Decrement(CGuiFunctionDef* func, CGuiControllerInfo* info) {
  mState = kS_Decreasing;
  mInputPending = true;
  return 1;
}

void CGuiSliderGroup::SetSelectionChangedCallback(
    const TFunctor2< CGuiSliderGroup* const, const float >& callback) {
  mChangeCallback = callback;
}
