#ifndef CTOOL_REMOVE
#define CTOOL_REMOVE

#include "CToolBase.hpp"

class CToolRemove final : public CToolBase
{
public:
    CToolRemove(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolRemove()
    {
    }

    static void Help()
    {
    }

    std::string toolName() const {return "remove";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_REMOVE
