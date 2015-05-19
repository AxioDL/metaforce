#ifndef MAIN_CPP
#error This file may only be included from main.cpp
#endif

#include "CToolBase.hpp"

class CToolHelp : public CToolBase
{
public:
    CToolHelp(const std::vector<std::string>& args)
    : CToolBase(args)
    {
    }
    ~CToolHelp()
    {
    }
};
