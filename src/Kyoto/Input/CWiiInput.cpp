#include "Kyoto/Input/CWiiInput.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Input/CInputFilter.hpp"
#include "Kyoto/Input/CWiiMotionProcessor.hpp"
#include "Kyoto/Math/CMath.hpp"

#include <dolphin/vi.h>
#include <revolution/os.h>

namespace {
// Inferred RAII helper: the original stores the saved state and an explicit restore flag.
class CInterruptGuard {
public:
  CInterruptGuard() : x0_enabled(OSDisableInterrupts()), x1_restored(false) {}
  ~CInterruptGuard() { Restore(); }
  void Restore() {
    if (!x1_restored) {
      OSRestoreInterrupts(x0_enabled);
      x1_restored = true;
    }
  }

private:
  bool x0_enabled;
  bool x1_restored;
};
} // namespace

static CWiiInput* sWiiInput;
static uint sButtonMasks[64] = {
    0, 0x8000, 0x1000, 0x800, 0x400,  0x100,  0x200, 0x10, 0x8,  0x4, 0x2, 0x1, 0xf, 0, 0, 0, 0,
    0, 0,      0,      0,     0x2000, 0x4000, 0,     0,    0,    0,   0,   0,   0,   0, 0, 0, 0,
    0, 0x100,  0x200,  0x400, 0x800,  0x1000, 0x10,  0x40, 0x20, 0x8, 0x2, 0x4, 0x1, 0, 0, 0, 0,
    0, 0,      0,      0,     0,      0,      0,     0,    0,    0,   0,   0,   0,
};
static uint sMotionButtonMasks[16] = {
    0x1,     0x2,     0x4,     0x8,     0x10,     0x20,     0x40,     0x80,
    0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000,
};
static uint sSwingButtonMasks[12] = {
    0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000,
};

void* CWiiInput::AllocateWpadMemory(u32 size) {
  return CMemory::Alloc(size, static_cast< IAllocator::EHint >(6));
}

int CWiiInput::FreeWpadMemory(void* memory) {
  CMemory::Free(memory);
  return 1;
}

CWiiInput::CWiiInput()
: mStatus(KPADStatus())
, mControllerTypes(kDT_Disconnected)
, mInput(CControllerData())
, mPointerRecenterMode(kPRM_Hold)
, mPointerReacquireFrames(4, 0)
, mMotionProcessors(4)
, mUnknownInputData(SUnknownInputData())
, mWpadInfo(4)
, mWpadInfoBuf(4)
, mInfoPollTimers(4)
, mMotionIdleTimes(4)
, mButtonIdleTimes(4)
, mPendingConnectionEvents(kCE_None)
, mPendingExtensionEvents(kEE_None)
, mPointerFilterX(4)
, mPointerFilterY(4)
, mMotorEnabledFlags(0xf)
, mPointerMinDistance(0.75f)
, mPointerMaxDistance(4.f)
, mPointerMinScale(1.3f)
, mPointerMaxScale(1.f)
, mAcceptAdditionalConnections(false) {
  sWiiInput = this;
  Initialize();
}

CWiiInput::~CWiiInput() {
  sWiiInput = nullptr;
  for (int i = 0; i < 4; ++i) {
    WPADSetConnectCallback(i, nullptr);
    WPADSetExtensionCallback(i, nullptr);
  }
}

void CWiiInput::Poll() {}

uint CWiiInput::GetDeviceCount() const { return 4; }

CControllerData& CWiiInput::GetInput(uint channel) { return mInput[channel]; }

IController::EDeviceType CWiiInput::GetControllerType(int channel) const {
  return mControllerTypes[channel];
}

void CWiiInput::QueueConnectionEvent(int channel, EConnectionEvent event) {
  mPendingConnectionEvents[channel] = event;
}

void CWiiInput::QueueExtensionEvent(int channel, EExtensionEvent event) {
  mPendingExtensionEvents[channel] = event;
}

void CWiiInput::ProcessConnectionEvents() {
  rstl::reserved_vector< EConnectionEvent, 4 > connections(4);
  rstl::reserved_vector< EExtensionEvent, 4 > extensions(4);
  CInterruptGuard interrupts;
  for (int i = 0; i < 4; ++i) {
    connections[i] = mPendingConnectionEvents[i];
    extensions[i] = mPendingExtensionEvents[i];
    mPendingConnectionEvents[i] = kCE_None;
    mPendingExtensionEvents[i] = kEE_None;
  }
  interrupts.Restore();

  for (int i = 0; i < 4; ++i) {
    switch (connections[i]) {
    case kCE_Connected:
      InitializeController(i);
      if (i == 0 && !mAcceptAdditionalConnections) {
        WPADSetAcceptConnection(false);
      }
      break;
    case kCE_Disconnected:
      if (i == 0 && !mAcceptAdditionalConnections) {
        WPADSetAcceptConnection(true);
      }
      break;
    case kCE_Rejected:
      WPADDisconnect(i);
      break;
    }
  }
  for (int i = 0; i < 4; ++i) {
    switch (extensions[i]) {
    case kEE_Wiimote:
      InitializeController(i);
      break;
    case kEE_Nunchuk:
      InitializeController(i);
      break;
    }
  }
}

void CWiiInput::Update(float dt) {
  KPADStatus samples[16];
  KPADUnifiedWpadStatus rawSamples[16];
  ProcessConnectionEvents();
  for (int channel = 0; channel < 1; ++channel) {
    if (mInput[channel].DeviceIsPresent()) {
      if (mControllerTypes[channel] != kDT_Unsupported) {
        CInterruptGuard interrupts;
        const int count = KPADRead(channel, samples, 16);
        if (count != 0 && samples[0].wpad_err == WPAD_ERR_OK) {
          CBasics::CopyMemory(&mStatus[channel], samples, sizeof(KPADStatus));
          mInfoPollTimers[channel] -= dt;
          if (mInfoPollTimers[channel] < 0.f) {
            mInfoPollTimers[channel] = 5.f;
            WPADGetInfoAsync(channel, &mWpadInfoBuf[channel], WpadInfoCallback);
          }
          UpdateIdleTimes(channel, dt);
          if (!mMotionProcessors[channel].null()) {
            KPADGetUnifiedWpadStatus(channel, rawSamples, 16);
            interrupts.Restore();
            if (rawSamples[0].u.core.err == WPAD_ERR_OK &&
                rawSamples[0].u.core.dev == WPAD_DEV_FS && uint(rawSamples[0].fmt - 3) <= 2) {
              for (int i = count - 1; i >= 0; --i) {
                mMotionProcessors[channel]->Update(rawSamples[i].u.fs, samples[i], 0.005f);
              }
              mInput[channel].mMotionMask = mMotionProcessors[channel]->GetMotionMask();
              mInput[channel].mSwingMask = mMotionProcessors[channel]->GetSwingMask();
            }
          }
          ProcessControllerInput(channel);
        }
      }
      if (uint(mControllerTypes[channel] - kDT_Unsupported) <= 1) {
        CBasics::ZeroMemory(&mStatus[channel], sizeof(KPADStatus));
        mStatus[channel].dev_type = 0xff;
        mStatus[channel].wpad_err = 0;
      }
    } else {
      CBasics::ZeroMemory(&mStatus[channel], sizeof(KPADStatus));
      mStatus[channel].dev_type = 0xff;
      mStatus[channel].wpad_err = 0;
    }
  }
}

bool CWiiInput::IsPointerValid(int channel) const {
  if (IsPointerDevicePresent(channel) && mStatus[channel].dpd_valid_fg > 0) {
    return true;
  }
  return false;
}

bool CWiiInput::IsPointerDevicePresent(int channel) const {
#if NONMATCHING
  if (channel < 0) {
    return false;
  }
#endif
  if (channel >= 4 || !mInput[channel].DeviceIsPresent()) {
    return false;
  }
  switch (mControllerTypes[channel]) {
  case kDT_Wiimote:
  case kDT_Nunchuk:
  case kDT_Classic:
    return true;
  default:
    return false;
  }
}

CControllerData::EPointerState CWiiInput::GetPointerState(int channel) const {
  if (IsPointerDevicePresent(channel)) {
    return mInput[channel].GetPointerState();
  }
  return CControllerData::kPS_RecentlyLost;
}

uint CWiiInput::GetPointerValidFrameCount(int channel) const {
  return mInput[channel].GetPointerValidFrameCount();
}

uint CWiiInput::GetPointerInvalidFrameCount(int channel) const {
  return mInput[channel].GetPointerInvalidFrameCount();
}

CVector2f CWiiInput::GetPointerPosition(int channel) const {
  return mInput[channel].GetPointerPosition();
}

void CWiiInput::ApplyPointerDistanceScale(int channel) {
  const float distance = (mInput[channel].mPointerDistance - mPointerMinDistance) /
                         (mPointerMaxDistance - mPointerMinDistance);
  const float scale = mPointerMinScale +
                      (mPointerMaxScale - mPointerMinScale) * CMath::FastClamp(0.f, distance, 1.f);
  mStatus[channel].pos.x =
      CMath::FastLimit(mStatus[channel].pos.x * scale, 1.f);
}

void CWiiInput::UpdatePointerState(int channel) {
  const bool valid = IsPointerValid(channel);
  CControllerData& input = mInput[channel];
  if (valid) {
    ++input.mPointerValidFrameCount;
    input.mPointerInvalidFrameCount = 0;
  } else {
    ++input.mPointerInvalidFrameCount;
    input.mPointerValidFrameCount = 0;
  }

  if (input.mPointerState == CControllerData::kPS_Tracking) {
    if (valid) {
      input.mPointerPosition[0] = mPointerFilterX[channel]->Filter(mStatus[channel].pos.x);
      input.mPointerPosition[1] = mPointerFilterY[channel]->Filter(mStatus[channel].pos.y);
      mStatus[channel].pos.x = input.mPointerPosition.GetX();
      mStatus[channel].pos.y = input.mPointerPosition.GetY();
    } else {
      input.mPointerState = CControllerData::kPS_RecentlyLost;
      mStatus[channel].pos.x = input.mPointerPosition.GetX();
      mStatus[channel].pos.y = input.mPointerPosition.GetY();
    }
  } else if (input.mPointerState == CControllerData::kPS_RecentlyLost) {
    if (valid) {
      if (input.mPointerValidFrameCount >= 3) {
        input.mPointerState = CControllerData::kPS_Reacquiring;
        input.mPointerValidFrameCount = 0;
        mPointerReacquireFrames[channel] = 0;
      }
    } else if (input.mPointerInvalidFrameCount >= 120) {
      input.mPointerState = CControllerData::kPS_Lost;
    }
    mStatus[channel].pos.x = input.mPointerPosition.GetX();
    mStatus[channel].pos.y = input.mPointerPosition.GetY();
  } else if (input.mPointerState == CControllerData::kPS_Lost) {
    bool recenter = true;
    if (valid && input.mPointerValidFrameCount > 3) {
      input.mPointerState = CControllerData::kPS_Reacquiring;
      input.mPointerValidFrameCount = 0;
      recenter = false;
    }
    if (recenter) {
      if (mPointerRecenterMode[channel] == kPRM_Both) {
        if (input.mPointerPosition.IsMagnitudeSafe()) {
          const float magnitude = input.mPointerPosition.Magnitude() * 0.95f;
          input.mPointerPosition = input.mPointerPosition.AsNormalized() * magnitude;
        }
      } else if (mPointerRecenterMode[channel] == kPRM_X) {
        input.mPointerPosition[0] *= 0.95f;
      } else if (mPointerRecenterMode[channel] == kPRM_Y) {
        input.mPointerPosition[1] *= 0.95f;
      }
    }
    mStatus[channel].pos.x = input.mPointerPosition.GetX();
    mStatus[channel].pos.y = input.mPointerPosition.GetY();
  } else if (input.mPointerState == CControllerData::kPS_Reacquiring) {
    if (valid) {
      if (++mPointerReacquireFrames[channel] >= 10) {
        input.mPointerState = CControllerData::kPS_Tracking;
      }
      mStatus[channel].pos.x = mPointerFilterX[channel]->Filter(mStatus[channel].pos.x);
      mStatus[channel].pos.y = mPointerFilterY[channel]->Filter(mStatus[channel].pos.y);
      const float blend = CMath::FastClamp(0.f, mPointerReacquireFrames[channel] / 10.f, 1.f);
      input.mPointerPosition[0] =
          (1.f - blend) * input.mPointerPosition.GetX() + blend * mStatus[channel].pos.x;
      input.mPointerPosition[1] =
          (1.f - blend) * input.mPointerPosition.GetY() + blend * mStatus[channel].pos.y;
    } else {
      input.mPointerState = CControllerData::kPS_Lost;
    }
  }
}

void CWiiInput::ProcessControllerInput(int channel) {
  if (IsPointerDevicePresent(channel) && mStatus[channel].wpad_err == WPAD_ERR_OK) {
    if (IsPointerValid(channel)) {
      mInput[channel].mPointerDistance = mStatus[channel].dist;
      ApplyPointerDistanceScale(channel);
    }
    UpdatePointerState(channel);
  }
  if (!mInput[channel].DeviceIsPresent()) {
    return;
  }
  ClearButtonEvents(channel);
  CControllerData& input = mInput[channel];
  switch (mControllerTypes[channel]) {
  case kDT_Nunchuk:
    if (mStatus[channel].wpad_err == WPAD_ERR_OK) {
      UpdateAnalogInput(0.7f, channel, 0, input.mButtons[20], input.mButtons[19]);
      UpdateAnalogInput(0.7f, channel, 1, input.mButtons[16], input.mButtons[13]);
      UpdateAnalogInput(0.7f, channel, 2, input.mButtons[14], input.mButtons[18]);
      UpdateAnalogInput(0.7f, channel, 3, input.mButtons[17], input.mButtons[15]);
      UpdateAnalogInput(0.5f, channel, 4, input.mButtons[23], input.mButtons[24]);
      UpdateAnalogInput(0.5f, channel, 5, input.mButtons[25], input.mButtons[26]);
      UpdateContinuousAngleAxis(channel, 0);
      UpdateContinuousAngleAxis(channel, 1);
    }
  case kDT_Classic:
  case kDT_Wiimote:
    if (mStatus[channel].wpad_err == WPAD_ERR_OK) {
      UpdateAnalogInput(0.7f, channel, 6, input.mButtons[30], input.mButtons[29]);
      UpdateAnalogInput(0.7f, channel, 7, input.mButtons[27], input.mButtons[28]);
      UpdateAnalogInput(0.5f, channel, 8, input.mButtons[31], input.mButtons[32]);
      UpdateAnalogInput(0.5f, channel, 9, input.mButtons[33], input.mButtons[34]);
      UpdateContinuousAngleAxis(channel, 2);
      UpdateContinuousAngleAxis(channel, 3);
      if (!mMotionProcessors[channel].null()) {
        input.SetSwingMask(0, mMotionProcessors[channel]->GetSwingMask(0));
        input.SetSwingMask(1, mMotionProcessors[channel]->GetSwingMask(1));
      }
    }
    break;
  }
  if (mControllerTypes[channel] == kDT_Unsupported) {
    CBasics::ZeroMemory(&mStatus[channel], sizeof(KPADStatus));
    mStatus[channel].dev_type = 0xff;
    for (int i = 0; i < 64; ++i) {
      input.mButtons[i].SetIsPressed(false);
      input.mButtons[i].SetPressEvent(false);
      input.mButtons[i].SetReleaseEvent(false);
    }
  } else {
    UpdateDigitalInput(channel);
  }
}

void CWiiInput::UpdateAnalogInput(float threshold, int channel, int axis,
                                  CControllerButton& negativeButton,
                                  CControllerButton& positiveButton) {
  CControllerData& input = mInput[channel];
  if (!input.DeviceIsPresent()) {
    return;
  }
  float value = 0.f;
  const float previous = input.mAxes[axis].GetAbsoluteValue();
  switch (mControllerTypes[channel]) {
  case kDT_Nunchuk:
    if (axis == 0) {
      CVector2f stick(mStatus[channel].ex_status.fs.stick.x,
                      mStatus[channel].ex_status.fs.stick.y);
      if (stick.IsMagnitudeSafe()) {
        const float magnitude = CMath::FastClamp(0.f, stick.Magnitude() / 0.707f, 1.f);
        stick = stick.AsNormalized() * magnitude;
      }
      value = stick.GetX();
    } else if (axis == 1) {
      CVector2f stick(mStatus[channel].ex_status.fs.stick.x,
                      mStatus[channel].ex_status.fs.stick.y);
      if (stick.IsMagnitudeSafe()) {
        const float magnitude = CMath::FastClamp(0.f, stick.Magnitude() / 0.707f, 1.f);
        stick = stick.AsNormalized() * magnitude;
      }
      value = stick.GetY();
    } else if (axis == 2) {
      const CVector2f stick(mStatus[channel].ex_status.fs.stick.x,
                            mStatus[channel].ex_status.fs.stick.y);
      if (stick.IsMagnitudeSafe()) {
        value = CVector2f::Dot(stick, CVector2f(1.f, -1.f).AsNormalized()) / 1.4142135f / 0.707f;
        value = CMath::FastClamp(0.f, value, 1.f);
      }
    } else if (axis == 3) {
      const CVector2f stick(mStatus[channel].ex_status.fs.stick.x,
                            mStatus[channel].ex_status.fs.stick.y);
      if (stick.IsMagnitudeSafe()) {
        value = CVector2f::Dot(stick, CVector2f(1.f, 1.f).AsNormalized()) / 1.4142135f / 0.707f;
        value = CMath::FastClamp(0.f, value, 1.f);
      }
    } else if (axis == 4) {
      if (!mMotionProcessors[channel].null()) {
        value =
            mMotionProcessors[channel]->GetNunchukTracker().GetWrappedRoll() / (M_PIF / 2.f);
        value = CMath::FastLimit(value, 2.f);
      }
    } else if (axis == 5 && !mMotionProcessors[channel].null()) {
      value = mMotionProcessors[channel]->GetNunchukTracker().GetWrappedPitch() / (M_PIF / 2.f);
      value = CMath::FastLimit(value, 2.f);
    }
  case kDT_Classic:
  case kDT_Wiimote:
    if (axis == 6) {
      value = mStatus[channel].pos.x;
    } else if (axis == 7) {
      value = mStatus[channel].pos.y;
    } else if (axis == 8) {
      if (!mMotionProcessors[channel].null()) {
        value =
            mMotionProcessors[channel]->GetWiimoteTracker().GetWrappedRoll() / (M_PIF / 2.f);
        value = CMath::FastLimit(value, 2.f);
      }
    } else if (axis == 9 && !mMotionProcessors[channel].null()) {
      value = mMotionProcessors[channel]->GetWiimoteTracker().GetWrappedPitch() / (M_PIF / 2.f);
      value = CMath::FastLimit(value, 2.f);
    }
    break;
  }

  input.mAxes[axis].SetRelativeValue(value - input.mAxes[axis].GetAbsoluteValue());
  input.mAxes[axis].SetAbsoluteValue(value);
  const bool wasNegative = negativeButton.GetIsPressed();
  bool negative = wasNegative;
  if (!negative) {
    if (previous > -threshold && value <= -threshold) {
      negative = true;
    }
  } else {
    const float releaseThreshold = -threshold + 0.2f;
    if (previous <= releaseThreshold && value > releaseThreshold) {
      negative = false;
    }
  }
  negativeButton.SetIsPressed(negative);
  negativeButton.SetPressEvent(negative & (negative ^ wasNegative));
  negativeButton.SetReleaseEvent(wasNegative & (negative ^ wasNegative));

  const bool wasPositive = positiveButton.GetIsPressed();
  bool positive = wasPositive;
  if (!positive) {
    if (previous < threshold && value >= threshold) {
      positive = true;
    }
  } else {
    const float releaseThreshold = threshold - 0.2f;
    if (previous >= releaseThreshold && value < releaseThreshold) {
      positive = false;
    }
  }
  positiveButton.SetIsPressed(positive);
  positiveButton.SetPressEvent(positive & (positive ^ wasPositive));
  positiveButton.SetReleaseEvent(wasPositive & (positive ^ wasPositive));
}

void CWiiInput::UpdateContinuousAngleAxis(int channel, int axis) {
  CControllerData& input = mInput[channel];
  if (!input.DeviceIsPresent()) {
    return;
  }
  CControllerAxis& data = input.mContinuousAngleAxes[axis];
  float value = 0.f;
  switch (mControllerTypes[channel]) {
  case kDT_Nunchuk:
    switch (axis) {
    case 0:
      if (!mMotionProcessors[channel].null()) {
        value = mMotionProcessors[channel]->GetNunchukTracker().GetContinuousRoll();
      }
      break;
    case 1:
      if (!mMotionProcessors[channel].null()) {
        value = mMotionProcessors[channel]->GetNunchukTracker().GetContinuousPitch();
      }
      break;
    }
  case kDT_Classic:
  case kDT_Wiimote:
    switch (axis) {
    case 2:
      if (!mMotionProcessors[channel].null()) {
        value = mMotionProcessors[channel]->GetWiimoteTracker().GetContinuousRoll();
      }
      break;
    case 3:
      if (!mMotionProcessors[channel].null()) {
        value = mMotionProcessors[channel]->GetWiimoteTracker().GetContinuousPitch();
      }
      break;
    }
    break;
  }
  data.SetRelativeValue(value - data.GetAbsoluteValue());
  data.SetAbsoluteValue(value);
}

void CWiiInput::ClearButtonEvents(int channel) {
  CControllerData& input = mInput[channel];
  if (!input.DeviceIsPresent()) {
    return;
  }
  for (int i = 0; i < 64; ++i) {
    if (sButtonMasks[i] != 0) {
      input.mButtons[i].SetPressEvent(false);
      input.mButtons[i].SetReleaseEvent(false);
    }
  }
  for (int i = 0; i < 16; ++i) {
    if (sMotionButtonMasks[i] != 0) {
      input.mMotionButtons[i].SetPressEvent(false);
      input.mMotionButtons[i].SetReleaseEvent(false);
    }
  }
  for (int i = 0; i < 12; ++i) {
    if (sSwingButtonMasks[i] != 0) {
      input.mSwingButtons[i].SetPressEvent(false);
      input.mSwingButtons[i].SetReleaseEvent(false);
    }
  }
}

void CWiiInput::UpdateDigitalInput(int channel) {
  if (mInput[channel].DeviceIsPresent()) {
    if (int(mControllerTypes[channel]) >= 0 && int(mControllerTypes[channel]) < 3) {
      const uint held = mStatus[channel].hold;
      for (int i = 1; i < 35; ++i) {
        if (sButtonMasks[i] != 0) {
          UpdateButton(held, mInput[channel].mButtons[i], sButtonMasks[i]);
        }
      }
    }
    UpdateMotionButtons(channel);
    UpdateSwingButtons(channel);
  }
}

void CWiiInput::UpdateButton(uint heldMask, CControllerButton& button, uint mask) {
  const int wasPressed = button.GetIsPressed();
  const bool pressed = (heldMask & mask) != 0;
  button.SetIsPressed(pressed);
  const int changed = pressed ^ wasPressed;
  button.SetPressEvent(pressed & changed);
  button.SetReleaseEvent(wasPressed & changed);
}

void CWiiInput::UpdateMotionButton(int channel, CControllerButton& button, uint mask) {
  if (!mInput[channel].DeviceIsPresent()) {
    return;
  }
  const int wasPressed = button.GetIsPressed();
  const bool pressed = (mInput[channel].mMotionMask & mask) != 0;
  button.SetIsPressed(pressed);
  const int changed = pressed ^ wasPressed;
  button.SetPressEvent(pressed & changed);
  button.SetReleaseEvent(wasPressed & changed);
}

void CWiiInput::UpdateMotionButtons(int channel) {
  CControllerData& input = mInput[channel];
  if (input.DeviceIsPresent()) {
    for (int i = 0; i < 16; ++i) {
      if (sMotionButtonMasks[i] != 0) {
        UpdateMotionButton(channel, input.mMotionButtons[i], sMotionButtonMasks[i]);
      }
    }
  }
}

void CWiiInput::UpdateSwingButton(int channel, CControllerButton& button, uint mask) {
  if (!mInput[channel].DeviceIsPresent()) {
    return;
  }
  const int wasPressed = button.GetIsPressed();
  const bool pressed = (mInput[channel].mSwingMask & mask) != 0;
  button.SetIsPressed(pressed);
  const int changed = pressed ^ wasPressed;
  button.SetPressEvent(pressed & changed);
  button.SetReleaseEvent(wasPressed & changed);
}

void CWiiInput::UpdateSwingButtons(int channel) {
  CControllerData& input = mInput[channel];
  if (input.DeviceIsPresent()) {
    for (int i = 0; i < 12; ++i) {
      if (sSwingButtonMasks[i] != 0) {
        UpdateSwingButton(channel, input.mSwingButtons[i], sSwingButtonMasks[i]);
      }
    }
  }
}

void CWiiInput::SetMotorState(int channel, EMotorState state) {
#if NONMATCHING
  if (uint(channel) >= 4) {
    return;
  }
#endif
  if (mInput[channel].DeviceIsPresent()) {
    mInput[channel].mMotorState = state;
    if (!IsControllerIdle(channel) && channel < 4 && (mMotorEnabledFlags & (1 << channel))) {
      switch (state) {
      case kMS_Rumble:
        WPADControlMotor(channel, WPAD_MOTOR_RUMBLE);
        break;
      case kMS_Stop:
        WPADControlMotor(channel, WPAD_MOTOR_STOP);
        break;
      }
    }
  }
}

void CWiiInput::SetMotorEnabled(int channel, bool enabled) {
#if NONMATCHING
  if (channel < 0) {
    return;
  }
#endif
  if (channel >= 4) {
    return;
  }
  const uint bit = 1 << channel;
  const uint value = uint(enabled) << channel;
  if (value == (mMotorEnabledFlags & bit)) {
    return;
  }
  mMotorEnabledFlags = (mMotorEnabledFlags & ~bit) | value;
  if (!enabled) {
    WPADControlMotor(channel, 0);
  }
}

bool CWiiInput::HasMotionActivity(uint channel) const {
#if NONMATCHING
  if (channel >= 4) {
#else
  if (channel > 4) {
#endif
    return false;
  }
  const KPADStatus& status = mStatus[channel];
  if (status.dev_type > WPAD_DEV_FS) {
    return false;
  }
  bool active = status.acc_speed > 0.03f || (status.speed > 0.f && status.dpd_valid_fg > 0);
  if (status.dev_type == WPAD_DEV_FS && status.ex_status.fs.acc_speed > 0.03f) {
    active = true;
  }
  return active;
}

float CWiiInput::GetMotionIdleTime(uint channel) const {
#if NONMATCHING
  if (channel < 4) {
#else
  if (channel <= 4) {
#endif
    return mMotionIdleTimes[channel];
  }
  return 0.f;
}

bool CWiiInput::HasButtonActivity(uint channel) const {
#if NONMATCHING
  if (channel >= 4) {
#else
  if (channel > 4) {
#endif
    return false;
  }
  const KPADStatus& status = mStatus[channel];
  if (status.dev_type > WPAD_DEV_FS) {
    return false;
  }
  bool active = false;
  if (status.hold != 0) {
    active = true;
  }
  if (status.dev_type == WPAD_DEV_FS &&
      (!CMath::IsEpsilon(status.ex_status.fs.stick.x, 0.f, 1.e-5f) ||
       !CMath::IsEpsilon(status.ex_status.fs.stick.y, 0.f, 1.e-5f))) {
    active = true;
  }
  return active;
}

float CWiiInput::GetButtonIdleTime(uint channel) const {
#if NONMATCHING
  if (channel < 4) {
#else
  if (channel <= 4) {
#endif
    return mButtonIdleTimes[channel];
  }
  return 0.f;
}

bool CWiiInput::IsControllerIdle(uint channel) const { return GetButtonIdleTime(channel) >= 60.f; }

void CWiiInput::UpdateIdleTimes(uint channel, float dt) {
#if NONMATCHING
  if (channel >= 4) {
#else
  if (channel > 4) {
#endif
    return;
  }
  if (mStatus[channel].dev_type <= WPAD_DEV_FS) {
    if (HasMotionActivity(channel)) {
      mMotionIdleTimes[channel] = 0.f;
    } else {
      mMotionIdleTimes[channel] += dt;
    }
    if (HasButtonActivity(channel)) {
      mButtonIdleTimes[channel] = 0.f;
    } else {
      const float previous = mButtonIdleTimes[channel];
      mButtonIdleTimes[channel] += dt;
      if (previous < 60.f && mButtonIdleTimes[channel] >= 60.f) {
        WPADControlMotor(channel, 0);
      }
    }
  }
}

bool CWiiInput::Initialize() {
  for (int i = 0; i < 4; ++i) {
    mInput[i].mConnected = false;
    mInput[i].mPointerPosition = CVector2f::Zero();
    mInput[i].mMotorState = kMS_StopHard;
    mInput[i].mPointerState = CControllerData::kPS_Tracking;
  }
  for (int i = 0; i < 4; ++i) {
    mPointerFilterX[i] = rs_new CAdaptiveInputFilter(2, 3, 0, 0.1f);
    mPointerFilterY[i] = rs_new CAdaptiveInputFilter(2, 3, 0, 0.1f);
  }

  VIWaitForRetrace();
  VIWaitForRetrace();
  VIWaitForRetrace();
  VIWaitForRetrace();
  WPADRegisterAllocator(AllocateWpadMemory, FreeWpadMemory);
  KPADInit();
  while (WPADGetStatus() != 3) {
  }

  for (int i = 0; i < 4; ++i) {
    KPADSetPosParam(i, 0.05f, 1.f);
    KPADSetDistParam(i, 0.05f, 1.f);
    KPADDisableAimingMode(i);
    WPADSetConnectCallback(i, WpadConnectCallback);
    mInfoPollTimers[i] = 1.f;
    mMotionIdleTimes[i] = 0.f;
    mButtonIdleTimes[i] = 0.f;
  }
  return true;
}

void CWiiInput::InitializeController(uint channel) {
#if NONMATCHING
  if (channel >= 4) {
#else
  if (channel > 4) {
#endif
    return;
  }
  if (int(mControllerTypes[channel]) >= 0 && int(mControllerTypes[channel]) < 3) {
    if (mMotionProcessors[channel].null()) {
      rstl::single_ptr< CWiiMotionProcessor > processor(rs_new CWiiMotionProcessor(channel, 0));
      mMotionProcessors[channel] = processor;
    }
    if (!mMotionProcessors[channel].null()) {
      mMotionProcessors[channel]->ConfigureFiltersAndCalibration(channel);
    }
    mInput[channel] = CControllerData();
    mInput[channel].mConnected = true;
  }
}

void CWiiInput::SetPointerRecenterMode(uint channel, EPointerRecenterMode mode) {
#if NONMATCHING
  if (channel >= 4) {
#else
  if (channel > 4) {
#endif
    return;
  }
  mPointerRecenterMode[channel] = mode;
}

void CWiiInput::SetControllerType(int channel, EDeviceType type) {
  mControllerTypes[channel] = type;
  mInput[channel].mConnected = type != kDT_Disconnected;
}

WPADInfo CWiiInput::GetWpadInfo(int channel) const { return mWpadInfo[channel]; }

const KPADStatus& CWiiInput::GetKpadStatus(int channel) const { return mStatus[channel]; }

void CWiiInput::CopyWpadInfo(int channel) { mWpadInfo[channel] = mWpadInfoBuf[channel]; }

void CWiiInput::WpadInfoCallback(s32 channel, s32 result) {
  if (sWiiInput != nullptr && result == 0) {
    sWiiInput->CopyWpadInfo(channel);
  }
}

void CWiiInput::WpadConnectCallback(s32 channel, s32 result) {
  if (sWiiInput != nullptr) {
    if (channel >= 1) {
      sWiiInput->QueueConnectionEvent(channel, kCE_Rejected);
    } else {
      switch (result) {
      case WPAD_ERR_OK:
        sWiiInput->QueueConnectionEvent(channel, kCE_Connected);
        sWiiInput->SetControllerType(channel, kDT_Wiimote);
        WPADSetExtensionCallback(channel, WpadExtensionCallback);
        break;
      case WPAD_ERR_NO_CONTROLLER:
        sWiiInput->QueueConnectionEvent(channel, kCE_Disconnected);
        sWiiInput->SetControllerType(channel, kDT_Disconnected);
        break;
      case WPAD_ERR_BUSY:
        sWiiInput->SetControllerType(channel, kDT_Disconnected);
        break;
      case WPAD_ERR_TRANSFER:
        sWiiInput->SetControllerType(channel, kDT_Disconnected);
        break;
      }
    }
  }
}

void CWiiInput::WpadExtensionCallback(s32 channel, s32 extension) {
  if (sWiiInput != nullptr) {
    if (channel >= 1) {
      sWiiInput->SetControllerType(channel, kDT_Wiimote);
    } else {
      switch (extension) {
      case WPAD_DEV_CORE:
        sWiiInput->QueueExtensionEvent(channel, kEE_Wiimote);
        sWiiInput->SetControllerType(channel, kDT_Wiimote);
        break;
      case WPAD_DEV_FS:
        sWiiInput->QueueExtensionEvent(channel, kEE_Nunchuk);
        sWiiInput->SetControllerType(channel, kDT_Nunchuk);
        break;
      case WPAD_DEV_CLASSIC:
        sWiiInput->QueueExtensionEvent(channel, kEE_Classic);
        sWiiInput->SetControllerType(channel, kDT_Classic);
        break;
      case 0xfd:
        sWiiInput->SetControllerType(channel, kDT_Wiimote);
        break;
      case 0xfb:
        sWiiInput->QueueExtensionEvent(channel, kEE_Unsupported);
        sWiiInput->SetControllerType(channel, kDT_Wiimote);
        break;
      default:
        sWiiInput->QueueExtensionEvent(channel, kEE_Unsupported);
        sWiiInput->SetControllerType(channel, kDT_Unsupported);
        break;
      }
    }
  }
}

void CWiiInput::SetAcceptAdditionalConnections(bool accept) {
  if (accept == mAcceptAdditionalConnections) {
    return;
  }
  mAcceptAdditionalConnections = accept;
  if (!mAcceptAdditionalConnections && mInput[0].DeviceIsPresent()) {
    WPADSetAcceptConnection(false);
  }
}

void CWiiInput::SetInput(const CControllerData& input, int channel) { mInput[channel] = input; }
