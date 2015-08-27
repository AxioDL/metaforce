#ifndef __RETRO_CMFGAMEBASE_HPP__
#define __RETRO_CMFGAMEBASE_HPP__

#include "CIOWin.hpp"

namespace Retro
{

class CMFGameBase : public CIOWin
{
public:
    CMFGameBase(const char* name) : CIOWin(name) {}
};

class CMFGameLoaderBase : public CIOWin
{
public:
    CMFGameLoaderBase(const char* name) : CIOWin(name) {}
};

}

#endif // __RETRO_CMFGAMEBASE_HPP__
