#include <stdio.h>
#include <string.h>
#include <HECLDatabase.hpp>

#define MAIN_CPP
#include "CToolInit.hpp"
#include "CToolAdd.hpp"
#include "CToolGroup.hpp"
#include "CToolCook.hpp"
#include "CToolClean.hpp"
#include "CToolPackage.hpp"
#include "CToolHelp.hpp"

void printHelp(const char* pname)
{
    printf("Usage: %s init|add|group|cook|clean|package|help\n", pname);
}

int main(int argc, const char** argv)
{
    if (argc == 1)
    {
        printHelp(argv[0]);
        return 0;
    }
    else if (argc == 0)
    {
        printHelp("hecl");
        return 0;
    }

    if (!strcasecmp(argv[1], "init"))


    return 0;
}
