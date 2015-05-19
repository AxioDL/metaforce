#ifndef CTOOL_HELP
#define CTOOL_HELP

#include "CToolBase.hpp"

class CToolHelp : public CToolBase
{
public:
    CToolHelp(const std::vector<std::string>& args)
    : CToolBase(args)
    {
    }
    ~CToolHelp()
    {
    }
};

#endif // CTOOL_HELP
