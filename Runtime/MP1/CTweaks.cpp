#include <LogVisor/LogVisor.hpp>
#include "CTweaks.hpp"
#include "CResFactory.hpp"
#include "CResLoader.hpp"
#include "GameGlobalObjects.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakGame.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakPlayer.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakPlayerControl.hpp"

namespace Retro
{
ITweakGame*   g_tweakGame = nullptr;
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
<<<<<<< 6710ed8da05c8dfedf42b6fddf11c6856d216026
#if 0
=======
    return;
>>>>>>> More work on allocator
    CResFactory& factory = *g_ResFactory;
    CResLoader& loader = factory.GetLoader();
    std::unique_ptr<CInputStream> strm;

    strm.reset(loader.LoadNewResourceSync(IDFromFactory(factory, "Game"), nullptr));
    TOneStatic<DNAMP1::CTweakGame> game(*strm);
    g_tweakGame = game.GetAllocSpace();
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
