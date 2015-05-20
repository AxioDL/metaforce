#ifndef CTOOL_COOK
#define CTOOL_COOK

#include "CToolBase.hpp"

class CToolCook final : public CToolBase
{
public:
    CToolCook(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolCook()
    {
    }

    static void Help()
    {
    }

    std::string toolName() const {return "cook";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_COOK
