#ifndef CTOOL_PACKAGE
#define CTOOL_PACKAGE

#include <vector>
#include <string>
#include "CToolBase.hpp"

class CToolPackage final : public CToolBase
{
public:
    CToolPackage(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolPackage()
    {
    }

    static void Help()
    {
    }

    std::string toolName() const {return "package";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_PACKAGE
