#ifndef _CCONTROLLERBUTTON
#define _CCONTROLLERBUTTON

class CControllerButton {
public:
  CControllerButton() : mPressed(false), mPressEvent(false), mReleaseEvent(false) {}
  void SetIsPressed(const bool pressed) { mPressed = pressed; }
  const bool GetIsPressed() const { return mPressed; }
  void SetPressEvent(const bool press) { mPressEvent = press; }
  const bool GetPressEvent() const { return mPressEvent; }
  void SetReleaseEvent(const bool release) { mReleaseEvent = release; };
  const bool GetReleaseEvent() const { return mReleaseEvent; }

private:
  bool mPressed;
  bool mPressEvent;
  bool mReleaseEvent;
};

#endif // _CCONTROLLERBUTTON
