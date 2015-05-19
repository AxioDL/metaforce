#ifndef CTOOL_GROUP
#define CTOOL_GROUP

#include "CToolBase.hpp"

class CToolGroup : public CToolBase
{
public:
    CToolGroup(const std::vector<std::string>& args)
    : CToolBase(args)
    {
    }
    ~CToolGroup()
    {
    }
};

#endif // CTOOL_GROUP
