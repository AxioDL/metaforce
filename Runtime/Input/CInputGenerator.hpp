#pragma once

#include <array>
#include <atomic>
#include <mutex>

#include "Runtime/Input/InputTypes.hpp"
#include "Runtime/Input/CFinalInput.hpp"
#include "Runtime/Input/CKeyboardMouseController.hpp"

namespace metaforce {
class CArchitectureQueue;

class CInputGenerator /*: public boo::DeviceFinder*/ {
  enum class EStatusChange { NoChange = 0, Connected = 1, Disconnected = 2 };

  /* When the sticks are used as logical (digital) input,
   * these thresholds determine the vector magnitude indicating
   * the logical state */
  float m_leftDiv;
  float m_rightDiv;
  CKeyboardMouseControllerData m_data;
  std::array<SAuroraControllerState, 4> m_state;

  CFinalInput m_lastUpdate;
  const CFinalInput& getFinalInput(unsigned idx, float dt);

  bool m_firstFrame = true;

public:
  CInputGenerator(float leftDiv, float rightDiv)
  : /*boo::DeviceFinder({dev_typeid(DolphinSmashAdapter)}),*/ m_leftDiv(leftDiv), m_rightDiv(rightDiv) {}

//  ~CInputGenerator() override {
//    if (smashAdapter) {
//      smashAdapter->setCallback(nullptr);
//      smashAdapter->closeDevice();
//    }
//  }

  void controllerAdded(uint32_t which) noexcept;

  void controllerRemoved(uint32_t which) noexcept;

  void controllerButton(uint32_t which, aurora::ControllerButton button, bool pressed) noexcept;

  void controllerAxis(uint32_t which, aurora::ControllerAxis axis, int16_t value) noexcept;


  /* Keyboard and mouse events are delivered on the main game
   * thread as part of the app's main event loop. The OS is responsible
   * for buffering events in its own way, then boo flushes the buffer
   * at the start of each frame, invoking these methods. No atomic locking
   * is necessary, only absolute state tracking. */

  void mouseDown(const SWindowCoord&, EMouseButton button, EModifierKey) {
    m_data.m_mouseButtons[size_t(button)] = true;
  }
  void mouseUp(const SWindowCoord&, EMouseButton button, EModifierKey) {
    m_data.m_mouseButtons[size_t(button)] = false;
  }
  void mouseMove(const SWindowCoord& coord) { m_data.m_mouseCoord = coord; }
  void scroll(const SWindowCoord&, const SScrollDelta& scroll) { m_data.m_accumScroll += scroll; }

  void charKeyDown(uint8_t charCode, aurora::ModifierKey, bool) {
    charCode = tolower(charCode);
    if (charCode > 255)
      return;
    m_data.m_charKeys[charCode] = true;
  }
  void charKeyUp(uint8_t charCode, aurora::ModifierKey mods) {
    charCode = tolower(charCode);
    if (charCode > 255)
      return;
    m_data.m_charKeys[charCode] = false;
  }
  void specialKeyDown(aurora::SpecialKey key, aurora::ModifierKey, bool) { m_data.m_specialKeys[size_t(key)] = true; }
  void specialKeyUp(aurora::SpecialKey key, aurora::ModifierKey) { m_data.m_specialKeys[size_t(key)] = false; }
  void modKeyDown(aurora::ModifierKey mod, bool) { m_data.m_modMask = m_data.m_modMask | mod; }
  void modKeyUp(aurora::ModifierKey mod) { m_data.m_modMask = m_data.m_modMask & ~mod; }

  void reset() { m_data.m_accumScroll.zeroOut(); }

//  /* Input via the smash adapter is received asynchronously on a USB
//   * report thread. This class atomically exchanges that data to the
//   * game thread as needed */
//  struct DolphinSmashAdapterCallback : boo::IDolphinSmashAdapterCallback {
//    std::array<std::atomic<EStatusChange>, 4> m_statusChanges;
//    std::array<bool, 4> m_connected{};
//    std::array<boo::DolphinControllerState, 4> m_states;
//    std::mutex m_stateLock;
//    void controllerConnected(unsigned idx, boo::EDolphinControllerType) override {
//      /* Controller thread */
//      m_statusChanges[idx].store(EStatusChange::Connected);
//    }
//    void controllerDisconnected(unsigned idx) override {
//      /* Controller thread */
//      std::unique_lock lk{m_stateLock};
//      m_statusChanges[idx].store(EStatusChange::Disconnected);
//      m_states[idx].reset();
//    }
//    void controllerUpdate(unsigned idx, boo::EDolphinControllerType,
//                          const boo::DolphinControllerState& state) override {
//      /* Controller thread */
//      std::unique_lock lk{m_stateLock};
//      m_states[idx] = state;
//    }
//
//    std::array<CFinalInput, 4> m_lastUpdates;
//    const CFinalInput& getFinalInput(unsigned idx, float dt, float leftDiv, float rightDiv) {
//      /* Game thread */
//      std::unique_lock lk{m_stateLock};
//      boo::DolphinControllerState state = m_states[idx];
//      lk.unlock();
//      state.clamp(); /* PADClamp equivalent */
//      m_lastUpdates[idx] = CFinalInput(idx, dt, state, m_lastUpdates[idx], leftDiv, rightDiv);
//      return m_lastUpdates[idx];
//    }
//    EStatusChange getStatusChange(unsigned idx, bool& connected) {
//      /* Game thread */
//      EStatusChange ch = m_statusChanges[idx].exchange(EStatusChange::NoChange);
//      if (ch == EStatusChange::Connected)
//        m_connected[idx] = true;
//      else if (ch == EStatusChange::Disconnected)
//        m_connected[idx] = false;
//      connected = m_connected[idx];
//      return ch;
//    }
//  } m_dolphinCb;

//  /* Device connection/disconnection events are handled on a separate thread
//   * using the relevant OS API. This thread blocks in a loop until an event is
//   * received. Device pointers should only be manipulated by this thread using
//   * the deviceConnected() and deviceDisconnected() callbacks. */
//  std::shared_ptr<boo::DolphinSmashAdapter> smashAdapter;
//  void deviceConnected(boo::DeviceToken& tok) override {
//    /* Device listener thread */
//    if (!smashAdapter) {
//      auto dev = tok.openAndGetDevice();
//      if (dev && dev->getTypeHash() == dev_typeid(DolphinSmashAdapter)) {
//        smashAdapter = std::static_pointer_cast<boo::DolphinSmashAdapter>(tok.openAndGetDevice());
//        smashAdapter->setCallback(&m_dolphinCb);
//      }
//    }
//  }
//  void deviceDisconnected(boo::DeviceToken&, boo::DeviceBase* device) override {
//    if (smashAdapter.get() == device)
//      smashAdapter.reset();
//  }
  void SetMotorState(EIOPort port, EMotorState state);
  void ControlAllMotors(const std::array<EMotorState, 4>& states) {
    for (u32 i = 0; i <= size_t(EIOPort::Player4); ++i ) {
      SetMotorState(EIOPort(i), states[i]);
    }
  }

  /* This is where the game thread enters */
  void Update(float dt, CArchitectureQueue& queue);
  CFinalInput GetLastInput() const { return m_lastUpdate; }
};

} // namespace metaforce
