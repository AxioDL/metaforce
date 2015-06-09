#include "HECLDatabase.hpp"

#include "DUMB.hpp"
#include "HMDL.hpp"
#include "MATR.hpp"
#include "STRG.hpp"
#include "TXTR.hpp"

const std::pair<std::string, std::string> DATA_SPECS[] =
{
    {"hecl-little", "Targets little-endian pc apps using the HECL runtime"},
    {"hecl-big", "Targets big-endian pc apps using the HECL runtime"},
    {"hecl-revolution", "Targets Wii apps using the HECL runtime"},
    {"hecl-cafe", "Targets Wii U apps using the HECL runtime"},

};
