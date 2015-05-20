#ifndef CTOOL_ADD
#define CTOOL_ADD

#include "CToolBase.hpp"

class CToolAdd final : public CToolBase
{
public:
    CToolAdd(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolAdd()
    {
    }

    static void Help()
    {
    }

    std::string toolName() const {return "add";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_ADD
