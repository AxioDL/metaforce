#ifndef __RETRO_CKEYBOARDMOUSECONTROLLER_HPP__
#define __RETRO_CKEYBOARDMOUSECONTROLLER_HPP__

#include <boo/boo.hpp>

namespace Retro
{

struct CKeyboardMouseControllerData
{
    bool m_charKeys[256] = {};
    bool m_specialKeys[26] = {};
    bool m_mouseButtons[6] = {};
    boo::IWindowCallback::EModifierKey m_modMask = boo::IWindowCallback::MKEY_NONE;
    boo::IWindowCallback::SWindowCoord m_mouseCoord;
    boo::IWindowCallback::SScrollDelta m_accumScroll;
};

}

#endif // __RETRO_CKEYBOARDMOUSECONTROLLER_HPP__
