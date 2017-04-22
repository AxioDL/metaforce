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
#include "DataSpec/DNAMP1/Tweaks/CTweakAutoMapper.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakTargeting.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakGui.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakParticle.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakBall.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakGuiColors.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakPlayerGun.hpp"
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
    const SObjectTag* tag;

    /* Particle */
    tag = factory.GetResourceIdByName("Particle");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakParticle = new DataSpec::DNAMP1::CTweakParticle(*strm);

    /* Player */
    tag = factory.GetResourceIdByName("Player");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakPlayer = new DataSpec::DNAMP1::CTweakPlayer(*strm);

    /* CameraBob */
    tag = factory.GetResourceIdByName("CameraBob");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    CPlayerCameraBob::ReadTweaks(*strm);

    /* Ball */
    tag = factory.GetResourceIdByName("Ball");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakBall = new DataSpec::DNAMP1::CTweakBall(*strm);

    /* PlayerGun */
    tag = factory.GetResourceIdByName("PlayerGun");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakPlayerGun = new DataSpec::DNAMP1::CTweakPlayerGun(*strm);

    /* Targeting */
    tag = factory.GetResourceIdByName("Targeting");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakTargeting = new DataSpec::DNAMP1::CTweakTargeting(*strm);

    /* Game */
    tag = factory.GetResourceIdByName("Game");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakGame = new DataSpec::DNAMP1::CTweakGame(*strm);

    /* GuiColors */
    tag = factory.GetResourceIdByName("GuiColors");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakGuiColors = new DataSpec::DNAMP1::CTweakGuiColors(*strm);

    /* AutoMapper */
    tag = factory.GetResourceIdByName("AutoMapper");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakAutoMapper = new DataSpec::DNAMP1::CTweakAutoMapper(*strm);
    CMappableObject::ReadAutoMapperTweaks(*g_tweakAutoMapper);

    /* Gui */
    tag = factory.GetResourceIdByName("Gui");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakGui = new DataSpec::DNAMP1::CTweakGui(*strm);

    /* PlayerControls */
    tag = factory.GetResourceIdByName("PlayerControls");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakPlayerControl = new DataSpec::DNAMP1::CTweakPlayerControl(*strm);

    /* PlayerControls2 */
    tag = factory.GetResourceIdByName("PlayerControls2");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakPlayerControlAlt = new DataSpec::DNAMP1::CTweakPlayerControl(*strm);

    g_currentPlayerControl = g_tweakPlayerControl;

    /* SlideShow */
    tag = factory.GetResourceIdByName("SlideShow");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakSlideShow = new DataSpec::DNAMP1::CTweakSlideShow(*strm);
}

void CTweaks::RegisterResourceTweaks()
{
    ProjectResourceFactoryMP1& factory = ProjectManager::g_SharedManager->resourceFactoryMP1();
    std::experimental::optional<CMemoryInStream> strm;
    
    const SObjectTag* tag = factory.GetResourceIdByName("GunRes");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakGunRes = new DataSpec::DNAMP1::CTweakGunRes(*strm);
    g_tweakGunRes->ResolveResources(factory);

    tag = factory.GetResourceIdByName("PlayerRes");
    strm.emplace(factory.LoadResourceSync(*tag).release(), factory.ResourceSize(*tag));
    g_tweakPlayerRes = new DataSpec::DNAMP1::CTweakPlayerRes(*strm);
    g_tweakPlayerRes->ResolveResources(factory);
}

}
}
