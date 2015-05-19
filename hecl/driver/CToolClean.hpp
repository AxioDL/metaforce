#ifndef CTOOL_CLEAN
#define CTOOL_CLEAN

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

#endif // CTOOL_CLEAN
