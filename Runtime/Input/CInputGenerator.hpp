#ifndef __RETRO_CINPUTGENERATOR_HPP__
#define __RETRO_CINPUTGENERATOR_HPP__

#include <atomic>
#include <boo/boo.hpp>
#include "CFinalInput.hpp"
#include "CKeyboardMouseController.hpp"

namespace Retro
{
class CArchitectureQueue;

class CInputGenerator : public boo::DeviceFinder
{
    enum EStatusChange
    {
        StatusNoChange = 0,
        StatusConnected = 1,
        StatusDisconnected = 2
    };

    /* When the sticks are used as logical (digital) input,
     * these thresholds determine the vector magnitude indicating
     * the logical state */
    float m_leftDiv;
    float m_rightDiv;
public:
    CInputGenerator(float leftDiv, float rightDiv)
    : boo::DeviceFinder({typeid(boo::DolphinSmashAdapter)}),
      m_leftDiv(leftDiv),
      m_rightDiv(rightDiv) {}

    /* Keyboard and mouse events are delivered on the main game
     * thread as part of the app's main event loop. The OS is responsible
     * for buffering events in its own way, then boo flushes the buffer
     * at the start of each frame, invoking these methods. No atomic locking
     * is necessary, only absolute state tracking. */
    struct WindowCallback : boo::IWindowCallback
    {
        CKeyboardMouseControllerData m_data;

        void mouseDown(const SWindowCoord&, EMouseButton button, EModifierKey)
        {
            m_data.m_mouseButtons[button] = true;
        }
        void mouseUp(const SWindowCoord&, EMouseButton button, EModifierKey)
        {
            m_data.m_mouseButtons[button] = false;
        }
        void mouseMove(const SWindowCoord& coord)
        {
            m_data.m_mouseCoord = coord;
        }
        void scroll(const SWindowCoord&, const SScrollDelta& scroll)
        {
            m_data.m_accumScroll += scroll;
        }

        void charKeyDown(unsigned long charCode, EModifierKey, bool)
        {
            charCode = tolower(charCode);
            if (charCode > 255)
                return;
            m_data.m_charKeys[charCode] = true;
        }
        void charKeyUp(unsigned long charCode, EModifierKey mods)
        {
            charCode = tolower(charCode);
            if (charCode > 255)
                return;
            m_data.m_charKeys[charCode] = false;
        }
        void specialKeyDown(ESpecialKey key, EModifierKey, bool)
        {
            m_data.m_specialKeys[key] = true;
        }
        void specialKeyUp(ESpecialKey key, EModifierKey)
        {
            m_data.m_specialKeys[key] = false;
        }
        void modKeyDown(EModifierKey mod, bool)
        {
            m_data.m_modMask = EModifierKey(m_data.m_modMask | mod);
        }
        void modKeyUp(EModifierKey mod)
        {
            m_data.m_modMask = EModifierKey(m_data.m_modMask & ~mod);
        }

        void reset()
        {
            m_data.m_accumScroll.zeroOut();
        }

        CFinalInput m_lastUpdate;
        const CFinalInput& getFinalInput(unsigned idx, float dt)
        {
            m_lastUpdate = CFinalInput(idx, dt, m_data, m_lastUpdate);
            return m_lastUpdate;
        }
    } m_windowCb;

    /* Input via the smash adapter is received asynchronously on a USB
     * report thread. This class atomically exchanges that data to the
     * game thread as needed */
    struct DolphinSmashAdapterCallback : boo::IDolphinSmashAdapterCallback
    {
        std::atomic<EStatusChange> m_statusChanges[4];
        bool m_connected[4] = {};
        boo::DolphinControllerState m_states[4];
        std::mutex m_stateLock;
        void controllerConnected(unsigned idx, boo::EDolphinControllerType)
        {
            /* Controller thread */
            m_statusChanges[idx].store(StatusConnected);
        }
        void controllerDisconnected(unsigned idx, boo::EDolphinControllerType)
        {
            /* Controller thread */
            std::unique_lock<std::mutex> lk(m_stateLock);
            m_statusChanges[idx].store(StatusDisconnected);
            m_states[idx].reset();
        }
        void controllerUpdate(unsigned idx, boo::EDolphinControllerType,
                              const boo::DolphinControllerState& state)
        {
            /* Controller thread */
            std::unique_lock<std::mutex> lk(m_stateLock);
            m_states[idx] = state;
        }

        CFinalInput m_lastUpdates[4];
        const CFinalInput& getFinalInput(unsigned idx, float dt,
                                         float leftDiv, float rightDiv)
        {
            /* Game thread */
            std::unique_lock<std::mutex> lk(m_stateLock);
            boo::DolphinControllerState state = m_states[idx];
            lk.unlock();
            state.clamp(); /* PADClamp equivalent */
            m_lastUpdates[idx] = CFinalInput(idx, dt, state, m_lastUpdates[idx], leftDiv, rightDiv);
            return m_lastUpdates[idx];
        }
        EStatusChange getStatusChange(unsigned idx, bool& connected)
        {
            /* Game thread */
            EStatusChange ch = m_statusChanges[idx].exchange(StatusNoChange);
            if (ch == StatusConnected)
                m_connected[idx] = true;
            else if (ch == StatusDisconnected)
                m_connected[idx] = false;
            connected = m_connected[idx];
            return ch;
        }
    } m_dolphinCb;

    /* Device connection/disconnection events are handled on a separate thread
     * using the relevant OS API. This thread blocks in a loop until an event is
     * received. Device pointers should only be manipulated by this thread using
     * the deviceConnected() and deviceDisconnected() callbacks. */
    std::unique_ptr<boo::DolphinSmashAdapter> smashAdapter = NULL;
    void deviceConnected(boo::DeviceToken& tok)
    {
        /* Device listener thread */
        if (!smashAdapter)
        {
            smashAdapter.reset(dynamic_cast<boo::DolphinSmashAdapter*>(tok.openAndGetDevice()));
            smashAdapter->setCallback(&m_dolphinCb);
        }
    }
    void deviceDisconnected(boo::DeviceToken&, boo::DeviceBase* device)
    {
        if (smashAdapter.get() == device)
            smashAdapter.reset(nullptr);
    }

    /* This is where the game thread enters */
    void Update(float dt, CArchitectureQueue& queue);

};

}

#endif // __RETRO_CINPUTGENERATOR_HPP__
