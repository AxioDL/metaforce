#include "Runtime/MP1/CSamusHud.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/GuiSys/CGuiCamera.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiLight.hpp"
#include "Runtime/GuiSys/CGuiModel.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptTrigger.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {

CSamusHud* CSamusHud::g_SamusHud = nullptr;

CSamusHud::CSamusHud(CStateManager& stateMgr)
: x8_targetingMgr(stateMgr)
, x258_frmeHelmet(g_SimplePool->GetObj("FRME_Helmet"))
, x268_frmeBaseHud(g_SimplePool->GetObj("FRME_BaseHud"))
, x2e0_27_energyLow(stateMgr.GetPlayer().IsEnergyLow(stateMgr))
, m_energyDrainFilter(g_tweakGui->GetEnergyDrainFilterAdditive() ? EFilterType::Add : EFilterType::Blend) {
  x33c_lights = std::make_unique<CActorLights>(8, zeus::skZero3f, 4, 1, true, 0, 0, 0.1f);
  x340_hudLights.resize(3, SCachedHudLight(zeus::skZero3f, zeus::skWhite, 0.f, 0.f, 0.f, 0.f));
  x46c_.resize(3);
  x568_fpCamDir = stateMgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform().basis[1];
  x5a4_videoBands.resize(4);
  x5d8_guiLights.resize(4);
  x7ac_.resize(15);
  UpdateStateTransition(1.f, stateMgr);
  g_SamusHud = this;

  for (size_t i = 0; i < x5ec_camFovTweaks.size(); ++i) {
    x5ec_camFovTweaks[i] = 5.f * float(i) + 40.f;
  }
  for (size_t i = 0; i < x62c_camYTweaks.size(); ++i) {
    x62c_camYTweaks[i] = -0.5f * float(i);
  }
  for (size_t i = 0; i < x72c_camZTweaks.size(); ++i) {
    x72c_camZTweaks[i] = 0.5f * float(i) - 8.f;
  }

  x264_loadedFrmeHelmet = x258_frmeHelmet.GetObj();
  x264_loadedFrmeHelmet->Reset();
  x264_loadedFrmeHelmet->SetMaxAspect(1.78f);
  x274_loadedFrmeBaseHud = x268_frmeBaseHud.GetObj();
  x274_loadedFrmeBaseHud->Reset();
  x274_loadedFrmeBaseHud->SetMaxAspect(1.78f);
  x2a0_helmetIntf = std::make_unique<CHudHelmetInterface>(*x264_loadedFrmeHelmet);

  rstl::reserved_vector<bool, 4> hasVisors = BuildPlayerHasVisors(stateMgr);
  x2a4_visorMenu = std::make_unique<CHudVisorBeamMenu>(*x274_loadedFrmeBaseHud,
                                                       CHudVisorBeamMenu::EHudVisorBeamMenu::Visor, hasVisors);

  rstl::reserved_vector<bool, 4> hasBeams = BuildPlayerHasBeams(stateMgr);
  x2a8_beamMenu = std::make_unique<CHudVisorBeamMenu>(*x274_loadedFrmeBaseHud,
                                                      CHudVisorBeamMenu::EHudVisorBeamMenu::Beam, hasBeams);

  x2ac_radarIntf = std::make_unique<CHudRadarInterface>(*x274_loadedFrmeBaseHud, stateMgr);

  InitializeFrameGluePermanent(stateMgr);
  UpdateEnergy(0.f, stateMgr, true);
  UpdateMissile(0.f, stateMgr, true);
  UpdateBallMode(stateMgr, true);
}

CSamusHud::~CSamusHud() {
  if (x3a4_damageSfx)
    CSfxManager::RemoveEmitter(x3a4_damageSfx);
  g_SamusHud = nullptr;
}

rstl::reserved_vector<bool, 4> CSamusHud::BuildPlayerHasVisors(const CStateManager& mgr) {
  rstl::reserved_vector<bool, 4> ret;
  ret.push_back(mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::CombatVisor));
  ret.push_back(mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::XRayVisor));
  ret.push_back(mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::ScanVisor));
  ret.push_back(mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::ThermalVisor));
  return ret;
}

rstl::reserved_vector<bool, 4> CSamusHud::BuildPlayerHasBeams(const CStateManager& mgr) {
  rstl::reserved_vector<bool, 4> ret;
  ret.push_back(mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PowerBeam));
  ret.push_back(mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::IceBeam));
  ret.push_back(mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::WaveBeam));
  ret.push_back(mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PlasmaBeam));
  return ret;
}

void CSamusHud::InitializeFrameGluePermanent(const CStateManager& mgr) {
  x588_base_basewidget_pivot = x274_loadedFrmeBaseHud->FindWidget("basewidget_pivot");
  x58c_helmet_BaseWidget_Pivot = x264_loadedFrmeHelmet->FindWidget("BaseWidget_Pivot");
  x590_base_Model_AutoMapper = static_cast<CGuiModel*>(x274_loadedFrmeBaseHud->FindWidget("Model_AutoMapper"));
  x594_base_textpane_counter = static_cast<CGuiTextPane*>(x274_loadedFrmeBaseHud->FindWidget("textpane_counter"));
  x594_base_textpane_counter->TextSupport().SetFontColor(g_tweakGuiColors->GetHudCounterFill());
  x594_base_textpane_counter->TextSupport().SetOutlineColor(g_tweakGuiColors->GetHudCounterOutline());
  x598_base_basewidget_message = x274_loadedFrmeBaseHud->FindWidget("basewidget_message");
  for (CGuiWidget* child = static_cast<CGuiWidget*>(x598_base_basewidget_message->GetChildObject()); child;
       child = static_cast<CGuiWidget*>(child->GetNextSibling()))
    child->SetDepthTest(false);
  x59c_base_textpane_message = static_cast<CGuiTextPane*>(x274_loadedFrmeBaseHud->FindWidget("textpane_message"));
  x5a0_base_model_abutton = static_cast<CGuiModel*>(x274_loadedFrmeBaseHud->FindWidget("model_abutton"));
  for (size_t i = 0; i < x5d8_guiLights.size(); ++i) {
    x5d8_guiLights[i] = x264_loadedFrmeHelmet->GetFrameLight(s32(i));
  }
  x5d8_guiLights[3]->SetColor(zeus::skBlack);
  for (size_t i = 0; i < x5a4_videoBands.size(); ++i) {
    SVideoBand& band = x5a4_videoBands[i];
    band.x0_videoband =
        static_cast<CGuiModel*>(x274_loadedFrmeBaseHud->FindWidget(fmt::format(FMT_STRING("model_videoband{}"), i)));
    band.x4_randA = 6 + (std::rand() % ((66 - 6) + 1));
    band.x8_randB = 16 + (std::rand() % ((256 - 16) + 1));
  }
  x59c_base_textpane_message->SetDepthTest(false);
  x598_base_basewidget_message->SetVisibility(false, ETraversalMode::Children);
  x59c_base_textpane_message->TextSupport().SetFontColor(g_tweakGuiColors->GetHudMessageFill());
  x59c_base_textpane_message->TextSupport().SetOutlineColor(g_tweakGuiColors->GetHudMessageOutline());
  x59c_base_textpane_message->TextSupport().SetControlTXTRMap(&g_GameState->GameOptions().GetControlTXTRMap());
  x590_base_Model_AutoMapper->SetDepthWrite(true);
  x304_basewidgetIdlePos = x588_base_basewidget_pivot->GetIdlePosition();
  x310_cameraPos = x274_loadedFrmeBaseHud->GetFrameCamera()->GetLocalPosition();
  RefreshHudOptions();
}

void CSamusHud::InitializeFrameGlueMutable(const CStateManager& mgr) {
  float lastTankEnergy = std::fmod(x2d0_playerHealth, CPlayerState::GetEnergyTankCapacity());
  u32 tanksFilled = x2d0_playerHealth / CPlayerState::GetEnergyTankCapacity();

  CPlayer& player = mgr.GetPlayer();
  CPlayerState& playerState = *mgr.GetPlayerState();
  CPlayerGun& gun = *player.GetPlayerGun();
  float chargeFactor = gun.IsCharging() ? gun.GetChargeBeamFactor() : 0.f;
  bool missilesActive = gun.GetMissleMode() == CPlayerGun::EMissileMode::Active;
  bool lockedOnObj = player.GetOrbitTargetId() != kInvalidUniqueId;

  switch (x2bc_nextState) {
  case EHudState::Combat: {
    x2b4_bossEnergyIntf = std::make_unique<CHudBossEnergyInterface>(*x288_loadedSelectedHud);

    x28c_energyIntf =
        std::make_unique<CHudEnergyInterface>(*x288_loadedSelectedHud, lastTankEnergy, x2d4_totalEnergyTanks,
                                              tanksFilled, bool(x2e0_27_energyLow), EHudType::Combat);

    if (!x290_threatIntf)
      x290_threatIntf = std::make_unique<CHudThreatInterface>(*x288_loadedSelectedHud, EHudType::Combat, 9999.f);
    else
      x290_threatIntf->SetIsVisibleGame(true);

    if (!x294_missileIntf)
      x294_missileIntf =
          std::make_unique<CHudMissileInterface>(*x288_loadedSelectedHud, x2dc_missileCapacity, x2d8_missileAmount,
                                                 chargeFactor, missilesActive, EHudType::Combat, mgr);
    else
      x294_missileIntf->SetIsVisibleGame(true, mgr);

    if (!x298_freeLookIntf)
      x298_freeLookIntf =
          std::make_unique<CHudFreeLookInterface>(*x288_loadedSelectedHud, EHudType::Combat, bool(x2e0_24_inFreeLook),
                                                  bool(x2e0_25_lookControlHeld), lockedOnObj);
    else
      x298_freeLookIntf->SetIsVisibleGame(true);

    if (!x29c_decoIntf)
      x29c_decoIntf = std::make_unique<CHudDecoInterfaceCombat>(*x288_loadedSelectedHud);
    else
      x29c_decoIntf->SetIsVisibleGame(true);

    x2ac_radarIntf->SetIsVisibleGame(true);
    x2a4_visorMenu->SetIsVisibleGame(true);
    x2a8_beamMenu->SetIsVisibleGame(true);
    InitializeDamageLight();
    UpdateEnergy(0.f, mgr, true);
    break;
  }
  case EHudState::Ball: {
    u32 numPBs = playerState.GetItemAmount(CPlayerState::EItemType::PowerBombs);
    u32 pbCap = playerState.GetItemCapacity(CPlayerState::EItemType::PowerBombs);
    u32 bombsAvailable;
    if (gun.IsBombReady())
      bombsAvailable = gun.GetBombCount();
    else
      bombsAvailable = 0;
    x2b0_ballIntf = std::make_unique<CHudBallInterface>(
        *x288_loadedSelectedHud, numPBs, pbCap, bombsAvailable,
        gun.IsPowerBombReady() && player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed,
        playerState.HasPowerUp(CPlayerState::EItemType::MorphBallBombs));

    x28c_energyIntf =
        std::make_unique<CHudEnergyInterface>(*x288_loadedSelectedHud, lastTankEnergy, x2d4_totalEnergyTanks,
                                              tanksFilled, bool(x2e0_27_energyLow), EHudType::Ball);

    x290_threatIntf.reset();
    x294_missileIntf.reset();
    x298_freeLookIntf.reset();
    x29c_decoIntf.reset();
    x3d4_damageLight = nullptr;

    x2ac_radarIntf->SetIsVisibleGame(false);
    x2a4_visorMenu->SetIsVisibleGame(false);
    x2a8_beamMenu->SetIsVisibleGame(false);
    UpdateEnergy(0.f, mgr, true);
    break;
  }
  case EHudState::Scan: {
    x2b4_bossEnergyIntf.reset();

    x28c_energyIntf =
        std::make_unique<CHudEnergyInterface>(*x288_loadedSelectedHud, lastTankEnergy, x2d4_totalEnergyTanks,
                                              tanksFilled, bool(x2e0_27_energyLow), EHudType::Scan);

    x290_threatIntf.reset();
    x294_missileIntf.reset();

    x298_freeLookIntf = std::make_unique<CHudFreeLookInterface>(
        *x288_loadedSelectedHud, EHudType::Scan, bool(x2e0_24_inFreeLook), bool(x2e0_25_lookControlHeld), lockedOnObj);

    x29c_decoIntf = std::make_unique<CHudDecoInterfaceScan>(*x288_loadedSelectedHud);
    InitializeDamageLight();
    UpdateEnergy(0.f, mgr, true);
    break;
  }
  case EHudState::XRay: {
    x2b4_bossEnergyIntf = std::make_unique<CHudBossEnergyInterface>(*x288_loadedSelectedHud);

    x28c_energyIntf =
        std::make_unique<CHudEnergyInterface>(*x288_loadedSelectedHud, lastTankEnergy, x2d4_totalEnergyTanks,
                                              tanksFilled, bool(x2e0_27_energyLow), EHudType::XRay);

    x290_threatIntf = std::make_unique<CHudThreatInterface>(*x288_loadedSelectedHud, EHudType::XRay, 9999.f);

    x294_missileIntf =
        std::make_unique<CHudMissileInterface>(*x288_loadedSelectedHud, x2dc_missileCapacity, x2d8_missileAmount,
                                               chargeFactor, missilesActive, EHudType::XRay, mgr);

    x298_freeLookIntf = std::make_unique<CHudFreeLookInterfaceXRay>(*x288_loadedSelectedHud, bool(x2e0_24_inFreeLook),
                                                                    bool(x2e0_25_lookControlHeld), lockedOnObj);

    x29c_decoIntf = std::make_unique<CHudDecoInterfaceXRay>(*x288_loadedSelectedHud);
    InitializeDamageLight();
    x2a4_visorMenu->SetIsVisibleGame(true);
    x2a8_beamMenu->SetIsVisibleGame(true);
    UpdateEnergy(0.f, mgr, true);
    break;
  }
  case EHudState::Thermal: {
    x2b4_bossEnergyIntf = std::make_unique<CHudBossEnergyInterface>(*x288_loadedSelectedHud);

    x28c_energyIntf =
        std::make_unique<CHudEnergyInterface>(*x288_loadedSelectedHud, lastTankEnergy, x2d4_totalEnergyTanks,
                                              tanksFilled, bool(x2e0_27_energyLow), EHudType::Thermal);

    x290_threatIntf = std::make_unique<CHudThreatInterface>(*x288_loadedSelectedHud, EHudType::Thermal, 9999.f);

    x294_missileIntf =
        std::make_unique<CHudMissileInterface>(*x288_loadedSelectedHud, x2dc_missileCapacity, x2d8_missileAmount,
                                               chargeFactor, missilesActive, EHudType::Thermal, mgr);

    x298_freeLookIntf =
        std::make_unique<CHudFreeLookInterface>(*x288_loadedSelectedHud, EHudType::Thermal, bool(x2e0_24_inFreeLook),
                                                bool(x2e0_25_lookControlHeld), lockedOnObj);

    x29c_decoIntf = std::make_unique<CHudDecoInterfaceThermal>(*x288_loadedSelectedHud);
    InitializeDamageLight();
    x2a4_visorMenu->SetIsVisibleGame(true);
    x2a8_beamMenu->SetIsVisibleGame(true);
    UpdateEnergy(0.f, mgr, true);
    break;
  }
  case EHudState::None:
    UninitializeFrameGlueMutable();
    break;
  default:
    break;
  }
}

void CSamusHud::UninitializeFrameGlueMutable() {
  x2b4_bossEnergyIntf.reset();
  x28c_energyIntf.reset();
  x29c_decoIntf.reset();
  x290_threatIntf.reset();
  x294_missileIntf.reset();
  x298_freeLookIntf.reset();
  x2b0_ballIntf.reset();
  x3d4_damageLight = nullptr;
}

void CSamusHud::InitializeDamageLight() {
  s16 lightId = x288_loadedSelectedHud->GetWidgetIdDB().AddWidget("DamageSpotLight");
  s16 parentId = x288_loadedSelectedHud->FindWidget("basewidget_pivot")->GetSelfId();
  CGuiWidget::CGuiWidgetParms parms(x288_loadedSelectedHud, false, lightId, parentId, true, true, false,
                                    g_tweakGuiColors->GetHudDamageLightColor(), CGuiWidget::EGuiModelDrawFlags::Alpha,
                                    false, false, "DamageSpotLight"s);

  std::shared_ptr<CGuiLight> light = std::make_shared<CGuiLight>(
      parms, CLight::BuildSpot(zeus::skZero3f, zeus::skForward, zeus::skWhite,
                               g_tweakGui->GetHudDamageLightSpotAngle()));
  x3d4_damageLight = light.get();
  x3d4_damageLight->SetColor(zeus::skBlack);

  zeus::CColor lightColor = g_tweakGuiColors->GetHudFrameColor();
  lightColor *= lightColor.a();
  lightColor.a() = 1.f;
  x3d4_damageLight->SetAmbientLightColor(lightColor);

  x3d4_damageLight->SetDistC(1.f);
  x3d4_damageLight->SetDistL(0.f);
  x3d4_damageLight->SetAngleC(g_tweakGui->GetDamageLightAngleC());
  x3d4_damageLight->SetAngleL(g_tweakGui->GetDamageLightAngleL());
  x3d4_damageLight->SetAngleQ(g_tweakGui->GetDamageLightAngleQ());
  x3d4_damageLight->SetLightId(4);

  x3d4_damageLight->SetLocalTransform(zeus::CTransform());

  x288_loadedSelectedHud->RegisterLight(std::move(light));
  x288_loadedSelectedHud->FindWidget(parentId)->AddChildWidget(x3d4_damageLight, false, true);
  x288_loadedSelectedHud->AddLight(x3d4_damageLight);

  zeus::CTransform lightXf = zeus::CTransform::Translate(g_tweakGui->GetDamageLightPreTranslate());

  x3d8_lightTransforms.clear();
  x3d8_lightTransforms.reserve(10);

  zeus::CTransform negX = zeus::CTransform::RotateX(zeus::degToRad(-g_tweakGui->GetDamageLightXfXAngle()));
  zeus::CTransform posX = zeus::CTransform::RotateX(zeus::degToRad(g_tweakGui->GetDamageLightXfXAngle()));
  zeus::CTransform negZ = zeus::CTransform::RotateZ(zeus::degToRad(-g_tweakGui->GetDamageLightXfZAngle()));
  zeus::CTransform posZ = zeus::CTransform::RotateZ(zeus::degToRad(g_tweakGui->GetDamageLightXfZAngle()));

  x3d8_lightTransforms.push_back(lightXf);
  x3d8_lightTransforms.push_back(zeus::CTransform::Translate(g_tweakGui->GetDamageLightCenterTranslate()) * lightXf);
  x3d8_lightTransforms.push_back(posX * lightXf);
  x3d8_lightTransforms.push_back(posX * negZ * lightXf);
  x3d8_lightTransforms.push_back(negZ * lightXf);
  x3d8_lightTransforms.push_back(negX * negZ * lightXf);
  x3d8_lightTransforms.push_back(negX * lightXf);
  x3d8_lightTransforms.push_back(negX * posZ * lightXf);
  x3d8_lightTransforms.push_back(posZ * lightXf);
  x3d8_lightTransforms.push_back(posX * posZ * lightXf);
}

void CSamusHud::UpdateEnergy(float dt, const CStateManager& mgr, bool init) {
  CPlayer& player = mgr.GetPlayer();
  CPlayerState& playerState = *mgr.GetPlayerState();
  float energy = std::max(0.f, std::ceil(playerState.GetHealthInfo().GetHP()));

  u32 numEnergyTanks = playerState.GetItemCapacity(CPlayerState::EItemType::EnergyTanks);
  x2e0_27_energyLow = player.IsEnergyLow(mgr);

  if (init || energy != x2d0_playerHealth || numEnergyTanks != x2d4_totalEnergyTanks) {
    float lastTankEnergy = energy;
    u32 filledTanks = 0;
    while (lastTankEnergy > CPlayerState::GetBaseHealthCapacity()) {
      ++filledTanks;
      lastTankEnergy -= CPlayerState::GetEnergyTankCapacity();
    }

    if (x2bc_nextState != EHudState::None) {
      if (x28c_energyIntf) {
        float curLastTankEnergy = x2d0_playerHealth;
        while (curLastTankEnergy > CPlayerState::GetBaseHealthCapacity())
          curLastTankEnergy -= CPlayerState::GetEnergyTankCapacity();
        x28c_energyIntf->SetCurrEnergy(lastTankEnergy,
          (curLastTankEnergy > lastTankEnergy) != (x2d0_playerHealth > energy));
      }
      x2d0_playerHealth = energy;
      if (x28c_energyIntf) {
        x28c_energyIntf->SetNumTotalEnergyTanks(numEnergyTanks);
        x28c_energyIntf->SetNumFilledEnergyTanks(filledTanks);
        x28c_energyIntf->SetEnergyLow(x2e0_27_energyLow);
      }
      x2d4_totalEnergyTanks = numEnergyTanks;
    }
  }

  if (x2b4_bossEnergyIntf) {
    const CEntity* bossEnt = mgr.GetObjectById(mgr.GetBossId());
    if (TCastToConstPtr<CActor> act = bossEnt) {
      if (const CHealthInfo* hInfo = act->GetHealthInfo(mgr)) {
        float bossEnergy = std::ceil(hInfo->GetHP());
        x2b4_bossEnergyIntf->SetBossParams(true, g_MainStringTable->GetString(mgr.GetBossStringIdx()), bossEnergy,
                                           mgr.GetTotalBossEnergy());
      } else {
        x2b4_bossEnergyIntf->SetBossParams(false, u"", 0.f, 0.f);
      }
    } else {
      x2b4_bossEnergyIntf->SetBossParams(false, u"", 0.f, 0.f);
    }
  }
}

void CSamusHud::UpdateFreeLook(float dt, const CStateManager& mgr) {
  TCastToConstPtr<CFirstPersonCamera> fpCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  CPlayer& player = mgr.GetPlayer();
  bool inFreeLook = player.IsInFreeLook() && fpCam;
  bool lookControlHeld = player.GetFreeLookStickState();
  if (x2e0_24_inFreeLook != inFreeLook) {
    if (inFreeLook)
      CSfxManager::SfxStart(SFXui_into_freelook, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    else
      CSfxManager::SfxStart(SFXui_outof_freelook, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    x2e0_24_inFreeLook = inFreeLook;
  }

  float deltaFrames = 60.f * dt * 0.999999f;
  float oldLookDeltaDot = x574_lookDeltaDot;
  zeus::CVector3f fpCamDir;
  if (!fpCam)
    fpCamDir = x568_fpCamDir;
  else
    fpCamDir = fpCam->GetTransform().basis[1];

  bool lookAdj = inFreeLook ? lookControlHeld : false;

  if (lookAdj) {
    x574_lookDeltaDot = fpCamDir.dot(x568_fpCamDir);
    if (std::fabs(x574_lookDeltaDot) > 1.f)
      x574_lookDeltaDot = (x574_lookDeltaDot >= 0.f) ? 1.f : -1.f;
  } else {
    x574_lookDeltaDot = 1.f;
  }

  x568_fpCamDir = fpCamDir;

  if ((oldLookDeltaDot >= deltaFrames && x574_lookDeltaDot < deltaFrames) ||
      (oldLookDeltaDot < deltaFrames && x574_lookDeltaDot >= deltaFrames)) {
    x578_freeLookSfxCycleTimer = 0.f;
  } else if (x578_freeLookSfxCycleTimer < 0.05f) {
    x578_freeLookSfxCycleTimer = std::min(x578_freeLookSfxCycleTimer + dt, 0.05f);
    if (x578_freeLookSfxCycleTimer == 0.05f) {
      if (x574_lookDeltaDot < deltaFrames) {
        if (!x564_freeLookSfx)
          x564_freeLookSfx = CSfxManager::SfxStart(SFXui_freelook_move_lp, 1.f, 0.f, true, 0x7f, true, kInvalidAreaId);
      } else {
        CSfxManager::SfxStop(x564_freeLookSfx);
        x564_freeLookSfx.reset();
      }
    }
  }

  if (fpCam) {
    zeus::CMatrix3f camRot = fpCam->GetTransform().buildMatrix3f();
    zeus::CVector3f camDir(camRot[1]);
    zeus::CUnitVector3f camDirNoZ = camDir;
    camDirNoZ.z() = 0.f;
    float offHorizonDot = camDir.dot(camDirNoZ);
    if (std::fabs(offHorizonDot) > 1.f)
      offHorizonDot = (offHorizonDot >= 0.f) ? 1.f : -1.f;
    float offHorizonAngle = std::fabs(std::acos(offHorizonDot));
    if (camDir.z() < 0.f)
      offHorizonAngle = -offHorizonAngle;

    if (x298_freeLookIntf)
      x298_freeLookIntf->SetFreeLookState(inFreeLook, lookControlHeld, player.GetOrbitTargetId() != kInvalidUniqueId,
                                          offHorizonAngle);

    if (x564_freeLookSfx) {
      float pitch = offHorizonAngle * (g_tweakGui->GetFreeLookSfxPitchScale() / 8192.f) / (M_PIF / 2.f);
      if (!g_tweakGui->GetNoAbsoluteFreeLookSfxPitch())
        pitch = std::fabs(pitch);
      CSfxManager::PitchBend(x564_freeLookSfx, pitch);
    }
  }
}

void CSamusHud::UpdateMissile(float dt, const CStateManager& mgr, bool init) {
  CPlayerGun& gun = *mgr.GetPlayer().GetPlayerGun();
  CPlayerState& playerState = *mgr.GetPlayerState();

  u32 numMissles = playerState.GetItemAmount(CPlayerState::EItemType::Missiles);
  u32 missileCap = playerState.GetItemCapacity(CPlayerState::EItemType::Missiles);
  CPlayerGun::EMissileMode missileMode = gun.GetMissleMode();
  float chargeFactor = gun.IsCharging() ? gun.GetChargeBeamFactor() : 0.f;

  if (x294_missileIntf)
    x294_missileIntf->SetChargeBeamFactor(chargeFactor);

  if (init || numMissles != x2d8_missileAmount || missileMode != x2ec_missileMode ||
      missileCap != x2dc_missileCapacity) {
    if (x294_missileIntf) {
      if (missileCap != x2dc_missileCapacity)
        x294_missileIntf->SetMissileCapacity(missileCap);
      if (numMissles != x2d8_missileAmount)
        x294_missileIntf->SetNumMissiles(numMissles, mgr);
      if (missileMode != x2ec_missileMode)
        x294_missileIntf->SetIsMissilesActive(missileMode == CPlayerGun::EMissileMode::Active);
    }
    x2d8_missileAmount = numMissles;
    x2ec_missileMode = missileMode;
    x2dc_missileCapacity = missileCap;
  }
}

void CSamusHud::UpdateVideoBands(float dt, const CStateManager& mgr) {
  for (auto& videoBand : x5a4_videoBands) {
    if (videoBand.x0_videoband) {
      videoBand.x0_videoband->SetIsVisible(false);
    }
  }
}

void CSamusHud::UpdateBallMode(const CStateManager& mgr, bool init) {
  if (!x2b0_ballIntf)
    return;

  CPlayer& player = mgr.GetPlayer();
  CPlayerGun& gun = *player.GetPlayerGun();
  CPlayerState& playerState = *mgr.GetPlayerState();
  u32 numPbs = playerState.GetItemAmount(CPlayerState::EItemType::PowerBombs);
  u32 pbCap = playerState.GetItemCapacity(CPlayerState::EItemType::PowerBombs);
  u32 bombCount = gun.IsBombReady() ? gun.GetBombCount() : 0;
  bool hasBombs = playerState.HasPowerUp(CPlayerState::EItemType::MorphBallBombs);
  bool pbReady =
      gun.IsPowerBombReady() && player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed;

  x2b0_ballIntf->SetBombParams(numPbs, pbCap, bombCount, hasBombs, pbReady, false);
}

void CSamusHud::UpdateThreatAssessment(float dt, const CStateManager& mgr) {
  CMaterialFilter filter(CMaterialList(EMaterialTypes::Trigger), CMaterialList(),
                         CMaterialFilter::EFilterType::Include);

  CPlayer& player = mgr.GetPlayer();
  zeus::CAABox playerAABB = zeus::skNullBox;
  if (std::optional<zeus::CAABox> aabb = player.GetTouchBounds())
    playerAABB = *aabb;

  zeus::CAABox aabb;
  aabb.accumulateBounds(player.GetTranslation() - g_tweakGui->GetThreatRange());
  aabb.accumulateBounds(player.GetTranslation() + g_tweakGui->GetThreatRange());
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildNearList(nearList, aabb, filter, nullptr);

  float threatDist = 9999.f;
  for (TUniqueId id : nearList) {
    const CEntity* ent = mgr.GetObjectById(id);
    if (TCastToConstPtr<CScriptTrigger> trigger = ent) {
      if (!bool(trigger->GetTriggerFlags() & ETriggerFlags::DetectPlayer))
        continue;
      if (trigger->GetDamageInfo().GetDamage() == 0.f)
        continue;
      if (std::optional<zeus::CAABox> aabb = trigger->GetTouchBounds()) {
        float dist = playerAABB.distanceBetween(*aabb);
        if (dist < threatDist)
          threatDist = dist;
      }
    }
  }

  if (player.GetThreatOverride() > 0.f)
    threatDist = std::min((1.f - player.GetThreatOverride()) * g_tweakGui->GetThreatRange(), threatDist);

  if (mgr.IsFullThreat())
    threatDist = 0.f;
  if (x290_threatIntf)
    x290_threatIntf->SetThreatDistance(threatDist);
}

void CSamusHud::UpdateVisorAndBeamMenus(float dt, const CStateManager& mgr) {
  CPlayer& player = mgr.GetPlayer();
  CPlayerGun& gun = *player.GetPlayerGun();
  CPlayerState& playerState = *mgr.GetPlayerState();

  float beamInterp = zeus::clamp(0.f, gun.GetHoloTransitionFactor(), 1.f);
  float visorInterp = playerState.GetVisorTransitionFactor();

  if (x2a8_beamMenu) {
    x2a8_beamMenu->SetSelection(int(gun.GetCurrentBeam()), int(gun.GetNextBeam()), beamInterp);
    x2a8_beamMenu->SetPlayerHas(BuildPlayerHasBeams(mgr));
  }

  if (x2a4_visorMenu) {
    x2a4_visorMenu->SetSelection(int(playerState.GetCurrentVisor()), int(playerState.GetTransitioningVisor()),
                                 visorInterp);
    x2a4_visorMenu->SetPlayerHas(BuildPlayerHasVisors(mgr));
  }
}

void CSamusHud::UpdateCameraDebugSettings() {
  const float fov = x5ec_camFovTweaks[g_tweakGui->GetHudCamFovTweak()];
  const float y = x62c_camYTweaks[g_tweakGui->GetHudCamYTweak()];
  const float z = x72c_camZTweaks[g_tweakGui->GetHudCamZTweak()];
  if (x2a0_helmetIntf) {
    x2a0_helmetIntf->UpdateCameraDebugSettings(fov, y, z);
  }
  if (x29c_decoIntf) {
    x29c_decoIntf->UpdateCameraDebugSettings(fov, y, z);
  }
  x274_loadedFrmeBaseHud->GetFrameCamera()->SetFov(fov);
  x310_cameraPos.y() = y;
  x310_cameraPos.z() = z;
}

void CSamusHud::UpdateEnergyLow(float dt, const CStateManager& mgr) {
  const bool cineCam = TCastToConstPtr<CCinematicCamera>(mgr.GetCameraManager()->GetCurrentCamera(mgr)).IsValid();
  float oldTimer = x57c_energyLowTimer;

  x57c_energyLowTimer = std::fmod(x57c_energyLowTimer + dt, 0.5f);
  if (x57c_energyLowTimer < 0.25f)
    x580_energyLowPulse = x57c_energyLowTimer / 0.25f;
  else
    x580_energyLowPulse = (0.5f - x57c_energyLowTimer) / 0.25f;

  if (!cineCam && x2e0_27_energyLow && x57c_energyLowTimer < oldTimer)
    CSfxManager::SfxStart(SFXui_energy_low, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
}

void CSamusHud::ApplyClassicLag(const zeus::CUnitVector3f& lookDir, zeus::CQuaternion& rot, const CStateManager& mgr,
                                float dt, bool invert) {
  zeus::CQuaternion lookRot = zeus::CQuaternion::lookAt(lookDir, zeus::skForward, 2.f * M_PIF);
  zeus::CQuaternion lookRot2;
  if (invert) {
    zeus::CUnitVector3f v1(lookRot.transform(x2f8_fpCamDir));
    lookRot2 = zeus::CQuaternion::lookAt(v1, zeus::skForward, 2.f * M_PIF);
  } else {
    zeus::CUnitVector3f v1(lookRot.transform(x2f8_fpCamDir));
    lookRot2 = zeus::CQuaternion::lookAt(zeus::skForward, v1, 2.f * M_PIF);
  }

  zeus::CQuaternion doubleRot = lookRot2 * lookRot2;
  float dot = doubleRot.toTransform().basis[1].dot(rot.toTransform().basis[1]);
  if (std::fabs(dot) > 1.f)
    dot = (dot >= 0.f) ? 1.f : -1.f;

  float angle = std::acos(dot);
  float tmp = 0.f;
  if (angle > 0.f)
    tmp = 0.5f * dt * g_tweakPlayer->GetFreeLookSpeed() / angle;

  float t = zeus::clamp(0.f, 18.f * dt * tmp, 1.f);
  rot = zeus::CQuaternion::slerp(rot, doubleRot, t);
}

void CSamusHud::UpdateHudLag(float dt, const CStateManager& mgr) {
  CPlayer& player = mgr.GetPlayer();

  if (x29c_decoIntf)
    x29c_decoIntf->SetDecoRotation(player.GetYaw());

  if (!g_GameState->GameOptions().GetHUDLag()) {
    if (x2a0_helmetIntf) {
      x2a0_helmetIntf->SetHudLagRotation(zeus::CMatrix3f());
      x2a0_helmetIntf->SetHudLagOffset(zeus::skZero3f);
    }
    if (x29c_decoIntf) {
      x29c_decoIntf->SetReticuleTransform(zeus::CMatrix3f());
      x29c_decoIntf->SetHudRotation(zeus::CQuaternion());
      x29c_decoIntf->SetHudOffset(zeus::skZero3f);
    }
    x588_base_basewidget_pivot->SetTransform(
        zeus::CTransform::Translate(x588_base_basewidget_pivot->GetWorldPosition()));
    x274_loadedFrmeBaseHud->GetFrameCamera()->SetO2WTransform(
        BuildFinalCameraTransform(zeus::CQuaternion(), x304_basewidgetIdlePos, x310_cameraPos));
    x8_targetingMgr.CompoundTargetReticle().SetLeadingOrientation(zeus::CQuaternion());
  } else {
    zeus::CVector3f fpCamDir = x2f8_fpCamDir;
    if (TCastToConstPtr<CFirstPersonCamera> fpCam = mgr.GetCameraManager()->GetCurrentCamera(mgr))
      fpCamDir = fpCam->GetTransform().buildMatrix3f()[1];

    ApplyClassicLag(fpCamDir, x31c_hudLag, mgr, dt, false);
    ApplyClassicLag(fpCamDir, x32c_invHudLag, mgr, dt, true);

    zeus::CQuaternion rot = zeus::CQuaternion::lookAt(zeus::CUnitVector3f(x2f8_fpCamDir), fpCamDir, 2.f * M_PIF);
    rot *= rot;
    rot *= rot;
    x8_targetingMgr.CompoundTargetReticle().SetLeadingOrientation(rot);

    zeus::CVector3f bobTranslation = player.GetCameraBob()->GetHelmetBobTranslation();

    zeus::CQuaternion lagRot = x44c_hudLagShakeRot * x31c_hudLag;
    zeus::CVector3f lagOff = x41c_decoShakeTranslate * g_tweakGui->GetHudLagOffsetScale();
    lagOff.z() += bobTranslation.z();
    if (x2a0_helmetIntf) {
      x2a0_helmetIntf->SetHudLagRotation(lagRot);
      x2a0_helmetIntf->SetHudLagOffset(lagOff);
    }
    if (x29c_decoIntf) {
      x29c_decoIntf->SetReticuleTransform(x32c_invHudLag);
      x29c_decoIntf->SetHudRotation(lagRot);
      x29c_decoIntf->SetHudOffset(lagOff);
    }

    x274_loadedFrmeBaseHud->GetFrameCamera()->SetO2WTransform(
        BuildFinalCameraTransform(lagRot, x304_basewidgetIdlePos + lagOff, x310_cameraPos));
    x2f8_fpCamDir = fpCamDir;
  }
}

bool CSamusHud::IsCachedLightInAreaLights(const SCachedHudLight& light, const CActorLights& areaLights) const {
  for (const CLight& l : areaLights.GetAreaLights()) {
    if (l.GetColor() != light.xc_color || l.GetPosition() != light.x0_pos)
      continue;
    return true;
  }
  return false;
}

bool CSamusHud::IsAreaLightInCachedLights(const CLight& light) const {
  for (const SCachedHudLight& l : x340_hudLights) {
    if (l.x1c_fader == 0.f)
      continue;
    if (l.xc_color != light.GetColor() || l.x0_pos != light.GetPosition())
      continue;
    return true;
  }
  return false;
}

int CSamusHud::FindEmptyHudLightSlot(const CLight& light) const {
  for (size_t i = 0; i < x340_hudLights.size(); ++i) {
    if (x340_hudLights[i].x1c_fader == 0.f) {
      return int(i);
    }
  }
  return -1;
}

zeus::CColor CSamusHud::GetVisorHudLightColor(const zeus::CColor& color, const CStateManager& mgr) const {
  zeus::CColor ret = color;
  const CPlayerState& playerState = *mgr.GetPlayerState();
  float t = playerState.GetVisorTransitionFactor();
  switch (playerState.GetCurrentVisor()) {
  case CPlayerState::EPlayerVisor::Scan:
    ret *= zeus::CColor::lerp(zeus::skWhite, g_tweakGuiColors->GetScanVisorHudLightMultiply(), t);
    break;
  case CPlayerState::EPlayerVisor::Thermal:
    ret *= g_tweakGuiColors->GetThermalVisorHudLightMultiply();
    break;
  case CPlayerState::EPlayerVisor::XRay:
    ret = zeus::CColor(zeus::CColor(0.3f, 0.6f, 0.1f).rgbDot(ret));
    break;
  default:
    break;
  }
  return ret;
}

void CSamusHud::UpdateHudDynamicLights(float dt, const CStateManager& mgr) {
  if (TCastToConstPtr<CFirstPersonCamera> fpCam = mgr.GetCameraManager()->GetCurrentCamera(mgr)) {
    zeus::CVector3f lookDir = fpCam->GetTransform().basis[1];
    zeus::CAABox camAABB(fpCam->GetTranslation() - 0.125f, fpCam->GetTranslation() + 0.125f);
    TAreaId playerArea = mgr.GetPlayer().GetAreaIdAlways();
    if (playerArea == kInvalidAreaId)
      return;
    x33c_lights->BuildAreaLightList(mgr, *mgr.GetWorld()->GetAreaAlways(playerArea), camAABB);
    for (SCachedHudLight& light : x340_hudLights)
      if (light.x1c_fader > 0.f && ((light.x0_pos - fpCam->GetTranslation()).normalized().dot(lookDir) <= 0.15707964f ||
                                    !IsCachedLightInAreaLights(light, *x33c_lights)))
        light.x1c_fader *= -1.f;
    int negCount = 0;
    for (SCachedHudLight& light : x340_hudLights)
      if (light.x1c_fader <= 0.f)
        ++negCount;
    --negCount;
    for (const CLight& light : x33c_lights->GetAreaLights()) {
      if (negCount < 1)
        break;
      if (IsAreaLightInCachedLights(light))
        continue;
      if ((light.GetPosition() - fpCam->GetTranslation()).normalized().dot(lookDir) > 0.15707964f) {
        int slot = FindEmptyHudLightSlot(light);
        if (slot == -1)
          continue;
        --negCount;
        SCachedHudLight& cachedLight = x340_hudLights[slot];
        cachedLight.x0_pos = light.GetPosition();
        cachedLight.xc_color = light.GetColor();
        cachedLight.x10_distC = light.GetAttenuationConstant();
        cachedLight.x14_distL = light.GetAttenuationLinear();
        cachedLight.x18_distQ = light.GetAttenuationQuadratic();
        cachedLight.x1c_fader = 0.001f;
      }
    }

    float dt2 = 2.f * dt;
    for (SCachedHudLight& light : x340_hudLights) {
      if (light.x1c_fader < 0.f)
        light.x1c_fader = std::min(0.f, light.x1c_fader + dt2);
      else if (light.x1c_fader < 1.f && light.x1c_fader != 0.f)
        light.x1c_fader = std::min(light.x1c_fader + dt2, 1.f);
    }

    CPlayerState& playerState = *mgr.GetPlayerState();
    CPlayerState::EPlayerVisor visor = playerState.GetCurrentVisor();
    float visorT = playerState.GetVisorTransitionFactor();
    zeus::CColor lightAdd =
        zeus::CColor::lerp(g_tweakGui->GetVisorHudLightAdd(0), g_tweakGui->GetVisorHudLightAdd(int(visor)), visorT);
    zeus::CColor lightMul = zeus::CColor::lerp(g_tweakGui->GetVisorHudLightMultiply(0),
                                               g_tweakGui->GetVisorHudLightMultiply(int(visor)), visorT);

    auto lightIt = x5d8_guiLights.begin();
    float maxIntensity = 0.f;
    int maxIntensityIdx = 0;
    for (size_t i = 0; i < x340_hudLights.size(); ++i) {
      SCachedHudLight& light = x340_hudLights[i];
      CGuiLight* lightWidget = *lightIt++;
      zeus::CVector3f lightToCam = fpCam->GetTranslation() - light.x0_pos;
      zeus::CVector3f lightNormal = fpCam->GetTransform().buildMatrix3f() * lightToCam.normalized();
      float dist = std::max(lightToCam.magnitude(), FLT_EPSILON);
      float falloffMul = 1.f / (dist * dist * light.x18_distQ * g_tweakGui->GetHudLightAttMulQuadratic() +
                                dist * light.x14_distL * g_tweakGui->GetHudLightAttMulLinear() +
                                light.x10_distC * g_tweakGui->GetHudLightAttMulConstant());
      falloffMul = std::min(falloffMul, 1.f);
      lightWidget->SetO2WTransform(zeus::lookAt(zeus::skZero3f, lightNormal));
      float fadedFalloff = falloffMul * std::fabs(light.x1c_fader);
      zeus::CColor lightColor = GetVisorHudLightColor(light.xc_color * zeus::CColor(fadedFalloff), mgr);
      lightWidget->SetColor(lightColor);
      lightAdd += lightColor * lightMul;
      float greyscale =
          fadedFalloff * zeus::skForward.dot(-lightNormal) * lightAdd.rgbDot(zeus::CColor(0.3f, 0.6f, 0.1f));
      if (greyscale > maxIntensity) {
        maxIntensity = greyscale;
        maxIntensityIdx = int(i);
      }
    }

    CLight brightestGameLight = CLight::BuildPoint(zeus::skZero3f, zeus::skBlack);
    for (CEntity* ent : mgr.GetLightObjectList()) {
      if (!ent || !ent->GetActive())
        continue;
      CGameLight& gameLight = static_cast<CGameLight&>(*ent);
      if (TCastToConstPtr<CGameProjectile>(mgr.GetObjectById(gameLight.GetParentId())))
        continue;
      CLight thisLight = gameLight.GetLight();
      if (thisLight.GetIntensity() > brightestGameLight.GetIntensity()) {
        zeus::CSphere sphere(thisLight.GetPosition(), thisLight.GetRadius());
        if (camAABB.intersects(sphere))
          brightestGameLight = thisLight;
      }
    }

    if (brightestGameLight.GetIntensity() > FLT_EPSILON) {
      zeus::CVector3f lightToCam = fpCam->GetTranslation() - brightestGameLight.GetPosition();
      float dist = std::max(lightToCam.magnitude(), FLT_EPSILON);
      float falloffMul =
          1.f / (dist * dist * brightestGameLight.GetAttenuationQuadratic() * g_tweakGui->GetHudLightAttMulQuadratic() +
                 dist * brightestGameLight.GetAttenuationLinear() * g_tweakGui->GetHudLightAttMulLinear() +
                 brightestGameLight.GetAttenuationConstant() * g_tweakGui->GetHudLightAttMulConstant());
      falloffMul = std::min(falloffMul, 1.f);
      zeus::CColor falloffColor = brightestGameLight.GetColor() * zeus::CColor(falloffMul);
      falloffColor = GetVisorHudLightColor(falloffColor, mgr);
      if (brightestGameLight.GetType() == ELightType::Spot) {
        float quarterCicleFactor = zeus::clamp(
            0.f,
            std::asin(std::max(0.f, fpCam->GetTransform().basis[1].dot(brightestGameLight.GetDirection()))) *
                (M_PIF / 2.f),
            1.f);
        falloffColor *= zeus::CColor(quarterCicleFactor);
      }
      lightAdd += falloffColor;
    }

    const CGuiLight& brightestLight = *x5d8_guiLights[maxIntensityIdx];
    lightAdd += x33c_lights->GetAmbientColor() * zeus::CColor(0.25f, 1.f);
    zeus::CVector3f revDir = -brightestLight.GetWorldTransform().basis[1];
    float foreDot = revDir.dot(zeus::skForward);
    x5d8_guiLights[3]->SetO2WTransform(
        zeus::lookAt(zeus::skZero3f, zeus::skForward * 2.f * foreDot - revDir));
    x5d8_guiLights[3]->SetColor(g_tweakGui->GetHudReflectivityLightColor() * brightestLight.GetIntermediateColor());
    x5d8_guiLights[3]->SetAmbientLightColor(lightAdd);
  }
}

void CSamusHud::UpdateHudDamage(float dt, const CStateManager& mgr, DataSpec::ITweakGui::EHelmetVisMode helmetVis) {
  CPlayer& player = mgr.GetPlayer();
  if (player.WasDamaged() && mgr.GetGameState() == CStateManager::EGameState::Running)
    x3e8_damageTIme += dt;
  else
    x3e8_damageTIme = 0.f;

  float pulseDur = g_tweakGui->GetHudDamagePulseDuration();
  float pulseTime = std::fabs(std::fmod(x3e8_damageTIme, pulseDur));
  if (pulseTime < 0.5f * pulseDur)
    x3ec_damageLightPulser = pulseTime / (0.5f * pulseDur);
  else
    x3ec_damageLightPulser = (pulseDur - pulseTime) / (0.5f * pulseDur);

  x3ec_damageLightPulser = zeus::clamp(
      0.f, g_tweakGui->GetHudDamageColorGain() * x3ec_damageLightPulser * std::min(0.5f, player.GetDamageAmount()),
      1.f);
  zeus::CColor damageAmbColor = g_tweakGuiColors->GetHudFrameColor();
  damageAmbColor *= damageAmbColor.a();
  damageAmbColor += zeus::CColor(x3ec_damageLightPulser);
  damageAmbColor.a() = 1.f;

  if (x3d4_damageLight)
    x3d4_damageLight->SetAmbientLightColor(damageAmbColor);

  if (x3f4_damageFilterAmt > 0.f) {
    x3f4_damageFilterAmt = std::max(0.f, x3f4_damageFilterAmt - dt);
    if (x3f4_damageFilterAmt == 0.f) {
      CSfxManager::RemoveEmitter(x3a4_damageSfx);
      x3a4_damageSfx.reset();
    }
  }

  float tmp = x3f0_damageFilterAmtInit * g_tweakGui->GetHudDamagePeakFactor();
  float colorGain;
  if (x3f4_damageFilterAmt > tmp)
    colorGain = (x3f0_damageFilterAmtInit - x3f4_damageFilterAmt) / (x3f0_damageFilterAmtInit - tmp);
  else
    colorGain = x3f4_damageFilterAmt / tmp;

  colorGain = zeus::clamp(0.f, colorGain * x3f8_damageFilterAmtGain, 1.f);
  zeus::CColor color0 = g_tweakGuiColors->GetDamageAmbientColor();
  color0.a() *= colorGain;

  zeus::CColor color1 = g_tweakGuiColors->GetDamageAmbientPulseColor();
  color1.a() *= x3ec_damageLightPulser;
  zeus::CColor color2 = color0 + color1;

  if (color2.a()) {
    if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Unmorphed)
      color2.a() *= 0.75f;
    x3a8_camFilter.SetFilter(EFilterType::Add, EFilterShape::Fullscreen, 0.f, color2, {});
  } else {
    x3a8_camFilter.DisableFilter(0.f);
  }

  if (x3a4_damageSfx)
    CSfxManager::UpdateEmitter(x3a4_damageSfx, player.GetTranslation(), player.GetTransform().basis[1], 1.f);

  if (x400_hudDamagePracticals > 0.f) {
    x400_hudDamagePracticals = std::max(0.f, x400_hudDamagePracticals - dt);
    float practicals = x400_hudDamagePracticals / x3fc_hudDamagePracticalsInit;
    if (x28c_energyIntf)
      x28c_energyIntf->SetFlashMagnitude(practicals);
    practicals = std::min(practicals * x404_hudDamagePracticalsGain, 1.f);
    x2a0_helmetIntf->AddHelmetLightValue(practicals);
    if (x29c_decoIntf) {
      x29c_decoIntf->SetFrameColorValue(practicals);
      if (practicals > 0.f) {
        x3d4_damageLight->SetColor(g_tweakGuiColors->GetHudDamageLightColor() * zeus::CColor(practicals));
        x3d4_damageLight->SetIsVisible(true);
      } else {
        x3d4_damageLight->SetIsVisible(false);
      }
    }
  }

  bool transformUpdate = false;
  if (x414_decoShakeTranslateAmt > 0.f) {
    x418_decoShakeTranslateAmtVel -= g_tweakGui->GetDecoDamageShakeDeceleration() * 60.f * dt;
    x414_decoShakeTranslateAmt = std::max(0.f, x414_decoShakeTranslateAmt + x418_decoShakeTranslateAmtVel);
    transformUpdate = true;
  }
  if (x460_decoShakeAmt > 0.f) {
    x460_decoShakeAmt = std::max(0.f, x460_decoShakeAmt - dt);
    x44c_hudLagShakeRot = zeus::CQuaternion();
    float rotMul = std::min(g_tweakGui->GetMaxDecoDamageShakeRotate(),
                            x460_decoShakeAmt / x45c_decoShakeAmtInit * x464_decoShakeAmtGain);
    float rotAng = rotMul * (2.f * M_PIF / 10.f);
    x44c_hudLagShakeRot.rotateX(rand() / float(RAND_MAX) * rotAng);
    x44c_hudLagShakeRot.rotateZ(rand() / float(RAND_MAX) * rotAng);
    std::array<zeus::CVector3f, 3> vecs{zeus::skRight, zeus::skForward, zeus::skUp};
    for (int i = 0; i < 4; ++i) {
      const int sel = rand() % 9;
      vecs[sel % 3][sel / 3] += (rand() / float(RAND_MAX) - dt) * rotMul;
    }
    x428_decoShakeRotate = zeus::CMatrix3f(vecs[0], vecs[1], vecs[2]).transposed();
    transformUpdate = true;
  }

  if (transformUpdate) {
    x41c_decoShakeTranslate =
        x408_damagerToPlayerNorm * std::min(g_tweakGui->GetMaxDecoDamageShakeTranslate(), x414_decoShakeTranslateAmt);
    if (x29c_decoIntf)
      x29c_decoIntf->SetDamageTransform(x428_decoShakeRotate,
                                        x41c_decoShakeTranslate * g_tweakGui->GetHudDecoShakeTranslateGain());
  }
}

void CSamusHud::UpdateStaticSfx(CSfxHandle& handle, float& cycleTimer, u16 sfxId, float dt, float oldStaticInterp,
                                float staticThreshold) {
  if ((oldStaticInterp > staticThreshold && x510_staticInterp <= staticThreshold) ||
      (oldStaticInterp <= staticThreshold && x510_staticInterp > staticThreshold)) {
    cycleTimer = 0.f;
  } else {
    if (cycleTimer < 0.1f)
      cycleTimer = std::min(cycleTimer + dt, 0.1f);
    if (cycleTimer == 0.1f) {
      if (x510_staticInterp > staticThreshold) {
        if (!handle)
          handle = CSfxManager::SfxStart(sfxId, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
      } else {
        CSfxManager::SfxStop(handle);
        handle.reset();
      }
    }
  }
}

void CSamusHud::UpdateStaticInterference(float dt, const CStateManager& mgr) {
  float intf = mgr.GetPlayerState()->GetStaticInterference().GetTotalInterference();
  float oldStaticInterp = x510_staticInterp;
  if (x510_staticInterp < intf)
    x510_staticInterp = std::min(x510_staticInterp + dt, intf);
  else
    x510_staticInterp = std::max(intf, x510_staticInterp - dt);

  UpdateStaticSfx(x508_staticSfxHi, x514_staticCycleTimerHi, SFXui_static_hi, dt, oldStaticInterp, 0.1f);
  UpdateStaticSfx(x50c_staticSfxLo, x518_staticCycleTimerLo, SFXui_static_lo, dt, oldStaticInterp, 0.5f);

  if (x510_staticInterp > 0.f) {
    zeus::CColor color = zeus::skWhite;
    color.a() = x510_staticInterp;
    x51c_camFilter2.SetFilter(EFilterType::Blend, EFilterShape::RandomStatic, 0.f, color, -1);
  } else {
    x51c_camFilter2.DisableFilter(0.f);
  }
}

int CSamusHud::GetRelativeDirection(const zeus::CVector3f& position, const CStateManager& mgr) {
  TCastToConstPtr<CFirstPersonCamera> fpCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  if (!fpCam)
    return 0;
  zeus::CVector3f camToPosLocal = fpCam->GetTransform().transposeRotate(position - fpCam->GetTranslation());
  if (camToPosLocal == position)
    return 0;
  float y = std::cos(2.f * M_PIF * 0.0027777778f * 0.5f * fpCam->GetFov());
  float x = std::cos(2.f * M_PIF * 0.0027777778f * 0.5f * fpCam->GetFov() * fpCam->GetAspectRatio());
  zeus::CVector2f camToPosXY = zeus::CVector2f(camToPosLocal.x(), camToPosLocal.y()).normalized();
  zeus::CVector2f camToPosYZ = zeus::CVector2f(camToPosLocal.y(), camToPosLocal.z()).normalized();
  if (camToPosXY.dot(zeus::CVector2f(0.f, 1.f)) > x && camToPosYZ.dot(zeus::CVector2f(1.f, 0.f)) > y)
    return 0;
  if (camToPosXY.dot(zeus::CVector2f(0.f, -1.f)) > x && camToPosYZ.dot(zeus::CVector2f(-1.f, 0.f)) > y)
    return 1;
  zeus::CVector3f camToPosNorm = camToPosLocal.normalized();
  zeus::CQuaternion quat;
  quat.rotateY(2.f * M_PIF / 8.f);
  zeus::CVector3f vec = zeus::skUp;
  float maxDot = -1.f;
  int ret = -1;
  for (int i = 0; i < 8; ++i) {
    float dot = camToPosNorm.dot(vec);
    if (dot > maxDot) {
      maxDot = dot;
      ret = i + 2;
    }
    vec = quat.transform(vec);
  }
  return ret;
}

void CSamusHud::ShowDamage(const zeus::CVector3f& position, float dam, float prevDam, const CStateManager& mgr) {
  CPlayer& player = mgr.GetPlayer();
  int dir = GetRelativeDirection(position, mgr);
  TCastToConstPtr<CFirstPersonCamera> fpCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  x404_hudDamagePracticalsGain =
      g_tweakGui->GetHudDamagePracticalsGainLinear() * dam + g_tweakGui->GetHudDamagePracticalsGainConstant();
  x3fc_hudDamagePracticalsInit = std::max(FLT_EPSILON, g_tweakGui->GetHudDamagePracticalsInitLinear() * dam +
                                                           g_tweakGui->GetHudDamagePracticalsInitConstant());
  x400_hudDamagePracticals = x3fc_hudDamagePracticalsInit;
  if (x3d4_damageLight)
    x3d4_damageLight->SetLocalTransform(x3d8_lightTransforms[dir]);
  x3f8_damageFilterAmtGain =
      g_tweakGui->GetHudDamageFilterGainLinear() * dam + g_tweakGui->GetHudDamageFilterGainConstant();
  x3f0_damageFilterAmtInit =
      g_tweakGui->GetHudDamageFilterInitLinear() * dam + g_tweakGui->GetHudDamageFilterInitConstant();
  x3f4_damageFilterAmt = x3f0_damageFilterAmtInit;
  if (!x3a4_damageSfx) {
    x3a4_damageSfx = CSfxManager::AddEmitter(SFXui_damage_lp, player.GetTranslation(), player.GetTransform().basis[1],
                                             0.f, false, true, 0xff, kInvalidAreaId);
  }
  if (fpCam) {
    x418_decoShakeTranslateAmtVel =
        g_tweakGui->GetHudDecoShakeTranslateVelLinear() * prevDam + g_tweakGui->GetHudDecoShakeTranslateVelConstant();
    x414_decoShakeTranslateAmt = x418_decoShakeTranslateAmtVel;
    x408_damagerToPlayerNorm = -(fpCam->GetTransform().inverse() * position).normalized();
    x464_decoShakeAmtGain = g_tweakGui->GetDecoShakeGainLinear() * prevDam + g_tweakGui->GetDecoShakeGainConstant();
    x45c_decoShakeAmtInit = g_tweakGui->GetDecoShakeInitLinear() * prevDam + g_tweakGui->GetDecoShakeInitConstant();
    x460_decoShakeAmt = x45c_decoShakeAmtInit;
  }
}

void CSamusHud::EnterFirstPerson(const CStateManager& mgr) {
  CSfxManager::SfxVolume(x508_staticSfxHi, 1.f);
  CSfxManager::SfxVolume(x50c_staticSfxLo, 1.f);
}

void CSamusHud::LeaveFirstPerson(const CStateManager& mgr) {
  CSfxManager::SfxVolume(x508_staticSfxHi, 0.f);
  CSfxManager::SfxVolume(x50c_staticSfxLo, 0.f);
}

EHudState CSamusHud::GetDesiredHudState(const CStateManager& mgr) {
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed ||
      mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphing ||
      mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphing)
    return EHudState::Ball;

  switch (mgr.GetPlayerState()->GetTransitioningVisor()) {
  case CPlayerState::EPlayerVisor::Combat:
    return EHudState::Combat;
  case CPlayerState::EPlayerVisor::XRay:
    return EHudState::XRay;
  case CPlayerState::EPlayerVisor::Scan:
    return EHudState::Scan;
  case CPlayerState::EPlayerVisor::Thermal:
    return EHudState::Thermal;
  default:
    return EHudState::None;
  }
}

void CSamusHud::Update(float dt, const CStateManager& mgr, CInGameGuiManager::EHelmetVisMode helmetVis, bool hudVis,
                       bool targetingManager) {
  CPlayer& player = mgr.GetPlayer();
  UpdateStateTransition(dt, mgr);
  bool firstPerson = player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed &&
                     !mgr.GetCameraManager()->IsInCinematicCamera();
  if (firstPerson != x2e0_26_latestFirstPerson) {
    if (firstPerson)
      EnterFirstPerson(mgr);
    else
      LeaveFirstPerson(mgr);
    x2e0_26_latestFirstPerson = firstPerson;
  }

  float morphT = 0.f;
  switch (player.GetMorphballTransitionState()) {
  case CPlayer::EPlayerMorphBallState::Morphed:
    morphT = 1.f;
    break;
  case CPlayer::EPlayerMorphBallState::Morphing:
    morphT = player.GetMorphFactor();
    break;
  case CPlayer::EPlayerMorphBallState::Unmorphing:
    morphT = 1.f - player.GetMorphFactor();
    break;
  default:
    break;
  }

  float scaleMul = 1.f - zeus::clamp(0.f, (g_Viewport.aspect - 1.33f) / (1.77f - 1.33f), 1.f);
  x500_viewportScale.y() = 1.f - scaleMul * morphT * g_tweakGui->GetBallViewportYReduction() * 1.2f;
  if (x2b0_ballIntf)
    x2b0_ballIntf->SetBallModeFactor(morphT);

  bool helmetVisible = false;
  bool glowVisible = false;
  bool decoVisible = false;
  if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed) {
    switch (helmetVis) {
    case CInGameGuiManager::EHelmetVisMode::HelmetOnly:
      helmetVisible = true;
      break;
    case CInGameGuiManager::EHelmetVisMode::GlowHelmetDeco:
      glowVisible = true;
      [[fallthrough]];
    case CInGameGuiManager::EHelmetVisMode::HelmetDeco:
      helmetVisible = true;
      [[fallthrough]];
    case CInGameGuiManager::EHelmetVisMode::Deco:
      decoVisible = true;
      break;
    default:
      break;
    }
  }

  if (x29c_decoIntf)
    x29c_decoIntf->SetIsVisibleDebug(decoVisible);
  if (x2a0_helmetIntf)
    x2a0_helmetIntf->SetIsVisibleDebug(helmetVisible, glowVisible);

  x590_base_Model_AutoMapper->SetIsVisible(false);

  UpdateEnergyLow(dt, mgr);

  for (auto& entry : x7ac_) {
    entry.x0_ = 0;
    entry.x4_ = 0;
  }

  if (x2ac_radarIntf)
    x2ac_radarIntf->Update(dt, mgr);

  UpdateHudLag(dt, mgr);

  UpdateHudDynamicLights(dt, mgr);

  if (targetingManager)
    x8_targetingMgr.Update(dt, mgr);

  UpdateHudDamage(dt, mgr, helmetVis);

  UpdateStaticInterference(dt, mgr);

  if (helmetVis != DataSpec::ITweakGui::EHelmetVisMode::ReducedUpdate) {
    if (x2bc_nextState != EHudState::None) {
      UpdateEnergy(dt, mgr, false);
      UpdateFreeLook(dt, mgr);
    }

    if (x2bc_nextState == EHudState::Ball) {
      UpdateBallMode(mgr, false);
    } else if (x2bc_nextState >= EHudState::Combat && x2bc_nextState <= EHudState::Scan) {
      UpdateThreatAssessment(dt, mgr);
      UpdateMissile(dt, mgr, false);
      UpdateVideoBands(dt, mgr);
    }

    UpdateVisorAndBeamMenus(dt, mgr);

    if (player.WasDamaged() && mgr.GetGameState() == CStateManager::EGameState::Running)
      ShowDamage(player.GetDamageLocationWR(), player.GetDamageAmount(), player.GetPrevDamageAmount(), mgr);
  }

  float oldAPulse = x584_abuttonPulse;
  if (!x554_hudMemoIdx) {
    x584_abuttonPulse += 2.f * dt;
    if (x584_abuttonPulse > 1.f)
      x584_abuttonPulse -= 2.f;
  }

  zeus::CColor abuttonColor = zeus::skWhite;
  abuttonColor.a() = std::fabs(x584_abuttonPulse);
  x5a0_base_model_abutton->SetColor(abuttonColor);

  if (!mgr.GetCameraManager()->IsInCinematicCamera() && oldAPulse < 0.f && x584_abuttonPulse >= 0.f &&
      x598_base_basewidget_message->GetIsVisible() && (x558_messageTextTime == 0.f || x558_messageTextTime >= 1.f)) {
    CSfxManager::SfxStart(SFXui_hud_memo_a_pulse, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
  }

  float allTextAlpha;
  if (x29c_decoIntf)
    allTextAlpha = x29c_decoIntf->GetHudTextAlpha();
  else
    allTextAlpha = 1.f;

  float messageTextAlpha = 1.f;
  if (x558_messageTextTime > 0.f)
    messageTextAlpha = std::min(x558_messageTextTime, 1.f);
  else if (!x59c_base_textpane_message->GetIsVisible() && !x598_base_basewidget_message->GetIsVisible())
    messageTextAlpha = 0.f;

  if (x2b4_bossEnergyIntf)
    x2b4_bossEnergyIntf->SetAlpha(1.f - messageTextAlpha);

  if (x550_hudMemoString && x550_hudMemoString.IsLoaded()) {
    SetMessage(x550_hudMemoString->GetString(x554_hudMemoIdx), x548_hudMemoParms);
    x550_hudMemoString = TLockedToken<CStringTable>();
  }

  if (x558_messageTextTime > 0.f) {
    x558_messageTextTime = std::max(0.f, x558_messageTextTime - dt);
    if (x558_messageTextTime == 0.f) {
      x59c_base_textpane_message->TextSupport().SetTypeWriteEffectOptions(false, 0.f, 1.f);
      x598_base_basewidget_message->SetVisibility(false, ETraversalMode::Children);
    }
  }

  CGuiWidget* messageWidget;
  if (x598_base_basewidget_message->GetIsVisible())
    messageWidget = x598_base_basewidget_message;
  else
    messageWidget = x59c_base_textpane_message;

  zeus::CColor messageColor = zeus::skWhite;
  float textScale = 1.f;
  messageColor.a() = std::min(allTextAlpha, messageTextAlpha);
  messageWidget->SetColor(messageColor);

  if (messageWidget == x598_base_basewidget_message) {
    if (x558_messageTextTime > 0.f)
      x560_messageTextScale = std::min(x558_messageTextTime, 1.f);
    else
      x560_messageTextScale = std::min(x560_messageTextScale + dt, 1.f);

    float textScaleT = std::max(0.f, (x560_messageTextScale - 0.75f) / 0.25f);
    if (textScaleT != 1.f) {
      if (textScaleT < 0.7f)
        textScale = textScaleT / 0.7f;
      else if (textScaleT < 0.85f)
        textScale = 0.1f * (1.f - (textScaleT - 0.7f) / 0.15f) + 0.9f;
      else
        textScale = 0.1f * ((textScaleT - 0.7f) - 0.15f) / 0.3f + 0.9f;
    }

    x598_base_basewidget_message->SetLocalTransform(x598_base_basewidget_message->GetTransform() *
                                                    zeus::CTransform::Scale(textScale, 1.f, 1.f));
  }

  float nextSfxChars = x55c_lastSfxChars + g_tweakGui->GetWorldTransManagerCharsPerSfx();
  if (x59c_base_textpane_message->TextSupport().GetNumCharsPrinted() >= nextSfxChars) {
    x55c_lastSfxChars = nextSfxChars;
    if (!x598_base_basewidget_message->GetIsVisible() || textScale == 1.f)
      CSfxManager::SfxStart(SFXui_hud_memo_type, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
  }

  if (mgr.GetEscapeSequenceTimer() > 0.f) {
    int minutes = mgr.GetEscapeSequenceTimer() / 60.f;
    int seconds = std::fmod(mgr.GetEscapeSequenceTimer(), 60.f);
    int hundredths = std::fmod(mgr.GetEscapeSequenceTimer() * 100.f, 100.f);
    std::string timeStr = fmt::format(FMT_STRING("{:02d}:{:02d}:{:02d}"), int(minutes), int(seconds), int(hundredths));
    x594_base_textpane_counter->TextSupport().SetText(timeStr);
    x594_base_textpane_counter->SetIsVisible(true);

    zeus::CColor counterColor = zeus::skWhite;
    counterColor.a() = zeus::clamp(0.f, std::min(1.f - std::min(x558_messageTextTime, 1.f), allTextAlpha), 1.f);
    x594_base_textpane_counter->SetColor(counterColor);
  } else {
    x594_base_textpane_counter->SetIsVisible(false);
  }

  x274_loadedFrmeBaseHud->Update(dt);
  if (x288_loadedSelectedHud)
    x288_loadedSelectedHud->Update(dt);
  if (x2b4_bossEnergyIntf)
    x2b4_bossEnergyIntf->Update(dt);
  if (x28c_energyIntf)
    x28c_energyIntf->Update(dt, x580_energyLowPulse);
  if (x290_threatIntf)
    x290_threatIntf->Update(dt);
  if (x294_missileIntf)
    x294_missileIntf->Update(dt, mgr);
  if (x298_freeLookIntf)
    x298_freeLookIntf->Update(dt);
  if (x2a0_helmetIntf)
    x2a0_helmetIntf->Update(dt);

  if (player.GetScanningState() == CPlayer::EPlayerScanState::NotScanning)
    x2f0_visorBeamMenuAlpha = std::min(x2f0_visorBeamMenuAlpha + 2.f * dt, 1.f);
  else
    x2f0_visorBeamMenuAlpha = std::max(0.f, x2f0_visorBeamMenuAlpha - 2.f * dt);

  CPlayerState::EPlayerVisor curVisor = mgr.GetPlayerState()->GetCurrentVisor();
  CPlayerState::EPlayerVisor transVisor = mgr.GetPlayerState()->GetTransitioningVisor();
  float transFactor = 0.f;
  if (curVisor != CPlayerState::EPlayerVisor::Scan) {
    if (transVisor == CPlayerState::EPlayerVisor::Scan)
      transFactor = mgr.GetPlayerState()->GetVisorTransitionFactor();
    else
      transFactor = 1.f;
  }

  if (x2a4_visorMenu) {
    float hudAlpha;
    if (g_GameState->GameOptions().GetSwapBeamControls())
      hudAlpha = transFactor;
    else
      hudAlpha = x2f0_visorBeamMenuAlpha;
    x2a4_visorMenu->UpdateHudAlpha(hudAlpha);
    x2a4_visorMenu->Update(dt, false);
  }

  if (x2a8_beamMenu) {
    float hudAlpha;
    if (g_GameState->GameOptions().GetSwapBeamControls())
      hudAlpha = x2f0_visorBeamMenuAlpha;
    else
      hudAlpha = transFactor;
    x2a8_beamMenu->UpdateHudAlpha(hudAlpha);
    x2a8_beamMenu->Update(dt, false);
  }

  UpdateCameraDebugSettings();

  if (x29c_decoIntf)
    x29c_decoIntf->Update(dt, mgr);
}

void CSamusHud::DrawAttachedEnemyEffect(const CStateManager& mgr) {
  const float drainTime = mgr.GetPlayer().GetEnergyDrain().GetEnergyDrainTime();
  if (drainTime <= 0.f) {
    return;
  }

  const float modPeriod = g_tweakGui->GetEnergyDrainModPeriod();
  float alpha;
  if (g_tweakGui->GetEnergyDrainSinusoidalPulse()) {
    alpha = (std::sin(-0.25f * modPeriod + 2.f * M_PIF * drainTime / modPeriod) + 1.f) * 0.5f;
  } else {
    const float halfModPeriod = 0.5f * modPeriod;
    const float tmp = std::fabs(std::fmod(drainTime, modPeriod));
    if (tmp < halfModPeriod) {
      alpha = tmp / halfModPeriod;
    } else {
      alpha = (modPeriod - tmp) / halfModPeriod;
    }
  }

  zeus::CColor filterColor = g_tweakGuiColors->GetEnergyDrainFilterColor();
  filterColor.a() *= alpha;
  m_energyDrainFilter.draw(filterColor);
}

void CSamusHud::Draw(const CStateManager& mgr, float alpha, CInGameGuiManager::EHelmetVisMode helmetVis, bool hudVis,
                     bool targetingManager) {
  if (x2bc_nextState == EHudState::None) {
    return;
  }
  SCOPED_GRAPHICS_DEBUG_GROUP("CSamusHud::Draw", zeus::skBlue);
  x3a8_camFilter.Draw();
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed) {
    DrawAttachedEnemyEffect(mgr);
    x51c_camFilter2.Draw();
    if (targetingManager) {
      x8_targetingMgr.Draw(mgr, false);
    }
  }

  if (helmetVis != CInGameGuiManager::EHelmetVisMode::ReducedUpdate &&
      helmetVis < CInGameGuiManager::EHelmetVisMode::HelmetOnly) {
    if (alpha < 1.f) {
      m_cookieCutterStatic.draw(zeus::skWhite, 1.f - alpha);
    }

    if (x288_loadedSelectedHud) {
      if (mgr.GetPlayer().GetDeathTime() > 0.f) {
        if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Unmorphed) {
          const CGuiWidgetDrawParms parms(
              x2c8_transT * zeus::clamp(0.f, 1.f - mgr.GetPlayer().GetDeathTime() / 6.f, 1.f), zeus::skZero3f);
          x288_loadedSelectedHud->Draw(parms);
        } else {
          const CGuiWidgetDrawParms parms(x2c8_transT, zeus::skZero3f);
          x288_loadedSelectedHud->Draw(parms);
        }
      } else {
        const CGuiWidgetDrawParms parms(x2c8_transT, zeus::skZero3f);
        x288_loadedSelectedHud->Draw(parms);
      }
    }

    if (x274_loadedFrmeBaseHud) {
      x274_loadedFrmeBaseHud->Draw(CGuiWidgetDrawParms::Default);
    }
  }

  if (x29c_decoIntf && !x2cc_preLoadCountdown) {
    x29c_decoIntf->Draw();
  }

  if (x2bc_nextState >= EHudState::Combat && x2bc_nextState <= EHudState::Scan) {
    if (hudVis && helmetVis != CInGameGuiManager::EHelmetVisMode::ReducedUpdate &&
        helmetVis < CInGameGuiManager::EHelmetVisMode::HelmetOnly) {
      float t;
      if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Combat) {
        t = mgr.GetPlayerState()->GetVisorTransitionFactor();
      } else {
        t = 0.f;
      }
      x2ac_radarIntf->Draw(mgr, t * alpha);
    }
    // Depth read/write enable
  }
}

void CSamusHud::DrawHelmet(const CStateManager& mgr, float camYOff) {
  if (!x264_loadedFrmeHelmet ||
      mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Unmorphed ||
      x2bc_nextState == EHudState::Ball) {
    return;
  }

  float t;
  if (x2c4_activeTransState == ETransitionState::Transitioning && x2b8_curState == EHudState::Ball) {
    t = x2c8_transT;
  } else {
    t = 1.f;
  }

  x264_loadedFrmeHelmet->Draw(CGuiWidgetDrawParms(t, zeus::CVector3f(0.f, 15.f * camYOff, 0.f)));
}

void CSamusHud::ProcessControllerInput(const CFinalInput& input) {
  if (x29c_decoIntf)
    x29c_decoIntf->ProcessInput(input);
}

void CSamusHud::UpdateStateTransition(float dt, const CStateManager& mgr) {
  if (x2cc_preLoadCountdown == 0) {
    EHudState desiredState = GetDesiredHudState(mgr);
    if (desiredState != x2c0_setState) {
      x2c0_setState = desiredState;
      if (desiredState == EHudState::Ball || x2bc_nextState == EHudState::Ball)
        x2c8_transT = FLT_EPSILON;
      x2c4_activeTransState = ETransitionState::Countdown;
    }
  }

  switch (x2c4_activeTransState) {
  case ETransitionState::Countdown:
    if (x2cc_preLoadCountdown == 0) {
      x2c8_transT = std::max(x2c8_transT - 5.f * dt, 0.f);
      if (x2c8_transT == 0.f) {
        x2cc_preLoadCountdown = 2;
        x288_loadedSelectedHud = nullptr;
      }
    } else {
      x2cc_preLoadCountdown -= 1;
      if (x2cc_preLoadCountdown == 0) {
        UninitializeFrameGlueMutable();
        x278_selectedHud = TLockedToken<CGuiFrame>();
        switch (x2c0_setState) {
        case EHudState::Thermal:
          x278_selectedHud = g_SimplePool->GetObj("FRME_ThermalHud");
          break;
        case EHudState::Combat:
          x278_selectedHud = g_SimplePool->GetObj("FRME_CombatHud");
          break;
        case EHudState::Scan:
          x278_selectedHud = g_SimplePool->GetObj("FRME_ScanHud");
          break;
        case EHudState::XRay:
          x278_selectedHud = g_SimplePool->GetObj("FRME_XRayHudNew");
          break;
        case EHudState::Ball:
          x278_selectedHud = g_SimplePool->GetObj("FRME_BallHud");
          break;
        default:
          break;
        }
        x2c4_activeTransState = ETransitionState::Loading;
      }
    }
    if (x2c4_activeTransState != ETransitionState::Loading)
      return;
    [[fallthrough]];
  case ETransitionState::Loading:
    if (x278_selectedHud) {
      if (!x278_selectedHud.IsLoaded() || !x278_selectedHud->GetIsFinishedLoading())
        return;
      x288_loadedSelectedHud = x278_selectedHud.GetObj();
      x288_loadedSelectedHud->Reset();
      x288_loadedSelectedHud->SetMaxAspect(1.78f);
      x2b8_curState = x2bc_nextState;
      x2bc_nextState = x2c0_setState;
      InitializeFrameGlueMutable(mgr);
      x2c4_activeTransState = ETransitionState::Transitioning;
      UpdateCameraDebugSettings();
    } else {
      x2b8_curState = x2bc_nextState;
      x2bc_nextState = x2c0_setState;
      x2c4_activeTransState = ETransitionState::NotTransitioning;
    }
    break;
  case ETransitionState::Transitioning:
    x2c8_transT = std::min(1.f, 5.f * dt + x2c8_transT);
    if (x2c8_transT == 1.f)
      x2c4_activeTransState = ETransitionState::NotTransitioning;
    break;
  default:
    break;
  }
}

bool CSamusHud::CheckLoadComplete(CStateManager& stateMgr) {
  switch (x4_loadPhase) {
  case ELoadPhase::Zero:
    if (!x8_targetingMgr.CheckLoadComplete())
      return false;
    x4_loadPhase = ELoadPhase::One;
    [[fallthrough]];
  case ELoadPhase::One:
    UpdateStateTransition(1.f, stateMgr);
    if (x2bc_nextState != x2c0_setState)
      return false;
    x4_loadPhase = ELoadPhase::Two;
    [[fallthrough]];
  case ELoadPhase::Two:
    if (!x264_loadedFrmeHelmet->GetIsFinishedLoading())
      return false;
    if (!x274_loadedFrmeBaseHud->GetIsFinishedLoading())
      return false;
    x4_loadPhase = ELoadPhase::Three;
    [[fallthrough]];
  case ELoadPhase::Three:
    return true;
  default:
    break;
  }

  return false;
}

void CSamusHud::OnNewInGameGuiState(EInGameGuiState state, CStateManager& stateMgr) {
  // Empty
}

void CSamusHud::RefreshHudOptions() {
  if (x29c_decoIntf)
    x29c_decoIntf->UpdateHudAlpha();
  if (x2a0_helmetIntf)
    x2a0_helmetIntf->UpdateHelmetAlpha();
}

void CSamusHud::Touch() {
  if (x264_loadedFrmeHelmet)
    x264_loadedFrmeHelmet->Touch();
  if (x274_loadedFrmeBaseHud)
    x274_loadedFrmeBaseHud->Touch();
  if (x288_loadedSelectedHud)
    x288_loadedSelectedHud->Touch();
}

zeus::CTransform CSamusHud::BuildFinalCameraTransform(const zeus::CQuaternion& rot, const zeus::CVector3f& pos,
                                                      const zeus::CVector3f& camPos) {
  zeus::CQuaternion invRot = rot.inverse();
  return zeus::CTransform(invRot, invRot.transform(camPos - pos) + pos);
}

void CSamusHud::SetMessage(std::u16string_view text, const CHUDMemoParms& info) {
  bool isWidgetVisible = x598_base_basewidget_message->GetIsVisible();
  if (!isWidgetVisible || info.IsHintMemo()) {
    if (info.IsFadeOutOnly()) {
      x558_messageTextTime = 1.f;
      if (!info.IsHintMemo() || !isWidgetVisible)
        return;
      CSfxManager::SfxStart(SFXui_hide_hint_memo, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      return;
    }
    x598_base_basewidget_message->SetColor(zeus::skWhite);
    x598_base_basewidget_message->SetVisibility(false, ETraversalMode::Children);
    CGuiWidget* pane = info.IsHintMemo() ? x598_base_basewidget_message : x59c_base_textpane_message;
    pane->SetVisibility(true, ETraversalMode::Children);
    x59c_base_textpane_message->TextSupport().SetTypeWriteEffectOptions(true, 0.1f, 40.f);
    if (info.IsClearMemoWindow()) {
      x55c_lastSfxChars = 0.f;
      x59c_base_textpane_message->TextSupport().SetCurTime(0.f);
      x59c_base_textpane_message->TextSupport().SetText(text);
    } else if (x59c_base_textpane_message->TextSupport().GetString().empty()) {
      x55c_lastSfxChars = 0.f;
      x59c_base_textpane_message->TextSupport().AddText(text);
    } else {
      x59c_base_textpane_message->TextSupport().AddText(std::u16string(u"\n") + text.data());
    }

    x59c_base_textpane_message->SetColor(zeus::skWhite);
    x598_base_basewidget_message->SetColor(zeus::skWhite);
    x558_messageTextTime = info.GetDisplayTime();
    if (info.IsHintMemo()) {
      if (!isWidgetVisible) {
        x584_abuttonPulse = 0.f;
        x560_messageTextScale = 0.f;
        CSfxManager::SfxStart(SFXui_show_hint_memo, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      }
    } else {
      x598_base_basewidget_message->SetLocalTransform(x598_base_basewidget_message->GetTransform());
    }
  }
}

void CSamusHud::InternalDeferHintMemo(CAssetId strg, u32 strgIdx, const CHUDMemoParms& info) {
  x548_hudMemoParms = info;
  x550_hudMemoString = g_SimplePool->GetObj(SObjectTag{FOURCC('STRG'), strg});
  x554_hudMemoIdx = strgIdx;
}

} // namespace urde::MP1
