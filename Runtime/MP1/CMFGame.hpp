#ifndef __RETRO_CMFGAME_HPP__
#define __RETRO_CMFGAME_HPP__

#include "CMFGameBase.hpp"

namespace Retro
{
namespace MP1
{

class CMFGame : public CMFGameBase
{
public:
    CMFGame() : CMFGameBase("CMFGame") {}
    CIOWin::EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
    void Draw() const;
};

class CMFGameLoader : public CMFGameLoaderBase
{
public:
    CMFGameLoader() : CMFGameLoaderBase("CMFGameLoader") {}
    EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
    void Draw() const;
};

}
}

#endif // __RETRO_CMFGAME_HPP__
