#include "Runtime/Input/CInputGenerator.hpp"

#include "Runtime/CArchitectureMessage.hpp"
#include "Runtime/CArchitectureQueue.hpp"

namespace metaforce {

void CInputGenerator::Update(float dt, CArchitectureQueue& queue) {
  if (m_firstFrame) {
    m_firstFrame = false;
    return;
  }

#ifdef __SWITCH__
  {
    CLibnxControllerData data;
    data.keysDown = hidKeysDown(CONTROLLER_P1_AUTO) | hidKeysHeld(CONTROLLER_P1_AUTO);
    hidJoystickRead(&data.lStick, CONTROLLER_P1_AUTO, JOYSTICK_LEFT);
    hidJoystickRead(&data.rStick, CONTROLLER_P1_AUTO, JOYSTICK_RIGHT);
    CFinalInput input(0, dt, data, m_lastUpdate);
    queue.Push(MakeMsg::CreateUserInput(EArchMsgTarget::Game, input));
    return;
  }
#endif

  /* Keyboard/Mouse first */
  CFinalInput kbInput = getFinalInput(0, dt);
  bool kbUsed = false;

  /* Dolphin controllers next */
  for (int i = 0; i < 4; ++i) {
    bool connected;
    EStatusChange change = m_dolphinCb.getStatusChange(i, connected);
    if (change != EStatusChange::NoChange)
      queue.Push(MakeMsg::CreateControllerStatus(EArchMsgTarget::Game, i, connected));
    if (connected) {
      CFinalInput input = m_dolphinCb.getFinalInput(i, dt, m_leftDiv, m_rightDiv);
      if (i == 0) /* Merge KB input with first controller */
      {
        input |= kbInput;
        kbUsed = true;
      }
      queue.Push(MakeMsg::CreateUserInput(EArchMsgTarget::Game, input));
    }
  }

  /* Send straight keyboard input if no first controller present */
  if (!kbUsed)
    queue.Push(MakeMsg::CreateUserInput(EArchMsgTarget::Game, kbInput));
}

} // namespace metaforce
