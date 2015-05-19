#ifndef MAIN_CPP
#error This file may only be included from main.cpp
#endif

#include "CToolBase.hpp"

class CToolAdd : public CToolBase
{
public:
    CToolAdd(const std::vector<std::string>& args)
    : CToolBase(argc, argv)
    {
    }
    ~CToolAdd()
    {
    }
};
