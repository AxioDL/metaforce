#ifndef CTOOL_PACKAGE
#define CTOOL_PACKAGE

#include <vector>
#include <string>
#include "CToolBase.hpp"
#include <stdio.h>

class CToolPackage final : public CToolBase
{
public:
    CToolPackage(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolPackage()
    {
    }

    static void Help(CHelpOutput& help)
    {
        help.secHead("NAME");
        help.beginWrap();
        help.wrap("hecl-pack\n"
                  "hecl-package - Package objects within the project database\n");
        help.endWrap();

        help.secHead("SYNOPSIS");
        help.beginWrap();
        help.wrap("hecl package [-a] [-o <package-out>] [<input-dir>]\n");
        help.endWrap();

        help.secHead("DESCRIPTION");
        help.beginWrap();
        help.wrap("This command initiates a packaging pass on the project database. Packaging "
                  "is analogous to linking in software development. All objects necessary to "
                  "generate a complete package are gathered, grouped, and indexed within an .hlpk file.\n");
        help.endWrap();

        help.secHead("OPTIONS");
        help.optionHead("<dir>", "input directory");
        help.beginWrap();
        help.wrap("Specifies a project subdirectory to root the resulting package from. "
                  "If any dependent-files fall outside this subdirectory, they will implicitly "
                  "be gathered and packaged.\n");
        help.endWrap();

        help.optionHead("-o <package-out>", "output package file");
        help.beginWrap();
        help.wrap("Specifies a target path to write the package. If not specified, the package "
                  "is written into <project-root>/out/<relative-input-dirs>/<input-dir>.hlpk\n");
        help.endWrap();

        help.optionHead("-a", "auto cook");
        help.beginWrap();
        help.wrap("Any referenced objects that haven't already been cooked are automatically cooked as "
                  "part of the packaging process. If this flag is omitted, the packaging process will abort.\n");
        help.endWrap();
    }

    std::string toolName() const {return "package";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_PACKAGE
