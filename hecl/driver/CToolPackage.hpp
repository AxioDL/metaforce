#ifndef CTOOL_PACKAGE
#define CTOOL_PACKAGE

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

#endif // CTOOL_PACKAGE
