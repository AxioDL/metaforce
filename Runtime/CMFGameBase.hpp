#ifndef __PSHAG_CMFGAMEBASE_HPP__
#define __PSHAG_CMFGAMEBASE_HPP__

#include "CIOWin.hpp"

namespace pshag
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

#endif // __PSHAG_CMFGAMEBASE_HPP__
