#ifndef CTOOL_INIT
#define CTOOL_INIT

#include "CToolBase.hpp"

class CToolInit : public CToolBase
{
public:
    CToolInit(const std::vector<std::string>& args)
    : CToolBase(args)
    {
    }
    ~CToolInit()
    {
    }
};

#endif // CTOOL_INIT
