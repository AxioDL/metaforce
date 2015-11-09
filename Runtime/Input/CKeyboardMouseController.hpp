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
    boo::EModifierKey m_modMask = boo::MKEY_NONE;
    boo::SWindowCoord m_mouseCoord;
    boo::SScrollDelta m_accumScroll;
};

}

#endif // __RETRO_CKEYBOARDMOUSECONTROLLER_HPP__
