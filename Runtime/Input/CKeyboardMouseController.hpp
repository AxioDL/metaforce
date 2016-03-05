#ifndef __PSHAG_CKEYBOARDMOUSECONTROLLER_HPP__
#define __PSHAG_CKEYBOARDMOUSECONTROLLER_HPP__

#include <boo/boo.hpp>

namespace urde
{

struct CKeyboardMouseControllerData
{
    bool m_charKeys[256] = {};
    bool m_specialKeys[26] = {};
    bool m_mouseButtons[6] = {};
    boo::EModifierKey m_modMask = boo::EModifierKey::None;
    boo::SWindowCoord m_mouseCoord;
    boo::SScrollDelta m_accumScroll;
};

}

#endif // __PSHAG_CKEYBOARDMOUSECONTROLLER_HPP__
