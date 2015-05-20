#ifndef CTOOL_GROUP
#define CTOOL_GROUP

#include "CToolBase.hpp"

class CToolGroup final : public CToolBase
{
public:
    CToolGroup(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolGroup()
    {
    }

    static void Help()
    {
    }

    std::string toolName() const {return "group";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_GROUP
