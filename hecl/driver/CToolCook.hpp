#ifndef MAIN_CPP
#error This file may only be included from main.cpp
#endif

#include "CToolBase.hpp"

class CToolCook : public CToolBase
{
public:
    CToolCook(const std::vector<std::string>& args)
    : CToolBase(args)
    {
    }
    ~CToolCook()
    {
    }
};
