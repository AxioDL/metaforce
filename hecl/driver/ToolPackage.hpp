#ifndef CTOOL_PACKAGE
#define CTOOL_PACKAGE

#include <vector>
#include <string>
#include "ToolBase.hpp"
#include <stdio.h>

class ToolPackage final : public ToolBase
{
public:
    ToolPackage(const ToolPassInfo& info)
    : ToolBase(info)
    {
        if (!info.project)
            LogModule.report(logvisor::Fatal, "hecl package must be ran within a project directory");
    }

    ~ToolPackage()
    {
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-pack\n")
                  _S("hecl-package - Package objects within the project database\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl package [-a] [-o <package-out>] [<input-dir>]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command initiates a packaging pass on the project database. Packaging ")
                  _S("is analogous to linking in software development. All objects necessary to ")
                  _S("generate a complete package are gathered, grouped, and indexed within an .hlpk file.\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<dir>"), _S("input directory"));
        help.beginWrap();
        help.wrap(_S("Specifies a project subdirectory to root the resulting package from. ")
                  _S("If any dependent-files fall outside this subdirectory, they will implicitly ")
                  _S("be gathered and packaged.\n"));
        help.endWrap();

        help.optionHead(_S("-o <package-out>"), _S("output package file"));
        help.beginWrap();
        help.wrap(_S("Specifies a target path to write the package. If not specified, the package ")
                  _S("is written into <project-root>/out/<relative-input-dirs>/<input-dir>.hlpk\n"));
        help.endWrap();

        help.optionHead(_S("-a"), _S("auto cook"));
        help.beginWrap();
        help.wrap(_S("Any referenced objects that haven't already been cooked are automatically cooked as ")
                  _S("part of the packaging process. If this flag is omitted, the packaging process will abort.\n"));
        help.endWrap();
    }

    hecl::SystemString toolName() const {return _S("package");}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_PACKAGE
