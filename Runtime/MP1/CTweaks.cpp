#include "Runtime/MP1/CTweaks.hpp"

#include "Runtime/MP1/Tweaks/CTweakGame.hpp"
#include "Runtime/MP1/Tweaks/CTweakPlayer.hpp"
#include "Runtime/MP1/Tweaks/CTweakPlayerControl.hpp"
#include "Runtime/MP1/Tweaks/CTweakGunRes.hpp"
#include "Runtime/MP1/Tweaks/CTweakPlayerRes.hpp"
#include "Runtime/MP1/Tweaks/CTweakSlideShow.hpp"
#include "Runtime/MP1/Tweaks/CTweakAutoMapper.hpp"
#include "Runtime/MP1/Tweaks/CTweakTargeting.hpp"
#include "Runtime/MP1/Tweaks/CTweakGui.hpp"
#include "Runtime/MP1/Tweaks/CTweakParticle.hpp"
#include "Runtime/MP1/Tweaks/CTweakBall.hpp"
#include "Runtime/MP1/Tweaks/CTweakGuiColors.hpp"
#include "Runtime/MP1/Tweaks/CTweakPlayerGun.hpp"

#include "Runtime/IMain.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/AutoMapper/CMappableObject.hpp"
#include "Runtime/World/CPlayerCameraBob.hpp"

#include <logvisor/logvisor.hpp>

namespace metaforce::MP1 {

static logvisor::Module Log("MP1::CTweaks");

void CTweaks::RegisterTweaks(CVarManager* cvarMgr) {
  std::optional<CMemoryInStream> strm;
  const SObjectTag* tag;

  /* Particle */
  tag = g_ResFactory->GetResourceIdByName("Particle");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag),
               CMemoryInStream::EOwnerShip::Owned);
  g_tweakParticle = new MP1::CTweakParticle(*strm);
  g_tweakParticle->initCVars(cvarMgr);

  /* Player */
  tag = g_ResFactory->GetResourceIdByName("Player");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag),
               CMemoryInStream::EOwnerShip::Owned);
  g_tweakPlayer = new MP1::CTweakPlayer(*strm);
  g_tweakPlayer->initCVars(cvarMgr);

  /* CameraBob */
  tag = g_ResFactory->GetResourceIdByName("CameraBob");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag),
               CMemoryInStream::EOwnerShip::Owned);
  CPlayerCameraBob::ReadTweaks(*strm);

  /* Ball */
  tag = g_ResFactory->GetResourceIdByName("Ball");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag),
               CMemoryInStream::EOwnerShip::Owned);
  g_tweakBall = new MP1::CTweakBall(*strm);
  g_tweakBall->initCVars(cvarMgr);

  /* PlayerGun */
  tag = g_ResFactory->GetResourceIdByName("PlayerGun");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag),
               CMemoryInStream::EOwnerShip::Owned);
  g_tweakPlayerGun = new MP1::CTweakPlayerGun(*strm);
  g_tweakPlayerGun->initCVars(cvarMgr);

  /* Targeting */
  tag = g_ResFactory->GetResourceIdByName("Targeting");
  u8* Args = g_ResFactory->LoadResourceSync(*tag).release();
  u32 size = g_ResFactory->ResourceSize(*tag);
  strm.emplace(Args, size, CMemoryInStream::EOwnerShip::Owned);
  g_tweakTargeting = new MP1::CTweakTargeting(*strm, g_Main->IsTrilogy() || g_Main->IsPAL() || g_Main->IsJapanese());
  g_tweakTargeting->initCVars(cvarMgr);
  /* Game */
  tag = g_ResFactory->GetResourceIdByName("Game");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag),
               CMemoryInStream::EOwnerShip::Owned);
  g_tweakGame = new MP1::CTweakGame(*strm);
  g_tweakGame->initCVars(cvarMgr);

  /* GuiColors */
  tag = g_ResFactory->GetResourceIdByName("GuiColors");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag),
               CMemoryInStream::EOwnerShip::Owned);
  g_tweakGuiColors = new MP1::CTweakGuiColors(*strm);
  g_tweakGuiColors->initCVars(cvarMgr);

  /* AutoMapper */
  tag = g_ResFactory->GetResourceIdByName("AutoMapper");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag),
               CMemoryInStream::EOwnerShip::Owned);
  g_tweakAutoMapper = new MP1::CTweakAutoMapper(*strm);
  CMappableObject::ReadAutoMapperTweaks(*g_tweakAutoMapper);
  g_tweakAutoMapper->initCVars(cvarMgr);

  /* Gui */
  tag = g_ResFactory->GetResourceIdByName("Gui");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag),
               CMemoryInStream::EOwnerShip::Owned);
  g_tweakGui = new MP1::CTweakGui(*strm);
  g_tweakPlayerGun->initCVars(cvarMgr);

  /* PlayerControls */
  tag = g_ResFactory->GetResourceIdByName("PlayerControls");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag),
               CMemoryInStream::EOwnerShip::Owned);
  g_tweakPlayerControl = new MP1::CTweakPlayerControl(*strm);

  /* PlayerControls2 */
  tag = g_ResFactory->GetResourceIdByName("PlayerControls2");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag),
               CMemoryInStream::EOwnerShip::Owned);
  g_tweakPlayerControlAlt = new MP1::CTweakPlayerControl(*strm);

  g_currentPlayerControl = g_tweakPlayerControl;

  /* SlideShow */
  tag = g_ResFactory->GetResourceIdByName("SlideShow");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag),
               CMemoryInStream::EOwnerShip::Owned);
  g_tweakSlideShow = new MP1::CTweakSlideShow(*strm);
  g_tweakSlideShow->initCVars(cvarMgr);
}

void CTweaks::RegisterResourceTweaks(CVarManager* cvarMgr) {
  std::optional<CMemoryInStream> strm;

  const SObjectTag* tag = g_ResFactory->GetResourceIdByName("GunRes");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag),
               CMemoryInStream::EOwnerShip::Owned);
  g_tweakGunRes = new MP1::CTweakGunRes(*strm);
  g_tweakGunRes->ResolveResources(*g_ResFactory);
  g_tweakGunRes->initCVars(cvarMgr);

  tag = g_ResFactory->GetResourceIdByName("PlayerRes");
  strm.emplace(g_ResFactory->LoadResourceSync(*tag).release(), g_ResFactory->ResourceSize(*tag),
               CMemoryInStream::EOwnerShip::Owned);
  g_tweakPlayerRes = new MP1::CTweakPlayerRes(*strm, g_Main->IsTrilogy() || g_Main->IsPAL() || g_Main->IsJapanese());
  g_tweakPlayerRes->ResolveResources(*g_ResFactory);
  g_tweakPlayerRes->initCVars(cvarMgr);
}

} // namespace metaforce::MP1
