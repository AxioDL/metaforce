#ifndef MAIN_CPP
#error This file may only be included from main.cpp
#endif

class CToolBase
{
protected:
    const std::vector<std::string>& m_args;
public:
    CToolBase(const std::vector<std::string>& args)
    : m_args(args) {}
    virtual ~CToolBase() {}
};

