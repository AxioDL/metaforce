#ifndef __RETRO_CINPUTGENERATOR_HPP__
#define __RETRO_CINPUTGENERATOR_HPP__

#include <boo/boo.hpp>

namespace Retro
{



class CInputGenerator
{
    struct WindowCallback : boo::IWindowCallback
    {
        void mouseDown(const SWindowCoord& coord, EMouseButton button, EModifierKey mods)
        {
            fprintf(stderr, "Mouse Down %d (%f,%f)\n", button, coord.norm[0], coord.norm[1]);
        }
        void mouseUp(const SWindowCoord& coord, EMouseButton button, EModifierKey mods)
        {
            fprintf(stderr, "Mouse Up %d (%f,%f)\n", button, coord.norm[0], coord.norm[1]);
        }
        void mouseMove(const SWindowCoord& coord)
        {
            //fprintf(stderr, "Mouse Move (%f,%f)\n", coord.norm[0], coord.norm[1]);
        }
        void scroll(const SWindowCoord& coord, const SScrollDelta& scroll)
        {
            fprintf(stderr, "Mouse Scroll (%f,%f) (%f,%f)\n", coord.norm[0], coord.norm[1], scroll.delta[0], scroll.delta[1]);
        }

        void touchDown(const STouchCoord& coord, uintptr_t tid)
        {
            //fprintf(stderr, "Touch Down %16lX (%f,%f)\n", tid, coord.coord[0], coord.coord[1]);
        }
        void touchUp(const STouchCoord& coord, uintptr_t tid)
        {
            //fprintf(stderr, "Touch Up %16lX (%f,%f)\n", tid, coord.coord[0], coord.coord[1]);
        }
        void touchMove(const STouchCoord& coord, uintptr_t tid)
        {
            //fprintf(stderr, "Touch Move %16lX (%f,%f)\n", tid, coord.coord[0], coord.coord[1]);
        }

        void charKeyDown(unsigned long charCode, EModifierKey mods, bool isRepeat)
        {

        }
        void charKeyUp(unsigned long charCode, EModifierKey mods)
        {

        }
        void specialKeyDown(ESpecialKey key, EModifierKey mods, bool isRepeat)
        {

        }
        void specialKeyUp(ESpecialKey key, EModifierKey mods)
        {

        }
        void modKeyDown(EModifierKey mod, bool isRepeat)
        {

        }
        void modKeyUp(EModifierKey mod)
        {

        }

    } m_windowCb;

    struct DolphinSmashAdapterCallback : boo::IDolphinSmashAdapterCallback
    {
        void controllerConnected(unsigned idx, boo::EDolphinControllerType)
        {
            printf("CONTROLLER %u CONNECTED\n", idx);
        }
        void controllerDisconnected(unsigned idx, boo::EDolphinControllerType)
        {
            printf("CONTROLLER %u DISCONNECTED\n", idx);
        }
        void controllerUpdate(unsigned idx, boo::EDolphinControllerType,
                              const boo::DolphinControllerState& state)
        {
            printf("CONTROLLER %u UPDATE %d %d\n", idx, state.m_leftStick[0], state.m_leftStick[1]);
        }
    };

    class ApplicationDeviceFinder : public boo::DeviceFinder
    {
        std::unique_ptr<boo::DolphinSmashAdapter> smashAdapter = NULL;
        DolphinSmashAdapterCallback m_cb;
    public:
        ApplicationDeviceFinder()
        : boo::DeviceFinder({typeid(boo::DolphinSmashAdapter)})
        {}
        void deviceConnected(boo::DeviceToken& tok)
        {
            if (!smashAdapter)
            {
                smashAdapter.reset(dynamic_cast<boo::DolphinSmashAdapter*>(tok.openAndGetDevice()));
                smashAdapter->setCallback(&m_cb);
                smashAdapter->startRumble(0);
            }
        }
        void deviceDisconnected(boo::DeviceToken&, boo::DeviceBase* device)
        {
            if (smashAdapter.get() == device)
                smashAdapter.reset(nullptr);
        }
    };

};

}

#endif // __RETRO_CINPUTGENERATOR_HPP__
