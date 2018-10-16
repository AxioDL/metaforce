#pragma once

#include "CIOWin.hpp"

namespace urde
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

