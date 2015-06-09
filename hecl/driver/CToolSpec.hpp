#ifndef CTOOL_SPEC
#define CTOOL_SPEC

#include "CToolBase.hpp"
#include <stdio.h>

class CToolSpec final : public CToolBase
{
public:
    CToolSpec(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolSpec()
    {
    }

    static void Help(CHelpOutput& help)
    {
        help.secHead("NAME");
        help.beginWrap();
        help.wrap("hecl-spec - Configure target data options\n");
        help.endWrap();

        help.secHead("SYNOPSIS");
        help.beginWrap();
        help.wrap("hecl spec [enable|disable] [<specname>...]\n");
        help.endWrap();

        help.secHead("DESCRIPTION");
        help.beginWrap();
        help.wrap("This command configures the HECL project with the user's preferred target DataSpecs.\n\n"
                  "Providing enable/disable argument will bulk-set the enable status of the provided spec(s)"
                  "list. If enable/disable is not provided, a list of supported DataSpecs is printed.\n\n");
        help.endWrap();

        help.secHead("OPTIONS");
        help.optionHead("<specname>...", "DataSpec name(s)");
        help.beginWrap();
        help.wrap("Specifies platform-names to enable/disable");
        help.endWrap();
    }

    std::string toolName() const {return "spec";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_SPEC
