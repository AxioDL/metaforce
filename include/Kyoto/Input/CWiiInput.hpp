#ifndef _CWIIINPUT
#define _CWIIINPUT

#include "Kyoto/Input/IController.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

#include <revolution/kpad.h>

class CScalarInputFilter;
class CWiiMotionProcessor;

// Inferred class and method names; the layout and virtual order come from Trilogy.
class CWiiInput : public IController {
public:
  ~CWiiInput() override;
  void Poll() override;
  void Update(float dt) override;
  uint GetDeviceCount() const override;
  CControllerData& GetInput(uint channel) override;
  void SetInput(const CControllerData& input, int channel) override;
  EDeviceType GetControllerType(int channel) const override;
  void SetMotorEnabled(int channel, bool enabled) override;
  void SetMotorState(int channel, EMotorState state) override;
  void SetAcceptAdditionalConnections(bool accept) override;
  CControllerData::EPointerState GetPointerState(int channel) const override;
  uint GetPointerValidFrameCount(int channel) const override;
  uint GetPointerInvalidFrameCount(int channel) const override;
  CVector2f GetPointerPosition(int channel) const override;
  void SetPointerRecenterMode(uint channel, EPointerRecenterMode mode) override;
  bool HasMotionActivity(uint channel) const override;
  virtual float GetMotionIdleTime(uint channel) const;
  virtual bool HasButtonActivity(uint channel) const;
  virtual float GetButtonIdleTime(uint channel) const;
  virtual bool IsControllerIdle(uint channel) const;

  CWiiInput();
  bool Initialize();
  bool IsPointerValid(int channel) const;
  bool IsPointerDevicePresent(int channel) const;
  WPADInfo GetWpadInfo(int channel) const;
  const KPADStatus& GetKpadStatus(int channel) const;

private:
  // Event names inferred from callback writes and deferred dispatch.
  enum EConnectionEvent { kCE_None, kCE_Connected, kCE_Disconnected, kCE_Rejected };
  enum EExtensionEvent { kEE_None, kEE_Wiimote, kEE_Nunchuk, kEE_Classic, kEE_Unsupported };

  // Only construction and copying are observed for this 0x21-byte record.
  struct SUnknownInputData {
    uchar mData[32];
    bool mFlag1 : 1;
    bool mFlag2 : 1;
    bool mFlag3 : 1;

    SUnknownInputData() : mFlag1(false), mFlag2(false), mFlag3(false) {}
  };

  static void* AllocateWpadMemory(u32 size);
  static int FreeWpadMemory(void* memory);
  static void WpadInfoCallback(s32 channel, s32 result);
  static void WpadConnectCallback(s32 channel, s32 result);
  static void WpadExtensionCallback(s32 channel, s32 extension);

  void QueueConnectionEvent(int channel, EConnectionEvent event);
  void QueueExtensionEvent(int channel, EExtensionEvent event);
  void ProcessConnectionEvents();
  void ApplyPointerDistanceScale(int channel);
  void UpdatePointerState(int channel);
  void ProcessControllerInput(int channel);
  void UpdateAnalogInput(float threshold, int channel, int axis, CControllerButton& negativeButton,
                         CControllerButton& positiveButton);
  void UpdateContinuousAngleAxis(int channel, int axis);
  void ClearButtonEvents(int channel);
  void UpdateDigitalInput(int channel);
  void UpdateButton(uint heldMask, CControllerButton& button, uint mask);
  void UpdateMotionButton(int channel, CControllerButton& button, uint mask);
  void UpdateMotionButtons(int channel);
  void UpdateSwingButton(int channel, CControllerButton& button, uint mask);
  void UpdateSwingButtons(int channel);
  void UpdateIdleTimes(uint channel, float dt);
  void InitializeController(uint channel);
  void SetControllerType(int channel, EDeviceType type);
  void CopyWpadInfo(int channel);

  rstl::reserved_vector< KPADStatus, 4 > mStatus;
  rstl::reserved_vector< EDeviceType, 4 > mControllerTypes;
  rstl::reserved_vector< CControllerData, 4 > mInput;
  rstl::reserved_vector< EPointerRecenterMode, 4 > mPointerRecenterMode;
  rstl::reserved_vector< int, 4 > mPointerReacquireFrames;
  rstl::reserved_vector< rstl::single_ptr< CWiiMotionProcessor >, 4 > mMotionProcessors;
  rstl::reserved_vector< SUnknownInputData, 4 > mUnknownInputData;
  rstl::reserved_vector< WPADInfo, 4 > mWpadInfo;
  rstl::reserved_vector< WPADInfo, 4 > mWpadInfoBuf;
  rstl::reserved_vector< float, 4 > mInfoPollTimers;
  rstl::reserved_vector< float, 4 > mMotionIdleTimes;
  rstl::reserved_vector< float, 4 > mButtonIdleTimes;
  rstl::reserved_vector< EConnectionEvent, 4 > mPendingConnectionEvents;
  rstl::reserved_vector< EExtensionEvent, 4 > mPendingExtensionEvents;
  rstl::reserved_vector< rstl::single_ptr< CScalarInputFilter >, 4 > mPointerFilterX;
  rstl::reserved_vector< rstl::single_ptr< CScalarInputFilter >, 4 > mPointerFilterY;
  uint mMotorEnabledFlags : 4;
  float mPointerMinDistance;
  float mPointerMaxDistance;
  float mPointerMinScale;
  float mPointerMaxScale;
  bool mAcceptAdditionalConnections;
};
CHECK_SIZEOF(CWiiInput, 0xc90)

#endif // _CWIIINPUT
