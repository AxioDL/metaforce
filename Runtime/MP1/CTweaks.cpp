#include "logvisor/logvisor.hpp"
#include "CTweaks.hpp"
#include "CResFactory.hpp"
#include "CResLoader.hpp"
#include "GameGlobalObjects.hpp"
#include "Editor/ProjectManager.hpp"
#include "Editor/ProjectResourceFactoryMP1.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakGame.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakPlayer.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakPlayerControl.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakGunRes.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakPlayerRes.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakSlideShow.hpp"
#include "World/CPlayerCameraBob.hpp"

namespace urde
{

namespace MP1
{

static logvisor::Module Log("MP1::CTweaks");

static const SObjectTag& IDFromFactory(CResFactory& factory, const char* name)
{
    const SObjectTag* tag = factory.GetResourceIdByName(name);
    if (!tag)
        Log.report(logvisor::Fatal, "Tweak Asset not found when loading... '%s'", name);
    return *tag;
}

void CTweaks::RegisterTweaks()
{
    ProjectResourceFactoryMP1& factory = ProjectManager::g_SharedManager->resourceFactoryMP1();
    std::experimental::optional<CMemoryInStream> strm;

    const SObjectTag* tag = factory.GetResourceIdByName("SlideShow");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakSlideShow = new DataSpec::DNAMP1::CTweakSlideShow(*strm);

    tag = factory.GetResourceIdByName("Player");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakPlayer = new DataSpec::DNAMP1::CTweakPlayer(*strm);

    tag = factory.GetResourceIdByName("CameraBob");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    CPlayerCameraBob::ReadTweaks(*strm);
}

void CTweaks::RegisterResourceTweaks()
{
    ProjectResourceFactoryMP1& factory = ProjectManager::g_SharedManager->resourceFactoryMP1();
    std::experimental::optional<CMemoryInStream> strm;
    
    const SObjectTag* tag = factory.GetResourceIdByName("GunRes");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakGunRes = new DataSpec::DNAMP1::CTweakGunRes(*strm);
    
    tag = factory.GetResourceIdByName("PlayerRes");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakPlayerRes = new DataSpec::DNAMP1::CTweakPlayerRes(*strm);
}

}
}
