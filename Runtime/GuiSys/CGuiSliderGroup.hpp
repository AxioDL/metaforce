#pragma once

#include "CGuiCompoundWidget.hpp"
#include <functional>

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
  CGuiWidget* xcc_sliderRangeWidgets[2] = {};
  std::function<void(CGuiSliderGroup*, float)> xd8_changeCallback;
  EState xf0_state = EState::None;
  union {
    struct {
      bool xf4_24_inputPending : 1;
      mutable bool m_mouseInside : 1;
      bool m_mouseDown : 1;
    };
    u32 _dummy = 0;
  };

  mutable float m_mouseT = 0.f;

  void StartDecreasing();
  void StartIncreasing();

public:
  CGuiSliderGroup(const CGuiWidgetParms& parms, float a, float b, float c, float d);
  FourCC GetWidgetTypeID() const { return FOURCC('SLGP'); }

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

  bool TestCursorHit(const zeus::CMatrix4f& vp, const zeus::CVector2f& point) const;

  void ProcessUserInput(const CFinalInput& input);
  void Update(float dt);

  bool AddWorkerWidget(CGuiWidget* worker);
  CGuiWidget* GetWorkerWidget(int id) const;

  static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
};

} // namespace urde
