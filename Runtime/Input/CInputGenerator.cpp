#include "Runtime/Input/CInputGenerator.hpp"

#include "Runtime/CArchitectureMessage.hpp"
#include "Runtime/CArchitectureQueue.hpp"
#include "imgui/magic_enum.hpp"
namespace metaforce {

void CInputGenerator::Update(float dt, CArchitectureQueue& queue) {
  if (m_firstFrame) {
    m_firstFrame = false;
    return;
  }

  const CFinalInput& kbInput = getFinalInput(0, dt);
  queue.Push(MakeMsg::CreateUserInput(EArchMsgTarget::Game, kbInput));

/* Dolphin controllers next */
//  for (int i = 0; i < 4; ++i) {
//    bool connected;
//    EStatusChange change = m_dolphinCb.getStatusChange(i, connected);
//    if (change != EStatusChange::NoChange)
//      queue.Push(MakeMsg::CreateControllerStatus(EArchMsgTarget::Game, i, connected));
//    if (connected) {
//      CFinalInput input = m_dolphinCb.getFinalInput(i, dt, m_leftDiv, m_rightDiv);
//      if (i == 0) /* Merge KB input with first controller */
//      {
//        input |= kbInput;
//        kbUsed = true;
//      }
//      m_lastUpdate = input;
//      queue.Push(MakeMsg::CreateUserInput(EArchMsgTarget::Game, input));
//    }
//  }

//  /* Send straight keyboard input if no first controller present */
//  if (!kbUsed) {
//    m_lastUpdate = kbInput;
//  }
}

void CInputGenerator::controllerAxis(uint32_t which, aurora::ControllerAxis axis, int16_t value) noexcept {
  s32 idx = aurora::get_controller_player_index(which);
  if (idx < 0) {
    return;
  }

  switch (axis) {
  case aurora::ControllerAxis::LeftY:
  case aurora::ControllerAxis::RightY:
    /* Value is inverted compared to what we expect on the Y axis */
    value = int16_t(-(value + 1));
    [[fallthrough]];
  case aurora::ControllerAxis::LeftX:
  case aurora::ControllerAxis::RightX:
    value /= int16_t(256);
    break;
  case aurora::ControllerAxis::TriggerLeft:
  case aurora::ControllerAxis::TriggerRight:
    value /= int16_t(128);
    break;
  default:
    break;
  }

  m_state[idx].m_axes[size_t(axis)] = value;
}

const CFinalInput& CInputGenerator::getFinalInput(unsigned int idx, float dt) {
  auto input = CFinalInput(idx, dt, m_data, m_lastUpdate);
  // Merge controller input with kb/m input
  auto state = m_state[idx];
  state.clamp();
  input |= CFinalInput(idx, dt, state, m_lastUpdate, m_leftDiv, m_rightDiv);
  m_lastUpdate = input;
  return m_lastUpdate;
}

} // namespace metaforce
