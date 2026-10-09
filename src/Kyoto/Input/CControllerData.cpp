#include "Kyoto/Input/CControllerData.hpp"

CControllerData::CControllerData()
: mConnected(false)
, mForceInputEvent(false)
, mMotorState(kMS_Stop)
, mPointerState(kPS_Tracking)
, mPointerValidFrameCount(0)
, mPointerInvalidFrameCount(0)
, mPointerPosition(CVector2f::Zero())
, mPointerDistance(0.f)
, mMotionMask(0)
, mSwingMask(0)
, mAxes(16)
, mContinuousAngleAxes(4)
, mReservedAxes(2)
, mButtons(64)
, mMotionButtons(16)
, mSwingButtons(12)
, mWiimoteSwingMask(0)
, mNunchukSwingMask(0) {}

void CControllerData::SetSwingMask(int device, uint mask) {
  switch (device) {
  case 0:
    mWiimoteSwingMask = mask;
    break;
  case 1:
    mNunchukSwingMask = mask;
    break;
  }
}
