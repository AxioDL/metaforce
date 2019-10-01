#include "Runtime/GuiSys/CGuiSliderGroup.hpp"

#include "Runtime/GuiSys/CGuiModel.hpp"
#include "Runtime/Input/CFinalInput.hpp"

namespace urde {

CGuiSliderGroup::CGuiSliderGroup(const CGuiWidgetParms& parms, float min, float max, float def, float inc)
: CGuiCompoundWidget(parms)
, xb8_minVal(min)
, xbc_maxVal(max)
, xc0_roundedCurVal(def)
, xc4_curVal(def)
, xc8_increment(inc) {}

void CGuiSliderGroup::SetSelectionChangedCallback(std::function<void(CGuiSliderGroup*, float)>&& func) {
  xd8_changeCallback = std::move(func);
}

void CGuiSliderGroup::SetCurVal(float cur) {
  xc0_roundedCurVal = zeus::clamp(xb8_minVal, cur, xbc_maxVal);
  xc4_curVal = xc0_roundedCurVal;
}

void CGuiSliderGroup::StartDecreasing() {
  xf0_state = EState::Decreasing;
  xf4_24_inputPending = true;
}

void CGuiSliderGroup::StartIncreasing() {
  xf0_state = EState::Increasing;
  xf4_24_inputPending = true;
}

bool CGuiSliderGroup::TestCursorHit(const zeus::CMatrix4f& vp, const zeus::CVector2f& point) const {
  if (xcc_sliderRangeWidgets[0]->GetWidgetTypeID() != FOURCC('MODL'))
    return false;
  CGuiModel* bar = static_cast<CGuiModel*>(xcc_sliderRangeWidgets[0]);
  auto& modelTok = bar->GetModel();
  if (!modelTok || !modelTok.IsLoaded())
    return false;

  const zeus::CVector3f& s0 = xcc_sliderRangeWidgets[0]->GetIdlePosition();
  const zeus::CVector3f& s1 = xcc_sliderRangeWidgets[1]->GetIdlePosition();

  zeus::CVector3f backupPos = bar->GetLocalPosition();
  bar->SetLocalPosition(s0);
  zeus::CVector2f p0 = vp.multiplyOneOverW(bar->GetWorldPosition()).toVec2f();
  auto aabb = modelTok->GetAABB().getTransformedAABox(bar->GetWorldTransform());
  bar->SetLocalPosition(s1);
  zeus::CVector2f p1 = vp.multiplyOneOverW(bar->GetWorldPosition()).toVec2f();
  aabb.accumulateBounds(modelTok->GetAABB().getTransformedAABox(bar->GetWorldTransform()));
  bar->SetLocalPosition(backupPos);

  zeus::CVector2f pDelta = p1 - p0;
  float magSq = pDelta.magSquared();
  float t = 0.f;
  if (magSq > 0.00001f)
    t = pDelta.dot(point - p0) / magSq;
  m_mouseT = zeus::clamp(0.f, t, 1.f);

  m_mouseInside = aabb.projectedPointTest(vp, point);
  return m_mouseInside;
}

void CGuiSliderGroup::ProcessUserInput(const CFinalInput& input) {
  if (input.DMouseButton(boo::EMouseButton::Primary) && m_mouseInside)
    m_mouseDown = true;
  else if (!input.DMouseButton(boo::EMouseButton::Primary))
    m_mouseDown = false;
  if (input.DLALeft()) {
    StartDecreasing();
    return;
  }
  if (input.DLARight()) {
    StartIncreasing();
    return;
  }
  if (input.PDPLeft()) {
    StartDecreasing();
    return;
  }
  if (input.PDPRight()) {
    StartIncreasing();
    return;
  }
}

void CGuiSliderGroup::Update(float dt) {
  float fullRange = xbc_maxVal - xb8_minVal;
  float t1 = fullRange * dt;

  float incCurVal;
  for (incCurVal = xb8_minVal; incCurVal <= xc4_curVal; incCurVal += xc8_increment) {}

  float upperIncVal = std::min(incCurVal, xbc_maxVal);
  float lowerIncVal = upperIncVal - xc8_increment;

  float oldCurVal = xc4_curVal;
  if (xf0_state == EState::Decreasing) {
    if (xf4_24_inputPending)
      xc4_curVal = std::max(oldCurVal - t1, xb8_minVal);
    else
      xc4_curVal = std::max(oldCurVal - t1, lowerIncVal);
  } else if (xf0_state == EState::Increasing) {
    if (xf4_24_inputPending)
      xc4_curVal = std::min(oldCurVal + t1, xbc_maxVal);
    else if (xc4_curVal != lowerIncVal)
      xc4_curVal = std::min(oldCurVal + t1, upperIncVal);
  }

  if (m_mouseDown) {
    xc4_curVal = m_mouseT * fullRange + xb8_minVal;
    xf0_state = EState::MouseMove;
  }

  if (xc4_curVal == oldCurVal)
    xf0_state = EState::None;

  oldCurVal = xc0_roundedCurVal;
  if (upperIncVal - xc4_curVal <= xc4_curVal - lowerIncVal)
    xc0_roundedCurVal = upperIncVal;
  else
    xc0_roundedCurVal = lowerIncVal;

  if (oldCurVal != xc0_roundedCurVal && xd8_changeCallback)
    xd8_changeCallback(this, oldCurVal);

  float fac;
  if (xbc_maxVal == xb8_minVal)
    fac = 0.f;
  else
    fac = (xc4_curVal - xb8_minVal) / (xbc_maxVal - xb8_minVal);

  const zeus::CVector3f& s0 = xcc_sliderRangeWidgets[0]->GetIdlePosition();
  const zeus::CVector3f& s1 = xcc_sliderRangeWidgets[1]->GetIdlePosition();

  xcc_sliderRangeWidgets[0]->SetLocalPosition(s1 * fac + s0 * (1.f - fac));
  xf4_24_inputPending = false;
}

bool CGuiSliderGroup::AddWorkerWidget(CGuiWidget* worker) {
  if (worker->GetWorkerId() < 0 || worker->GetWorkerId() > 1)
    return true;
  xcc_sliderRangeWidgets[worker->GetWorkerId()] = worker;
  return true;
}

CGuiWidget* CGuiSliderGroup::GetWorkerWidget(int id) const {
  if (id < 0 || id > 1)
    return nullptr;
  return xcc_sliderRangeWidgets[id];
}

std::shared_ptr<CGuiWidget> CGuiSliderGroup::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);

  float min = in.readFloatBig();
  float max = in.readFloatBig();
  float cur = in.readFloatBig();
  float increment = in.readFloatBig();

  std::shared_ptr<CGuiWidget> ret = std::make_shared<CGuiSliderGroup>(parms, min, max, cur, increment);
  ret->ParseBaseInfo(frame, in, parms);
  return ret;
}

} // namespace urde
