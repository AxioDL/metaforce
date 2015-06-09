#ifndef CTOOL_PLATFORM
#define CTOOL_PLATFORM

#include "CToolBase.hpp"
#include <stdio.h>

class CToolPlatform final : public CToolBase
{
public:
    CToolPlatform(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolPlatform()
    {
    }

    static void Help(CHelpOutput& help)
    {
        help.secHead("NAME");
        help.beginWrap();
        help.wrap("hecl-platform - Configure platform target options\n");
        help.endWrap();

        help.secHead("SYNOPSIS");
        help.beginWrap();
        help.wrap("hecl platform [enable|disable] [<platname>...]\n");
        help.endWrap();

        help.secHead("DESCRIPTION");
        help.beginWrap();
        help.wrap("This command configures the HECL project with the user's preferred target platforms.\n\n"
                  "Providing enable/disable argument will bulk-set the enable status of the provided platform"
                  "list. If enable/disable is not provided, a list of supported platforms is printed.\n\n");
        help.endWrap();

        help.secHead("OPTIONS");
        help.optionHead("<platname>...", "platform name(s)");
        help.beginWrap();
        help.wrap("Specifies platform-names to enable/disable");
        help.endWrap();
    }

    std::string toolName() const {return "platform";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_PLATFORM
