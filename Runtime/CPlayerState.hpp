#ifndef __RETRO_CPLAYERSTATE_HPP__
#define __RETRO_CPLAYERSTATE_HPP__

#include "RetroTemplates.hpp"
#include "CStaticInterference.hpp"

class CPlayerState : TOneStatic<CPlayerState>
{
    CStaticInterference m_staticIntf;
    class CPowerUp
    {
        int m_a;
        int m_b;
    public:
        CPowerUp(int a, int b) : m_a(a), m_b(b) {}
    };
    CPowerUp m_powerups[29];
};

#endif // __RETRO_CPLAYERSTATE_HPP__
