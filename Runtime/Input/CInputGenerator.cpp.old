#include "Runtime/Input/CInputGenerator.hpp"

#include "Runtime/CArchitectureMessage.hpp"
#include "Runtime/CArchitectureQueue.hpp"

#include <magic_enum.hpp>

namespace metaforce {
static logvisor::Module Log("CInputGenerator");

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

void CInputGenerator::controllerAdded(uint32_t which) noexcept {
  s32 player = aurora::get_controller_player_index(which);
  if (player < 0) {
    player = 0;
    aurora::set_controller_player_index(which, 0);
  }

  if (m_state[player].m_hasRumble && m_state[player].m_isGamecube) {
    /* The GameCube controller can get stuck in a state where it's always rumbling if the game crashes
     * (this can actually happen on hardware in certain cases)
     * so lets toggle the motors to ensure they're off, this happens so quickly the player doesn't notice
     */
    aurora::controller_rumble(which, 1, 1);
    aurora::controller_rumble(which, 0, 0);
  }
  m_state[player] =
      SAuroraControllerState(which, aurora::is_controller_gamecube(which), aurora::controller_has_rumble(which));
}

void CInputGenerator::controllerRemoved(uint32_t which) noexcept {
  auto it = std::find_if(m_state.begin(), m_state.end(), [&which](const auto& s) { return s.m_which == which; });
  if (it == m_state.end()) {
    return;
  }

  (*it) = SAuroraControllerState();
}

void CInputGenerator::controllerButton(uint32_t which, aurora::ControllerButton button, bool pressed) noexcept {
  s32 player = aurora::get_controller_player_index(which);
  if (player < 0) {
    return;
  }
  m_state[player].m_btns.set(size_t(button), pressed);
}

void CInputGenerator::controllerAxis(uint32_t which, aurora::ControllerAxis axis, int16_t value) noexcept {
  s32 player = aurora::get_controller_player_index(which);
  if (player < 0) {
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

  m_state[player].m_axes[size_t(axis)] = value;
}

void CInputGenerator::SetMotorState(EIOPort port, EMotorState state) {
  if (m_state[size_t(port)].m_hasRumble && m_state[size_t(port)].m_isGamecube) {
    if (state == EMotorState::Rumble) {
      aurora::controller_rumble(m_state[size_t(port)].m_which, 1, 1);
    } else if (state == EMotorState::Stop) {
      aurora::controller_rumble(m_state[size_t(port)].m_which, 0, 1);
    } else if (state == EMotorState::StopHard) {
      aurora::controller_rumble(m_state[size_t(port)].m_which, 0, 0);
    }
  } // TODO: Figure out good intensity values for generic controllers with rumble, support HAPTIC?
}

const CFinalInput& CInputGenerator::getFinalInput(unsigned int idx, float dt) {
#if 0
  auto input = CFinalInput(idx, dt, m_data, m_lastUpdate);
  // Merge controller input with kb/m input
  auto state = m_state[idx];
  state.clamp();
  input |= CFinalInput(idx, dt, state, m_lastUpdate, m_leftDiv, m_rightDiv);
  m_lastUpdate = input;
#endif
  return m_lastUpdate;
}

} // namespace metaforce
