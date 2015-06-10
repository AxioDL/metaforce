#ifndef CTOOL_SPEC
#define CTOOL_SPEC

#include "ToolBase.hpp"
#include <stdio.h>

class ToolSpec final : public ToolBase
{
public:
    ToolSpec(const ToolPassInfo& info)
    : ToolBase(info)
    {
    }

    ~ToolSpec()
    {
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-spec - Configure target data options\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl spec [enable|disable] [<specname>...]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command configures the HECL project with the user's preferred target DataSpecs.\n\n"
                  "Providing enable/disable argument will bulk-set the enable status of the provided spec(s)"
                  "list. If enable/disable is not provided, a list of supported DataSpecs is printed.\n\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<specname>..."), _S("DataSpec name(s)"));
        help.beginWrap();
        help.wrap(_S("Specifies platform-names to enable/disable"));
        help.endWrap();
    }

    HECL::SystemString toolName() const {return _S("spec");}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_SPEC
