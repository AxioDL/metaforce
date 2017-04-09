#include "CSamusHud.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "World/CPlayer.hpp"
#include "World/CScriptTrigger.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiLight.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiCamera.hpp"
#include "Camera/CFirstPersonCamera.hpp"
#include "TCastTo.hpp"

namespace urde
{
namespace MP1
{

CSamusHud* CSamusHud::g_SamusHud = nullptr;

CSamusHud::CSamusHud(CStateManager& stateMgr)
: x8_targetingMgr(stateMgr),
  x258_frmeHelmet(g_SimplePool->GetObj("FRME_Helmet")),
  x268_frmeBaseHud(g_SimplePool->GetObj("FRME_BaseHud"))
{
    x2e0_26_latestFirstPerson = true;
    x2e0_27_energyLow = stateMgr.GetPlayer().IsEnergyLow(stateMgr);
    x33c_lights = std::make_unique<CActorLights>(8, zeus::CVector3f::skZero, 4, 1, true, 0, 0, 0.1f);
    x340_hudLights.resize(3, SCachedHudLight(zeus::CVector3f::skZero, zeus::CColor::skWhite,
                                             0.f, 0.f, 0.f, 0.f));
    x46c_.resize(3);
    x568_fpCamDir = stateMgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform().basis[1];
    x5a4_videoBands.resize(4);
    x5d8_guiLights.resize(4);
    x7ac_.resize(15);
    UpdateStateTransition(1.f, stateMgr);
    g_SamusHud = this;

    for (int i=0 ; i<16 ; ++i)
        x5ec_camFovTweaks[i] = 5.f * i + 40.f;
    for (int i=0 ; i<64 ; ++i)
        x62c_camYTweaks[i] = -0.5f * i;
    for (int i=0 ; i<32 ; ++i)
        x72c_camZTweaks[i] = 0.5f * i - 8.f;

    x264_loadedFrmeHelmet = x258_frmeHelmet.GetObj();
    x274_loadedFrmeBaseHud = x268_frmeBaseHud.GetObj();
    x2a0_helmetIntf = std::make_unique<CHudHelmetInterface>(*x264_loadedFrmeHelmet);

    rstl::prereserved_vector<bool, 4> hasVisors = BuildPlayerHasVisors(stateMgr);
    x2a4_visorMenu = std::make_unique<CHudVisorBeamMenu>(*x274_loadedFrmeBaseHud,
                                                         CHudVisorBeamMenu::EHudVisorBeamMenu::Visor,
                                                         hasVisors);

    rstl::prereserved_vector<bool, 4> hasBeams = BuildPlayerHasBeams(stateMgr);
    x2a8_beamMenu = std::make_unique<CHudVisorBeamMenu>(*x274_loadedFrmeBaseHud,
                                                        CHudVisorBeamMenu::EHudVisorBeamMenu::Beam,
                                                        hasBeams);

    x2ac_radarIntf = std::make_unique<CHudRadarInterface>(*x274_loadedFrmeBaseHud, stateMgr);

    InitializeFrameGluePermanent(stateMgr);
    UpdateEnergy(0.f, stateMgr, true);
    UpdateMissile(0.f, stateMgr, true);
    UpdateBallMode(stateMgr, true);
}

CSamusHud::~CSamusHud()
{
    if (x3a4_damageSfx)
        CSfxManager::RemoveEmitter(x3a4_damageSfx);
    g_SamusHud = nullptr;
}

rstl::prereserved_vector<bool, 4> CSamusHud::BuildPlayerHasVisors(const CStateManager& mgr)
{
    rstl::prereserved_vector<bool, 4> ret;
    ret[0] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::CombatVisor);
    ret[1] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::XRayVisor);
    ret[2] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::ScanVisor);
    ret[3] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::ThermalVisor);
    return ret;
}

rstl::prereserved_vector<bool, 4> CSamusHud::BuildPlayerHasBeams(const CStateManager& mgr)
{
    rstl::prereserved_vector<bool, 4> ret;
    ret[0] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PowerBeam);
    ret[1] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::IceBeam);
    ret[2] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::WaveBeam);
    ret[3] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PlasmaBeam);
    return ret;
}

void CSamusHud::InitializeFrameGluePermanent(const CStateManager& mgr)
{
    x588_base_basewidget_pivot = x274_loadedFrmeBaseHud->FindWidget("basewidget_pivot");
    x58c_helmet_BaseWidget_Pivot = x264_loadedFrmeHelmet->FindWidget("BaseWidget_Pivot");
    x590_base_Model_AutoMapper = static_cast<CGuiModel*>(x274_loadedFrmeBaseHud->FindWidget("Model_AutoMapper"));
    x594_base_textpane_counter = static_cast<CGuiTextPane*>(x274_loadedFrmeBaseHud->FindWidget("textpane_counter"));
    x594_base_textpane_counter->TextSupport()->SetFontColor(g_tweakGuiColors->GetHudCounterFill());
    x594_base_textpane_counter->TextSupport()->SetOutlineColor(g_tweakGuiColors->GetHudCounterOutline());
    x598_base_basewidget_message = x274_loadedFrmeBaseHud->FindWidget("basewidget_message");
    for (CGuiWidget* child = static_cast<CGuiWidget*>(x598_base_basewidget_message->GetChildObject());
         child ; child = static_cast<CGuiWidget*>(child->GetNextSibling()))
        child->SetDepthTest(false);
    x59c_base_textpane_message = static_cast<CGuiTextPane*>(x274_loadedFrmeBaseHud->FindWidget("textpane_message"));
    x5a0_base_model_abutton = static_cast<CGuiModel*>(x274_loadedFrmeBaseHud->FindWidget("model_abutton"));
    for (int i=0 ; i<4 ; ++i)
        x5d8_guiLights[i] = x264_loadedFrmeHelmet->GetFrameLight(i);
    x5d8_guiLights[3]->SetColor(zeus::CColor::skBlack);
    for (int i=0 ; i<4 ; ++i)
    {
        SVideoBand& band = x5a4_videoBands[i];
        band.x0_videoband = static_cast<CGuiModel*>(x274_loadedFrmeBaseHud->FindWidget(hecl::Format("model_videoband%d", i)));
        band.x4_randA = CRandom16::GetRandomNumber()->Range(6.f, 66.f);
        band.x8_randB = CRandom16::GetRandomNumber()->Range(16.f, 256.f);
    }
    x59c_base_textpane_message->SetDepthTest(false);
    x598_base_basewidget_message->SetVisibility(false, ETraversalMode::Children);
    x59c_base_textpane_message->TextSupport()->SetFontColor(g_tweakGuiColors->GetHudMessageFill());
    x59c_base_textpane_message->TextSupport()->SetOutlineColor(g_tweakGuiColors->GetHudMessageOutline());
    x59c_base_textpane_message->TextSupport()->SetScanStates(&g_GameState->GameOptions().GetScanStates());
    x590_base_Model_AutoMapper->SetDepthWrite(true);
    x304_basewidgetIdlePos = x588_base_basewidget_pivot->GetIdlePosition();
    x310_cameraPos = x274_loadedFrmeBaseHud->GetFrameCamera()->GetLocalPosition();
    RefreshHudOptions();
}

void CSamusHud::InitializeFrameGlueMutable(const CStateManager& mgr)
{
    float lastTankEnergy = std::fmod(x2d0_playerHealth, CPlayerState::GetEnergyTankCapacity());
    u32 tanksFilled = x2d0_playerHealth / CPlayerState::GetEnergyTankCapacity();

    CPlayer& player = mgr.GetPlayer();
    CPlayerState& playerState = *mgr.GetPlayerState();
    CPlayerGun& gun = *player.GetPlayerGun();
    float chargeFactor = gun.IsCharging() ? gun.GetChargeBeamFactor() : 0.f;
    bool missilesActive = gun.GetMissleMode() == CPlayerGun::EMissleMode::Active;
    bool lockedOnObj = player.GetLockonObjectId() != kInvalidUniqueId;

    switch (x2bc_nextState)
    {
    case EHudState::Combat:
    {
        x2b4_bossEnergyIntf = std::make_unique<CHudBossEnergyInterface>(*x288_loadedSelectedHud);

        x28c_energyIntf = std::make_unique<CHudEnergyInterface>(*x288_loadedSelectedHud, lastTankEnergy,
                                                                x2d4_totalEnergyTanks, tanksFilled,
                                                                bool(x2e0_27_energyLow), EHudType::Combat);

        if (!x290_threatIntf)
            x290_threatIntf = std::make_unique<CHudThreatInterface>(*x288_loadedSelectedHud, EHudType::Combat,
                                                                    9999.f);
        else
            x290_threatIntf->SetIsVisibleGame(true);

        if (!x294_missileIntf)
            x294_missileIntf = std::make_unique<CHudMissileInterface>(*x288_loadedSelectedHud, x2dc_missileCapacity,
                                                                      x2d8_missileAmount, chargeFactor, missilesActive,
                                                                      EHudType::Combat, mgr);
        else
            x294_missileIntf->SetIsVisibleGame(true, mgr);

        if (!x298_freeLookIntf)
            x298_freeLookIntf = std::make_unique<CHudFreeLookInterface>(*x288_loadedSelectedHud, EHudType::Combat,
                                                                        bool(x2e0_24_inFreeLook), bool(x2e0_25_lookControlHeld),
                                                                        lockedOnObj);
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
    case EHudState::Ball:
    {
        u32 numPBs = playerState.GetItemAmount(CPlayerState::EItemType::PowerBombs);
        u32 pbCap = playerState.GetItemCapacity(CPlayerState::EItemType::PowerBombs);
        u32 bombsAvailable;
        if (gun.IsBombReady())
            bombsAvailable = gun.GetBombCount();
        else
            bombsAvailable = 0;
        x2b0_ballIntf = std::make_unique<CHudBallInterface>(*x288_loadedSelectedHud, numPBs, pbCap,
                                                            bombsAvailable, gun.IsPowerBombReady() &&
                                                            player.GetMorphballTransitionState() ==
                                                            CPlayer::EPlayerMorphBallState::Morphed,
                                                            playerState.HasPowerUp(
                                                            CPlayerState::EItemType::MorphBallBombs));

        x28c_energyIntf = std::make_unique<CHudEnergyInterface>(*x288_loadedSelectedHud, lastTankEnergy,
                                                                x2d4_totalEnergyTanks, tanksFilled,
                                                                bool(x2e0_27_energyLow), EHudType::Ball);

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
    case EHudState::Scan:
    {
        x2b4_bossEnergyIntf.reset();

        x28c_energyIntf = std::make_unique<CHudEnergyInterface>(*x288_loadedSelectedHud, lastTankEnergy,
                                                                x2d4_totalEnergyTanks, tanksFilled,
                                                                bool(x2e0_27_energyLow), EHudType::Scan);

        x290_threatIntf.reset();
        x294_missileIntf.reset();

        x298_freeLookIntf = std::make_unique<CHudFreeLookInterface>(*x288_loadedSelectedHud, EHudType::Scan,
                                                                    bool(x2e0_24_inFreeLook), bool(x2e0_25_lookControlHeld),
                                                                    lockedOnObj);

        x29c_decoIntf = std::make_unique<CHudDecoInterfaceScan>(*x288_loadedSelectedHud);
        InitializeDamageLight();
        UpdateEnergy(0.f, mgr, true);
        break;
    }
    case EHudState::XRay:
    {
        x2b4_bossEnergyIntf = std::make_unique<CHudBossEnergyInterface>(*x288_loadedSelectedHud);

        x28c_energyIntf = std::make_unique<CHudEnergyInterface>(*x288_loadedSelectedHud, lastTankEnergy,
                                                                x2d4_totalEnergyTanks, tanksFilled,
                                                                bool(x2e0_27_energyLow), EHudType::XRay);

        x290_threatIntf = std::make_unique<CHudThreatInterface>(*x288_loadedSelectedHud, EHudType::XRay,
                                                                9999.f);

        x294_missileIntf = std::make_unique<CHudMissileInterface>(*x288_loadedSelectedHud, x2dc_missileCapacity,
                                                                  x2d8_missileAmount, chargeFactor, missilesActive,
                                                                  EHudType::XRay, mgr);

        x298_freeLookIntf = std::make_unique<CHudFreeLookInterfaceXRay>(*x288_loadedSelectedHud,
                                                                        bool(x2e0_24_inFreeLook), bool(x2e0_25_lookControlHeld),
                                                                        lockedOnObj);

        x29c_decoIntf = std::make_unique<CHudDecoInterfaceXRay>(*x288_loadedSelectedHud);
        InitializeDamageLight();
        x2a4_visorMenu->SetIsVisibleGame(true);
        x2a8_beamMenu->SetIsVisibleGame(true);
        UpdateEnergy(0.f, mgr, true);
        break;
    }
    case EHudState::Thermal:
    {
        x2b4_bossEnergyIntf = std::make_unique<CHudBossEnergyInterface>(*x288_loadedSelectedHud);

        x28c_energyIntf = std::make_unique<CHudEnergyInterface>(*x288_loadedSelectedHud, lastTankEnergy,
                                                                x2d4_totalEnergyTanks, tanksFilled,
                                                                bool(x2e0_27_energyLow), EHudType::Thermal);

        x290_threatIntf = std::make_unique<CHudThreatInterface>(*x288_loadedSelectedHud, EHudType::Thermal,
                                                                9999.f);

        x294_missileIntf = std::make_unique<CHudMissileInterface>(*x288_loadedSelectedHud, x2dc_missileCapacity,
                                                                  x2d8_missileAmount, chargeFactor, missilesActive,
                                                                  EHudType::Thermal, mgr);

        x298_freeLookIntf = std::make_unique<CHudFreeLookInterface>(*x288_loadedSelectedHud, EHudType::Thermal,
                                                                    bool(x2e0_24_inFreeLook), bool(x2e0_25_lookControlHeld),
                                                                    lockedOnObj);

        x29c_decoIntf = std::make_unique<CHudDecoInterfaceThermal>(*x288_loadedSelectedHud);
        InitializeDamageLight();
        x2a4_visorMenu->SetIsVisibleGame(true);
        x2a8_beamMenu->SetIsVisibleGame(true);
        UpdateEnergy(0.f, mgr, true);
        break;
    }
    case EHudState::None:
        UninitializeFrameGlueMutable();
    default: break;
    }
}

void CSamusHud::UninitializeFrameGlueMutable()
{
    x2b4_bossEnergyIntf.reset();
    x28c_energyIntf.reset();
    x29c_decoIntf.reset();
    x290_threatIntf.reset();
    x294_missileIntf.reset();
    x298_freeLookIntf.reset();
    x2b0_ballIntf.reset();
    x3d4_damageLight = nullptr;
}

void CSamusHud::InitializeDamageLight()
{
    s16 lightId = x288_loadedSelectedHud->GetWidgetIdDB().AddWidget("DamageSpotLight");
    s16 parentId = x288_loadedSelectedHud->FindWidget("basewidget_pivot")->GetSelfId();
    CGuiWidget::CGuiWidgetParms parms(x288_loadedSelectedHud, false, lightId,
                                      parentId, true, true, false,
                                      g_tweakGuiColors->GetHudDamageLightColor(),
                                      CGuiWidget::EGuiModelDrawFlags::Alpha, false, false);

    std::shared_ptr<CGuiLight> light = std::make_shared<CGuiLight>(
        parms, CLight::BuildSpot(zeus::CVector3f::skZero, zeus::CVector3f::skForward,
        zeus::CColor::skWhite, g_tweakGui->GetHudDamageLightSpotAngle()));
    x3d4_damageLight = light.get();
    x3d4_damageLight->SetColor(zeus::CColor::skWhite);

    zeus::CColor lightColor = g_tweakGuiColors->GetHudFrameColor();
    lightColor.r *= lightColor.a;
    lightColor.g *= lightColor.a;
    lightColor.b *= lightColor.a;
    lightColor.a = 1.f;
    x3d4_damageLight->SetColor(lightColor);

    x3d4_damageLight->SetDistC(1.f);
    x3d4_damageLight->SetDistL(0.f);
    x3d4_damageLight->SetAngleC(g_tweakGui->GetDamageLightAngleC());
    x3d4_damageLight->SetAngleL(g_tweakGui->GetDamageLightAngleL());
    x3d4_damageLight->SetAngleQ(g_tweakGui->GetDamageLightAngleQ());
    x3d4_damageLight->SetLoadedIdx(4);

    x3d4_damageLight->SetLocalTransform(zeus::CTransform::Identity());

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

void CSamusHud::UpdateEnergy(float dt, const CStateManager& mgr, bool init)
{
    CPlayer& player = mgr.GetPlayer();
    CPlayerState& playerState = *mgr.GetPlayerState();
    float energy = std::max(0.f, std::ceil(playerState.GetHealthInfo().GetHP()));

    u32 numEnergyTanks = playerState.GetItemCapacity(CPlayerState::EItemType::EnergyTanks);
    x2e0_27_energyLow = player.IsEnergyLow(mgr);

    if (init || energy != x2d0_playerHealth || numEnergyTanks != x2d4_totalEnergyTanks)
    {
        float lastTankEnergy = energy;
        u32 filledTanks = 0;
        while (lastTankEnergy > CPlayerState::GetBaseHealthCapacity())
        {
            ++filledTanks;
            lastTankEnergy -= CPlayerState::GetEnergyTankCapacity();
        }

        if (x2bc_nextState != EHudState::None)
        {
            if (x28c_energyIntf)
            {
                float curLastTankEnergy = x2d0_playerHealth;
                while (curLastTankEnergy > CPlayerState::GetBaseHealthCapacity())
                    curLastTankEnergy -= CPlayerState::GetEnergyTankCapacity();
                x28c_energyIntf->SetCurrEnergy(lastTankEnergy, curLastTankEnergy < lastTankEnergy);
            }
            x2d0_playerHealth = energy;
            if (x28c_energyIntf)
            {
                x28c_energyIntf->SetNumTotalEnergyTanks(numEnergyTanks);
                x28c_energyIntf->SetNumFilledEnergyTanks(filledTanks);
                x28c_energyIntf->SetEnergyLow(x2e0_27_energyLow);
            }
            x2d4_totalEnergyTanks = numEnergyTanks;
        }
    }

    if (x2b4_bossEnergyIntf)
    {
        const CEntity* bossEnt = mgr.GetObjectById(mgr.GetBossId());
        if (TCastToConstPtr<CActor> act = bossEnt)
        {
            if (const CHealthInfo* hInfo = act->GetHealthInfo())
            {
                float bossEnergy = std::ceil(hInfo->GetHP());
                x2b4_bossEnergyIntf->SetBossParams(true, g_MainStringTable->GetString(mgr.GetBossStringIdx()),
                                                   bossEnergy, mgr.GetTotalBossEnergy());
            }
            else
            {
                x2b4_bossEnergyIntf->SetBossParams(false, u"", 0.f, 0.f);
            }
        }
        else
        {
            x2b4_bossEnergyIntf->SetBossParams(false, u"", 0.f, 0.f);
        }
    }
}

void CSamusHud::UpdateFreeLook(float dt, const CStateManager& mgr)
{
    TCastToConstPtr<CFirstPersonCamera> fpCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    CPlayer& player = mgr.GetPlayer();
    bool inFreeLook = player.IsInFreeLook() && fpCam;
    bool lookControlHeld = player.IsLookControlHeld();
    if (x2e0_24_inFreeLook != inFreeLook)
    {
        if (inFreeLook)
            CSfxManager::SfxStart(1392, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        else
            CSfxManager::SfxStart(1390, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
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

    if (lookAdj)
    {
        x574_lookDeltaDot = fpCamDir.dot(x568_fpCamDir);
        if (std::fabs(x574_lookDeltaDot) > 1.f)
            x574_lookDeltaDot = (x574_lookDeltaDot >= 0.f) ? 1.f : -1.f;
    }
    else
    {
        x574_lookDeltaDot = 1.f;
    }

    x568_fpCamDir = fpCamDir;

    if ((oldLookDeltaDot >= deltaFrames && x574_lookDeltaDot < deltaFrames) ||
        (oldLookDeltaDot < deltaFrames && x574_lookDeltaDot >= deltaFrames))
    {
        x578_freeLookSfxCycleTimer = 0.f;
    }
    else if (x578_freeLookSfxCycleTimer < 0.05f)
    {
        x578_freeLookSfxCycleTimer = std::min(x578_freeLookSfxCycleTimer + dt, 0.05f);
        if (x578_freeLookSfxCycleTimer == 0.05f)
        {
            if (x574_lookDeltaDot < deltaFrames)
            {
                if (!x564_freeLookSfx)
                    x564_freeLookSfx = CSfxManager::SfxStart(1394, 1.f, 0.f, true, 0x7f, true, kInvalidAreaId);
            }
            else
            {
                CSfxManager::SfxStop(x564_freeLookSfx);
                x564_freeLookSfx.reset();
            }
        }
    }

    if (fpCam)
    {
        zeus::CMatrix3f camRot = fpCam->GetTransform().buildMatrix3f();
        zeus::CVector3f camDir(camRot[1]);
        zeus::CUnitVector3f camDirNoZ(camDir.x, camDir.y, 0.f);
        float offHorizonDot = camDir.dot(camDirNoZ);
        if (std::fabs(offHorizonDot) > 1.f)
            offHorizonDot = (offHorizonDot >= 0.f) ? 1.f : -1.f;
        float offHorizonAngle = std::fabs(std::acos(offHorizonDot));
        if (camDir.z < 0.f)
            offHorizonAngle = -offHorizonAngle;

        if (x298_freeLookIntf)
            x298_freeLookIntf->SetFreeLookState(inFreeLook, lookControlHeld,
                                                player.GetLockonObjectId() != kInvalidUniqueId,
                                                offHorizonAngle);

        if (x564_freeLookSfx)
        {
            float pitch = offHorizonAngle * (g_tweakGui->GetFreeLookSfxPitchScale() / 8192.f) / (M_PIF / 2.f);
            if (!g_tweakGui->GetNoAbsoluteFreeLookSfxPitch())
                pitch = std::fabs(pitch);
            CSfxManager::PitchBend(x564_freeLookSfx, pitch);
        }
    }
}

void CSamusHud::UpdateMissile(float dt, const CStateManager& mgr, bool init)
{
    CPlayerGun& gun = *mgr.GetPlayer().GetPlayerGun();
    CPlayerState& playerState = *mgr.GetPlayerState();

    u32 numMissles = playerState.GetItemAmount(CPlayerState::EItemType::Missiles);
    u32 missileCap = playerState.GetItemCapacity(CPlayerState::EItemType::Missiles);
    CPlayerGun::EMissleMode missileMode = gun.GetMissleMode();
    float chargeFactor = gun.IsCharging() ? gun.GetChargeBeamFactor() : 0.f;

    if (x294_missileIntf)
        x294_missileIntf->SetChargeBeamFactor(chargeFactor);

    if (init || numMissles != x2d8_missileAmount || missileMode != x2ec_missileMode ||
        missileCap != x2dc_missileCapacity)
    {
        if (x294_missileIntf)
        {
            if (missileCap != x2dc_missileCapacity)
                x294_missileIntf->SetMissileCapacity(missileCap);
            if (numMissles != x2d8_missileAmount)
                x294_missileIntf->SetNumMissiles(numMissles, mgr);
            if (missileMode != x2ec_missileMode)
                x294_missileIntf->SetIsMissilesActive(missileMode == CPlayerGun::EMissleMode::Active);
        }
        x2d8_missileAmount = numMissles;
        x2ec_missileMode = missileMode;
        x2dc_missileCapacity = missileCap;
    }
}

void CSamusHud::UpdateVideoBands(float dt, const CStateManager& mgr)
{
    for (int i=0 ; i<4 ; ++i)
        if (x5a4_videoBands[i].x0_videoband)
            x5a4_videoBands[i].x0_videoband->SetIsVisible(false);
}

void CSamusHud::UpdateBallMode(const CStateManager& mgr, bool init)
{
    if (!x2b0_ballIntf)
        return;

    CPlayer& player = mgr.GetPlayer();
    CPlayerGun& gun = *player.GetPlayerGun();
    CPlayerState& playerState = *mgr.GetPlayerState();
    u32 numPbs = playerState.GetItemAmount(CPlayerState::EItemType::PowerBombs);
    u32 pbCap = playerState.GetItemCapacity(CPlayerState::EItemType::PowerBombs);
    u32 bombCount = gun.IsBombReady() ? gun.GetBombCount() : 0;
    bool hasBombs = playerState.HasPowerUp(CPlayerState::EItemType::MorphBallBombs);
    bool pbReady = gun.IsPowerBombReady() &&
        player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed;

    x2b0_ballIntf->SetBombParams(numPbs, pbCap, bombCount, hasBombs, pbReady, false);
}

void CSamusHud::UpdateThreatAssessment(float dt, const CStateManager& mgr)
{
    CMaterialFilter filter(CMaterialList(EMaterialTypes::Trigger), CMaterialList(),
                           CMaterialFilter::EFilterType::Include);

    CPlayer& player = mgr.GetPlayer();
    zeus::CAABox playerAABB = zeus::CAABox::skNullBox;
    if (rstl::optional_object<zeus::CAABox> aabb = player.GetTouchBounds())
        playerAABB = *aabb;

    zeus::CAABox aabb;
    aabb.accumulateBounds(player.GetTranslation() - g_tweakGui->GetThreatRange());
    aabb.accumulateBounds(player.GetTranslation() + g_tweakGui->GetThreatRange());
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, aabb, filter, nullptr);

    float threatDist = 9999.f;
    for (TUniqueId id : nearList)
    {
        const CEntity* ent = mgr.GetObjectById(id);
        if (TCastToConstPtr<CScriptTrigger> trigger = ent)
        {
            if (!bool(trigger->GetTriggerFlags() & ETriggerFlags::DetectPlayer))
                continue;
            if (trigger->GetDamageInfo().GetDamage() == 0.f)
                continue;
            if (rstl::optional_object<zeus::CAABox> aabb = trigger->GetTouchBounds())
            {
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

void CSamusHud::UpdateVisorAndBeamMenus(float dt, const CStateManager& mgr)
{
    CPlayer& player = mgr.GetPlayer();
    CPlayerGun& gun = *player.GetPlayerGun();
    CPlayerState& playerState = *mgr.GetPlayerState();

    float beamInterp = zeus::clamp(0.f, gun.GetGunMorph().x18_, 1.f);
    float visorInterp = playerState.GetVisorTransitionFactor();

    if (x2a8_beamMenu)
    {
        x2a8_beamMenu->SetSelection(gun.GetSelectedBeam(), gun.GetPendingSelectedBeam(), beamInterp);
        x2a8_beamMenu->SetPlayerHas(BuildPlayerHasBeams(mgr));
    }

    if (x2a4_visorMenu)
    {
        x2a4_visorMenu->SetSelection(int(playerState.GetCurrentVisor()),
                                     int(playerState.GetTransitioningVisor()), visorInterp);
        x2a4_visorMenu->SetPlayerHas(BuildPlayerHasVisors(mgr));
    }
}

void CSamusHud::UpdateCameraDebugSettings()
{
    float fov = x5ec_camFovTweaks[g_tweakGui->GetHudCamFovTweak()];
    float y = x62c_camYTweaks[g_tweakGui->GetHudCamYTweak()];
    float z = x72c_camZTweaks[g_tweakGui->GetHudCamZTweak()];
    if (x2a0_helmetIntf)
        x2a0_helmetIntf->UpdateCameraDebugSettings(fov, y, z);
    if (x29c_decoIntf)
        x29c_decoIntf->UpdateCameraDebugSettings(fov, y, z);
    x274_loadedFrmeBaseHud->GetFrameCamera()->SetFov(fov);
    x310_cameraPos.y = y;
    x310_cameraPos.z = z;
}

void CSamusHud::UpdateEnergyLow(float dt, const CStateManager& mgr)
{
    bool cineCam = TCastToConstPtr<CCinematicCamera>(mgr.GetCameraManager()->GetCurrentCamera(mgr));
    float oldTimer = x57c_energyLowTimer;

    x57c_energyLowTimer = std::fmod(x57c_energyLowTimer + dt, 0.5f);
    if (x57c_energyLowTimer < 0.25f)
        x580_energyLowPulse = x57c_energyLowTimer / 0.25f;
    else
        x580_energyLowPulse = (0.5f - x57c_energyLowTimer) / 0.25f;

    if (!cineCam && x2e0_27_energyLow && x57c_energyLowTimer < oldTimer)
        CSfxManager::SfxStart(1405, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
}

void CSamusHud::ApplyClassicLag(const zeus::CUnitVector3f& lookDir, zeus::CQuaternion& rot,
                                const CStateManager& mgr, float dt, bool invert)
{
    zeus::CQuaternion lookRot = zeus::CQuaternion::lookAt(lookDir, zeus::CVector3f::skForward, 2.f * M_PIF);
    zeus::CQuaternion lookRot2;
    if (invert)
    {
        zeus::CUnitVector3f v1(lookRot.transform(x2f8_fpCamDir));
        lookRot2 = zeus::CQuaternion::lookAt(v1, zeus::CVector3f::skForward, 2.f * M_PIF);
    }
    else
    {
        zeus::CUnitVector3f v1(lookRot.transform(x2f8_fpCamDir));
        lookRot2 = zeus::CQuaternion::lookAt(zeus::CVector3f::skForward, v1, 2.f * M_PIF);
    }

    zeus::CQuaternion doubleRot = lookRot2 * lookRot2;
    float dot = doubleRot.toTransform().basis[1].dot(rot.toTransform().basis[1]);
    if (std::fabs(dot) > 1.f)
        dot = (dot >= 0.f) ? 1.f : -1.f;

    float angle = std::acos(dot);
    float tmp = 0.f;
    if (angle > 0.f)
        tmp = 0.5f * dt * g_tweakPlayer->GetHudLagAmount() / angle;

    float t = zeus::clamp(0.f, 18.f * dt * tmp, 1.f);
    rot = zeus::CQuaternion::slerp(rot, doubleRot, t);
}

void CSamusHud::UpdateHudLag(float dt, const CStateManager& mgr)
{
    CPlayer& player = mgr.GetPlayer();

    if (x29c_decoIntf)
        x29c_decoIntf->SetDecoRotation(player.GetYaw());

    if (!g_GameState->GameOptions().GetHUDLag())
    {
        if (x2a0_helmetIntf)
        {
            x2a0_helmetIntf->SetHudLagRotation(zeus::CMatrix3f::skIdentityMatrix3f);
            x2a0_helmetIntf->SetHudLagOffset(zeus::CVector3f::skZero);
        }
        if (x29c_decoIntf)
        {
            x29c_decoIntf->SetReticuleTransform(zeus::CMatrix3f::skIdentityMatrix3f);
            x29c_decoIntf->SetHudRotation(zeus::CQuaternion::skNoRotation);
            x29c_decoIntf->SetHudOffset(zeus::CVector3f::skZero);
        }
        x588_base_basewidget_pivot->SetTransform(
            zeus::CTransform::Translate(x588_base_basewidget_pivot->GetWorldPosition()));
        x274_loadedFrmeBaseHud->GetFrameCamera()->SetO2WTransform(
            BuildFinalCameraTransform(zeus::CQuaternion::skNoRotation, x304_basewidgetIdlePos, x310_cameraPos));
        x8_targetingMgr.SetRotation(zeus::CQuaternion::skNoRotation);
    }
    else
    {
        zeus::CVector3f fpCamDir = x2f8_fpCamDir;
        if (TCastToConstPtr<CFirstPersonCamera> fpCam = mgr.GetCameraManager()->GetCurrentCamera(mgr))
            fpCamDir = fpCam->GetTransform().buildMatrix3f()[1];

        ApplyClassicLag(fpCamDir, x31c_hudLag, mgr, dt, false);
        ApplyClassicLag(fpCamDir, x32c_invHudLag, mgr, dt, true);

        zeus::CQuaternion rot = zeus::CQuaternion::lookAt(zeus::CUnitVector3f(x2f8_fpCamDir), fpCamDir, 2.f * M_PIF);
        rot *= rot;
        rot *= rot;
        x8_targetingMgr.SetRotation(rot);

        zeus::CVector3f bobTranslation = player.GetCameraBob()->GetHelmetBobTranslation();

        zeus::CQuaternion lagRot = x44c_ * x31c_hudLag;
        zeus::CVector3f lagOff = x41c_ * g_tweakGui->GetHudLagOffsetScale();
        lagOff.z += bobTranslation.z;
        if (x2a0_helmetIntf)
        {
            x2a0_helmetIntf->SetHudLagRotation(lagRot);
            x2a0_helmetIntf->SetHudLagOffset(lagOff);
        }
        if (x29c_decoIntf)
        {
            x29c_decoIntf->SetReticuleTransform(x32c_invHudLag);
            x29c_decoIntf->SetHudRotation(lagRot);
            x29c_decoIntf->SetHudOffset(lagOff);
        }

        x274_loadedFrmeBaseHud->GetFrameCamera()->SetO2WTransform(
            BuildFinalCameraTransform(lagRot, x304_basewidgetIdlePos + lagOff, x310_cameraPos));
        x2f8_fpCamDir = fpCamDir;
    }
}

void CSamusHud::UpdateHudDynamicLights(float dt, const CStateManager& mgr)
{
    if (TCastToConstPtr<CFirstPersonCamera> fpCam = mgr.GetCameraManager()->GetCurrentCamera(mgr))
    {

    }
}

void CSamusHud::UpdateHudDamage(float dt, const CStateManager& mgr,
                                DataSpec::ITweakGui::EHelmetVisMode helmetVis)
{

}

void CSamusHud::UpdateStaticSfx(CSfxHandle& handle, float& cycleTimer, u16 sfxId, float dt,
                                float oldStaticInterp, float staticThreshold)
{
    if ((oldStaticInterp > staticThreshold && x510_staticInterp <= staticThreshold) ||
        (oldStaticInterp <= staticThreshold && x510_staticInterp > staticThreshold))
    {
        cycleTimer = 0.f;
    }
    else
    {
        if (cycleTimer < 0.1f)
            cycleTimer = std::min(cycleTimer + dt, 0.1f);
        if (cycleTimer == 0.1f)
        {
            if (x510_staticInterp > staticThreshold)
            {
                if (!handle)
                    handle = CSfxManager::SfxStart(sfxId, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
            }
            else
            {
                CSfxManager::SfxStop(handle);
                handle.reset();
            }
        }
    }
}

void CSamusHud::UpdateStaticInterference(float dt, const CStateManager& mgr)
{
    float intf = mgr.GetPlayerState()->GetStaticInterference().GetTotalInterference();
    float oldStaticInterp = x510_staticInterp;
    if (x510_staticInterp < intf)
        x510_staticInterp = std::min(x510_staticInterp + dt, intf);
    else
        x510_staticInterp = std::max(intf, x510_staticInterp - dt);

    UpdateStaticSfx(x508_staticSfxHi, x514_staticCycleTimerHi, 1402, dt, oldStaticInterp, 0.1f);
    UpdateStaticSfx(x50c_staticSfxLo, x518_staticCycleTimerLo, 1403, dt, oldStaticInterp, 0.5f);

    if (x510_staticInterp > 0.f)
    {
        zeus::CColor color = zeus::CColor::skWhite;
        color.a = x510_staticInterp;
        x51c_camFilter2.SetFilter(CCameraFilterPass::EFilterType::Blend,
                                  CCameraFilterPass::EFilterShape::RandomStatic, 0.f, color, -1);
    }
    else
    {
        x51c_camFilter2.DisableFilter(0.f);
    }
}

void CSamusHud::ShowDamage(const zeus::CVector3f& position, float dam, float prevDam,
                           const CStateManager& mgr)
{

}

void CSamusHud::EnterFirstPerson(const CStateManager& mgr)
{
    CSfxManager::SfxVolume(x508_staticSfxHi, 1.f);
    CSfxManager::SfxVolume(x50c_staticSfxLo, 1.f);
}

void CSamusHud::LeaveFirstPerson(const CStateManager& mgr)
{
    CSfxManager::SfxVolume(x508_staticSfxHi, 0.f);
    CSfxManager::SfxVolume(x50c_staticSfxLo, 0.f);
}

EHudState CSamusHud::GetDesiredHudState(const CStateManager& mgr)
{
    if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed ||
        mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphing)
        return EHudState::Ball;

    switch (mgr.GetPlayerState()->GetTransitioningVisor())
    {
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

void CSamusHud::Update(float dt, const CStateManager& mgr,
                       DataSpec::ITweakGui::EHelmetVisMode helmetVis,
                       bool hudVis, bool targetingManager)
{
    CPlayer& player = mgr.GetPlayer();
    UpdateStateTransition(dt, mgr);
    bool firstPerson = player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed &&
                       !mgr.GetCameraManager()->IsInCinematicCamera();
    if (firstPerson != x2e0_26_latestFirstPerson)
    {
        if (firstPerson)
            EnterFirstPerson(mgr);
        else
            LeaveFirstPerson(mgr);
        x2e0_26_latestFirstPerson = firstPerson;
    }

    float morphT = 0.f;
    switch (player.GetMorphballTransitionState())
    {
    case CPlayer::EPlayerMorphBallState::Morphed:
        morphT = 1.f;
        break;
    case CPlayer::EPlayerMorphBallState::Morphing:
        if (player.GetMorphDuration() == 0.f)
            morphT = 0.f;
        else
            morphT = zeus::clamp(0.f, player.GetMorphTime() / player.GetMorphDuration(), 1.f);
        break;
    case CPlayer::EPlayerMorphBallState::Unmorphing:
        if (player.GetMorphDuration() != 0.f)
            morphT = zeus::clamp(0.f, player.GetMorphTime() / player.GetMorphDuration(), 1.f);
        morphT = 1.f - morphT;
        break;
    default: break;
    }

    x500_viewportScale.y = 1.f - morphT * g_tweakGui->GetBallViewportYReduction();
    if (x2b0_ballIntf)
        x2b0_ballIntf->SetBallModeFactor(morphT);

    bool helmetVisible = false;
    bool glowVisible = false;
    bool decoVisible = false;
    if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed)
    {
        switch (helmetVis)
        {
        case DataSpec::ITweakGui::EHelmetVisMode::HelmetOnly:
            helmetVisible = true;
            break;
        case DataSpec::ITweakGui::EHelmetVisMode::GlowHelmetDeco:
            glowVisible = true;
        case DataSpec::ITweakGui::EHelmetVisMode::HelmetDeco:
            helmetVisible = true;
        case DataSpec::ITweakGui::EHelmetVisMode::Deco:
            decoVisible = true;
        default: break;
        }
    }

    if (x29c_decoIntf)
        x29c_decoIntf->SetIsVisibleDebug(decoVisible);
    if (x2a0_helmetIntf)
        x2a0_helmetIntf->SetIsVisibleDebug(helmetVisible, glowVisible);

    x590_base_Model_AutoMapper->SetIsVisible(false);

    UpdateEnergyLow(dt, mgr);

    for (int i=0 ; i<15 ; ++i)
    {
        x7ac_[i].x0_ = 0;
        x7ac_[i].x4_ = 0;
    }

    if (x2ac_radarIntf)
        x2ac_radarIntf->Update(dt, mgr);

    UpdateHudLag(dt, mgr);

    UpdateHudDynamicLights(dt, mgr);

    if (targetingManager)
        x8_targetingMgr.Update(dt, mgr);

    UpdateHudDamage(dt, mgr, helmetVis);

    UpdateStaticInterference(dt, mgr);

    if (helmetVis != DataSpec::ITweakGui::EHelmetVisMode::ReducedUpdate)
    {
        if (x2bc_nextState != EHudState::None)
        {
            UpdateEnergy(dt, mgr, false);
            UpdateFreeLook(dt, mgr);
        }

        if (x2bc_nextState == EHudState::Ball)
        {
            UpdateBallMode(mgr, false);
        }
        else if (x2bc_nextState >= EHudState::Combat && x2bc_nextState <= EHudState::Scan)
        {
            UpdateThreatAssessment(dt, mgr);
            UpdateMissile(dt, mgr, false);
            UpdateVideoBands(dt, mgr);
        }

        UpdateVisorAndBeamMenus(dt, mgr);

        if (player.WasDamaged() && mgr.GetGameState() == CStateManager::EGameState::Running)
            ShowDamage(player.GetDamageLocationWR(), player.GetDamageAmount(), player.GetPrevDamageAmount(), mgr);
    }

    float oldAPulse = x584_abuttonPulse;
    if (!x554_hudMemoIdx)
    {
        x584_abuttonPulse += 2.f * dt;
        if (x584_abuttonPulse > 1.f)
            x584_abuttonPulse -= 2.f;
    }

    zeus::CColor abuttonColor = zeus::CColor::skWhite;
    abuttonColor.a = std::fabs(x584_abuttonPulse);
    x5a0_base_model_abutton->SetColor(abuttonColor);

    if (!mgr.GetCameraManager()->IsInCinematicCamera() && oldAPulse < 0.f && x584_abuttonPulse >= 0.f &&
        x598_base_basewidget_message->GetIsVisible() && (x558_messageTextAlpha == 0.f || x558_messageTextAlpha >= 1.f))
    {
        CSfxManager::SfxStart(1442, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    }

    float allTextAlpha;
    if (x29c_decoIntf)
        allTextAlpha = x29c_decoIntf->GetHudTextAlpha();
    else
        allTextAlpha = 1.f;

    float messageTextAlpha = 1.f;
    if (x558_messageTextAlpha > 0.f)
        messageTextAlpha = std::min(x558_messageTextAlpha, 1.f);
    else if (!x59c_base_textpane_message->GetIsVisible() && !x598_base_basewidget_message->GetIsVisible())
        messageTextAlpha = 0.f;

    if (x2b4_bossEnergyIntf)
        x2b4_bossEnergyIntf->SetAlpha(1.f - messageTextAlpha);

    if (x550_hudMemoString && x550_hudMemoString.IsLoaded())
    {
        SetMessage(x550_hudMemoString->GetString(x554_hudMemoIdx), x548_hudMemoParms);
        x550_hudMemoString = TLockedToken<CStringTable>();
    }

    if (x558_messageTextAlpha > 0.f)
    {
        x558_messageTextAlpha = std::max(0.f, x558_messageTextAlpha - dt);
        if (x558_messageTextAlpha == 0.f)
        {
            x59c_base_textpane_message->TextSupport()->SetTypeWriteEffectOptions(false, 0.f, 1.f);
            x598_base_basewidget_message->SetVisibility(false, ETraversalMode::Children);
        }
    }

    CGuiWidget* messageWidget;
    if (x598_base_basewidget_message->GetIsVisible())
        messageWidget = x598_base_basewidget_message;
    else
        messageWidget = x59c_base_textpane_message;

    zeus::CColor messageColor = zeus::CColor::skWhite;
    float textScale = 1.f;
    messageColor.a = std::min(allTextAlpha, messageTextAlpha);
    messageWidget->SetColor(messageColor);

    if (messageWidget == x598_base_basewidget_message)
    {
        if (x558_messageTextAlpha > 0.f)
            x560_messageTextScale = std::min(x558_messageTextAlpha, 1.f);
        else
            x560_messageTextScale = std::min(x560_messageTextScale + dt, 1.f);

        float textScaleT = std::max(0.f, (x560_messageTextScale - 0.75f) / 0.25f);
        if (textScaleT != 1.f)
        {
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
    if (x59c_base_textpane_message->TextSupport()->GetNumCharsPrinted() >= nextSfxChars)
    {
        x55c_lastSfxChars = nextSfxChars;
        if (!x598_base_basewidget_message->GetIsVisible() || textScale == 1.f)
            CSfxManager::SfxStart(1418, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    }

    if (mgr.GetEscapeSequenceTimer() > 0.f)
    {
        int minutes = mgr.GetEscapeSequenceTimer() / 60.f;
        int seconds = std::fmod(mgr.GetEscapeSequenceTimer(), 60.f);
        int hundredths = std::fmod(mgr.GetEscapeSequenceTimer() * 100.f, 100.f);
        std::string timeStr = hecl::Format("%02d:%02d:%02d",
                                 int(minutes), int(seconds), int(hundredths));
        x594_base_textpane_counter->TextSupport()->SetText(timeStr);
        x594_base_textpane_counter->SetIsVisible(true);

        zeus::CColor counterColor = zeus::CColor::skWhite;
        counterColor.a = zeus::clamp(0.f, std::min(1.f - std::min(x558_messageTextAlpha, 1.f), allTextAlpha), 1.f);
        x594_base_textpane_counter->SetColor(counterColor);
    }
    else
    {
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
    if (curVisor != CPlayerState::EPlayerVisor::Scan)
    {
        if (transVisor == CPlayerState::EPlayerVisor::Scan)
            transFactor = mgr.GetPlayerState()->GetVisorTransitionFactor();
        else
            transFactor = 1.f;
    }

    if (x2a4_visorMenu)
    {
        float hudAlpha;
        if (g_GameState->GameOptions().GetSwapBeamControls())
            hudAlpha = transFactor;
        else
            hudAlpha = x2f0_visorBeamMenuAlpha;
        x2a4_visorMenu->UpdateHudAlpha(hudAlpha);
        x2a4_visorMenu->Update(dt, false);
    }

    if (x2a8_beamMenu)
    {
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

void CSamusHud::UpdateStateTransition(float dt, const CStateManager& mgr)
{
    if (x2cc_preLoadCountdown == 0)
    {
        EHudState desiredState = GetDesiredHudState(mgr);
        if (desiredState != x2c0_setState)
        {
            x2c0_setState = desiredState;
            if (desiredState == EHudState::Ball || x2bc_nextState == EHudState::Ball)
                x2c8_transT = FLT_EPSILON;
            x2c4_activeTransState = ETransitionState::Countdown;
        }
    }

    switch (x2c4_activeTransState)
    {
    case ETransitionState::Countdown:
        if (x2cc_preLoadCountdown == 0)
        {
            x2c8_transT = std::max(x2c8_transT - 5.f * dt, 0.f);
            if (x2c8_transT == 0.f)
            {
                x2cc_preLoadCountdown = 2;
                x288_loadedSelectedHud = nullptr;
            }
        }
        else
        {
            x2cc_preLoadCountdown -= 1;
            if (x2cc_preLoadCountdown == 0)
            {
                UninitializeFrameGlueMutable();
                x278_selectedHud = TLockedToken<CGuiFrame>();
                switch (x2c0_setState)
                {
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
    case ETransitionState::Loading:
        if (x278_selectedHud)
        {
            if (!x278_selectedHud.IsLoaded() || !x278_selectedHud->GetIsFinishedLoading())
                return;
            x288_loadedSelectedHud = x278_selectedHud.GetObj();
            x2b8_curState = x2bc_nextState;
            x2bc_nextState = x2c0_setState;
            InitializeFrameGlueMutable(mgr);
            x2c4_activeTransState = ETransitionState::Transitioning;
            UpdateCameraDebugSettings();
        }
        else
        {
            x2b8_curState = x2bc_nextState;
            x2bc_nextState = x2c0_setState;
            x2c4_activeTransState = ETransitionState::NotTransitioning;
        }
        break;
    case ETransitionState::Transitioning:
        x2c8_transT = std::min(1.f, 5.f * dt + x2c8_transT);
        if (x2c8_transT == 1.f)
            x2c4_activeTransState = ETransitionState::NotTransitioning;
    default: break;
    }
}

bool CSamusHud::CheckLoadComplete(CStateManager& stateMgr)
{
    switch (x4_loadPhase)
    {
    case ELoadPhase::Zero:
        if (!x8_targetingMgr.CheckLoadComplete())
            return false;
        x4_loadPhase = ELoadPhase::One;
    case ELoadPhase::One:
        UpdateStateTransition(1.f, stateMgr);
        if (x2bc_nextState != x2c0_setState)
            return false;
        x4_loadPhase = ELoadPhase::Two;
    case ELoadPhase::Two:
        if (!x264_loadedFrmeHelmet->GetIsFinishedLoading())
            return false;
        if (!x274_loadedFrmeBaseHud->GetIsFinishedLoading())
            return false;
        x4_loadPhase = ELoadPhase::Three;
    case ELoadPhase::Three:
        return true;
    default: break;
    }

    return false;
}

void CSamusHud::OnNewInGameGuiState(EInGameGuiState state, CStateManager& stateMgr)
{
    // Empty
}

void CSamusHud::RefreshHudOptions()
{
    if (x29c_decoIntf)
        x29c_decoIntf->UpdateHudAlpha();
    if (x2a0_helmetIntf)
        x2a0_helmetIntf->UpdateHelmetAlpha();
}

void CSamusHud::Touch()
{
    if (x264_loadedFrmeHelmet)
        x264_loadedFrmeHelmet->Touch();
    if (x274_loadedFrmeBaseHud)
        x274_loadedFrmeBaseHud->Touch();
    if (x288_loadedSelectedHud)
        x288_loadedSelectedHud->Touch();
}

zeus::CTransform CSamusHud::BuildFinalCameraTransform(const zeus::CQuaternion& rot,
                                                      const zeus::CVector3f& pos,
                                                      const zeus::CVector3f& camPos)
{
    zeus::CQuaternion invRot = rot.inverse();
    return zeus::CTransform(invRot, invRot.transform(camPos - pos) + pos);
}

void CSamusHud::SetMessage(const std::u16string& text, const CHUDMemoParms& info)
{

}

void CSamusHud::InternalDeferHintMemo(ResId strg, u32 timePeriods, const CHUDMemoParms& info)
{

}

}
}
