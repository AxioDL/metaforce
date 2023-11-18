#include "Runtime/Input/CInputGenerator.hpp"

#include "Runtime/Input/IController.hpp"
#include "Runtime/Input/CFinalInput.hpp"

#include "Runtime/CArchitectureQueue.hpp"

namespace metaforce {
CInputGenerator::CInputGenerator(/*COsContext& context, */ float leftDiv, float rightDiv)
/*:  x0_context(context) */ {
  x4_controller.reset(IController::Create());
  xc_leftDiv = leftDiv;
  x10_rightDiv = rightDiv;
}

void CInputGenerator::Update(float dt, CArchitectureQueue& queue) {
#if 0
  if (!x0_context.Update()) {
    return;
  }
#endif

  u32 availSlot = 0;
  bool firstController = false;
  if (x4_controller) {
    x4_controller->Poll();
    for (u32 i = 0; i < x4_controller->GetDeviceCount(); ++i) {
      auto cont = x4_controller->GetGamepadData(i);
      if (cont.DeviceIsPresent()) {
        if (i == 0) {
          firstController = true;
        }
        auto tmp = CFinalInput(i, dt, cont, xc_leftDiv, x10_rightDiv);
        if (i == 0) {
          m_lastInput = tmp;
        }
        queue.Push(MakeMsg::CreateUserInput(EArchMsgTarget::Game, tmp));
        ++availSlot;
      }

      if (x8_connectedControllers[i] != cont.DeviceIsPresent()) {
        queue.Push(MakeMsg::CreateControllerStatus(EArchMsgTarget::Game, i, cont.DeviceIsPresent()));
        x8_connectedControllers[i] = cont.DeviceIsPresent();
      }
    }
  }
#if 0
  if (firstController) {
    queue.Push(MakeMsg::CreateUserInput(EArchMsgTarget::Game, CFinalInput(availSlot, dt, x0_osContext)));
  } else {
    queue.Push(MakeMsg::CreateUserInput(EArchMsgTarget::Game, CFinalInput(0, dt, x0_osContext)));
  }
#endif
}
} // namespace metaforce::WIP
