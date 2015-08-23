#ifndef __RETRO_CPLAYERSTATE_HPP__
#define __RETRO_CPLAYERSTATE_HPP__

#include "RetroTypes.hpp"
#include "CBasics.hpp"
#include "CStaticInterference.hpp"
#include "IOStreams.hpp"

namespace Retro
{

class CPlayerState
{
    CStaticInterference m_staticIntf;
    class CPowerUp
    {
        int m_a = 0;
        int m_b = 0;
    public:
        CPowerUp() {}
        CPowerUp(int a, int b) : m_a(a), m_b(b) {}
    };
    CPowerUp m_powerups[29];

public:
    CPlayerState() : m_staticIntf(5) {}
    CPlayerState(CInputStream& stream);
};

}

#endif // __RETRO_CPLAYERSTATE_HPP__
