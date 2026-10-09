#ifndef _CCONTROLLERDATA
#define _CCONTROLLERDATA

#include "types.h"

#include "Kyoto/Input/CControllerAxis.hpp"
#include "Kyoto/Input/CControllerButton.hpp"
#include "Kyoto/Input/InputTypes.hpp"
#include "Kyoto/Math/CVector2f.hpp"

#include "rstl/reserved_vector.hpp"

// Inferred name for the controller snapshot shared by the Wii input interface.
class CControllerData {
public:
  enum EPointerState {
    kPS_Tracking,
    kPS_RecentlyLost,
    kPS_Lost,
    kPS_Reacquiring,
  };

  CControllerData();
  void SetSwingMask(int device, uint mask);

  bool DeviceIsPresent() const { return mConnected; }

  bool GetForceInputEvent() const { return mForceInputEvent; }

  EPointerState GetPointerState() const { return static_cast< EPointerState >(mPointerState); }

  uint GetPointerValidFrameCount() const { return mPointerValidFrameCount; }

  uint GetPointerInvalidFrameCount() const { return mPointerInvalidFrameCount; }

  const CVector2f& GetPointerPosition() const { return mPointerPosition; }

  const CControllerAxis& GetAxis(int axis) const { return mAxes[axis]; }

  const CControllerAxis& GetContinuousAngleAxis(int axis) const {
    return mContinuousAngleAxes[axis];
  }

  const CControllerButton& GetButton(int button) const { return mButtons[button]; }

  void SetButton(int index, CControllerButton button) { mButtons[index] = button; }

  const CControllerButton& GetMotionButton(int button) const { return mMotionButtons[button]; }

  const CControllerButton& GetSwingButton(int button) const { return mSwingButtons[button]; }

  CControllerButton& Button(int button) {
    CControllerButton* buttons = mButtons.data();
    return buttons[button];
  }

  CControllerButton& MotionButton(int button) {
    CControllerButton* buttons = mMotionButtons.data();
    return buttons[button];
  }

  CControllerButton& SwingButton(int button) {
    CControllerButton* buttons = mSwingButtons.data();
    return buttons[button];
  }

private:
  friend class CRevolutionController;
  friend class CFinalInput;

  bool mConnected;
  bool mForceInputEvent;
  EMotorState mMotorState;
  short mPointerState;
  uint mPointerValidFrameCount;
  uint mPointerInvalidFrameCount;
  CVector2f mPointerPosition;
  float mPointerDistance;
  uint mMotionMask;
  uint mSwingMask;
  rstl::reserved_vector< CControllerAxis, 16 > mAxes;
  rstl::reserved_vector< CControllerAxis, 4 > mContinuousAngleAxes;
  rstl::reserved_vector< CControllerAxis, 2 > mReservedAxes;
  rstl::reserved_vector< CControllerButton, 64 > mButtons;
  rstl::reserved_vector< CControllerButton, 16 > mMotionButtons;
  rstl::reserved_vector< CControllerButton, 12 > mSwingButtons;
  uint mWiimoteSwingMask;
  uint mNunchukSwingMask;
};
CHECK_SIZEOF(CControllerData, 0x20c)

#endif // _CCONTROLLERDATA
