#ifndef CTOOL_CLEAN
#define CTOOL_CLEAN

#include "CToolBase.hpp"

class CToolClean final : public CToolBase
{
public:
    CToolClean(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolClean()
    {
    }

    static void Help()
    {
    }

    std::string toolName() const {return "clean";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_CLEAN
