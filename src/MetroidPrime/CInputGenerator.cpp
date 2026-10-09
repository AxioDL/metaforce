#include "MetroidPrime/CInputGenerator.hpp"

#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/Decode.hpp"

#include "Kyoto/Basics/COsContext.hpp"

#if VERSION >= VERSION_R3IJ_00
CInputGenerator::CInputGenerator(COsContext* context, float leftDivisor, float rightDivisor)
: mContext(context), mLeftDivisor(leftDivisor), mRightDivisor(rightDivisor) {
  for (uint i = 0; i < 4; ++i) {
    mConnectedControllers[i] = false;
  }
}

bool CInputGenerator::Update(float dt, CArchitectureQueue& queue) {
  if (!mContext->Update()) {
    return false;
  }

  if (gpController != nullptr) {
    const int count = gpController->GetDeviceCount();
    gpController->Update(dt);
    for (int i = 0; i < count; ++i) {
      const CControllerData& data = gpController->GetInput(i);
      if (data.DeviceIsPresent() || data.GetForceInputEvent()) {
        const CFinalInput input(i, dt, data, 1);
        const CArchitectureMessage msg = MakeMsg::CreateUserInput(kAMT_Game, input);
        queue.Push(msg);
      }

      const bool connected = data.DeviceIsPresent();
      if (mConnectedControllers[i] != connected) {
        const CArchitectureMessage msg = MakeMsg::CreateControllerStatus(kAMT_Game, i, connected);
        queue.Push(msg);
        mConnectedControllers[i] = connected;
      }
    }
  }
  return true;
}
#else
CInputGenerator::CInputGenerator(COsContext* ctx, float leftDiv, float rightDiv)
: mContext(ctx), mController(IController::Create(*ctx)), mLeftDiv(leftDiv), mRightDiv(rightDiv) {
  for (uint i = 0; i <= kIOP_Player4; ++i) {
    mConnectedControllers[i] = false;
  }
}

bool CInputGenerator::Update(float dt, CArchitectureQueue& queue) {
  int availSlot = 0;
  if (!mContext->Update()) {
    return false;
  }

  bool firstController = false;
  if (!mController.null()) {
    const int count = mController->GetDeviceCount();
    mController->Poll();
    for (int i = 0; i < count; ++i) {
      const CControllerGamepadData& cont = mController->GetGamepadData(i);
      if (cont.DeviceIsPresent()) {
        if (i == 0) {
          firstController = true;
        }
        {
          const CFinalInput input(i, dt, cont, mLeftDiv, mRightDiv);
          const CArchitectureMessage msg = MakeMsg::CreateUserInput(kAMT_Game, input);
          queue.Push(msg);
        }
        availSlot++;
      }

      const bool connected = cont.DeviceIsPresent();
      if (mConnectedControllers[i] != connected) {
        const CArchitectureMessage msg = MakeMsg::CreateControllerStatus(kAMT_Game, i, connected);
        queue.Push(msg);
        mConnectedControllers[i] = connected;
      }
    }
  }

  if (!firstController) {
    const CArchitectureMessage msg =
        MakeMsg::CreateUserInput(kAMT_Game, CFinalInput(0, dt, *mContext));
    queue.Push(msg);
  } else {
    const CArchitectureMessage msg =
        MakeMsg::CreateUserInput(kAMT_Game, CFinalInput(availSlot, dt, *mContext));
    queue.Push(msg);
  }
  return true;
}

#endif
