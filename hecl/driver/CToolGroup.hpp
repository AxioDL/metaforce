#ifndef MAIN_CPP
#error This file may only be included from main.cpp
#endif

#include "CToolBase.hpp"

class CToolGroup : public CToolBase
{
public:
    CToolGroup(const std::vector<std::string>& args)
    : CToolBase(args)
    {
    }
    ~CToolGroup()
    {
    }
};
