#include "Runtime/Input/NewCInputGenerator.hpp"

#include "Runtime/Input/IController.hpp"
#include "Runtime/Input/CFinalInput.hpp"

#include "Runtime/CArchitectureQueue.hpp"

namespace metaforce::WIP {
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

  bool firstController = false;
  if (x4_controller) {
    x4_controller->Poll();
    for (u32 i = 0; i < x4_controller->GetDeviceCount(); ++i) {
      auto cont = x4_controller->GetGamepadData(i);
      if (!cont.DeviceIsPresent()) {
        continue;
      }
      if (i == 0) {
        firstController = true;
      }

      queue.Push(MakeMsg::CreateUserInput(EArchMsgTarget::Game, CFinalInput(i, dt, cont, xc_leftDiv, x10_rightDiv)));
      // TODO: Finish
    }
  }
}
} // namespace metaforce::WIP
