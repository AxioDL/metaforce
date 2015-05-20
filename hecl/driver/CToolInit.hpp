#ifndef CTOOL_INIT
#define CTOOL_INIT

#include "CToolBase.hpp"

class CToolInit final : public CToolBase
{
public:
    CToolInit(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolInit()
    {
    }

    static void Help()
    {
    }

    std::string toolName() const {return "init";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_INIT
