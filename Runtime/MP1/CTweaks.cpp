#include <LogVisor/LogVisor.hpp>
#include "CTweaks.hpp"
#include "CResFactory.hpp"
#include "CResLoader.hpp"
#include "GameGlobalObjects.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakPlayer.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakPlayerControl.hpp"

namespace Retro
{
ITweakPlayer* g_tweakPlayer = nullptr;
ITweakPlayerControl* g_tweakPlayerControl = nullptr;

namespace MP1
{

LogVisor::LogModule Log("MP1::CTweaks");

static const SObjectTag& IDFromFactory(CResFactory& factory, const char* name)
{
    const SObjectTag* tag = factory.GetResourceIdByName(name);
    if (!tag)
        Log.report(LogVisor::FatalError, "Tweak Asset not found when loading... '%s'", name);
    return *tag;
}

void CTweaks::RegisterTweaks()
{
#if 0
    CResFactory& factory = *g_ResFactory;
    CResLoader& loader = factory.GetLoader();
    std::unique_ptr<CInputStream> strm;

    strm.reset(loader.LoadNewResourceSync(IDFromFactory(factory, "Player"), nullptr));
    TOneStatic<DNAMP1::CTweakPlayer> player(*strm);
    g_tweakPlayer = player.GetAllocSpace();

    strm.reset(loader.LoadNewResourceSync(IDFromFactory(factory, "PlayerControls"), nullptr));
    TOneStatic<DNAMP1::CTweakPlayerControl> playerControl(*strm);
    g_tweakPlayerControl = playerControl.GetAllocSpace();
#endif
}

void CTweaks::RegisterResourceTweaks()
{
}

}
}
