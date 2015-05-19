#ifndef MAIN_CPP
#error This file may only be included from main.cpp
#endif

#include <vector>
#include <string>
#include "CToolBase.hpp"

class CToolPackage : public CToolBase
{
public:
    CToolPackage(const std::vector<std::string>& args)
    : CToolBase(args)
    {
    }
    ~CToolPackage()
    {
    }
};
