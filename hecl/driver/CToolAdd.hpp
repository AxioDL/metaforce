#ifndef CTOOL_ADD
#define CTOOL_ADD

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

#endif // CTOOL_ADD
