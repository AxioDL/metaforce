#ifndef __URDE_CMFGAME_HPP__
#define __URDE_CMFGAME_HPP__

#include "CMFGameBase.hpp"

namespace urde
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

#endif // __URDE_CMFGAME_HPP__
