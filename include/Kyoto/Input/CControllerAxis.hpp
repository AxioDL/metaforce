#ifndef _CCONTROLLERAXIS
#define _CCONTROLLERAXIS

class CControllerAxis {
public:
  CControllerAxis() : mRelative(0.f), mAbsolute(0.f) {}
  void SetRelativeValue(float val) { mRelative = val; }
  float GetRelativeValue() const { return mRelative; }
  void SetAbsoluteValue(float val) { mAbsolute = val; }
  float GetAbsoluteValue() const { return mAbsolute; }

public:
  float mRelative;
  float mAbsolute;
};

#endif // _CCONTROLLERAXIS
