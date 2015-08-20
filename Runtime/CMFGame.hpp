#ifndef __RETRO_CMFGAME_HPP__
#define __RETRO_CMFGAME_HPP__

#include "CIOWin.hpp"

namespace Retro
{

class CMFGameLoader : public CIOWin
{
public:
    CMFGameLoader() : CIOWin("CMFGameLoader") {}
    bool OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
    void Draw() const;
};

}

#endif // __RETRO_CMFGAME_HPP__
