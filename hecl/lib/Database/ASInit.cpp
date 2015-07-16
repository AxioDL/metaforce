#include "HECL/Database.hpp"

namespace HECL
{
namespace Database
{

static std::string StringFactory(unsigned int byteLength, const char *s)
{
    return std::string(s, byteLength);
}

ASStringType asSTRINGTYPE;
ASStringType::ASStringType() : ASType<std::string>("", "string")
{
    assert(asENGINE->RegisterStringFactory("string",
                                           AngelScript::asFUNCTION(StringFactory),
                                           AngelScript::asCALL_CDECL) >= 0);
}

}
}


