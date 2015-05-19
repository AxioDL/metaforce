#ifndef MAIN_CPP
#error This file may only be included from main.cpp
#endif

#include "CToolBase.hpp"

class CToolInit : public CToolBase
{
public:
    CToolInit(const std::vector<std::string>& args)
    : CToolBase(args)
    {
    }
    ~CToolInit()
    {
    }
};
