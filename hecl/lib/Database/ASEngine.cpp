#include "HECL/Database.hpp"

namespace HECL
{
namespace Database
{

/* Centralized AngelScript engine */
AngelScript::asIScriptEngine* asENGINE = nullptr;

static bool InitEntered = false;
void InitASEngine()
{
    if (InitEntered)
        return;
    InitEntered = true;
    assert(asENGINE = AngelScript::asCreateScriptEngine(ANGELSCRIPT_VERSION));
}

}
}
