#ifndef CTOOL_COOK
#define CTOOL_COOK

#include "CToolBase.hpp"

class CToolCook : public CToolBase
{
public:
    CToolCook(const std::vector<std::string>& args)
    : CToolBase(args)
    {
    }
    ~CToolCook()
    {
    }
};

#endif // CTOOL_COOK
