#include "Runtime/MP1/CTweaks.hpp"

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

#include "Editor/ProjectManager.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CPlayerCameraBob.hpp"

#include <logvisor/logvisor.hpp>

namespace urde {

namespace MP1 {

static logvisor::Module Log("MP1::CTweaks");

void CTweaks::RegisterTweaks(hecl::CVarManager* cvarMgr) {
  std::optional<CMemoryInStream> strm;
  const SObjectTag* tag;

  /* Particle */
  tag = g_ResFactory->GetResourceIdByName("Particle");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  g_tweakParticle = new DataSpec::DNAMP1::CTweakParticle(*strm);
  g_tweakParticle->initCVars(cvarMgr);

  /* Player */
  tag = g_ResFactory->GetResourceIdByName("Player");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  g_tweakPlayer = new DataSpec::DNAMP1::CTweakPlayer(*strm);
  g_tweakPlayer->initCVars(cvarMgr);

  /* CameraBob */
  tag = g_ResFactory->GetResourceIdByName("CameraBob");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  CPlayerCameraBob::ReadTweaks(*strm);

  /* Ball */
  tag = g_ResFactory->GetResourceIdByName("Ball");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  g_tweakBall = new DataSpec::DNAMP1::CTweakBall(*strm);
  g_tweakBall->initCVars(cvarMgr);

  /* PlayerGun */
  tag = g_ResFactory->GetResourceIdByName("PlayerGun");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  g_tweakPlayerGun = new DataSpec::DNAMP1::CTweakPlayerGun(*strm);
  g_tweakPlayerGun->initCVars(cvarMgr);

  /* Targeting */
  tag = g_ResFactory->GetResourceIdByName("Targeting");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  if (g_Main->IsTrilogy() || g_Main->IsPAL() || g_Main->IsJapanese()) {
    g_tweakTargeting = new DataSpec::DNAMP1::CTweakTargeting<true>(*strm);
    g_tweakTargeting->initCVars(cvarMgr);
  } else {
    g_tweakTargeting = new DataSpec::DNAMP1::CTweakTargeting<false>(*strm);
    g_tweakTargeting->initCVars(cvarMgr);
  }

  /* Game */
  tag = g_ResFactory->GetResourceIdByName("Game");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  g_tweakGame = new DataSpec::DNAMP1::CTweakGame(*strm);
  g_tweakGame->initCVars(cvarMgr);

  /* GuiColors */
  tag = g_ResFactory->GetResourceIdByName("GuiColors");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  g_tweakGuiColors = new DataSpec::DNAMP1::CTweakGuiColors(*strm);
  g_tweakGuiColors->initCVars(cvarMgr);

  /* AutoMapper */
  tag = g_ResFactory->GetResourceIdByName("AutoMapper");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  g_tweakAutoMapper = new DataSpec::DNAMP1::CTweakAutoMapper(*strm);
  CMappableObject::ReadAutoMapperTweaks(*g_tweakAutoMapper);
  g_tweakAutoMapper->initCVars(cvarMgr);

  /* Gui */
  tag = g_ResFactory->GetResourceIdByName("Gui");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  g_tweakGui = new DataSpec::DNAMP1::CTweakGui(*strm);
  g_tweakPlayerGun->initCVars(cvarMgr);

  /* PlayerControls */
  tag = g_ResFactory->GetResourceIdByName("PlayerControls");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  g_tweakPlayerControl = new DataSpec::DNAMP1::CTweakPlayerControl(*strm);

  /* PlayerControls2 */
  tag = g_ResFactory->GetResourceIdByName("PlayerControls2");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  g_tweakPlayerControlAlt = new DataSpec::DNAMP1::CTweakPlayerControl(*strm);

  g_currentPlayerControl = g_tweakPlayerControl;

  /* SlideShow */
  tag = g_ResFactory->GetResourceIdByName("SlideShow");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  g_tweakSlideShow = new DataSpec::DNAMP1::CTweakSlideShow(*strm);
  g_tweakSlideShow->initCVars(cvarMgr);
}

void CTweaks::RegisterResourceTweaks(hecl::CVarManager* cvarMgr) {
  std::optional<CMemoryInStream> strm;

  const SObjectTag* tag = g_ResFactory->GetResourceIdByName("GunRes");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  g_tweakGunRes = new DataSpec::DNAMP1::CTweakGunRes(*strm);
  g_tweakGunRes->ResolveResources(*g_ResFactory);
  g_tweakGunRes->initCVars(cvarMgr);

  tag = g_ResFactory->GetResourceIdByName("PlayerRes");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag), true);
  if (g_Main->IsTrilogy() || g_Main->IsPAL() || g_Main->IsJapanese()) {
    g_tweakPlayerRes = new DataSpec::DNAMP1::CTweakPlayerRes<true>(*strm);
  } else {
    g_tweakPlayerRes = new DataSpec::DNAMP1::CTweakPlayerRes<false>(*strm);
  }

  g_tweakPlayerRes->ResolveResources(*g_ResFactory);
  g_tweakPlayerRes->initCVars(cvarMgr);
}

} // namespace MP1
} // namespace urde
