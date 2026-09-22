#include "MetroidPrime/HUD/CSamusHud.hpp"

#include "Collision/CollisionUtil.hpp"
#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiLight.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/HUD/CHudBallInterface.hpp"
#include "MetroidPrime/HUD/CHudBossEnergyInterface.hpp"
#include "MetroidPrime/HUD/CHudDecoInterface.hpp"
#include "MetroidPrime/HUD/CHudEnergyInterface.hpp"
#include "MetroidPrime/HUD/CHudFreeLookInterface.hpp"
#include "MetroidPrime/HUD/CHudHelmetInterface.hpp"
#include "MetroidPrime/HUD/CHudMissileInterface.hpp"
#include "MetroidPrime/HUD/CHudRadarInterface.hpp"
#include "MetroidPrime/HUD/CHudThreatInterface.hpp"
#include "MetroidPrime/HUD/CHudVisorBeamMenu.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/SFX/UI.h"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "rstl/math.hpp"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static const char sBaseHudName[] = "FRME_BaseHud";
static const char sHelmetName[] = "FRME_Helmet";
static const char sCombatHudName[] = "FRME_CombatHud";
static const char sXRayHudName[] = "FRME_XRayHudNew";
static const char sScanHudName[] = "FRME_ScanHud";
static const char sThermalHudName[] = "FRME_ThermalHud";
static const char sBallHudName[] = "FRME_BallHud";
static const char sPivotName[] = "basewidget_pivot";
static const char sAutoMapperName[] = "Model_AutoMapper";
static const char sCounterName[] = "textpane_counter";
static const char sVideoBandName[] = "model_videoband";
static const char sMessageName[] = "textpane_message";

const char* const CSamusHud::skHudElementNames[15] = {
    "              Radar", "                Lag", "             Lights", "          Targeting",
    "             Damage", "          FrameGlue", "          BaseFrame", "        EnergyGroup",
    "        ThreatGroup", "       MissileGroup", "      FreeLookGroup", "        HelmetGroup",
    "          DecoGroup", "           CamDebug", "              Total",
};

static CVector3f sLaggedVector = CVector3f::Zero();
static CSamusHud* spSamusHud;
static const float kStaticSfxCycleTime = 0.1f;
static const float kFreeLookSfxCycleTime = 0.05f;
static const float kHudLightDistanceEpsilon = FLT_EPSILON;
static const float kDamagePulseLimit = 0.5f;
static const float kDamagePracticalsEpsilon = FLT_EPSILON;

static inline CQuaternion InvertHudRotation(const CQuaternion& rotation) {
  const CVector3f& inverseVector = -rotation.GetVector();
  return CQuaternion::ScalarVector(rotation.GetScalar(), inverseVector);
}

CTransform4f CSamusHud::BuildFinalCameraTransform(const CQuaternion& rot, const CVector3f& pos,
                                                  const CVector3f& camPos) {
  CVector3f cameraPosition = camPos - pos;
  cameraPosition = InvertHudRotation(rot).Transform(cameraPosition);
  cameraPosition += pos;
  return CTransform4f(InvertHudRotation(rot).BuildTransform(), cameraPosition);
}

void CSamusHud::InitializeFrameGluePermanent(const CStateManager& mgr) {
  x588_base_basewidget_pivot = x274_loadedFrmeBaseHud->FindWidget(sPivotName);
  x58c_helmet_BaseWidget_Pivot = x264_loadedFrmeHelmet->FindWidget("BaseWidget_Pivot");
  x590_base_Model_AutoMapper =
      static_cast< CGuiModel* >(x274_loadedFrmeBaseHud->FindWidget(sAutoMapperName));
  x594_base_textpane_counter =
      static_cast< CGuiTextPane* >(x274_loadedFrmeBaseHud->FindWidget(sCounterName));
  x594_base_textpane_counter->TextSupport().SetFontColor(gpTweakGuiColors->GetHudCounterFill());
  x594_base_textpane_counter->TextSupport().SetOutlineColor(
      gpTweakGuiColors->GetHudCounterOutline());
  x598_base_basewidget_message = x274_loadedFrmeBaseHud->FindWidget("basewidget_message");
  for (CGuiWidget* child = static_cast< CGuiWidget* >(x598_base_basewidget_message->ChildObject());
       child != nullptr; child = static_cast< CGuiWidget* >(child->NextSibling())) {
    child->SetDepthTest(false);
  }
  x59c_base_textpane_message =
      static_cast< CGuiTextPane* >(x274_loadedFrmeBaseHud->FindWidget(sMessageName));
  x5a0_base_model_abutton =
      static_cast< CGuiModel* >(x274_loadedFrmeBaseHud->FindWidget("model_abutton"));
  for (int i = 0; i < 4; ++i) {
    x5d8_guiLights[i] = x264_loadedFrmeHelmet->GetFrameLight(i);
  }
  x5d8_guiLights[3]->SetColor(CColor::Black());
  for (int i = 0; i < 4; ++i) {
    SVideoBand& band = x5a4_videoBands[i];
    band.x0_videoband = static_cast< CGuiModel* >(x274_loadedFrmeBaseHud->FindWidget(
        rstl::string(CBasics::Stringize("%s%d", sVideoBandName, i))));
    band.x4_randA = 6.f + 60.f * rand() / static_cast< float >(RAND_MAX);
    band.x8_randB = 16.f + 240.f * rand() / static_cast< float >(RAND_MAX);
  }
  x59c_base_textpane_message->SetDepthTest(false);
  x598_base_basewidget_message->SetVisibility(false, kTM_Children);
  x59c_base_textpane_message->TextSupport().SetFontColor(gpTweakGuiColors->GetHudMessageFill());
  x59c_base_textpane_message->TextSupport().SetOutlineColor(
      gpTweakGuiColors->GetHudMessageOutline());
  x59c_base_textpane_message->TextSupport().SetControlTXTRMap(
      &gpGameState->GameOptions().GetControlTXTRMap());
  x590_base_Model_AutoMapper->SetIsAlwaysDepthWrite(true);
  x304_basewidgetIdlePos = x588_base_basewidget_pivot->GetTransform().GetTranslation();
  x310_cameraPos = x274_loadedFrmeBaseHud->GetFrameCamera()->GetLocalPosition();
  RefreshHudOptions();
}

void CSamusHud::RefreshHudOptions() {
  if (!x29c_decoIntf.null()) {
    x29c_decoIntf->UpdateHudAlpha();
  }
  if (!x2a0_helmetIntf.null()) {
    x2a0_helmetIntf->UpdateHelmetAlpha();
  }
}

void CSamusHud::InitializeDamageLight() {
  const short lightId = x288_loadedSelectedHud->AddWidgetToIDDB(rstl::string_l("DamageSpotLight"));
  CGuiWidget* parent = x288_loadedSelectedHud->FindWidget(rstl::string_l(sPivotName));
  const CGuiWidget::CGuiWidgetParms parms(
      x288_loadedSelectedHud, false, lightId, parent->GetWidgetID(), true, true, false,
      gpTweakGuiColors->GetHudDamageLightColor(), CGuiWidget::kGMDF_Alpha, false, false);
  const CColor& white = CColor::White();
  CLight spotLight = CLight::BuildSpot(CVector3f::Zero(), CVector3f::Forward(), white,
                                       gpTweakGui->GetHudDamageLightSpotAngle());
  x3d4_damageLight = rs_new CGuiLight(parms, spotLight);
  x3d4_damageLight->SetColor(CColor::White());
  const CColor color = gpTweakGuiColors->GetHudFrameColor();
  x3d4_damageLight->SetAmbientContribution(CColor(color.GetRed() * color.GetAlpha(),
                                                  color.GetGreen() * color.GetAlpha(),
                                                  color.GetBlue() * color.GetAlpha(), 1.f));
  x3d4_damageLight->SetConstantAttenuation(1.f);
  x3d4_damageLight->SetLinearAttenuation(0.f);
  x3d4_damageLight->SetConstantAngleAttenuation(gpTweakGui->GetDamageLightAngleC());
  x3d4_damageLight->SetLinearAngleAttenuation(gpTweakGui->GetDamageLightAngleL());
  x3d4_damageLight->SetQuadraticAngleAttenuation(gpTweakGui->GetDamageLightAngleQ());
  x3d4_damageLight->SetLightIndex(4);
  x3d4_damageLight->SetO2PTransform(CTransform4f::Identity());
  x288_loadedSelectedHud->FindWidget(parms.x8_parentId)
      ->AddChildWidget(x3d4_damageLight, false, true);
  x288_loadedSelectedHud->AddLight(x3d4_damageLight);
  const float xAngle = gpTweakGui->GetDamageLightXfXAngle();
  const float zAngle = gpTweakGui->GetDamageLightXfZAngle();
  const CTransform4f lightXf = CTransform4f::Translate(gpTweakGui->GetDamageLightPreTranslate());
  x3d8_lightTransforms.clear();
  x3d8_lightTransforms.reserve(10);
  x3d8_lightTransforms.push_back(lightXf);
  x3d8_lightTransforms.push_back(
      CTransform4f::Translate(gpTweakGui->GetDamageLightCenterTranslate()) * lightXf);
  x3d8_lightTransforms.push_back(CTransform4f::RotateX(CRelAngle::FromDegrees(xAngle)) * lightXf);
  x3d8_lightTransforms.push_back(CTransform4f::RotateX(CRelAngle::FromDegrees(xAngle)) *
                                 CTransform4f::RotateZ(CRelAngle::FromDegrees(-zAngle)) * lightXf);
  x3d8_lightTransforms.push_back(CTransform4f::RotateZ(CRelAngle::FromDegrees(-zAngle)) * lightXf);
  x3d8_lightTransforms.push_back(CTransform4f::RotateX(CRelAngle::FromDegrees(-xAngle)) *
                                 CTransform4f::RotateZ(CRelAngle::FromDegrees(-zAngle)) * lightXf);
  x3d8_lightTransforms.push_back(CTransform4f::RotateX(CRelAngle::FromDegrees(-xAngle)) * lightXf);
  x3d8_lightTransforms.push_back(CTransform4f::RotateX(CRelAngle::FromDegrees(-xAngle)) *
                                 CTransform4f::RotateZ(CRelAngle::FromDegrees(zAngle)) * lightXf);
  x3d8_lightTransforms.push_back(CTransform4f::RotateZ(CRelAngle::FromDegrees(zAngle)) * lightXf);
  x3d8_lightTransforms.push_back(CTransform4f::RotateX(CRelAngle::FromDegrees(xAngle)) *
                                 CTransform4f::RotateZ(CRelAngle::FromDegrees(zAngle)) * lightXf);
}

void CSamusHud::InitializeFrameGlueMutable(const CStateManager& mgr) {
  const float lastTankEnergy = fmodf(x2d0_playerHealth, CPlayerState::GetEnergyTankCapacity());
  const int tanksFilled = x2d0_playerHealth / CPlayerState::GetEnergyTankCapacity();
  const CPlayer& player = *mgr.GetPlayer();
  const float charge = player.GetPlayerGun()->GetChargePercentage();
  bool missilesActive = player.GetPlayerGun()->GetMissileMode() == CPlayerGun::kMM_Active;
  bool lockedOn = player.GetOrbitTargetId() != kInvalidUniqueId;
  switch (x2bc_nextState) {
  case kHS_Combat: {
    x2b4_bossEnergyIntf = rs_new CHudBossEnergyInterface(*x288_loadedSelectedHud);
    x28c_energyIntf =
        rs_new CHudEnergyInterface(*x288_loadedSelectedHud, lastTankEnergy, x2d4_totalEnergyTanks,
                                   tanksFilled, x2e0_27_energyLow, kHT_Combat);
    if (x290_threatIntf.null()) {
      x290_threatIntf = rs_new CHudThreatInterface(*x288_loadedSelectedHud, kHT_Combat, 9999.f);
    } else {
      x290_threatIntf->SetIsVisibleGame(true);
    }
    if (x294_missileIntf.null()) {
      x294_missileIntf =
          rs_new CHudMissileInterface(*x288_loadedSelectedHud, x2dc_missileCapacity,
                                      x2d8_missileAmount, charge, missilesActive, kHT_Combat, mgr);
    } else {
      x294_missileIntf->SetIsVisibleGame(true, mgr);
    }
    if (x298_freeLookIntf.null()) {
      x298_freeLookIntf =
          rs_new CHudFreeLookInterface(*x288_loadedSelectedHud, kHT_Combat, x2e0_24_inFreeLook,
                                       x2e0_25_lookControlHeld, lockedOn);
    } else {
      x298_freeLookIntf->SetIsVisibleGame(true);
    }
    if (x29c_decoIntf.null()) {
      x29c_decoIntf = rs_new CHudDecoInterfaceCombat(*x288_loadedSelectedHud);
    } else {
      x29c_decoIntf->SetIsVisibleGame(true);
    }
    x2ac_radarIntf->SetIsVisibleGame(true);
    x2a4_visorMenu->SetIsVisibleGame(true);
    x2a8_beamMenu->SetIsVisibleGame(true);
    InitializeDamageLight();
    UpdateEnergy(0.f, mgr, true);
    break;
  }
  case kHS_Ball: {
    const CPlayerState& state = *mgr.GetPlayerState();
    const CPlayerGun& ballGun = *mgr.GetPlayer()->GetPlayerGun();
    const int powerBombs = state.GetItemAmount(CPlayerState::kIT_PowerBombs);
    const int capacity = state.GetItemCapacity(CPlayerState::kIT_PowerBombs);
    const int bombs = ballGun.IsBombReady() ? ballGun.GetBombsPending() : 0;
    bool hasBombs = state.HasPowerUp(CPlayerState::kIT_MorphBallBombs);
    bool pbReady = ballGun.IsPowerBombReady() &&
                   mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed;
    x2b4_bossEnergyIntf = nullptr;
    x2b0_ballIntf = rs_new CHudBallInterface(*x288_loadedSelectedHud, powerBombs, capacity, bombs,
                                             pbReady, hasBombs);
    x28c_energyIntf =
        rs_new CHudEnergyInterface(*x288_loadedSelectedHud, lastTankEnergy, x2d4_totalEnergyTanks,
                                   tanksFilled, x2e0_27_energyLow, kHT_Ball);
    x290_threatIntf = nullptr;
    x294_missileIntf = nullptr;
    x298_freeLookIntf = nullptr;
    x29c_decoIntf = nullptr;
    x3d4_damageLight = nullptr;
    x2ac_radarIntf->SetIsVisibleGame(false);
    x2a4_visorMenu->SetIsVisibleGame(false);
    x2a8_beamMenu->SetIsVisibleGame(false);
    UpdateEnergy(0.f, mgr, true);
    break;
  }
  case kHS_Scan: {
    x2b4_bossEnergyIntf = nullptr;
    x28c_energyIntf =
        rs_new CHudEnergyInterface(*x288_loadedSelectedHud, lastTankEnergy, x2d4_totalEnergyTanks,
                                   tanksFilled, x2e0_27_energyLow, kHT_Scan);
    x290_threatIntf = nullptr;
    x294_missileIntf = nullptr;
    x298_freeLookIntf = rs_new CHudFreeLookInterface(
        *x288_loadedSelectedHud, kHT_Scan, x2e0_24_inFreeLook, x2e0_25_lookControlHeld, lockedOn);
    x29c_decoIntf = rs_new CHudDecoInterfaceScan(*x288_loadedSelectedHud);
    InitializeDamageLight();
    UpdateEnergy(0.f, mgr, true);
    break;
  }
  case kHS_XRay: {
    x2b4_bossEnergyIntf = rs_new CHudBossEnergyInterface(*x288_loadedSelectedHud);
    x28c_energyIntf =
        rs_new CHudEnergyInterface(*x288_loadedSelectedHud, lastTankEnergy, x2d4_totalEnergyTanks,
                                   tanksFilled, x2e0_27_energyLow, kHT_XRay);
    x290_threatIntf = rs_new CHudThreatInterface(*x288_loadedSelectedHud, kHT_XRay, 9999.f);
    x294_missileIntf =
        rs_new CHudMissileInterface(*x288_loadedSelectedHud, x2dc_missileCapacity,
                                    x2d8_missileAmount, charge, missilesActive, kHT_XRay, mgr);
    x298_freeLookIntf = rs_new CHudFreeLookInterfaceXRay(
        *x288_loadedSelectedHud, x2e0_24_inFreeLook, x2e0_25_lookControlHeld, lockedOn);
    x29c_decoIntf = rs_new CHudDecoInterfaceXRay(*x288_loadedSelectedHud);
    InitializeDamageLight();
    x2a4_visorMenu->SetIsVisibleGame(true);
    x2a8_beamMenu->SetIsVisibleGame(true);
    UpdateEnergy(0.f, mgr, true);
    break;
  }
  case kHS_Thermal: {
    x2b4_bossEnergyIntf = rs_new CHudBossEnergyInterface(*x288_loadedSelectedHud);
    x28c_energyIntf =
        rs_new CHudEnergyInterface(*x288_loadedSelectedHud, lastTankEnergy, x2d4_totalEnergyTanks,
                                   tanksFilled, x2e0_27_energyLow, kHT_Thermal);
    x290_threatIntf = rs_new CHudThreatInterface(*x288_loadedSelectedHud, kHT_Thermal, 9999.f);
    x294_missileIntf =
        rs_new CHudMissileInterface(*x288_loadedSelectedHud, x2dc_missileCapacity,
                                    x2d8_missileAmount, charge, missilesActive, kHT_Thermal, mgr);
    x298_freeLookIntf =
        rs_new CHudFreeLookInterface(*x288_loadedSelectedHud, kHT_Thermal, x2e0_24_inFreeLook,
                                     x2e0_25_lookControlHeld, lockedOn);
    x29c_decoIntf = rs_new CHudDecoInterfaceThermal(*x288_loadedSelectedHud);
    InitializeDamageLight();
    x2a4_visorMenu->SetIsVisibleGame(true);
    x2a8_beamMenu->SetIsVisibleGame(true);
    UpdateEnergy(0.f, mgr, true);
    break;
  }
  case kHS_None:
    UninitializeFrameGlueMutable();
    break;
  }
}

void CSamusHud::UninitializeFrameGlueMutable() {
  x2b4_bossEnergyIntf = nullptr;
  x28c_energyIntf = nullptr;
  x29c_decoIntf = nullptr;
  x290_threatIntf = nullptr;
  x294_missileIntf = nullptr;
  x298_freeLookIntf = nullptr;
  x2b0_ballIntf = nullptr;
  x3d4_damageLight = nullptr;
}

void CSamusHud::DisplayHudMemo(const rstl::wstring& text, const CHUDMemoParms& info) {
  if (spSamusHud != nullptr) {
    spSamusHud->InternalDisplayHudMemo(text, info);
  }
}

void CSamusHud::DeferHintMemo(uint strg, uint hintNum, const CHUDMemoParms& info) {
  if (spSamusHud != nullptr) {
    spSamusHud->_DeferHintMemo(strg, hintNum, info);
  }
}

void CSamusHud::InternalDisplayHudMemo(const rstl::wstring& text, const CHUDMemoParms& info) {
  SetMessage(text, info);
}

void CSamusHud::_DeferHintMemo(uint strg, uint hintNum, const CHUDMemoParms& info) {
  x548_hudMemoParms = info;
  x550_hudMemoString =
      rs_new TToken< CStringTable >(gpSimplePool->GetObj(SObjectTag('STRG', strg)));
  x550_hudMemoString->Lock();
  x554_hudMemoIdx = hintNum;
}

void CSamusHud::UpdateThreatAssessment(float dt, const CStateManager& mgr) {
  const CMaterialFilter filter =
      CMaterialFilter(CMaterialList(kMT_Trigger), CMaterialList(), CMaterialFilter::kFT_Include);
  const CPlayer& player = *mgr.GetPlayer();
  const CVector3f position = player.GetTranslation();
  CAABox playerBounds = CAABox::MakeNullBox();
  const rstl::optional_object< CAABox > playerTouch = player.GetTouchBounds();
  if (playerTouch.valid()) {
    playerBounds = *playerTouch;
  }
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  const float range = gpTweakGui->GetThreatRange();
  bounds.AccumulateBounds(position + CVector3f(-range, -range, -range));
  bounds.AccumulateBounds(position + CVector3f(range, range, range));
  TEntityList nearList;
  mgr.BuildNearList(nearList, bounds, filter, nullptr);
  float threatDistance = 9999.f;
  for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
    const CScriptTrigger* trigger = TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(*it));
    if (trigger != nullptr && (trigger->GetTriggerFlags() & kTFL_DetectPlayer) != 0 &&
        trigger->GetDamageInfo().GetDamage() != 0.f) {
      const rstl::optional_object< CAABox > touchBounds = trigger->GetTouchBounds();
      if (touchBounds.valid()) {
        const CAABox triggerBounds = *touchBounds;
        const float distance = CAABox::DistanceBetween(playerBounds, triggerBounds);
        if (distance < threatDistance) {
          threatDistance = distance;
        }
      }
        }
  }
  const float threatOverride = mgr.GetPlayer()->GetThreatOverride();
  if (threatOverride > 0.f) {
    threatDistance =
        rstl::min_val(threatDistance, (1.f - threatOverride) * gpTweakGui->GetThreatRange());
  }
  if (mgr.IsFullThreat()) {
    threatDistance = 0.f;
  }
  if (!x290_threatIntf.null()) {
    x290_threatIntf->SetThreatDistance(threatDistance);
  }
}

CSamusHud::CSamusHud(const CStateManager& mgr)
: x4_loadPhase(kLP_Zero)
, x8_targetingMgr(mgr)
, x258_frmeHelmet(gpSimplePool->GetObj(sHelmetName))
, x264_loadedFrmeHelmet(nullptr)
, x268_frmeBaseHud(gpSimplePool->GetObj(sBaseHudName))
, x274_loadedFrmeBaseHud(nullptr)
, x278_selectedHud(rstl::optional_object_null())
, x288_loadedSelectedHud(nullptr)
, x2b8_curState(kHS_None)
, x2bc_nextState(kHS_None)
, x2c0_setState(kHS_None)
, x2c4_activeTransState(kTS_NotTransitioning)
, x2c8_transT(1.f)
, x2cc_preLoadCountdown(0)
, x2d0_playerHealth(0.f)
, x2d4_totalEnergyTanks(0)
, x2d8_missileAmount(0)
, x2dc_missileCapacity(0)
, x2e0_24_inFreeLook(false)
, x2e0_25_lookControlHeld(false)
, x2e0_26_latestFirstPerson(true)
, x2e0_27_energyLow(mgr.GetPlayer()->IsEnergyLow(mgr))
, x2e4_(0)
, x2e8_(0)
, x2ec_missileMode(CPlayerGun::kMM_Inactive)
, x2f0_visorBeamMenuAlpha(1.f)
, x2f8_fpCamDir(mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform().GetForward())
, x304_basewidgetIdlePos(CVector3f::Zero())
, x310_cameraPos(CVector3f::Zero())
, x31c_hudLag(CQuaternion::NoRotation())
, x32c_invHudLag(CQuaternion::NoRotation())
, x33c_lights(rs_new CActorLights(8, CVector3f::Zero(), 4, 1,
                                  CActorLights::kDefaultPositionUpdateThreshold, true, false,
                                  false))
, x340_hudLights(3, SCachedHudLight(CVector3f::Zero(), CColor::White(), 0.f, 0.f, 0.f, 0.f))
, x3d4_damageLight(nullptr)
, x3e8_damageTime(0.f)
, x3ec_damageLightPulser(0.f)
, x3f0_damageFilterAmtInit(1.f)
, x3f4_damageFilterAmt(0.f)
, x3f8_damageFilterAmtGain(0.f)
, x3fc_hudDamagePracticalsInit(0.f)
, x400_hudDamagePracticals(0.f)
, x404_hudDamagePracticalsGain(0.f)
, x408_damagerToPlayerNorm(CVector3f::Zero())
, x414_decoShakeTranslateAmt(0.f)
, x418_decoShakeTranslateAmtVel(0.f)
, x41c_decoShakeTranslate(CVector3f::Zero())
, x428_decoShakeRotate(CMatrix3f::Identity())
, x44c_hudLagShakeRot(CQuaternion::NoRotation())
, x45c_decoShakeAmtInit(0.f)
, x460_decoShakeAmt(0.f)
, x464_decoShakeAmtGain(0.f)
, x468_(0)
, x46c_(3, CTransform4f::Identity())
, x500_viewportScaleX(1.f)
, x504_viewportScaleY(1.f)
, x510_staticInterp(0.f)
, x514_staticCycleTimerHi(0.f)
, x518_staticCycleTimerLo(0.f)
, x548_hudMemoParms(0.f, false, false, false)
, x554_hudMemoIdx(0)
, x558_messageTextTime(0.f)
, x55c_lastSfxChars(0.f)
, x560_messageTextScale(0.f)
, x568_fpCamDir(mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform().GetForward())
, x574_lookDeltaDot(1.f)
, x578_freeLookSfxCycleTimer(0.f)
, x57c_energyLowTimer(0.f)
, x580_energyLowPulse(0.f)
, x584_abuttonPulse(0.f)
, x5a4_videoBands(4, SVideoBand())
, x5d8_guiLights(4, nullptr)
, x7ac_profileInfo(15, SProfileInfo()) {
  UpdateStateTransition(1.f, mgr);
  spSamusHud = this;
  for (int i = 0; i < 16; ++i) {
    x5ec_camFovTweaks[i] = 5.f * i + 40.f;
  }
  for (int i = 0; i < 64; ++i) {
    x62c_camYTweaks[i] = -0.5f * i;
  }
  for (int i = 0; i < 32; ++i) {
    x72c_camZTweaks[i] = 0.5f * i + -8.f;
  }
  x264_loadedFrmeHelmet = *x258_frmeHelmet;
  x274_loadedFrmeBaseHud = *x268_frmeBaseHud;
  x2a0_helmetIntf = rs_new CHudHelmetInterface(*x264_loadedFrmeHelmet);
  x2a4_visorMenu = rs_new CHudVisorBeamMenu(*x274_loadedFrmeBaseHud, CHudVisorBeamMenu::kVBM_Visor,
                                            BuildPlayerHasVisors(mgr));
  x2a8_beamMenu = rs_new CHudVisorBeamMenu(*x274_loadedFrmeBaseHud, CHudVisorBeamMenu::kVBM_Beam,
                                           BuildPlayerHasBeams(mgr));
  x2ac_radarIntf = rs_new CHudRadarInterface(*x274_loadedFrmeBaseHud, mgr);
  InitializeFrameGluePermanent(mgr);
  UpdateEnergy(0.f, mgr, true);
  UpdateMissile(0.f, mgr, true);
  UpdateBallMode(mgr, true);
}

void CSamusHud::UpdateEnergyLow(float dt, const CStateManager& mgr) {
  const bool cineCam =
      TCastToConstPtr< CCinematicCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr)) != nullptr;
  const float oldTimer = x57c_energyLowTimer;
  x57c_energyLowTimer = fmod(x57c_energyLowTimer + dt, 0.5);
  x580_energyLowPulse = x57c_energyLowTimer < 0.25f ? x57c_energyLowTimer / 0.25f
                                                    : (0.5f - x57c_energyLowTimer) / 0.25f;
  if (!cineCam && x2e0_27_energyLow && x57c_energyLowTimer < oldTimer) {
    CSfxManager::SfxStart(SFXui_x_warning_02, 127, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
}

CSamusHud::~CSamusHud() {
  if (x3a4_damageSfx) {
    CSfxManager::RemoveEmitter(x3a4_damageSfx);
  }
  spSamusHud = nullptr;
}

bool CSamusHud::CheckLoadComplete(const CStateManager& mgr) {
  switch (x4_loadPhase) {
  case kLP_Zero:
    if (x8_targetingMgr.CheckLoadComplete()) {
      x4_loadPhase = kLP_One;
    } else {
      return false;
    }
  case kLP_One:
    UpdateStateTransition(1.f, mgr);
    if (x2bc_nextState != x2c0_setState) {
      return false;
    }
    x4_loadPhase = kLP_Two;
  case kLP_Two:
    if (x264_loadedFrmeHelmet->GetIsFinishedLoading() &&
        x274_loadedFrmeBaseHud->GetIsFinishedLoading()) {
      x4_loadPhase = kLP_Three;
    } else {
      return false;
    }
  case kLP_Three:
    return true;
  default:
    break;
  }
  return false;
}

void CSamusHud::UpdateVisorAndBeamMenus(float dt, const CStateManager& mgr) {
  const CPlayerGun& gun = *mgr.GetPlayer()->GetPlayerGun();
  const CPlayerState::EBeamId currentBeam = gun.GetPrimaryWeaponId();
  const CPlayerState::EBeamId nextBeam = gun.GetPrimaryDestWeaponId();
  const float beamInterp = CMath::Clamp(0.f, gun.GetHoloTransitionFactor(), 1.f);
  const CPlayerState& state = *mgr.GetPlayerState();
  const CPlayerState::EPlayerVisor visor = state.GetCurrentVisor();
  const CPlayerState::EPlayerVisor nextVisor = state.GetTransitioningVisor();
  const float visorInterp = state.GetVisorTransitionFactor();
  if (!x2a8_beamMenu.null()) {
    x2a8_beamMenu->SetSelection(currentBeam, nextBeam, beamInterp);
    x2a8_beamMenu->SetPlayerHas(BuildPlayerHasBeams(mgr));
  }
  if (!x2a4_visorMenu.null()) {
    x2a4_visorMenu->SetSelection(visor, nextVisor, visorInterp);
    x2a4_visorMenu->SetPlayerHas(BuildPlayerHasVisors(mgr));
  }
}


void CSamusHud::UpdateFreeLook(float dt, const CStateManager& mgr) {
  const CFirstPersonCamera* const fpCam =
      TCastToConstPtr< CFirstPersonCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr));
  bool inFreeLook = mgr.GetPlayer()->IsInFreeLook() && fpCam != nullptr;
  bool lookHeld = mgr.GetPlayer()->GetFreeLookStickState();
  if (x2e0_24_inFreeLook != inFreeLook) {
    if (inFreeLook) {
      CSfxManager::SfxStart(SFXui_x_freeon_00, 127, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    } else {
      CSfxManager::SfxStart(SFXui_x_freeoff_00, 127, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    }
    x2e0_24_inFreeLook = inFreeLook;
  }
  const float deltaFrames = 60.f * (0.99999f * dt);
  const float oldDot = x574_lookDeltaDot;
  const CVector3f direction = !fpCam ? x568_fpCamDir : fpCam->GetTransform().GetForward();
  bool adjusting = inFreeLook;
  if (adjusting) {
    adjusting = lookHeld;
  }
  x574_lookDeltaDot = adjusting ? CMath::Limit(CVector3f::Dot(direction, x568_fpCamDir), 1.f) : 1.f;
  x568_fpCamDir = direction;
  bool crossed = (oldDot >= deltaFrames && x574_lookDeltaDot < deltaFrames) ||
                 (oldDot < deltaFrames && x574_lookDeltaDot >= deltaFrames);
  if (crossed) {
    x578_freeLookSfxCycleTimer = 0.f;
  } else if (x578_freeLookSfxCycleTimer < 0.05f) {
    x578_freeLookSfxCycleTimer =
        rstl::min_val(kFreeLookSfxCycleTime, x578_freeLookSfxCycleTimer + dt);
    if (x578_freeLookSfxCycleTimer == 0.05f) {
      if (x574_lookDeltaDot < deltaFrames) {
        if (!x564_freeLookSfx) {
          x564_freeLookSfx =
              CSfxManager::SfxStart(SFXui_x_samrot_lp_00, 127, 64, true, CSfxManager::kMedPriority,
                                    true, CSfxManager::kAllAreas);
        }
      } else {
        CSfxManager::SfxStop(x564_freeLookSfx);
        x564_freeLookSfx.Clear();
      }
    }
  }
  if (fpCam) {
    const CMatrix3f cameraRotation = fpCam->GetTransform().BuildMatrix3f();
    const CUnitVector3f cameraDirection(cameraRotation.GetColumn(1));
    CVector3f horizonDirection(cameraDirection.GetX(), cameraDirection.GetY(), 0.f);
    horizonDirection.Normalize();
    const float dot = CMath::Limit(CVector3f::Dot(cameraDirection, horizonDirection), 1.f);
    float angle = CMath::AbsF(acosf(dot));
    if (cameraDirection.GetZ() < 0.f) {
      angle = -angle;
    }
    if (!x298_freeLookIntf.null()) {
      x298_freeLookIntf->SetFreeLookState(
          inFreeLook, lookHeld, mgr.GetPlayer()->GetOrbitTargetId() != kInvalidUniqueId, angle);
    }
    if (x564_freeLookSfx) {
      float pitch = angle * gpTweakGui->GetFreeLookSfxPitchScale() / (M_PIF / 2.f);
      if (!gpTweakGui->GetNoAbsoluteFreeLookSfxPitch()) {
        pitch = CMath::AbsF(pitch);
      }
      CSfxManager::PitchBend(x564_freeLookSfx, static_cast< int >(8192.f + pitch));
    }
  }
}

void CSamusHud::UpdateVideoBands(float dt, const CStateManager& mgr) {
  for (int i = 0; i < 4; ++i) {
    if (x5a4_videoBands[i].x0_videoband != nullptr) {
      x5a4_videoBands[i].x0_videoband->SetIsVisible(false);
    }
  }
}

void CSamusHud::UpdateStaticInterference(float dt, const CStateManager& mgr) {
  float interference = mgr.GetPlayerState()->GetHudStaticInterferenceAmount();
  const float oldInterference = x510_staticInterp;
  if (x510_staticInterp < interference) {
    x510_staticInterp = rstl::min_val(interference, x510_staticInterp + dt);
  } else if (x510_staticInterp > interference) {
    x510_staticInterp = rstl::max_val(interference, x510_staticInterp - dt);
  }
  UpdateStaticSfx(x508_staticSfxHi, x514_staticCycleTimerHi, SFXui_x_static_lp_0, dt,
                  oldInterference, 0.1f);
  UpdateStaticSfx(x50c_staticSfxLo, x518_staticCycleTimerLo, SFXui_x_static_lp_01, dt,
                  oldInterference, 0.5f);
  if (x510_staticInterp > 0.f) {
    const CColor color = CColor::White().WithAlphaOf(x510_staticInterp);
    x51c_camFilter2.SetFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_RandomStatic,
                              0.f, color, kInvalidAssetId);
  } else {
    x51c_camFilter2.DisableFilter(0.f);
  }
}

void CSamusHud::UpdateStaticSfx(CSfxHandle& handle, float& cycleTimer, ushort sfxId, float dt,
                                float oldStaticInterp, float staticThreshold) {
  bool crossed = (oldStaticInterp > staticThreshold && x510_staticInterp <= staticThreshold) ||
                 (oldStaticInterp <= staticThreshold && x510_staticInterp > staticThreshold);
  if (crossed) {
    cycleTimer = 0.f;
  } else if (cycleTimer < 0.1f) {
    cycleTimer = rstl::min_val(kStaticSfxCycleTime, cycleTimer + dt);
    if (cycleTimer == 0.1f) {
      if (x510_staticInterp > staticThreshold) {
        if (!handle) {
          handle = CSfxManager::SfxStart(sfxId, 127, 64, false, CSfxManager::kMedPriority, true,
                                         CSfxManager::kAllAreas);
        }
      } else {
        CSfxManager::SfxStop(handle);
        handle.Clear();
      }
    }
  }
}

void CSamusHud::UpdateEnergy(float dt, const CStateManager& mgr, bool init) {
  const float energy =
      rstl::max_val(0.f, CMath::CeilingF(mgr.GetPlayerState()->GetHealthInfo().GetHP()));
  const int numEnergyTanks = mgr.GetPlayerState()->GetItemCapacity(CPlayerState::kIT_EnergyTanks);
  x2e0_27_energyLow = mgr.GetPlayer()->IsEnergyLow(mgr);
  if (init || energy != x2d0_playerHealth || numEnergyTanks != x2d4_totalEnergyTanks) {
    float lastTankEnergy = energy;
    uint filledTanks = 0;
    while (lastTankEnergy > CPlayerState::GetBaseHealthCapacity()) {
      ++filledTanks;
      lastTankEnergy -= CPlayerState::GetEnergyTankCapacity();
    }
    if (x2bc_nextState != kHS_None) {
      if (!x28c_energyIntf.null()) {
        float currentTankEnergy = x2d0_playerHealth;
        while (currentTankEnergy > CPlayerState::GetBaseHealthCapacity()) {
          currentTankEnergy -= CPlayerState::GetEnergyTankCapacity();
        }
        x28c_energyIntf->SetCurrEnergy(lastTankEnergy, (currentTankEnergy > lastTankEnergy) !=
                                                           (x2d0_playerHealth > energy));
      }
      x2d0_playerHealth = energy;
      if (!x28c_energyIntf.null()) {
        x28c_energyIntf->SetNumTotalEnergyTanks(numEnergyTanks);
        x28c_energyIntf->SetNumFilledEnergyTanks(filledTanks);
        x28c_energyIntf->SetEnergyLow(x2e0_27_energyLow);
      }
      x2d4_totalEnergyTanks = numEnergyTanks;
    }
  }
  if (!x2b4_bossEnergyIntf.null()) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetBossId()))) {
      if (const CHealthInfo* health = actor->GetHealthInfo(mgr)) {
        const float bossEnergy = CMath::CeilingF(health->GetHP());
        const float maxEnergy = mgr.GetTotalBossEnergy();
        const rstl::wstring name =
            rstl::wstring_l(gpStringTable->GetString(mgr.GetBossStringIdx()));
        x2b4_bossEnergyIntf->SetBossParams(true, name, bossEnergy, maxEnergy);
      } else {
        x2b4_bossEnergyIntf->SetBossParams(false, rstl::wstring_l(L""), 0.f, 0.f);
      }
    } else {
      x2b4_bossEnergyIntf->SetBossParams(false, rstl::wstring_l(L""), 0.f, 0.f);
    }
  }
}

void CSamusHud::UpdateMissile(float dt, const CStateManager& mgr, bool init) {
  const CPlayerGun& gun = *mgr.GetPlayer()->GetPlayerGun();
  const CPlayerState& state = *mgr.GetPlayerState();
  const CPlayerGun::EMissileMode mode = gun.GetMissileMode();
  const int missiles = state.GetItemAmount(CPlayerState::kIT_Missiles);
  const int capacity = state.GetItemCapacity(CPlayerState::kIT_Missiles);
  const float charge = gun.GetChargePercentage();
  if (!x294_missileIntf.null()) {
    x294_missileIntf->SetChargeBeamFactor(charge);
  }
  if (init || missiles != x2d8_missileAmount || mode != x2ec_missileMode ||
      capacity != x2dc_missileCapacity) {
    if (!x294_missileIntf.null()) {
      if (x2dc_missileCapacity != capacity) {
        x294_missileIntf->SetMissileCapacity(capacity);
      }
      if (x2d8_missileAmount != missiles) {
        x294_missileIntf->SetNumMissiles(missiles, mgr);
      }
      if (x2ec_missileMode != mode) {
        x294_missileIntf->SetIsMissilesActive(mode == CPlayerGun::kMM_Active);
      }
    }
    x2d8_missileAmount = missiles;
    x2ec_missileMode = mode;
    x2dc_missileCapacity = capacity;
  }
}

void CSamusHud::UpdateBallMode(const CStateManager& mgr, bool init) {
  if (x2b0_ballIntf.null()) {
    return;
  }
  const CPlayerState& state = *mgr.GetPlayerState();
  const CPlayerGun& gun = *mgr.GetPlayer()->GetPlayerGun();
  const int powerBombs = state.GetItemAmount(CPlayerState::kIT_PowerBombs);
  const int capacity = state.GetItemCapacity(CPlayerState::kIT_PowerBombs);
  const int bombs = gun.IsBombReady() ? gun.GetBombsPending() : 0;
  bool hasBombs = state.HasPowerUp(CPlayerState::kIT_MorphBallBombs);
  bool pbReady = gun.IsPowerBombReady() &&
                 mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed;
  x2b0_ballIntf->SetBombParams(powerBombs, capacity, bombs, hasBombs, pbReady, false);
}

void CSamusHud::OnNewInGameGuiState(EInGameGuiState state, const CStateManager& mgr) {}

bool CSamusHud::IsCachedLightInAreaLights(const SCachedHudLight& cached,
                                          const CActorLights& areaLights) const {
  for (uint i = 0; i < areaLights.GetActiveAreaLightCount(); ++i) {
    const CLight& light = areaLights.GetLight(i);
    if (light.GetColor() == cached.xc_color && light.GetPosition() == cached.x0_pos) {
      return true;
    }
  }
  return false;
}

bool CSamusHud::IsAreaLightInCachedLights(const CLight& light) const {
  for (int i = 0; i < 3; ++i) {
    const SCachedHudLight& cached = x340_hudLights[i];
    if (cached.x1c_fader != 0.f && light.GetColor() == cached.xc_color &&
        light.GetPosition() == cached.x0_pos) {
      return true;
    }
  }
  return false;
}

int CSamusHud::FindEmptyHudLightSlot(const CLight& light) const {
  for (int i = 0; i < 3; ++i) {
    if (x340_hudLights[i].x1c_fader == 0.f) {
      return i;
    }
  }
  return -1;
}

void CSamusHud::UpdateHudDynamicLights(float dt, const CStateManager& mgr) {
  const CFirstPersonCamera* const camera =
      TCastToConstPtr< CFirstPersonCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr));
  if (camera == nullptr) {
    return;
  }
  const CVector3f position = camera->GetTranslation();
  const CVector3f lookDirection = camera->GetTransform().GetForward();
  const CAABox bounds(position - CVector3f(0.125f, 0.125f, 0.125f),
                      position + CVector3f(0.125f, 0.125f, 0.125f));
  CActorLights& lights = *x33c_lights;
  lights.SetFindShadowLight(false);
  const TAreaId area = mgr.GetPlayer()->GetCurrentAreaId();
  if (area == kInvalidAreaId) {
    return;
  }
  lights.BuildAreaLightList(mgr, mgr.GetWorld()->GetAreaAlways(area), bounds);
  for (int i = 0; i < 3; ++i) {
    SCachedHudLight& light = x340_hudLights[i];
    const CVector3f direction = (light.x0_pos - position).AsNormalized();
    if (light.x1c_fader > 0.f && (CVector3f::Dot(lookDirection, direction) <= 0.15707964f ||
                                  !IsCachedLightInAreaLights(light, lights))) {
      light.x1c_fader *= -1.f;
    }
  }
  int available = 0;
  for (int i = 0; i < 3; ++i) {
    if (x340_hudLights[i].x1c_fader <= 0.f) {
      ++available;
    }
  }
  --available;
  for (uint i = 0; i < lights.GetActiveAreaLightCount(); ++i) {
    if (available < 1) {
      break;
    }
    const CLight& light = lights.GetLight(i);
    const CVector3f direction = (light.GetPosition() - position).AsNormalized();
    if (!IsAreaLightInCachedLights(light) &&
        CVector3f::Dot(lookDirection, direction) > 0.15707964f) {
      const int slot = FindEmptyHudLightSlot(light);
      if (slot != -1) {
        --available;
        x340_hudLights[slot] =
            SCachedHudLight(light.GetPosition(), light.GetColor(), light.GetAttenuationConstant(),
                            light.GetAttenuationLinear(), light.GetAttenuationQuadratic(), 0.001f);
      }
    }
  }
  for (int i = 0; i < 3; ++i) {
    SCachedHudLight& light = x340_hudLights[i];
    if (light.x1c_fader < 0.f) {
      light.x1c_fader = rstl::min_val(light.x1c_fader + 2.f * dt, 0.f);
    } else if (light.x1c_fader < 1.f && light.x1c_fader != 0.f) {
      light.x1c_fader = rstl::min_val(light.x1c_fader + 2.f * dt, 1.f);
    }
  }
  const CPlayerState& playerState = *mgr.GetPlayerState();
  const CPlayerState::EPlayerVisor visor = playerState.GetCurrentVisor();
  const float visorTransition = playerState.GetVisorTransitionFactor();
  const CColor addColors[4] = {
      gpTweakGui->GetCombatVisorHudLightAdd(), gpTweakGui->GetXRayVisorHudLightAdd(),
      gpTweakGui->GetScanVisorHudLightAdd(), gpTweakGui->GetThermalVisorHudLightAdd()};
  const CColor multiplyColors[4] = {
      gpTweakGui->GetCombatVisorHudLightMultiply(), gpTweakGui->GetXRayVisorHudLightMultiply(),
      gpTweakGui->GetScanVisorHudLightMultiply(), gpTweakGui->GetThermalVisorHudLightMultiply()};
  CColor lightAdd = CColor::Lerp(addColors[0], addColors[visor], visorTransition);
  const CColor lightMultiply =
      CColor::Lerp(multiplyColors[0], multiplyColors[visor], visorTransition);
  AUTO(lightIt, x5d8_guiLights.begin());
  float maxIntensity = 0.f;
  int maxIntensityIndex = 0;
  for (int i = 0; i < x340_hudLights.size(); ++i) {
    const SCachedHudLight& light = x340_hudLights[i];
    CGuiLight* widget = *lightIt;
    const CVector3f toCamera = position - light.x0_pos;
    const CMatrix3f& rotation = camera->GetTransform().BuildMatrix3f().GetTranspose();
    const CVector3f direction = rotation * toCamera.AsNormalized();
    const float distance = rstl::max_val(toCamera.Magnitude(), kHudLightDistanceEpsilon);
    const float falloff = rstl::min_val(
        1.f, 1.f / (gpTweakGui->GetHudLightAttMulConstant() * light.x10_distC +
                    distance * (gpTweakGui->GetHudLightAttMulLinear() * light.x14_distL) +
                    distance *
                        (distance * (gpTweakGui->GetHudLightAttMulQuadratic() * light.x18_distQ))));
    widget->SetO2WTransform(CTransform4f::LookAt(CVector3f::Zero(), direction));
    const float fadedFalloff = falloff * CMath::AbsF(light.x1c_fader);
    CColor lightColor =
        CColor::Modulate(light.xc_color, CColor(fadedFalloff, fadedFalloff, fadedFalloff, 1.f));
    lightColor = GetVisorHudLightColor(lightColor, mgr);
    widget->SetColor(lightColor);
    lightAdd = CColor::Add(lightAdd, CColor::Modulate(lightColor, lightMultiply));
    const float intensity =
        (fadedFalloff * CVector3f::Dot(CVector3f::Forward(), -1.f * direction)) *
        (0.3f * lightColor.GetRed() + 0.6f * lightColor.GetGreen() + 0.1f * lightColor.GetBlue());
    if (intensity > maxIntensity) {
      maxIntensity = intensity;
      maxIntensityIndex = i;
    }
    ++lightIt;
  }
  const CObjectList& gameLights = mgr.GetObjectListById(kOL_GameLight);
  CLight brightest = CLight::BuildPoint(CVector3f::Zero(), CColor::Black());
  for (int i = gameLights.GetFirstObjectIndex(); i != -1; i = gameLights.GetNextObjectIndex(i)) {
    const CEntity* entity = gameLights[i];
    if (entity != nullptr && entity->GetActive()) {
      const CGameLight* light = static_cast< const CGameLight* >(entity);
      if (!TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(light->GetParentId()))) {
        const CLight& candidate = light->GetLight();
        if (candidate.GetIntensity() > brightest.GetIntensity()) {
          if (CollisionUtil::AABoxSphereIntersection(
                  bounds, CSphere(candidate.GetPosition(), candidate.GetRadius()))) {
            brightest = candidate;
          }
        }
      }
    }
  }
  if (brightest.GetIntensity() > FLT_EPSILON) {
    const CVector3f toCamera = position - brightest.GetPosition();
    const float distance = rstl::max_val(toCamera.Magnitude(), kHudLightDistanceEpsilon);
    const float falloff = rstl::min_val(
        1.f, 1.f / (gpTweakGui->GetHudLightAttMulConstant() * brightest.GetAttenuationConstant() +
                    distance *
                        (gpTweakGui->GetHudLightAttMulLinear() * brightest.GetAttenuationLinear()) +
                    distance * (distance * (gpTweakGui->GetHudLightAttMulQuadratic() *
                                            brightest.GetAttenuationQuadratic()))));
    CColor lightColor =
        CColor::Modulate(brightest.GetColor(), CColor(falloff, falloff, falloff, 1.f));
    lightColor = GetVisorHudLightColor(lightColor, mgr);
    if (brightest.GetType() == kLT_Spot) {
      const float dot = rstl::max_val(
          0.f, CVector3f::Dot(camera->GetTransform().GetForward(), brightest.GetDirection()));
      const float factor = CMath::Clamp(0.f, (2.f / M_PIF) * CMath::ArcSineR(dot), 1.f);
      lightColor = CColor::Modulate(lightColor, CColor(factor, factor, factor, 1.f));
    }
    lightAdd = CColor::Add(lightAdd, lightColor);
  }
  const CColor ambientScale(uchar(64), uchar(64), uchar(64), uchar(255));
  lightAdd = CColor::Add(lightAdd, CColor::Modulate(lights.GetAmbientColor(), ambientScale));
  const CGuiLight& brightestWidget = *x5d8_guiLights[maxIntensityIndex];
  const CVector3f direction = -1.f * brightestWidget.GetWorldTransform().GetForward();
  const float forwardDot = CVector3f::Dot(CVector3f::Forward(), direction);
  const CVector3f reflectedDirection = forwardDot * (2.f * CVector3f::Forward()) - direction;
  CGuiLight* reflection = x5d8_guiLights[3];
  reflection->SetO2WTransform(CTransform4f::LookAt(CVector3f::Zero(), reflectedDirection));
  reflection->SetColor(
      CColor::Modulate(gpTweakGui->GetHudReflectivityLightColor(), brightestWidget.GetColor()));
  reflection->SetAmbientContribution(lightAdd);
}

CColor CSamusHud::GetVisorHudLightColor(const CColor& color, const CStateManager& mgr) {
  const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetCurrentVisor();
  const float t = mgr.GetPlayerState()->GetVisorTransitionFactor();
  CColor result = color;
  switch (visor) {
  case CPlayerState::kPV_Scan: {
    const CColor& white = CColor::White();
    const CColor multiplier =
        CColor::Lerp(white, gpTweakGuiColors->GetScanVisorHudLightMultiply(), t);
    result = CColor::Modulate(result, multiplier);
    break;
  }
  case CPlayerState::kPV_Thermal: {
    const CColor multiplier = gpTweakGuiColors->GetThermalVisorHudLightMultiply();
    result = CColor::Modulate(result, multiplier);
    break;
  }
  case CPlayerState::kPV_XRay: {
    const float intensity =
        0.3f * result.GetRed() + 0.6f * result.GetGreen() + 0.1f * result.GetBlue();
    result = CColor(intensity, intensity, intensity, 1.f);
    break;
  }
  case CPlayerState::kPV_Combat:
  default:
    break;
  }
  return result;
}

void CSamusHud::UpdateHudDamage(float dt, const CStateManager& mgr, uint helmetVis) {
  if (mgr.GetPlayer()->WasDamaged() && mgr.GetGameState() == CStateManager::kGS_Running) {
    x3e8_damageTime += dt;
  } else {
    x3e8_damageTime = 0.f;
  }
  const float pulseDuration = gpTweakGui->GetHudDamagePulseDuration();
  const float pulseTime = CMath::AbsF(fmodf(x3e8_damageTime, pulseDuration));
  x3ec_damageLightPulser = pulseTime < 0.5f * pulseDuration
                               ? pulseTime / (0.5f * pulseDuration)
                               : (pulseDuration - pulseTime) / (0.5f * pulseDuration);
  x3ec_damageLightPulser =
      CMath::Clamp(0.f,
                   gpTweakGui->GetHudDamageColorGain() *
                       (x3ec_damageLightPulser *
                        rstl::min_val(kDamagePulseLimit, mgr.GetPlayer()->GetDamageAmount())),
                   1.f);
  const CColor frameColor = gpTweakGuiColors->GetHudFrameColor();
  const CColor damageAmbient(frameColor.GetRed() * frameColor.GetAlpha() + x3ec_damageLightPulser,
                             frameColor.GetGreen() * frameColor.GetAlpha() + x3ec_damageLightPulser,
                             frameColor.GetBlue() * frameColor.GetAlpha() + x3ec_damageLightPulser,
                             1.f);
  if (x3d4_damageLight) {
    x3d4_damageLight->SetAmbientContribution(damageAmbient);
  }
  if (x3f4_damageFilterAmt > 0.f) {
    x3f4_damageFilterAmt = rstl::max_val(0.f, x3f4_damageFilterAmt - dt);
    if (x3f4_damageFilterAmt == 0.f) {
      CSfxManager::RemoveEmitter(x3a4_damageSfx);
      x3a4_damageSfx.Clear();
    }
  }
  const float peak = x3f0_damageFilterAmtInit * gpTweakGui->GetHudDamagePeakFactor();
  const CColor& ambientColor = gpTweakGuiColors->GetDamageAmbientColor();
  float colorGain =
      x3f4_damageFilterAmt > peak
          ? (x3f0_damageFilterAmtInit - x3f4_damageFilterAmt) / (x3f0_damageFilterAmtInit - peak)
          : x3f4_damageFilterAmt / peak;
  colorGain *= x3f8_damageFilterAmtGain;
  colorGain = CMath::Clamp(0.f, colorGain, 1.f);
  const CColor color0 = ambientColor.WithAlphaModulatedBy(colorGain);
  const CColor color1 =
      gpTweakGuiColors->GetDamageAmbientPulseColor().WithAlphaModulatedBy(x3ec_damageLightPulser);
  CColor filterColor = CColor::Add(color0, color1);
  if (filterColor.GetAlphau8()) {
    if (mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Unmorphed) {
      filterColor = filterColor.WithAlphaModulatedBy(0.75f);
    }
    x3a8_camFilter.SetFilter(CCameraFilterPass::kFT_Add, CCameraFilterPass::kFS_Fullscreen, 0.f,
                             filterColor, kInvalidAssetId);
  } else {
    x3a8_camFilter.DisableFilter(0.f);
  }
  if (x3a4_damageSfx) {
    CSfxManager::UpdateEmitter(x3a4_damageSfx, mgr.GetPlayer()->GetTranslation(),
                               mgr.GetPlayer()->GetTransform().GetForward(), 127);
  }
  if (x400_hudDamagePracticals > 0.f) {
    x400_hudDamagePracticals = rstl::max_val(0.f, x400_hudDamagePracticals - dt);
    float practicals = x400_hudDamagePracticals / x3fc_hudDamagePracticalsInit;
    if (x28c_energyIntf.get()) {
      x28c_energyIntf->SetFlashMagnitude(practicals);
    }
    practicals = rstl::min_val(1.f, practicals * x404_hudDamagePracticalsGain);
    x2a0_helmetIntf->AddHelmetLightValue(practicals);
    if (x29c_decoIntf.get()) {
      x29c_decoIntf->SetFrameColorValue(practicals);
      if (practicals > 0.f) {
        const CColor practicalColor(practicals, practicals, practicals, practicals);
        x3d4_damageLight->SetColor(
            CColor::Modulate(gpTweakGuiColors->GetHudDamageLightColor(), practicalColor));
        x3d4_damageLight->SetIsVisible(true);
      } else {
        x3d4_damageLight->SetIsVisible(false);
      }
    }
  }
  bool updateTransform = false;
  if (x414_decoShakeTranslateAmt > 0.f) {
    const float deceleration = (60.f * dt) * gpTweakGui->GetDecoDamageShakeDeceleration();
    x418_decoShakeTranslateAmtVel -= deceleration;
    x414_decoShakeTranslateAmt =
        rstl::max_val(0.f, x414_decoShakeTranslateAmt + x418_decoShakeTranslateAmtVel);
    updateTransform = true;
  }
  if (x460_decoShakeAmt > 0.f) {
    x460_decoShakeAmt = rstl::max_val(0.f, x460_decoShakeAmt - dt);
    const float rotateFraction = x460_decoShakeAmt / x45c_decoShakeAmtInit;
    const float rotate = rstl::min_val(rotateFraction * x464_decoShakeAmtGain,
                                       gpTweakGui->GetMaxDecoDamageShakeRotate());
    const int xRandom = rand();
    const float xAngle = (2.f * M_PIF / 10.f) * ((xRandom / float(RAND_MAX)) * rotate);
    const CQuaternion xRotation = CQuaternion::XRotation(CRelAngle::FromRadians(xAngle));
    const int zRandom = rand();
    const float zAngle = (2.f * M_PIF / 10.f) * ((zRandom / float(RAND_MAX)) * rotate);
    const CQuaternion zRotation = CQuaternion::ZRotation(CRelAngle::FromRadians(zAngle));
    x44c_hudLagShakeRot = xRotation * zRotation;
    CVector3f vectors[3] = {CVector3f::Right(), CVector3f::Forward(), CVector3f::Up()};
    for (int i = 0; i < 4; ++i) {
      const int random = rand();
      const int component = rand() % 9;
      const float amount = (random / float(RAND_MAX) - 0.5f) * rotate;
      vectors[component % 3][component / 3] += amount;
    }
    x428_decoShakeRotate = CMatrix3f(vectors[0], vectors[1], vectors[2]);
    updateTransform = true;
  }
  if (updateTransform) {
    x41c_decoShakeTranslate =
        rstl::min_val(x414_decoShakeTranslateAmt, gpTweakGui->GetMaxDecoDamageShakeTranslate()) *
        x408_damagerToPlayerNorm;
    if (x29c_decoIntf.get()) {
      x29c_decoIntf->SetDamageTransform(x428_decoShakeRotate,
                                        gpTweakGui->GetHudDecoShakeTranslateGain() *
                                            x41c_decoShakeTranslate);
    }
  }
}

void CSamusHud::UpdateStateTransition(float dt, const CStateManager& mgr) {
  if (x2cc_preLoadCountdown == 0) {
    const EHudState desired = GetDesiredHudState(mgr);
    if (desired != x2c0_setState) {
      x2c0_setState = desired;
      bool ballTransition = desired == kHS_Ball || x2bc_nextState == kHS_Ball;
      x2c8_transT = ballTransition ? FLT_EPSILON : x2c8_transT;
      x2c4_activeTransState = kTS_Countdown;
    }
  }
  switch (x2c4_activeTransState) {
  case kTS_Countdown:
    if (x2cc_preLoadCountdown == 0) {
      x2c8_transT = rstl::max_val(x2c8_transT - 5.f * dt, 0.f);
      if (x2c8_transT == 0.f) {
        x2cc_preLoadCountdown = 2;
        x288_loadedSelectedHud = nullptr;
      }
    } else {
      --x2cc_preLoadCountdown;
      if (x2cc_preLoadCountdown == 0) {
        UninitializeFrameGlueMutable();
        x278_selectedHud = rstl::optional_object_null();
        switch (x2c0_setState) {
        case kHS_Thermal:
          x278_selectedHud = TCachedToken< CGuiFrame >(gpSimplePool->GetObj(sThermalHudName));
          x278_selectedHud->Lock();
          break;
        case kHS_Combat:
          x278_selectedHud = TCachedToken< CGuiFrame >(gpSimplePool->GetObj(sCombatHudName));
          x278_selectedHud->Lock();
          break;
        case kHS_Scan:
          x278_selectedHud = TCachedToken< CGuiFrame >(gpSimplePool->GetObj(sScanHudName));
          x278_selectedHud->Lock();
          break;
        case kHS_XRay:
          x278_selectedHud = TCachedToken< CGuiFrame >(gpSimplePool->GetObj(sXRayHudName));
          x278_selectedHud->Lock();
          break;
        case kHS_Ball:
          x278_selectedHud = TCachedToken< CGuiFrame >(gpSimplePool->GetObj(sBallHudName));
          x278_selectedHud->Lock();
          break;
        default:
          x278_selectedHud = rstl::optional_object_null();
          break;
        }
        x2c4_activeTransState = kTS_Loading;
      }
    }
    if (x2c4_activeTransState != kTS_Loading) {
      return;
    }
  case kTS_Loading:
    if (x278_selectedHud.valid()) {
      if (x278_selectedHud->TryCache() && x278_selectedHud->GetObject()->GetIsFinishedLoading()) {
        x288_loadedSelectedHud = x278_selectedHud->GetObject();
        x2b8_curState = x2bc_nextState;
        x2bc_nextState = x2c0_setState;
        InitializeFrameGlueMutable(mgr);
        x2c4_activeTransState = kTS_Transitioning;
        UpdateCameraDebugSettings();
      }
    } else {
      x2b8_curState = x2bc_nextState;
      x2bc_nextState = x2c0_setState;
      x2c4_activeTransState = kTS_NotTransitioning;
    }
    break;
  case kTS_Transitioning:
    x2c8_transT = rstl::min_val(5.f * dt + x2c8_transT, 1.f);
    if (x2c8_transT == 1.f) {
      x2c4_activeTransState = kTS_NotTransitioning;
    }
    break;
  case kTS_NotTransitioning:
    break;
  }
}

void CSamusHud::Update(float dt, const CStateManager& mgr, uint helmetVis, bool hudVis,
                       bool targetingManager) {
  const bool updateHud = helmetVis != 0;
  UpdateStateTransition(dt, mgr);
  bool firstPerson = mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed &&
                     !mgr.GetCameraManager()->IsInCinematicCamera();
  if (firstPerson != x2e0_26_latestFirstPerson) {
    if (firstPerson) {
      EnterFirstPerson(mgr);
    } else {
      LeaveFirstPerson(mgr);
    }
    x2e0_26_latestFirstPerson = firstPerson;
  }
  const bool notMorphed = mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Morphed;
  const CPlayer::EPlayerMorphBallState ballState = mgr.GetPlayer()->GetMorphballTransitionState();
  float morphFactor = 0.f;
  switch (ballState) {
  case CPlayer::kMS_Morphed:
    morphFactor = 1.f;
    break;
  case CPlayer::kMS_Unmorphed:
    morphFactor = 0.f;
    break;
  default:
    break;
  case CPlayer::kMS_Morphing:
    morphFactor = mgr.GetPlayer()->GetMorphBallTransitionFactor();
    break;
  case CPlayer::kMS_Unmorphing:
    morphFactor = 1.f - mgr.GetPlayer()->GetMorphBallTransitionFactor();
    break;
  }
  x504_viewportScaleY = 1.f - morphFactor * gpTweakGui->GetBallViewportYReduction();
  if (!x2b0_ballIntf.null()) {
    x2b0_ballIntf->SetBallModeFactor(morphFactor);
  }
  bool helmetVisible = false;
  bool glowVisible = false;
  bool decoVisible = false;
  if (notMorphed) {
    switch (static_cast< int >(helmetVis)) {
    case 5:
      helmetVisible = true;
      break;
    case 4:
      glowVisible = true;
    case 3:
      helmetVisible = true;
    case 2:
      decoVisible = true;
      break;
    case 0:
    case 1:
    default:
      break;
    }
  }
  if (!x29c_decoIntf.null()) {
    x29c_decoIntf->SetIsVisibleDebug(decoVisible);
  }
  if (!x2a0_helmetIntf.null()) {
    x2a0_helmetIntf->SetIsVisibleDebug(helmetVisible, glowVisible);
  }
  x590_base_Model_AutoMapper->SetIsVisible(false);
  UpdateEnergyLow(dt, mgr);
  for (int i = 0; i < 15; ++i) {
    x7ac_profileInfo[i].x0_updateUsec = 0;
  }
  if (!x2ac_radarIntf.null()) {
    x2ac_radarIntf->Update(dt, mgr);
  }
  UpdateHudLag(dt, mgr);
  UpdateHudDynamicLights(dt, mgr);
  if (targetingManager) {
    x8_targetingMgr.Update(dt, mgr);
  }
  UpdateHudDamage(dt, mgr, helmetVis);
  UpdateStaticInterference(dt, mgr);
  if (updateHud) {
    if (x2bc_nextState != kHS_None) {
      UpdateEnergy(dt, mgr, false);
      UpdateFreeLook(dt, mgr);
    }
    if (x2bc_nextState == kHS_Ball) {
      UpdateBallMode(mgr, false);
    } else {
      bool firstPersonHud = x2bc_nextState >= kHS_Combat && x2bc_nextState <= kHS_Scan;
      if (firstPersonHud) {
        UpdateThreatAssessment(dt, mgr);
        UpdateMissile(dt, mgr, false);
        UpdateVideoBands(dt, mgr);
      }
    }
    UpdateVisorAndBeamMenus(dt, mgr);
    const CPlayer& damagedPlayer = *mgr.GetPlayer();
    if (damagedPlayer.WasDamaged() && mgr.GetGameState() == CStateManager::kGS_Running) {
      const CVector3f position = damagedPlayer.GetDamageLocationWR();
      const float damage = damagedPlayer.GetDamageAmount();
      const float previousDamage = damagedPlayer.GetPrevDamageAmount();
      ShowDamage(position, damage, previousDamage, mgr);
    }
  }
  const float oldPulse = x584_abuttonPulse;
  if (x554_hudMemoIdx == 0) {
    x584_abuttonPulse += 2.f * dt;
    if (x584_abuttonPulse > 1.f) {
      x584_abuttonPulse -= 2.f;
    }
  }
  const float pulseAlpha = CMath::AbsF(x584_abuttonPulse);
  x5a0_base_model_abutton->SetColor(CColor::White().WithAlphaOf(pulseAlpha));
  bool pulseSound = !mgr.GetCameraManager()->IsInCinematicCamera() && oldPulse < 0.f &&
                    x584_abuttonPulse >= 0.f && x598_base_basewidget_message->GetIsVisible() &&
                    (x558_messageTextTime == 0.f || x558_messageTextTime >= 1.f);
  if (pulseSound) {
    CSfxManager::SfxStart(SFXui_x_hintflas_00, 127, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
  const float allTextAlpha = x29c_decoIntf.null() ? 1.f : x29c_decoIntf->GetMessageTextAlpha();
  float messageTextAlpha = 1.f;
  if (x558_messageTextTime > 0.f) {
    messageTextAlpha = rstl::min_val(1.f, x558_messageTextTime);
  } else if (!x59c_base_textpane_message->GetIsVisible() &&
             !x598_base_basewidget_message->GetIsVisible()) {
    messageTextAlpha = 0.f;
  }
  if (!x2b4_bossEnergyIntf.null()) {
    x2b4_bossEnergyIntf->SetAlpha(1.f - messageTextAlpha);
  }
  if (!x550_hudMemoString.null() && x550_hudMemoString->IsLoaded()) {
    SetMessage(rstl::wstring((**x550_hudMemoString)->GetString(x554_hudMemoIdx)),
               x548_hudMemoParms);
    x550_hudMemoString = nullptr;
  }
  if (x558_messageTextTime > 0.f) {
    x558_messageTextTime = rstl::max_val(0.f, x558_messageTextTime - dt);
    if (x558_messageTextTime == 0.f) {
      x59c_base_textpane_message->TextSupport().SetTypeWriteEffectOptions(false, 0.f, 1.f);
      x598_base_basewidget_message->SetVisibility(false, kTM_Children);
    }
  }
  float textScale = 1.f;
  const float messageAlpha = rstl::min_val(messageTextAlpha, allTextAlpha);
  CGuiWidget* messageWidget = x598_base_basewidget_message->GetIsVisible()
                                  ? x598_base_basewidget_message
                                  : x59c_base_textpane_message;
  messageWidget->SetColor(CColor::White().WithAlphaOf(messageAlpha));
  if (messageWidget == x598_base_basewidget_message) {
    if (x558_messageTextTime > 0.f) {
      x560_messageTextScale = rstl::min_val(1.f, x558_messageTextTime);
    } else {
      x560_messageTextScale = rstl::min_val(1.f, x560_messageTextScale + dt);
    }
    const float t = rstl::max_val(0.f, (x560_messageTextScale - 0.75f) / 0.25f);
    if (t != 1.f) {
      if (t < 0.7f) {
        textScale = t / 0.7f;
      } else if (t < 0.85f) {
        textScale = 0.9f + (1.f - 0.9f) * (1.f - (t - 0.7f) / 0.15f);
      } else {
        textScale = 0.9f + (1.f - 0.9f) * (((t - 0.7f) - 0.15f) / 0.3f);
      }
    }
    x598_base_basewidget_message->SetO2PTransform(x598_base_basewidget_message->GetIdleXform() *
                                                  CTransform4f::Scale(textScale, 1.f, 1.f));
  }
  const float printed = x59c_base_textpane_message->TextSupport().GetNumCharactersPrinted();
  const float charsPerSfx = gpTweakGui->GetWorldTransManagerCharsPerSfx();
  if (printed >= x55c_lastSfxChars + charsPerSfx) {
    x55c_lastSfxChars += charsPerSfx;
    if (!x598_base_basewidget_message->GetIsVisible() || textScale == 1.f) {
      CSfxManager::SfxStart(SFXui_x_type_00, 127, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    }
  }
  const float escapeTime = mgr.GetEscapeSequenceTimer();
  if (escapeTime > 0.f) {
    const int seconds = static_cast< int >(escapeTime);
    const int hundredths = static_cast< int >(100.f * escapeTime);
    char text[16];
#if NONMATCHING
    snprintf(text, sizeof(text), "%02d:%02d:%02d", seconds / 60, seconds % 60, hundredths % 100);
#else
    sprintf(text, "%02d:%02d:%02d", seconds / 60, seconds % 60, hundredths % 100);
#endif
    x594_base_textpane_counter->TextSupport().SetText(rstl::string(text), false);
    x594_base_textpane_counter->SetIsVisible(true);
    const float counterAlpha =
        rstl::min_val(allTextAlpha, 1.f - rstl::min_val(1.f, x558_messageTextTime));
    x594_base_textpane_counter->SetColor(
        CColor::White().WithAlphaOf(CMath::Clamp(0.f, counterAlpha, 1.f)));
  } else {
    x594_base_textpane_counter->SetIsVisible(false);
  }
  x274_loadedFrmeBaseHud->Update(dt);
  if (x288_loadedSelectedHud != nullptr) {
    x288_loadedSelectedHud->Update(dt);
  }
  if (!x2b4_bossEnergyIntf.null()) {
    x2b4_bossEnergyIntf->Update(dt);
  }
  if (!x28c_energyIntf.null()) {
    x28c_energyIntf->Update(dt, x580_energyLowPulse);
  }
  if (!x290_threatIntf.null()) {
    x290_threatIntf->Update(dt);
  }
  if (!x294_missileIntf.null()) {
    x294_missileIntf->Update(dt, mgr);
  }
  if (!x298_freeLookIntf.null()) {
    x298_freeLookIntf->Update(dt);
  }
  if (!x2a0_helmetIntf.null()) {
    x2a0_helmetIntf->Update(dt);
  }
  const CPlayerState& state = *mgr.GetPlayerState();
  const CPlayerState::EPlayerVisor visor = state.GetCurrentVisor();
  const CPlayerState::EPlayerVisor nextVisor = state.GetTransitioningVisor();
  if (mgr.GetPlayer()->GetPlayerScanState() == CPlayer::kSS_NotScanning) {
    x2f0_visorBeamMenuAlpha = rstl::min_val(1.f, x2f0_visorBeamMenuAlpha + 2.f * dt);
  } else {
    x2f0_visorBeamMenuAlpha = rstl::max_val(0.f, x2f0_visorBeamMenuAlpha - 2.f * dt);
  }
  float transitionFactor = 0.f;
  if (visor != CPlayerState::kPV_Scan) {
    if (nextVisor == CPlayerState::kPV_Scan) {
      transitionFactor = state.GetVisorTransitionFactor();
    } else {
      transitionFactor = 1.f;
    }
  }
  if (!x2a4_visorMenu.null()) {
    x2a4_visorMenu->UpdateHudAlpha(gpGameState->GameOptions().GetSwapBeamControls()
                                       ? transitionFactor
                                       : x2f0_visorBeamMenuAlpha);
    x2a4_visorMenu->Update(dt, false);
  }
  if (!x2a8_beamMenu.null()) {
    x2a8_beamMenu->UpdateHudAlpha(gpGameState->GameOptions().GetSwapBeamControls()
                                      ? x2f0_visorBeamMenuAlpha
                                      : transitionFactor);
    x2a8_beamMenu->Update(dt, false);
  }
  UpdateCameraDebugSettings();
  if (!x29c_decoIntf.null()) {
    x29c_decoIntf->Update(dt, mgr);
  }
}

void CSamusHud::Touch() const {
  if (x264_loadedFrmeHelmet != nullptr) {
    x264_loadedFrmeHelmet->Touch();
  }
  if (x274_loadedFrmeBaseHud != nullptr) {
    x274_loadedFrmeBaseHud->Touch();
  }
  if (x288_loadedSelectedHud != nullptr) {
    x288_loadedSelectedHud->Touch();
  }
}

void CSamusHud::DrawAttachedEnemyEffect(const CStateManager& mgr) const {
  const float drainTime = mgr.GetPlayer()->GetPlayerEnergyDrain().GetEnergyDrainTime();
  if (drainTime > 0.f) {
    const float period = gpTweakGui->GetEnergyDrainModPeriod();
    const float phaseOffset = -0.25f * period;
    const CColor& filterColor = gpTweakGuiColors->GetEnergyDrainFilterColor();
    float alpha;
    if (gpTweakGui->GetEnergyDrainSinusoidalPulse()) {
      alpha = 0.5f * (1.f + CMath::FastSinR(phaseOffset + 2.f * M_PIF * drainTime / period));
    } else {
      float phase = CMath::AbsF(CMath::ModF(drainTime, period));
      const float halfPeriod = 0.5f * period;
      if (phase < halfPeriod) {
        phase /= halfPeriod;
      } else {
        phase = (period - phase) / halfPeriod;
      }
      alpha = phase;
    }
    const CColor color = filterColor.WithAlphaModulatedBy(alpha);
    CCameraFilterPass::DrawFilter(gpTweakGui->GetEnergyDrainFilterAdditive()
                                      ? CCameraFilterPass::kFT_Add
                                      : CCameraFilterPass::kFT_Blend,
                                  CCameraFilterPass::kFS_Fullscreen, color, nullptr, 1.f);
  }
}

rstl::reserved_vector< bool, 4 > CSamusHud::BuildPlayerHasVisors(const CStateManager& mgr) const {
  const CPlayerState& state = *mgr.GetPlayerState();
  rstl::reserved_vector< bool, 4 > ret;
  ret.push_back(state.HasPowerUp(CPlayerState::kIT_CombatVisor));
  ret.push_back(state.HasPowerUp(CPlayerState::kIT_XRayVisor));
  ret.push_back(state.HasPowerUp(CPlayerState::kIT_ScanVisor));
  ret.push_back(state.HasPowerUp(CPlayerState::kIT_ThermalVisor));
  return ret;
}

rstl::reserved_vector< bool, 4 > CSamusHud::BuildPlayerHasBeams(const CStateManager& mgr) const {
  const CPlayerState& state = *mgr.GetPlayerState();
  rstl::reserved_vector< bool, 4 > ret;
  ret.push_back(state.HasPowerUp(CPlayerState::kIT_PowerBeam));
  ret.push_back(state.HasPowerUp(CPlayerState::kIT_IceBeam));
  ret.push_back(state.HasPowerUp(CPlayerState::kIT_WaveBeam));
  ret.push_back(state.HasPowerUp(CPlayerState::kIT_PlasmaBeam));
  return ret;
}

void CSamusHud::EnterFirstPerson(const CStateManager& mgr) {
  CSfxManager::SfxVolume(x508_staticSfxHi, 127);
  CSfxManager::SfxVolume(x50c_staticSfxLo, 127);
}

void CSamusHud::LeaveFirstPerson(const CStateManager& mgr) {
  CSfxManager::SfxVolume(x508_staticSfxHi, 0);
  CSfxManager::SfxVolume(x50c_staticSfxLo, 0);
}

void CSamusHud::DrawHelmet(const CStateManager& mgr, float camYOff) {
  bool unmorphed = mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed;
  if (x264_loadedFrmeHelmet != nullptr && unmorphed && x2bc_nextState != kHS_Ball) {
    bool leavingBall = x2c4_activeTransState == kTS_Transitioning && x2b8_curState == kHS_Ball;
    const float alpha = leavingBall ? x2c8_transT : 1.f;
    const CGuiWidgetDrawParms parms(alpha, CVector3f(0.f, 15.f * camYOff, 0.f));
    x264_loadedFrmeHelmet->Draw(parms);
  }
}

void CSamusHud::Draw(const CStateManager& mgr, float alpha, uint helmetVis, bool hudVis,
                     bool targetingManager) const {
  if (x2bc_nextState == kHS_None) {
    return;
  }
  const CPlayer::EPlayerMorphBallState ballState = mgr.GetPlayer()->GetMorphballTransitionState();
  x3a8_camFilter.Draw();
  if (ballState == CPlayer::kMS_Unmorphed) {
    DrawAttachedEnemyEffect(mgr);
    x51c_camFilter2.Draw();
    if (targetingManager) {
      x8_targetingMgr.Draw(mgr, false);
    }
  }
  CStopwatch timer;
  u64 currentTime = 0;
  for (int i = 0; i < 15; ++i) {
    x7ac_profileInfo[i].x8_drawUsec = 0;
  }
  if (helmetVis != 0) {
    const u64 previousTime = currentTime;
    currentTime = timer.GetElapsedMicros();
    x7ac_profileInfo[11].x8_drawUsec = currentTime - previousTime;
    if (helmetVis < 5) {
      if (alpha < 1.f) {
        CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_NoColor,
                                      CCameraFilterPass::kFS_CookieCutterDepthRandomStatic,
                                      CColor::White(), nullptr, 1.f - alpha);
      }
      if (x288_loadedSelectedHud != nullptr) {
        if (mgr.GetPlayer()->GetDeathTime() > 0.f) {
          if (mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Unmorphed) {
            const CGuiWidgetDrawParms parms(
                x2c8_transT *
                    CMath::Clamp(0.f, 1.f - mgr.GetPlayer()->GetDeathTime() / gkBallDeathTime, 1.f),
                CVector3f::Zero());
            x288_loadedSelectedHud->Draw(parms);
          } else {
            const CGuiWidgetDrawParms parms(x2c8_transT, CVector3f::Zero());
            x288_loadedSelectedHud->Draw(parms);
          }
        } else {
          const CGuiWidgetDrawParms parms(x2c8_transT, CVector3f::Zero());
          x288_loadedSelectedHud->Draw(parms);
        }
        const u64 previousTime = currentTime;
        currentTime = timer.GetElapsedMicros();
        x7ac_profileInfo[5].x8_drawUsec = currentTime - previousTime;
      }
      if (x274_loadedFrmeBaseHud != nullptr) {
        x274_loadedFrmeBaseHud->Draw(CGuiWidgetDrawParms::sDefaultDrawParms);
        const u64 previousTime = currentTime;
        currentTime = timer.GetElapsedMicros();
        x7ac_profileInfo[6].x8_drawUsec = currentTime - previousTime;
      }
    }
  }
  if (!x29c_decoIntf.null() && x2cc_preLoadCountdown == 0) {
    x29c_decoIntf->Draw();
    const u64 previousTime = currentTime;
    currentTime = timer.GetElapsedMicros();
    x7ac_profileInfo[12].x8_drawUsec = currentTime - previousTime;
  }
  bool firstPerson = x2bc_nextState >= kHS_Combat && x2bc_nextState <= kHS_Scan;
  if (firstPerson) {
    if (hudVis && helmetVis != 0 && helmetVis < 5) {
      float t = mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Combat
                    ? mgr.GetPlayerState()->GetVisorTransitionFactor()
                    : 0.f;
      x2ac_radarIntf->Draw(mgr, alpha * t);
      const u64 previousTime = currentTime;
      currentTime = timer.GetElapsedMicros();
      x7ac_profileInfo[0].x8_drawUsec = currentTime - previousTime;
    }
    gpRender->SetDepthReadWrite(true, true);
  }
  x7ac_profileInfo[14].x8_drawUsec = timer.GetElapsedMicros();
}

void CSamusHud::ProcessControllerInput(const CFinalInput& input) {
  if (!x29c_decoIntf.null()) {
    x29c_decoIntf->ProcessInput(input);
  }
}

const CTargetingManager& CSamusHud::GetTargetingManager() const { return x8_targetingMgr; }

EHudState CSamusHud::GetDesiredHudState(const CStateManager& mgr) const {
  const CPlayer::EPlayerMorphBallState state = mgr.GetPlayer()->GetMorphballTransitionState();
  if (state == CPlayer::kMS_Morphed || state == CPlayer::kMS_Morphing ||
      state == CPlayer::kMS_Unmorphing) {
    return kHS_Ball;
  }
  switch (mgr.GetPlayerState()->GetTransitioningVisor()) {
  case CPlayerState::kPV_Combat:
    return kHS_Combat;
  case CPlayerState::kPV_XRay:
    return kHS_XRay;
  case CPlayerState::kPV_Scan:
    return kHS_Scan;
  case CPlayerState::kPV_Thermal:
    return kHS_Thermal;
  default:
    return kHS_None;
  }
}

int CSamusHud::GetRelativeDirection(const CVector3f& position, const CStateManager& mgr) const {
  const CFirstPersonCamera* camera =
      TCastToConstPtr< CFirstPersonCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr));
  if (camera == nullptr) {
    return 0;
  }
  const CVector3f localPosition = camera->GetTransform().TransposeMultiply(position);
  bool samePosition = localPosition == position;
  if (samePosition) {
    return 0;
  }
  const float halfFov = 0.5f * camera->GetFov();
  const float horizontalHalfFov = halfFov * camera->GetAspectRatio();
  const float verticalCos = cosf(2.f * M_PIF * (0.0027777778f * halfFov));
  const float horizontalCos = cosf(2.f * M_PIF * (0.0027777778f * horizontalHalfFov));
  const CVector2f xy = CVector2f(localPosition.GetX(), localPosition.GetY()).AsNormalized();
  const CVector2f xyForward(0.f, 1.f);
  const CVector2f yz = CVector2f(localPosition.GetY(), localPosition.GetZ()).AsNormalized();
  const CVector2f yzForward(1.f, 0.f);
  const float frontHorizontal = CVector2f::Dot(xy, xyForward);
  const float frontVertical = CVector2f::Dot(yz, yzForward);
  if (frontHorizontal > horizontalCos && frontVertical > verticalCos) {
    return 0;
  }
  const float backHorizontal = CVector2f::Dot(xy, xyForward * -1.f);
  const float backVertical = CVector2f::Dot(yz, yzForward * -1.f);
  if (backHorizontal > horizontalCos && backVertical > verticalCos) {
    return 1;
  }
  const CVector3f direction = localPosition.AsNormalized();
  CVector3f testDirection(0.f, 0.f, 1.f);
  float bestDot = -1.f;
  int result = -1;
  const CQuaternion rotation = CQuaternion::YRotation(CRelAngle(2.f * M_PIF / 8.f));
  for (int i = 0; i < 8; ++i) {
    const int directionIndex = i + 2;
    const float dot = CVector3f::Dot(direction, testDirection);
    if (dot > bestDot) {
      bestDot = dot;
      result = directionIndex;
    }
    testDirection = rotation.Transform(testDirection);
  }
  return result;
}

void CSamusHud::ShowDamage(CVector3f position, float damage, float previousDamage,
                           const CStateManager& mgr) {
  const int direction = GetRelativeDirection(position, mgr);
  const CFirstPersonCamera* const camera =
      TCastToConstPtr< CFirstPersonCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr));
  const float hudDamagePracticalsGainLinear = gpTweakGui->GetHudDamagePracticalsGainLinear();
  x404_hudDamagePracticalsGain =
      hudDamagePracticalsGainLinear * damage + gpTweakGui->GetHudDamagePracticalsGainConstant();
  const float hudDamagePracticalsInitLinear = gpTweakGui->GetHudDamagePracticalsInitLinear();
  x3fc_hudDamagePracticalsInit =
      rstl::max_val(kDamagePracticalsEpsilon, hudDamagePracticalsInitLinear * damage +
                                                  gpTweakGui->GetHudDamagePracticalsInitConstant());
  x400_hudDamagePracticals = x3fc_hudDamagePracticalsInit;
  if (x3d4_damageLight != nullptr) {
    x3d4_damageLight->SetO2PTransform(x3d8_lightTransforms[direction]);
  }
  const float hudDamageFilterGainLinear = gpTweakGui->GetHudDamageFilterGainLinear();
  x3f8_damageFilterAmtGain =
      hudDamageFilterGainLinear * damage + gpTweakGui->GetHudDamageFilterGainConstant();
  const float hudDamageFilterInitLinear = gpTweakGui->GetHudDamageFilterInitLinear();
  x3f0_damageFilterAmtInit =
      hudDamageFilterInitLinear * damage + gpTweakGui->GetHudDamageFilterInitConstant();
  x3f4_damageFilterAmt = x3f0_damageFilterAmtInit;
  if (!x3a4_damageSfx) {
    x3a4_damageSfx =
        CSfxManager::AddEmitter(SFXsam_r_damage_lp_00, mgr.GetPlayer()->GetTranslation(),
                                mgr.GetPlayer()->GetTransform().GetForward(), false, true,
                                CSfxManager::kMaxPriority, CSfxManager::kAllAreas);
  }
  if (camera != nullptr) {
    const CVector3f cameraToDamage = camera->GetTransform().GetQuickInverse() * position;
    x418_decoShakeTranslateAmtVel =
        gpTweakGui->GetHudDecoShakeTranslateVelConstant() +
        gpTweakGui->GetHudDecoShakeTranslateVelLinear() * previousDamage;
    x414_decoShakeTranslateAmt = x418_decoShakeTranslateAmtVel;
    x408_damagerToPlayerNorm = -1.f * cameraToDamage.AsNormalized();
    const float decoShakeGainLinear = gpTweakGui->GetDecoShakeGainLinear();
    x464_decoShakeAmtGain =
        decoShakeGainLinear * previousDamage + gpTweakGui->GetDecoShakeGainConstant();
    const float decoShakeInitLinear = gpTweakGui->GetDecoShakeInitLinear();
    x45c_decoShakeAmtInit =
        decoShakeInitLinear * previousDamage + gpTweakGui->GetDecoShakeInitConstant();
    x460_decoShakeAmt = x45c_decoShakeAmtInit;
  }
}

void CSamusHud::UpdateHudLag(float dt, const CStateManager& mgr) {
  if (!x29c_decoIntf.null()) {
    const float yaw = mgr.GetPlayer()->GetYaw();
    x29c_decoIntf->SetDecoRotation(yaw);
  }
  if (!gpGameState->GameOptions().GetHUDLag()) {
    if (!x2a0_helmetIntf.null()) {
      x2a0_helmetIntf->SetHudLagRotation(CMatrix3f::Identity());
      x2a0_helmetIntf->SetHudLagOffset(CVector3f::Zero());
    }
    if (!x29c_decoIntf.null()) {
      x29c_decoIntf->SetReticuleTransform(CMatrix3f::Identity());
      x29c_decoIntf->SetHudRotation(CQuaternion::NoRotation());
      x29c_decoIntf->SetHudOffset(CVector3f::Zero());
    }
    x588_base_basewidget_pivot->SetIdleXform(
        CTransform4f(CMatrix3f::Identity(), x588_base_basewidget_pivot->GetWorldPosition()));
    x274_loadedFrmeBaseHud->GetFrameCamera()->SetO2WTransform(BuildFinalCameraTransform(
        CQuaternion::NoRotation(), x304_basewidgetIdlePos, x310_cameraPos));
    x8_targetingMgr.CompoundTargetReticle().SetLeadingOrientation(CQuaternion::NoRotation());
  } else {
    CUnitVector3f cameraDirection(x2f8_fpCamDir, CUnitVector3f::kN_No);
    if (const CFirstPersonCamera* camera =
            TCastToConstPtr< CFirstPersonCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr))) {
      const CMatrix3f rotation = camera->GetTransform().BuildMatrix3f();
      const CVector3f direction = rotation.GetColumn(1);
      cameraDirection.SetY(direction.GetY());
      cameraDirection.SetX(direction.GetX());
      cameraDirection.SetZ(direction.GetZ());
    }
    ApplyClassicLag(cameraDirection, x31c_hudLag, mgr, dt, false);
    ApplyClassicLag(cameraDirection, x32c_invHudLag, mgr, dt, true);
    CQuaternion rotation =
        CQuaternion::LookAt(CUnitVector3f(x2f8_fpCamDir), cameraDirection, CRelAngle(2.f * M_PIF));
    rotation *= rotation;
    rotation *= rotation;
    x8_targetingMgr.CompoundTargetReticle().SetLeadingOrientation(rotation);
    const CVector3f bob = mgr.GetPlayer()->GetCameraBobObject()->GetHelmetBobTranslation();
    const CVector3f lagOffset = CVector3f(0.f, 0.f, bob.GetZ()) +
                                gpTweakGui->GetHudLagOffsetScale() * x41c_decoShakeTranslate;
    const CQuaternion lagRotation = x44c_hudLagShakeRot * x31c_hudLag;
    if (!x2a0_helmetIntf.null()) {
      x2a0_helmetIntf->SetHudLagRotation(lagRotation.BuildTransform());
      x2a0_helmetIntf->SetHudLagOffset(lagOffset);
    }
    if (!x29c_decoIntf.null()) {
      x29c_decoIntf->SetReticuleTransform(x32c_invHudLag.BuildTransform());
      x29c_decoIntf->SetHudRotation(lagRotation);
      x29c_decoIntf->SetHudOffset(lagOffset);
    }
    x274_loadedFrmeBaseHud->GetFrameCamera()->SetO2WTransform(
        BuildFinalCameraTransform(lagRotation, x304_basewidgetIdlePos + lagOffset, x310_cameraPos));
    x2f8_fpCamDir = cameraDirection;
  }
}

void CSamusHud::ApplyClassicLag(const CUnitVector3f& lookDir, CQuaternion& rotation,
                                const CStateManager& mgr, float dt, bool invert) {
  const CQuaternion lookRotation = CQuaternion::LookAt(
      CUnitVector3f(lookDir), CUnitVector3f(CVector3f::Forward()), CRelAngle(2.f * M_PIF));
  CQuaternion targetRotation =
      invert ? CQuaternion::LookAt(CUnitVector3f(lookRotation.Transform(x2f8_fpCamDir)),
                                   CUnitVector3f(CVector3f::Forward()), CRelAngle(2.f * M_PIF))
             : CQuaternion::LookAt(CUnitVector3f(CVector3f::Forward()),
                                   CUnitVector3f(lookRotation.Transform(x2f8_fpCamDir)),
                                   CRelAngle(2.f * M_PIF));
  targetRotation *= targetRotation;
  const CVector3f targetDirection = targetRotation.BuildTransform().GetColumn(1);
  const CVector3f currentDirection = rotation.BuildTransform().GetColumn(1);
  float angularStep = 0.5f * (dt * gpTweakPlayer->GetFreeLookSpeed());
  float dot = CVector3f::Dot(currentDirection, targetDirection);
  dot = CMath::Limit(dot, 1.f);
  const float angle = acosf(dot);
  const float step = angle > 0.f ? angularStep / angle : 0.f;
  const float t = CMath::Clamp(0.f, (18.f * dt) * step, 1.f);
  targetRotation = CQuaternion::SlerpLocal(rotation, targetRotation, t);
  rotation = targetRotation;
}

void CSamusHud::UpdateCameraDebugSettings() {
  const float fov = x5ec_camFovTweaks[gpTweakGui->GetHudCamFovTweak()];
  const float y = x62c_camYTweaks[gpTweakGui->GetHudCamYTweak()];
  const float z = x72c_camZTweaks[gpTweakGui->GetHudCamZTweak()];
  if (!x2a0_helmetIntf.null()) {
    x2a0_helmetIntf->UpdateCameraDebugSettings(fov, y, z);
  }
  if (!x29c_decoIntf.null()) {
    x29c_decoIntf->UpdateCameraDebugSettings(fov, y, z);
  }
  CGuiCamera* camera = x274_loadedFrmeBaseHud->GetFrameCamera();
  CGuiCamera::UCameraParms parms = camera->GetParms();
  parms.perspective.fov = fov;
  camera->SetParms(parms);
  x310_cameraPos = CVector3f(0.f, y, z);
}

void CSamusHud::SetMessage(const rstl::wstring& text, const CHUDMemoParms& info) {
  bool visible = x598_base_basewidget_message->GetIsVisible();
  if (!visible || info.IsHintMemo()) {
    if (info.IsFadeOutOnly()) {
      x558_messageTextTime = 1.f;
      if (info.IsHintMemo() && visible) {
        CSfxManager::SfxStart(SFXui_x_hintoff_00, 127, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
      }
      return;
    }
    x598_base_basewidget_message->SetColor(CColor::White());
    x598_base_basewidget_message->SetVisibility(false, kTM_Children);
    CGuiWidget* pane =
        info.IsHintMemo() ? x598_base_basewidget_message : x59c_base_textpane_message;
    pane->SetVisibility(true, kTM_Children);
    x59c_base_textpane_message->TextSupport().SetTypeWriteEffectOptions(true, 0.1f, 40.f);
    if (info.IsClearMemoWindow()) {
      x55c_lastSfxChars = 0.f;
      x59c_base_textpane_message->TextSupport().SetCurTime(0.f);
      x59c_base_textpane_message->TextSupport().SetText(text);
    } else if (static_cast< int >(x59c_base_textpane_message->TextSupport().GetText().size()) ==
               0) {
      x55c_lastSfxChars = 0.f;
      x59c_base_textpane_message->TextSupport().AddText(text);
    } else {
      x59c_base_textpane_message->TextSupport().AddText(rstl::wstring_l(L"\n") + text);
    }
    x59c_base_textpane_message->SetColor(CColor::White());
    x598_base_basewidget_message->SetColor(CColor::White());
    x558_messageTextTime = info.GetDisplayTime();
    if (info.IsHintMemo()) {
      if (!visible) {
        x584_abuttonPulse = 0.f;
        x560_messageTextScale = 0.f;
        CSfxManager::SfxStart(SFXui_x_hinton_00, 127, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
      }
    } else {
      x598_base_basewidget_message->SetO2PTransform(x598_base_basewidget_message->GetTransform());
    }
  }
}
