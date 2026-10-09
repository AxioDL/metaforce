#ifndef _ICONTROLLER
#define _ICONTROLLER

#include <types.h>

#include "Kyoto/Input/InputTypes.hpp"
#if VERSION >= VERSION_R3IJ_00
#include "Kyoto/Input/CControllerData.hpp"

class COsContext;
class IController {
protected:
  static const float kAbsoluteMinimum;
  static const float kAbsoluteMaximum;
  static const float kRelativeMinimum;
  static const float kRelativeMaximum;

public:
  enum EDeviceType {
    kDT_Wiimote,
    kDT_Nunchuk,
    kDT_Classic,
    kDT_Unsupported,
    kDT_Disconnected,
  };

  enum EPointerRecenterMode {
    kPRM_Hold,
    kPRM_Both,
    kPRM_X,
    kPRM_Y,
  };

  virtual ~IController();
  virtual void Poll() = 0;
  virtual void Update(float dt) = 0;
  virtual uint GetDeviceCount() const = 0;
  virtual CControllerData& GetInput(uint channel) = 0;
  virtual void SetInput(const CControllerData& input, int channel) = 0;
  virtual EDeviceType GetControllerType(int channel) const = 0;
  virtual void SetMotorEnabled(int channel, bool enabled) = 0;
  virtual void SetMotorState(int channel, EMotorState state) = 0;
  virtual void SetAcceptAdditionalConnections(bool accept) = 0;
  virtual CControllerData::EPointerState GetPointerState(int channel) const = 0;
  virtual uint GetPointerValidFrameCount(int channel) const = 0;
  virtual uint GetPointerInvalidFrameCount(int channel) const = 0;
  virtual CVector2f GetPointerPosition(int channel) const = 0;
  virtual void SetPointerRecenterMode(uint channel, EPointerRecenterMode mode) = 0;
  virtual bool HasMotionActivity(uint channel) const = 0;

  IController();
  static IController* Create(const COsContext& ctx);
};
CHECK_SIZEOF(IController, 0x4)

#else
#include "Kyoto/Input/CControllerGamepadData.hpp"

class COsContext;
class IController {
protected:
  static const float kAbsoluteMinimum;
  static const float kAbsoluteMaximum;
  static const float kRelativeMinimum;
  static const float kRelativeMaximum;

public:
  IController();
  virtual ~IController();
  virtual void Poll() = 0;
  virtual uint GetDeviceCount() const = 0;
  virtual CControllerGamepadData& GetGamepadData(int controller) = 0;
  virtual uint GetControllerType(int) const = 0;
  virtual void SetMotorState(EIOPort port, EMotorState state) = 0;

  static IController* Create(const COsContext& ctx);
};

#endif

#endif // _ICONTROLLER
