#pragma once

#include <array>
#include <functional>
#include <memory>

#include "Runtime/GuiSys/CGuiCompoundWidget.hpp"

namespace urde {
class CSimplePool;

class CGuiSliderGroup : public CGuiCompoundWidget {
public:
  enum class EState { None, Decreasing, Increasing, MouseMove };

private:
  float xb8_minVal;
  float xbc_maxVal;
  float xc0_roundedCurVal;
  float xc4_curVal;
  float xc8_increment;
  std::array<CGuiWidget*, 2> xcc_sliderRangeWidgets{};
  std::function<void(CGuiSliderGroup*, float)> xd8_changeCallback;
  EState xf0_state = EState::None;
  bool xf4_24_inputPending : 1 = false;
  mutable bool m_mouseInside : 1 = false;
  bool m_mouseDown : 1 = false;

  mutable float m_mouseT = 0.f;

  void StartDecreasing();
  void StartIncreasing();

public:
  CGuiSliderGroup(const CGuiWidgetParms& parms, float a, float b, float c, float d);
  FourCC GetWidgetTypeID() const override { return FOURCC('SLGP'); }

  EState GetState() const { return xf0_state; }
  void SetSelectionChangedCallback(std::function<void(CGuiSliderGroup*, float)>&& func);
  void SetIncrement(float inc) { xc8_increment = inc; }
  void SetMinVal(float min) {
    xb8_minVal = min;
    SetCurVal(xc0_roundedCurVal);
  }
  void SetMaxVal(float max) {
    xbc_maxVal = max;
    SetCurVal(xc0_roundedCurVal);
  }
  void SetCurVal(float cur);
  float GetGurVal() const { return xc0_roundedCurVal; }

  bool TestCursorHit(const zeus::CMatrix4f& vp, const zeus::CVector2f& point) const override;

  void ProcessUserInput(const CFinalInput& input) override;
  void Update(float dt) override;

  bool AddWorkerWidget(CGuiWidget* worker) override;
  CGuiWidget* GetWorkerWidget(int id) const override;

  static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
};

} // namespace urde
