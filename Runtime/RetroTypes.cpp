#include "RetroTypes.hpp"
#include "GameGlobalObjects.hpp"
#include "IMain.hpp"

namespace urde
{
logvisor::Module Log("urde::RetroTypes::CAssetId");

CAssetId::CAssetId(CInputStream& in)
{
    if (g_Main)
    {
        if (g_Main->GetExpectedIdSize() == sizeof(u32))
            Assign(in.readUint32Big());
        else if (g_Main->GetExpectedIdSize() == sizeof(u64))
            Assign(in.readUint64Big());
        else
            Log.report(logvisor::Fatal, "Unsupported id length %i", g_Main->GetExpectedIdSize());
    }
    else
        Log.report(logvisor::Fatal, "Input constructor called before runtime Main entered!");
}

void CAssetId::PutTo(COutputStream& out)
{
    if (g_Main)
    {
        if (g_Main->GetExpectedIdSize() == sizeof(u32))
            out.writeUint32Big(u32(id));
        else if (g_Main->GetExpectedIdSize() == sizeof(u64))
            out.writeUint64Big(id);
        else
            Log.report(logvisor::Fatal, "Unsupported id length %i", g_Main->GetExpectedIdSize());
    }
    else
        Log.report(logvisor::Fatal, "PutTo called before runtime Main entered!");
}

}