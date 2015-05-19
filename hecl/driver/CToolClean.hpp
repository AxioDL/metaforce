#ifndef MAIN_CPP
#error This file may only be included from main.cpp
#endif

#include "CToolBase.hpp"

class CToolClean : public CToolBase
{
public:
    CToolClean(const std::vector<std::string>& args)
    : CToolBase(args)
    {
    }
    ~CToolClean()
    {
    }
};
