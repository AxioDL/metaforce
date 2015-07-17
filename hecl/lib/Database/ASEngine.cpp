#include "HECL/Database.hpp"

namespace HECL
{
namespace Database
{

/* Centralized AngelScript engine */
AngelScript::asIScriptEngine* asENGINE = nullptr;

/* AngelScript Logger */
static LogVisor::LogModule Log("AngelScript");
static void MessageCallback(const AngelScript::asSMessageInfo* msg, void*)
{
    LogVisor::Level lv = LogVisor::Error;
    if (msg->type == AngelScript::asMSGTYPE_WARNING)
        lv = LogVisor::Warning;
    else if (msg->type == AngelScript::asMSGTYPE_INFORMATION)
        lv = LogVisor::Info;
    Log.reportSource(lv, msg->section, msg->row, msg->message);
}

static bool InitEntered = false;
void InitASEngine()
{
    if (InitEntered)
        return;
    InitEntered = true;
    assert(asENGINE = AngelScript::asCreateScriptEngine(ANGELSCRIPT_VERSION));
    assert(asENGINE->SetEngineProperty(AngelScript::asEP_COPY_SCRIPT_SECTIONS, false) >= 0);
    assert(asENGINE->SetMessageCallback(AngelScript::asFUNCTION(MessageCallback), nullptr, AngelScript::asCALL_CDECL) >= 0);
}

}
}
