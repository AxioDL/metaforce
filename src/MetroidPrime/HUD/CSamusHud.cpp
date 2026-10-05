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
  mBase_basewidget_pivot = mLoadedFrmeBaseHud->FindWidget(sPivotName);
  mHelmet_BaseWidget_Pivot = mLoadedFrmeHelmet->FindWidget("BaseWidget_Pivot");
  mBase_Model_AutoMapper =
      static_cast< CGuiModel* >(mLoadedFrmeBaseHud->FindWidget(sAutoMapperName));
  mBase_textpane_counter =
      static_cast< CGuiTextPane* >(mLoadedFrmeBaseHud->FindWidget(sCounterName));
  mBase_textpane_counter->TextSupport().SetFontColor(gpTweakGuiColors->GetHudCounterFill());
  mBase_textpane_counter->TextSupport().SetOutlineColor(
      gpTweakGuiColors->GetHudCounterOutline());
  mBase_basewidget_message = mLoadedFrmeBaseHud->FindWidget("basewidget_message");
  for (CGuiWidget* child = static_cast< CGuiWidget* >(mBase_basewidget_message->ChildObject());
       child != nullptr; child = static_cast< CGuiWidget* >(child->NextSibling())) {
    child->SetDepthTest(false);
  }
  mBase_textpane_message =
      static_cast< CGuiTextPane* >(mLoadedFrmeBaseHud->FindWidget(sMessageName));
  mBase_model_abutton =
      static_cast< CGuiModel* >(mLoadedFrmeBaseHud->FindWidget("model_abutton"));
  for (int i = 0; i < 4; ++i) {
    mGuiLights[i] = mLoadedFrmeHelmet->GetFrameLight(i);
  }
  mGuiLights[3]->SetColor(CColor::Black());
  for (int i = 0; i < 4; ++i) {
    SVideoBand& band = mVideoBands[i];
    band.mVideoband = static_cast< CGuiModel* >(mLoadedFrmeBaseHud->FindWidget(
        rstl::string(CBasics::Stringize("%s%d", sVideoBandName, i))));
    band.mRandA = 6.f + 60.f * rand() / static_cast< float >(RAND_MAX);
    band.mRandB = 16.f + 240.f * rand() / static_cast< float >(RAND_MAX);
  }
  mBase_textpane_message->SetDepthTest(false);
  mBase_basewidget_message->SetVisibility(false, kTM_Children);
  mBase_textpane_message->TextSupport().SetFontColor(gpTweakGuiColors->GetHudMessageFill());
  mBase_textpane_message->TextSupport().SetOutlineColor(
      gpTweakGuiColors->GetHudMessageOutline());
  mBase_textpane_message->TextSupport().SetControlTXTRMap(
      &gpGameState->GameOptions().GetControlTXTRMap());
  mBase_Model_AutoMapper->SetIsAlwaysDepthWrite(true);
  mBasewidgetIdlePos = mBase_basewidget_pivot->GetTransform().GetTranslation();
  mCameraPos = mLoadedFrmeBaseHud->GetFrameCamera()->GetLocalPosition();
  RefreshHudOptions();
}

void CSamusHud::RefreshHudOptions() {
  if (!mDecoIntf.null()) {
    mDecoIntf->UpdateHudAlpha();
  }
  if (!mHelmetIntf.null()) {
    mHelmetIntf->UpdateHelmetAlpha();
  }
}

void CSamusHud::InitializeDamageLight() {
  const short lightId = mLoadedSelectedHud->AddWidgetToIDDB(rstl::string_l("DamageSpotLight"));
  CGuiWidget* parent = mLoadedSelectedHud->FindWidget(rstl::string_l(sPivotName));
  const CGuiWidget::CGuiWidgetParms parms(
      mLoadedSelectedHud, false, lightId, parent->GetWidgetID(), true, true, false,
      gpTweakGuiColors->GetHudDamageLightColor(), CGuiWidget::kGMDF_Alpha, false, false);
  const CColor& white = CColor::White();
  CLight spotLight = CLight::BuildSpot(CVector3f::Zero(), CVector3f::Forward(), white,
                                       gpTweakGui->GetHudDamageLightSpotAngle());
  mDamageLight = rs_new CGuiLight(parms, spotLight);
  mDamageLight->SetColor(CColor::White());
  const CColor color = gpTweakGuiColors->GetHudFrameColor();
  mDamageLight->SetAmbientContribution(CColor(color.GetRed() * color.GetAlpha(),
                                                  color.GetGreen() * color.GetAlpha(),
                                                  color.GetBlue() * color.GetAlpha(), 1.f));
  mDamageLight->SetConstantAttenuation(1.f);
  mDamageLight->SetLinearAttenuation(0.f);
  mDamageLight->SetConstantAngleAttenuation(gpTweakGui->GetDamageLightAngleC());
  mDamageLight->SetLinearAngleAttenuation(gpTweakGui->GetDamageLightAngleL());
  mDamageLight->SetQuadraticAngleAttenuation(gpTweakGui->GetDamageLightAngleQ());
  mDamageLight->SetLightIndex(4);
  mDamageLight->SetO2PTransform(CTransform4f::Identity());
  mLoadedSelectedHud->FindWidget(parms.mParentId)
      ->AddChildWidget(mDamageLight, false, true);
  mLoadedSelectedHud->AddLight(mDamageLight);
  const float xAngle = gpTweakGui->GetDamageLightXfXAngle();
  const float zAngle = gpTweakGui->GetDamageLightXfZAngle();
  const CTransform4f lightXf = CTransform4f::Translate(gpTweakGui->GetDamageLightPreTranslate());
  mLightTransforms.clear();
  mLightTransforms.reserve(10);
  mLightTransforms.push_back(lightXf);
  mLightTransforms.push_back(
      CTransform4f::Translate(gpTweakGui->GetDamageLightCenterTranslate()) * lightXf);
  mLightTransforms.push_back(CTransform4f::RotateX(CRelAngle::FromDegrees(xAngle)) * lightXf);
  mLightTransforms.push_back(CTransform4f::RotateX(CRelAngle::FromDegrees(xAngle)) *
                                 CTransform4f::RotateZ(CRelAngle::FromDegrees(-zAngle)) * lightXf);
  mLightTransforms.push_back(CTransform4f::RotateZ(CRelAngle::FromDegrees(-zAngle)) * lightXf);
  mLightTransforms.push_back(CTransform4f::RotateX(CRelAngle::FromDegrees(-xAngle)) *
                                 CTransform4f::RotateZ(CRelAngle::FromDegrees(-zAngle)) * lightXf);
  mLightTransforms.push_back(CTransform4f::RotateX(CRelAngle::FromDegrees(-xAngle)) * lightXf);
  mLightTransforms.push_back(CTransform4f::RotateX(CRelAngle::FromDegrees(-xAngle)) *
                                 CTransform4f::RotateZ(CRelAngle::FromDegrees(zAngle)) * lightXf);
  mLightTransforms.push_back(CTransform4f::RotateZ(CRelAngle::FromDegrees(zAngle)) * lightXf);
  mLightTransforms.push_back(CTransform4f::RotateX(CRelAngle::FromDegrees(xAngle)) *
                                 CTransform4f::RotateZ(CRelAngle::FromDegrees(zAngle)) * lightXf);
}

void CSamusHud::InitializeFrameGlueMutable(const CStateManager& mgr) {
  const float lastTankEnergy = fmodf(mPlayerHealth, CPlayerState::GetEnergyTankCapacity());
  const int tanksFilled = mPlayerHealth / CPlayerState::GetEnergyTankCapacity();
  const CPlayer& player = *mgr.GetPlayer();
  const float charge = player.GetPlayerGun()->GetChargePercentage();
  bool missilesActive = player.GetPlayerGun()->GetMissileMode() == CPlayerGun::kMM_Active;
  bool lockedOn = player.GetOrbitTargetId() != kInvalidUniqueId;
  switch (mNextState) {
  case kHS_Combat: {
    mBossEnergyIntf = rs_new CHudBossEnergyInterface(*mLoadedSelectedHud);
    mEnergyIntf =
        rs_new CHudEnergyInterface(*mLoadedSelectedHud, lastTankEnergy, mTotalEnergyTanks,
                                   tanksFilled, mEnergyLow, kHT_Combat);
    if (mThreatIntf.null()) {
      mThreatIntf = rs_new CHudThreatInterface(
        *mLoadedSelectedHud,
#if VERSION >= VERSION_GM8P_00
        nullptr, // TODO
#endif
        kHT_Combat,
        9999.f
      );
    } else {
      mThreatIntf->SetIsVisibleGame(true);
    }
    if (mMissileIntf.null()) {
      mMissileIntf =
          rs_new CHudMissileInterface(*mLoadedSelectedHud, mMissileCapacity,
                                      mMissileAmount, charge, missilesActive, kHT_Combat, mgr);
    } else {
      mMissileIntf->SetIsVisibleGame(true, mgr);
    }
    if (mFreeLookIntf.null()) {
      mFreeLookIntf =
          rs_new CHudFreeLookInterface(*mLoadedSelectedHud, kHT_Combat, mInFreeLook,
                                       mLookControlHeld, lockedOn);
    } else {
      mFreeLookIntf->SetIsVisibleGame(true);
    }
    if (mDecoIntf.null()) {
      mDecoIntf = rs_new CHudDecoInterfaceCombat(*mLoadedSelectedHud);
    } else {
      mDecoIntf->SetIsVisibleGame(true);
    }
    mRadarIntf->SetIsVisibleGame(true);
    mVisorMenu->SetIsVisibleGame(true);
    mBeamMenu->SetIsVisibleGame(true);
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
    const bool hasBombs = state.HasPowerUp(CPlayerState::kIT_MorphBallBombs);
    const bool pbReady = ballGun.IsPowerBombReady() &&
                         mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed;
    mBossEnergyIntf = nullptr;
    mBallIntf = rs_new CHudBallInterface(*mLoadedSelectedHud, powerBombs, capacity, bombs,
                                             pbReady, hasBombs);
    mEnergyIntf =
        rs_new CHudEnergyInterface(*mLoadedSelectedHud, lastTankEnergy, mTotalEnergyTanks,
                                   tanksFilled, mEnergyLow, kHT_Ball);
    mThreatIntf = nullptr;
    mMissileIntf = nullptr;
    mFreeLookIntf = nullptr;
    mDecoIntf = nullptr;
    mDamageLight = nullptr;
    mRadarIntf->SetIsVisibleGame(false);
    mVisorMenu->SetIsVisibleGame(false);
    mBeamMenu->SetIsVisibleGame(false);
    UpdateEnergy(0.f, mgr, true);
    break;
  }
  case kHS_Scan: {
    mBossEnergyIntf = nullptr;
    mEnergyIntf =
        rs_new CHudEnergyInterface(*mLoadedSelectedHud, lastTankEnergy, mTotalEnergyTanks,
                                   tanksFilled, mEnergyLow, kHT_Scan);
    mThreatIntf = nullptr;
    mMissileIntf = nullptr;
    mFreeLookIntf = rs_new CHudFreeLookInterface(
        *mLoadedSelectedHud, kHT_Scan, mInFreeLook, mLookControlHeld, lockedOn);
    mDecoIntf = rs_new CHudDecoInterfaceScan(*mLoadedSelectedHud);
    InitializeDamageLight();
    UpdateEnergy(0.f, mgr, true);
    break;
  }
  case kHS_XRay: {
    mBossEnergyIntf = rs_new CHudBossEnergyInterface(*mLoadedSelectedHud);
    mEnergyIntf =
        rs_new CHudEnergyInterface(*mLoadedSelectedHud, lastTankEnergy, mTotalEnergyTanks,
                                   tanksFilled, mEnergyLow, kHT_XRay);
    mThreatIntf = rs_new CHudThreatInterface(
      *mLoadedSelectedHud,
#if VERSION >= VERSION_GM8P_00
      nullptr, // TODO
#endif
      kHT_XRay,
      9999.f
    );
    mMissileIntf =
        rs_new CHudMissileInterface(*mLoadedSelectedHud, mMissileCapacity,
                                    mMissileAmount, charge, missilesActive, kHT_XRay, mgr);
    mFreeLookIntf = rs_new CHudFreeLookInterfaceXRay(
        *mLoadedSelectedHud, mInFreeLook, mLookControlHeld, lockedOn);
    mDecoIntf = rs_new CHudDecoInterfaceXRay(*mLoadedSelectedHud);
    InitializeDamageLight();
    mVisorMenu->SetIsVisibleGame(true);
    mBeamMenu->SetIsVisibleGame(true);
    UpdateEnergy(0.f, mgr, true);
    break;
  }
  case kHS_Thermal: {
    mBossEnergyIntf = rs_new CHudBossEnergyInterface(*mLoadedSelectedHud);
    mEnergyIntf =
        rs_new CHudEnergyInterface(*mLoadedSelectedHud, lastTankEnergy, mTotalEnergyTanks,
                                   tanksFilled, mEnergyLow, kHT_Thermal);
    mThreatIntf = rs_new CHudThreatInterface(
      *mLoadedSelectedHud,
#if VERSION >= VERSION_GM8P_00
      nullptr, // TODO
#endif
      kHT_Thermal,
      9999.f
    );
    mMissileIntf =
        rs_new CHudMissileInterface(*mLoadedSelectedHud, mMissileCapacity,
                                    mMissileAmount, charge, missilesActive, kHT_Thermal, mgr);
    mFreeLookIntf =
        rs_new CHudFreeLookInterface(*mLoadedSelectedHud, kHT_Thermal, mInFreeLook,
                                     mLookControlHeld, lockedOn);
    mDecoIntf = rs_new CHudDecoInterfaceThermal(*mLoadedSelectedHud);
    InitializeDamageLight();
    mVisorMenu->SetIsVisibleGame(true);
    mBeamMenu->SetIsVisibleGame(true);
    UpdateEnergy(0.f, mgr, true);
    break;
  }
  case kHS_None:
    UninitializeFrameGlueMutable();
    break;
  }
}

void CSamusHud::UninitializeFrameGlueMutable() {
  mBossEnergyIntf = nullptr;
  mEnergyIntf = nullptr;
  mDecoIntf = nullptr;
  mThreatIntf = nullptr;
  mMissileIntf = nullptr;
  mFreeLookIntf = nullptr;
  mBallIntf = nullptr;
  mDamageLight = nullptr;
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
  mHudMemoParms = info;
  mHudMemoString =
      rs_new TToken< CStringTable >(gpSimplePool->GetObj(SObjectTag('STRG', strg)));
  mHudMemoString->Lock();
  mHudMemoIdx = hintNum;
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
  if (!mThreatIntf.null()) {
    mThreatIntf->SetThreatDistance(threatDistance);
  }
}

CSamusHud::CSamusHud(const CStateManager& mgr)
: mLoadPhase(kLP_Zero)
, mTargetingMgr(mgr)
, mFrmeHelmet(gpSimplePool->GetObj(sHelmetName))
, mLoadedFrmeHelmet(nullptr)
, mFrmeBaseHud(gpSimplePool->GetObj(sBaseHudName))
, mLoadedFrmeBaseHud(nullptr)
, mSelectedHud(rstl::optional_object_null())
, mLoadedSelectedHud(nullptr)
, mCurState(kHS_None)
, mNextState(kHS_None)
, mSetState(kHS_None)
, mActiveTransState(kTS_NotTransitioning)
, mTransT(1.f)
, mPreLoadCountdown(0)
, mPlayerHealth(0.f)
, mTotalEnergyTanks(0)
, mMissileAmount(0)
, mMissileCapacity(0)
, mInFreeLook(false)
, mLookControlHeld(false)
, mLatestFirstPerson(true)
, mEnergyLow(mgr.GetPlayer()->IsEnergyLow(mgr))
, x2e4_(0)
, x2e8_(0)
, mMissileMode(CPlayerGun::kMM_Inactive)
, mVisorBeamMenuAlpha(1.f)
, mFpCamDirA(mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform().GetForward())
, mBasewidgetIdlePos(CVector3f::Zero())
, mCameraPos(CVector3f::Zero())
, mHudLag(CQuaternion::NoRotation())
, mInvHudLag(CQuaternion::NoRotation())
, mLights(rs_new CActorLights(8, CVector3f::Zero(), 4, 1,
                                  CActorLights::kDefaultPositionUpdateThreshold, true, false,
                                  false))
, mHudLights(3, SCachedHudLight(CVector3f::Zero(), CColor::White(), 0.f, 0.f, 0.f, 0.f))
, mDamageLight(nullptr)
, mDamageTime(0.f)
, mDamageLightPulser(0.f)
, mDamageFilterAmtInit(1.f)
, mDamageFilterAmt(0.f)
, mDamageFilterAmtGain(0.f)
, mHudDamagePracticalsInit(0.f)
, mHudDamagePracticals(0.f)
, mHudDamagePracticalsGain(0.f)
, mDamagerToPlayerNorm(CVector3f::Zero())
, mDecoShakeTranslateAmt(0.f)
, mDecoShakeTranslateAmtVel(0.f)
, mDecoShakeTranslate(CVector3f::Zero())
, mDecoShakeRotate(CMatrix3f::Identity())
, mHudLagShakeRot(CQuaternion::NoRotation())
, mDecoShakeAmtInit(0.f)
, mDecoShakeAmt(0.f)
, mDecoShakeAmtGain(0.f)
, x468_(0)
, x46c_(3, CTransform4f::Identity())
, mViewportScaleX(1.f)
, mViewportScaleY(1.f)
, mStaticInterp(0.f)
, mStaticCycleTimerHi(0.f)
, mStaticCycleTimerLo(0.f)
, mHudMemoParms(0.f, false, false, false)
, mHudMemoIdx(0)
, mMessageTextTime(0.f)
, mLastSfxChars(0.f)
, mMessageTextScale(0.f)
, mFpCamDirB(mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform().GetForward())
, mLookDeltaDot(1.f)
, mFreeLookSfxCycleTimer(0.f)
, mEnergyLowTimer(0.f)
, mEnergyLowPulse(0.f)
, mAbuttonPulse(0.f)
, mVideoBands(4, SVideoBand())
, mGuiLights(4, nullptr)
, mProfileInfo(15, SProfileInfo()) {
  UpdateStateTransition(1.f, mgr);
  spSamusHud = this;
  for (int i = 0; i < 16; ++i) {
    mCamFovTweaks[i] = 5.f * i + 40.f;
  }
  for (int i = 0; i < 64; ++i) {
    mCamYTweaks[i] = -0.5f * i;
  }
  for (int i = 0; i < 32; ++i) {
    mCamZTweaks[i] = 0.5f * i + -8.f;
  }
  mLoadedFrmeHelmet = *mFrmeHelmet;
  mLoadedFrmeBaseHud = *mFrmeBaseHud;
  mHelmetIntf = rs_new CHudHelmetInterface(*mLoadedFrmeHelmet);
  mVisorMenu = rs_new CHudVisorBeamMenu(*mLoadedFrmeBaseHud, CHudVisorBeamMenu::kVBM_Visor,
                                            BuildPlayerHasVisors(mgr));
  mBeamMenu = rs_new CHudVisorBeamMenu(*mLoadedFrmeBaseHud, CHudVisorBeamMenu::kVBM_Beam,
                                           BuildPlayerHasBeams(mgr));
  mRadarIntf = rs_new CHudRadarInterface(*mLoadedFrmeBaseHud, mgr);
  InitializeFrameGluePermanent(mgr);
  UpdateEnergy(0.f, mgr, true);
  UpdateMissile(0.f, mgr, true);
  UpdateBallMode(mgr, true);
}

void CSamusHud::UpdateEnergyLow(float dt, const CStateManager& mgr) {
  const bool cineCam =
      TCastToConstPtr< CCinematicCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr)) != nullptr;
  const float oldTimer = mEnergyLowTimer;
  mEnergyLowTimer = fmod(mEnergyLowTimer + dt, 0.5);
  mEnergyLowPulse = mEnergyLowTimer < 0.25f ? mEnergyLowTimer / 0.25f
                                                    : (0.5f - mEnergyLowTimer) / 0.25f;
  if (!cineCam && mEnergyLow && mEnergyLowTimer < oldTimer) {
    CSfxManager::SfxStart(SFXui_x_warning_02, 127, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
}

CSamusHud::~CSamusHud() {
  if (mDamageSfx) {
    CSfxManager::RemoveEmitter(mDamageSfx);
  }
  spSamusHud = nullptr;
}

bool CSamusHud::CheckLoadComplete(const CStateManager& mgr) {
  switch (mLoadPhase) {
  case kLP_Zero:
    if (mTargetingMgr.CheckLoadComplete()) {
      mLoadPhase = kLP_One;
    } else {
      return false;
    }
  case kLP_One:
    UpdateStateTransition(1.f, mgr);
    if (mNextState != mSetState) {
      return false;
    }
    mLoadPhase = kLP_Two;
  case kLP_Two:
    if (mLoadedFrmeHelmet->GetIsFinishedLoading() &&
        mLoadedFrmeBaseHud->GetIsFinishedLoading()) {
      mLoadPhase = kLP_Three;
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
  if (!mBeamMenu.null()) {
    mBeamMenu->SetSelection(currentBeam, nextBeam, beamInterp);
    mBeamMenu->SetPlayerHas(BuildPlayerHasBeams(mgr));
  }
  if (!mVisorMenu.null()) {
    mVisorMenu->SetSelection(visor, nextVisor, visorInterp);
    mVisorMenu->SetPlayerHas(BuildPlayerHasVisors(mgr));
  }
}


void CSamusHud::UpdateFreeLook(float dt, const CStateManager& mgr) {
  const CFirstPersonCamera* const fpCam =
      TCastToConstPtr< CFirstPersonCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr));
  bool inFreeLook = mgr.GetPlayer()->IsInFreeLook() && fpCam != nullptr;
  bool lookHeld = mgr.GetPlayer()->GetFreeLookStickState();
  if (mInFreeLook != inFreeLook) {
    if (inFreeLook) {
      CSfxManager::SfxStart(SFXui_x_freeon_00, 127, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    } else {
      CSfxManager::SfxStart(SFXui_x_freeoff_00, 127, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    }
    mInFreeLook = inFreeLook;
  }
  const float deltaFrames = 60.f * (0.99999f * dt);
  const float oldDot = mLookDeltaDot;
  const CVector3f direction = !fpCam ? mFpCamDirB : fpCam->GetTransform().GetForward();
  bool adjusting = inFreeLook;
  if (adjusting) {
    adjusting = lookHeld;
  }
  mLookDeltaDot = adjusting ? CMath::Limit(CVector3f::Dot(direction, mFpCamDirB), 1.f) : 1.f;
  mFpCamDirB = direction;
  bool crossed = (oldDot >= deltaFrames && mLookDeltaDot < deltaFrames) ||
                 (oldDot < deltaFrames && mLookDeltaDot >= deltaFrames);
  if (crossed) {
    mFreeLookSfxCycleTimer = 0.f;
  } else if (mFreeLookSfxCycleTimer < 0.05f) {
    mFreeLookSfxCycleTimer =
        rstl::min_val(kFreeLookSfxCycleTime, mFreeLookSfxCycleTimer + dt);
    if (mFreeLookSfxCycleTimer == 0.05f) {
      if (mLookDeltaDot < deltaFrames) {
        if (!mFreeLookSfx) {
          mFreeLookSfx =
              CSfxManager::SfxStart(SFXui_x_samrot_lp_00, 127, 64, true, CSfxManager::kMedPriority,
                                    true, CSfxManager::kAllAreas);
        }
      } else {
        CSfxManager::SfxStop(mFreeLookSfx);
        mFreeLookSfx.Clear();
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
    if (!mFreeLookIntf.null()) {
      mFreeLookIntf->SetFreeLookState(
          inFreeLook, lookHeld, mgr.GetPlayer()->GetOrbitTargetId() != kInvalidUniqueId, angle);
    }
    if (mFreeLookSfx) {
      float pitch = angle * gpTweakGui->GetFreeLookSfxPitchScale() / (M_PIF / 2.f);
      if (!gpTweakGui->GetNoAbsoluteFreeLookSfxPitch()) {
        pitch = CMath::AbsF(pitch);
      }
      CSfxManager::PitchBend(mFreeLookSfx, static_cast< int >(8192.f + pitch));
    }
  }
}

void CSamusHud::UpdateVideoBands(float dt, const CStateManager& mgr) {
  for (int i = 0; i < 4; ++i) {
    if (mVideoBands[i].mVideoband != nullptr) {
      mVideoBands[i].mVideoband->SetIsVisible(false);
    }
  }
}

void CSamusHud::UpdateStaticInterference(float dt, const CStateManager& mgr) {
  float interference = mgr.GetPlayerState()->GetHudStaticInterferenceAmount();
  const float oldInterference = mStaticInterp;
  if (mStaticInterp < interference) {
    mStaticInterp = rstl::min_val(interference, mStaticInterp + dt);
  } else if (mStaticInterp > interference) {
    mStaticInterp = rstl::max_val(interference, mStaticInterp - dt);
  }
  UpdateStaticSfx(mStaticSfxHi, mStaticCycleTimerHi, SFXui_x_static_lp_0, dt,
                  oldInterference, 0.1f);
  UpdateStaticSfx(mStaticSfxLo, mStaticCycleTimerLo, SFXui_x_static_lp_01, dt,
                  oldInterference, 0.5f);
  if (mStaticInterp > 0.f) {
    const CColor color = CColor::White().WithAlphaOf(mStaticInterp);
    mCamFilter2.SetFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_RandomStatic,
                              0.f, color, kInvalidAssetId);
  } else {
    mCamFilter2.DisableFilter(0.f);
  }
}

void CSamusHud::UpdateStaticSfx(CSfxHandle& handle, float& cycleTimer, ushort sfxId, float dt,
                                float oldStaticInterp, float staticThreshold) {
  bool crossed = (oldStaticInterp > staticThreshold && mStaticInterp <= staticThreshold) ||
                 (oldStaticInterp <= staticThreshold && mStaticInterp > staticThreshold);
  if (crossed) {
    cycleTimer = 0.f;
  } else if (cycleTimer < 0.1f) {
    cycleTimer = rstl::min_val(kStaticSfxCycleTime, cycleTimer + dt);
    if (cycleTimer == 0.1f) {
      if (mStaticInterp > staticThreshold) {
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
  mEnergyLow = mgr.GetPlayer()->IsEnergyLow(mgr);
  if (init || energy != mPlayerHealth || numEnergyTanks != mTotalEnergyTanks) {
    float lastTankEnergy = energy;
    uint filledTanks = 0;
    while (lastTankEnergy > CPlayerState::GetBaseHealthCapacity()) {
      ++filledTanks;
      lastTankEnergy -= CPlayerState::GetEnergyTankCapacity();
    }
    if (mNextState != kHS_None) {
      if (!mEnergyIntf.null()) {
        float currentTankEnergy = mPlayerHealth;
        while (currentTankEnergy > CPlayerState::GetBaseHealthCapacity()) {
          currentTankEnergy -= CPlayerState::GetEnergyTankCapacity();
        }
        mEnergyIntf->SetCurrEnergy(lastTankEnergy, (currentTankEnergy > lastTankEnergy) !=
                                                           (mPlayerHealth > energy));
      }
      mPlayerHealth = energy;
      if (!mEnergyIntf.null()) {
        mEnergyIntf->SetNumTotalEnergyTanks(numEnergyTanks);
        mEnergyIntf->SetNumFilledEnergyTanks(filledTanks);
        mEnergyIntf->SetEnergyLow(mEnergyLow);
      }
      mTotalEnergyTanks = numEnergyTanks;
    }
  }
  if (!mBossEnergyIntf.null()) {
    const TUniqueId bossId = mgr.GetBossId();
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(bossId))) {
      if (const CHealthInfo* health = actor->GetHealthInfo(mgr)) {
        const float bossEnergy = CMath::CeilingF(health->GetHP());
        const float maxEnergy = mgr.GetTotalBossEnergy();
        const rstl::wstring name =
            rstl::wstring_l(gpStringTable->GetString(mgr.GetBossStringIdx()));
        mBossEnergyIntf->SetBossParams(true, name, bossEnergy, maxEnergy);
      } else {
        mBossEnergyIntf->SetBossParams(false, rstl::wstring_l(L""), 0.f, 0.f);
      }
    } else {
      mBossEnergyIntf->SetBossParams(false, rstl::wstring_l(L""), 0.f, 0.f);
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
  if (!mMissileIntf.null()) {
    mMissileIntf->SetChargeBeamFactor(charge);
  }
  if (init || missiles != mMissileAmount || mode != mMissileMode ||
      capacity != mMissileCapacity) {
    if (!mMissileIntf.null()) {
      if (mMissileCapacity != capacity) {
        mMissileIntf->SetMissileCapacity(capacity);
      }
      if (mMissileAmount != missiles) {
        mMissileIntf->SetNumMissiles(missiles, mgr);
      }
      if (mMissileMode != mode) {
        mMissileIntf->SetIsMissilesActive(mode == CPlayerGun::kMM_Active);
      }
    }
    mMissileAmount = missiles;
    mMissileMode = mode;
    mMissileCapacity = capacity;
  }
}

void CSamusHud::UpdateBallMode(const CStateManager& mgr, bool init) {
  if (mBallIntf.null()) {
    return;
  }
  const CPlayerState& state = *mgr.GetPlayerState();
  const CPlayerGun& gun = *mgr.GetPlayer()->GetPlayerGun();
  const int powerBombs = state.GetItemAmount(CPlayerState::kIT_PowerBombs);
  const int capacity = state.GetItemCapacity(CPlayerState::kIT_PowerBombs);
  const int bombs = gun.IsBombReady() ? gun.GetBombsPending() : 0;
  const bool hasBombs = state.HasPowerUp(CPlayerState::kIT_MorphBallBombs);
  mBallIntf->SetBombParams(powerBombs, capacity, bombs, hasBombs,
                           gun.IsPowerBombReady() &&
                               mgr.GetPlayer()->GetMorphballTransitionState() ==
                                   CPlayer::kMS_Morphed,
                           false);
}

void CSamusHud::OnNewInGameGuiState(EInGameGuiState state, const CStateManager& mgr) {}

bool CSamusHud::IsCachedLightInAreaLights(const SCachedHudLight& cached,
                                          const CActorLights& areaLights) const {
  for (uint i = 0; i < areaLights.GetActiveAreaLightCount(); ++i) {
    const CLight& light = areaLights.GetLight(i);
    if (light.GetColor() == cached.mColor && light.GetPosition() == cached.mPos) {
      return true;
    }
  }
  return false;
}

bool CSamusHud::IsAreaLightInCachedLights(const CLight& light) const {
  for (int i = 0; i < 3; ++i) {
    const SCachedHudLight& cached = mHudLights[i];
    if (cached.mFader != 0.f && light.GetColor() == cached.mColor &&
        light.GetPosition() == cached.mPos) {
      return true;
    }
  }
  return false;
}

int CSamusHud::FindEmptyHudLightSlot(const CLight& light) const {
  for (int i = 0; i < 3; ++i) {
    if (mHudLights[i].mFader == 0.f) {
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
  CActorLights& lights = *mLights;
  lights.SetFindShadowLight(false);
  const TAreaId area = mgr.GetPlayer()->GetCurrentAreaId();
  if (area == kInvalidAreaId) {
    return;
  }
  lights.BuildAreaLightList(mgr, mgr.GetWorld()->GetAreaAlways(area), bounds);
  for (int i = 0; i < 3; ++i) {
    SCachedHudLight& light = mHudLights[i];
    const CVector3f direction = (light.mPos - position).AsNormalized();
    if (light.mFader > 0.f && (CVector3f::Dot(lookDirection, direction) <= 0.15707964f ||
                                  !IsCachedLightInAreaLights(light, lights))) {
      light.mFader *= -1.f;
    }
  }
  int available = 0;
  for (int i = 0; i < 3; ++i) {
    if (mHudLights[i].mFader <= 0.f) {
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
        mHudLights[slot] =
            SCachedHudLight(light.GetPosition(), light.GetColor(), light.GetAttenuationConstant(),
                            light.GetAttenuationLinear(), light.GetAttenuationQuadratic(), 0.001f);
      }
    }
  }
  for (int i = 0; i < 3; ++i) {
    SCachedHudLight& light = mHudLights[i];
    if (light.mFader < 0.f) {
      light.mFader = rstl::min_val(light.mFader + 2.f * dt, 0.f);
    } else if (light.mFader < 1.f && light.mFader != 0.f) {
      light.mFader = rstl::min_val(light.mFader + 2.f * dt, 1.f);
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
  AUTO(lightIt, mGuiLights.begin());
  float maxIntensity = 0.f;
  int maxIntensityIndex = 0;
  for (int i = 0; i < mHudLights.size(); ++i) {
    const SCachedHudLight& light = mHudLights[i];
    CGuiLight* widget = *lightIt;
    const CVector3f toCamera = position - light.mPos;
    const CMatrix3f& rotation = camera->GetTransform().BuildMatrix3f().GetTranspose();
    const CVector3f direction = rotation * toCamera.AsNormalized();
    const float distance = rstl::max_val(toCamera.Magnitude(), kHudLightDistanceEpsilon);
    const float attQuadratic = gpTweakGui->GetHudLightAttMulQuadratic();
    const float falloff = rstl::min_val(
        1.f, 1.f / (gpTweakGui->GetHudLightAttMulConstant() * light.mDistC +
                    distance * (gpTweakGui->GetHudLightAttMulLinear() * light.mDistL) +
                    distance * (distance * (attQuadratic * light.mDistQ))));
    widget->SetO2WTransform(CTransform4f::LookAt(CVector3f::Zero(), direction));
    const float fadedFalloff = falloff * CMath::AbsF(light.mFader);
    CColor lightColor =
        CColor::Modulate(light.mColor, CColor(fadedFalloff, fadedFalloff, fadedFalloff, 1.f));
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
  const CGuiLight& brightestWidget = *mGuiLights[maxIntensityIndex];
  const CVector3f direction = -1.f * brightestWidget.GetWorldTransform().GetForward();
  const float forwardDot = CVector3f::Dot(CVector3f::Forward(), direction);
  const CVector3f reflectedDirection = forwardDot * (2.f * CVector3f::Forward()) - direction;
  CGuiLight* reflection = mGuiLights[3];
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
    mDamageTime += dt;
  } else {
    mDamageTime = 0.f;
  }
  const float pulseDuration = gpTweakGui->GetHudDamagePulseDuration();
  const float pulseTime = CMath::AbsF(fmodf(mDamageTime, pulseDuration));
  mDamageLightPulser = pulseTime < 0.5f * pulseDuration
                               ? pulseTime / (0.5f * pulseDuration)
                               : (pulseDuration - pulseTime) / (0.5f * pulseDuration);
  mDamageLightPulser =
      CMath::Clamp(0.f,
                   gpTweakGui->GetHudDamageColorGain() *
                       (mDamageLightPulser *
                        rstl::min_val(kDamagePulseLimit, mgr.GetPlayer()->GetDamageAmount())),
                   1.f);
  const CColor frameColor = gpTweakGuiColors->GetHudFrameColor();
  const CColor damageAmbient(frameColor.GetRed() * frameColor.GetAlpha() + mDamageLightPulser,
                             frameColor.GetGreen() * frameColor.GetAlpha() + mDamageLightPulser,
                             frameColor.GetBlue() * frameColor.GetAlpha() + mDamageLightPulser,
                             1.f);
  if (mDamageLight) {
    mDamageLight->SetAmbientContribution(damageAmbient);
  }
  if (mDamageFilterAmt > 0.f) {
    mDamageFilterAmt = rstl::max_val(0.f, mDamageFilterAmt - dt);
    if (mDamageFilterAmt == 0.f) {
      CSfxManager::RemoveEmitter(mDamageSfx);
      mDamageSfx.Clear();
    }
  }
  const float peak = mDamageFilterAmtInit * gpTweakGui->GetHudDamagePeakFactor();
  const CColor& ambientColor = gpTweakGuiColors->GetDamageAmbientColor();
  float colorGain =
      mDamageFilterAmt > peak
          ? (mDamageFilterAmtInit - mDamageFilterAmt) / (mDamageFilterAmtInit - peak)
          : mDamageFilterAmt / peak;
  colorGain *= mDamageFilterAmtGain;
  colorGain = CMath::Clamp(0.f, colorGain, 1.f);
  const CColor color0 = ambientColor.WithAlphaModulatedBy(colorGain);
  const CColor color1 =
      gpTweakGuiColors->GetDamageAmbientPulseColor().WithAlphaModulatedBy(mDamageLightPulser);
  CColor filterColor = CColor::Add(color0, color1);
  if (filterColor.GetAlphau8()) {
    if (mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Unmorphed) {
      filterColor = filterColor.WithAlphaModulatedBy(0.75f);
    }
    mCamFilter.SetFilter(CCameraFilterPass::kFT_Add, CCameraFilterPass::kFS_Fullscreen, 0.f,
                             filterColor, kInvalidAssetId);
  } else {
    mCamFilter.DisableFilter(0.f);
  }
  if (mDamageSfx) {
    CSfxManager::UpdateEmitter(mDamageSfx, mgr.GetPlayer()->GetTranslation(),
                               mgr.GetPlayer()->GetTransform().GetForward(), 127);
  }
  if (mHudDamagePracticals > 0.f) {
    mHudDamagePracticals = rstl::max_val(0.f, mHudDamagePracticals - dt);
    float practicals = mHudDamagePracticals / mHudDamagePracticalsInit;
    if (mEnergyIntf.get()) {
      mEnergyIntf->SetFlashMagnitude(practicals);
    }
    practicals = rstl::min_val(1.f, practicals * mHudDamagePracticalsGain);
    mHelmetIntf->AddHelmetLightValue(practicals);
    if (mDecoIntf.get()) {
      mDecoIntf->SetFrameColorValue(practicals);
      if (practicals > 0.f) {
        const CColor practicalColor(practicals, practicals, practicals, practicals);
        mDamageLight->SetColor(
            CColor::Modulate(gpTweakGuiColors->GetHudDamageLightColor(), practicalColor));
        mDamageLight->SetIsVisible(true);
      } else {
        mDamageLight->SetIsVisible(false);
      }
    }
  }
  bool updateTransform = false;
  if (mDecoShakeTranslateAmt > 0.f) {
    const float deceleration = (60.f * dt) * gpTweakGui->GetDecoDamageShakeDeceleration();
    mDecoShakeTranslateAmtVel -= deceleration;
    mDecoShakeTranslateAmt =
        rstl::max_val(0.f, mDecoShakeTranslateAmt + mDecoShakeTranslateAmtVel);
    updateTransform = true;
  }
  if (mDecoShakeAmt > 0.f) {
    mDecoShakeAmt = rstl::max_val(0.f, mDecoShakeAmt - dt);
    const float rotateFraction = mDecoShakeAmt / mDecoShakeAmtInit;
    const float rotate = rstl::min_val(rotateFraction * mDecoShakeAmtGain,
                                       gpTweakGui->GetMaxDecoDamageShakeRotate());
    const int xRandom = rand();
    const float xAngle = (2.f * M_PIF / 10.f) * ((xRandom / float(RAND_MAX)) * rotate);
    const CQuaternion xRotation = CQuaternion::XRotation(CRelAngle::FromRadians(xAngle));
    const int zRandom = rand();
    const float zAngle = (2.f * M_PIF / 10.f) * ((zRandom / float(RAND_MAX)) * rotate);
    const CQuaternion zRotation = CQuaternion::ZRotation(CRelAngle::FromRadians(zAngle));
    mHudLagShakeRot = xRotation * zRotation;
    CVector3f vectors[3] = {CVector3f::Right(), CVector3f::Forward(), CVector3f::Up()};
    for (int i = 0; i < 4; ++i) {
      const int random = rand();
      const int component = rand() % 9;
      const float amount = (random / float(RAND_MAX) - 0.5f) * rotate;
      vectors[component % 3][component / 3] += amount;
    }
    mDecoShakeRotate = CMatrix3f(vectors[0], vectors[1], vectors[2]);
    updateTransform = true;
  }
  if (updateTransform) {
    mDecoShakeTranslate =
        rstl::min_val(mDecoShakeTranslateAmt, gpTweakGui->GetMaxDecoDamageShakeTranslate()) *
        mDamagerToPlayerNorm;
    if (mDecoIntf.get()) {
      mDecoIntf->SetDamageTransform(mDecoShakeRotate,
                                        gpTweakGui->GetHudDecoShakeTranslateGain() *
                                            mDecoShakeTranslate);
    }
  }
}

void CSamusHud::UpdateStateTransition(float dt, const CStateManager& mgr) {
  if (mPreLoadCountdown == 0) {
    const EHudState desired = GetDesiredHudState(mgr);
    if (desired != mSetState) {
      mSetState = desired;
      bool ballTransition = desired == kHS_Ball || mNextState == kHS_Ball;
      mTransT = ballTransition ? FLT_EPSILON : mTransT;
      mActiveTransState = kTS_Countdown;
    }
  }
  switch (mActiveTransState) {
  case kTS_Countdown:
    if (mPreLoadCountdown == 0) {
      mTransT = rstl::max_val(mTransT - 5.f * dt, 0.f);
      if (mTransT == 0.f) {
        mPreLoadCountdown = 2;
        mLoadedSelectedHud = nullptr;
      }
    } else {
      --mPreLoadCountdown;
      if (mPreLoadCountdown == 0) {
        UninitializeFrameGlueMutable();
        mSelectedHud = rstl::optional_object_null();
        switch (mSetState) {
        case kHS_Thermal:
          mSelectedHud = TCachedToken< CGuiFrame >(gpSimplePool->GetObj(sThermalHudName));
          mSelectedHud->Lock();
          break;
        case kHS_Combat:
          mSelectedHud = TCachedToken< CGuiFrame >(gpSimplePool->GetObj(sCombatHudName));
          mSelectedHud->Lock();
          break;
        case kHS_Scan:
          mSelectedHud = TCachedToken< CGuiFrame >(gpSimplePool->GetObj(sScanHudName));
          mSelectedHud->Lock();
          break;
        case kHS_XRay:
          mSelectedHud = TCachedToken< CGuiFrame >(gpSimplePool->GetObj(sXRayHudName));
          mSelectedHud->Lock();
          break;
        case kHS_Ball:
          mSelectedHud = TCachedToken< CGuiFrame >(gpSimplePool->GetObj(sBallHudName));
          mSelectedHud->Lock();
          break;
        default:
          mSelectedHud = rstl::optional_object_null();
          break;
        }
        mActiveTransState = kTS_Loading;
      }
    }
    if (mActiveTransState != kTS_Loading) {
      return;
    }
  case kTS_Loading:
    if (mSelectedHud.valid()) {
      if (mSelectedHud->TryCache() && mSelectedHud->GetObject()->GetIsFinishedLoading()) {
        mLoadedSelectedHud = mSelectedHud->GetObject();
        mCurState = mNextState;
        mNextState = mSetState;
        InitializeFrameGlueMutable(mgr);
        mActiveTransState = kTS_Transitioning;
        UpdateCameraDebugSettings();
      }
    } else {
      mCurState = mNextState;
      mNextState = mSetState;
      mActiveTransState = kTS_NotTransitioning;
    }
    break;
  case kTS_Transitioning:
    mTransT = rstl::min_val(5.f * dt + mTransT, 1.f);
    if (mTransT == 1.f) {
      mActiveTransState = kTS_NotTransitioning;
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
  if (firstPerson != mLatestFirstPerson) {
    if (firstPerson) {
      EnterFirstPerson(mgr);
    } else {
      LeaveFirstPerson(mgr);
    }
    mLatestFirstPerson = firstPerson;
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
  mViewportScaleY = 1.f - morphFactor * gpTweakGui->GetBallViewportYReduction();
  if (!mBallIntf.null()) {
    mBallIntf->SetBallModeFactor(morphFactor);
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
  if (!mDecoIntf.null()) {
    mDecoIntf->SetIsVisibleDebug(decoVisible);
  }
  if (!mHelmetIntf.null()) {
    mHelmetIntf->SetIsVisibleDebug(helmetVisible, glowVisible);
  }
  mBase_Model_AutoMapper->SetIsVisible(false);
  UpdateEnergyLow(dt, mgr);
  for (int i = 0; i < 15; ++i) {
    mProfileInfo[i].mUpdateUsec = 0;
  }
  if (!mRadarIntf.null()) {
    mRadarIntf->Update(dt, mgr);
  }
  UpdateHudLag(dt, mgr);
  UpdateHudDynamicLights(dt, mgr);
  if (targetingManager) {
    mTargetingMgr.Update(dt, mgr);
  }
  UpdateHudDamage(dt, mgr, helmetVis);
  UpdateStaticInterference(dt, mgr);
  if (updateHud) {
    if (mNextState != kHS_None) {
      UpdateEnergy(dt, mgr, false);
      UpdateFreeLook(dt, mgr);
    }
    if (mNextState == kHS_Ball) {
      UpdateBallMode(mgr, false);
    } else {
      bool firstPersonHud = mNextState >= kHS_Combat && mNextState <= kHS_Scan;
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
  const float oldPulse = mAbuttonPulse;
  if (mHudMemoIdx == 0) {
    mAbuttonPulse += 2.f * dt;
    if (mAbuttonPulse > 1.f) {
      mAbuttonPulse -= 2.f;
    }
  }
  const float pulseAlpha = CMath::AbsF(mAbuttonPulse);
  mBase_model_abutton->SetColor(CColor::White().WithAlphaOf(pulseAlpha));
  bool pulseSound = !mgr.GetCameraManager()->IsInCinematicCamera() && oldPulse < 0.f &&
                    mAbuttonPulse >= 0.f && mBase_basewidget_message->GetIsVisible() &&
                    (mMessageTextTime == 0.f || mMessageTextTime >= 1.f);
  if (pulseSound) {
    CSfxManager::SfxStart(SFXui_x_hintflas_00, 127, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
  const float allTextAlpha = mDecoIntf.null() ? 1.f : mDecoIntf->GetMessageTextAlpha();
  float messageTextAlpha = 1.f;
  if (mMessageTextTime > 0.f) {
    messageTextAlpha = rstl::min_val(1.f, mMessageTextTime);
  } else if (!mBase_textpane_message->GetIsVisible() &&
             !mBase_basewidget_message->GetIsVisible()) {
    messageTextAlpha = 0.f;
  }
  if (!mBossEnergyIntf.null()) {
    mBossEnergyIntf->SetAlpha(1.f - messageTextAlpha);
  }
  if (!mHudMemoString.null() && mHudMemoString->IsLoaded()) {
    SetMessage(rstl::wstring((**mHudMemoString)->GetString(mHudMemoIdx)),
               mHudMemoParms);
    mHudMemoString = nullptr;
  }
  if (mMessageTextTime > 0.f) {
    mMessageTextTime = rstl::max_val(0.f, mMessageTextTime - dt);
    if (mMessageTextTime == 0.f) {
      mBase_textpane_message->TextSupport().SetTypeWriteEffectOptions(false, 0.f, 1.f);
      mBase_basewidget_message->SetVisibility(false, kTM_Children);
    }
  }
  float textScale = 1.f;
  const float messageAlpha = rstl::min_val(messageTextAlpha, allTextAlpha);
  CGuiWidget* messageWidget = mBase_basewidget_message->GetIsVisible()
                                  ? mBase_basewidget_message
                                  : mBase_textpane_message;
  messageWidget->SetColor(CColor::White().WithAlphaOf(messageAlpha));
  if (messageWidget == mBase_basewidget_message) {
    if (mMessageTextTime > 0.f) {
      mMessageTextScale = rstl::min_val(1.f, mMessageTextTime);
    } else {
      mMessageTextScale = rstl::min_val(1.f, mMessageTextScale + dt);
    }
    const float t = rstl::max_val(0.f, (mMessageTextScale - 0.75f) / 0.25f);
    if (t != 1.f) {
      if (t < 0.7f) {
        textScale = t / 0.7f;
      } else if (t < 0.85f) {
        textScale = 0.9f + (1.f - 0.9f) * (1.f - (t - 0.7f) / 0.15f);
      } else {
        textScale = 0.9f + (1.f - 0.9f) * (((t - 0.7f) - 0.15f) / 0.3f);
      }
    }
    mBase_basewidget_message->SetO2PTransform(mBase_basewidget_message->GetIdleXform() *
                                                  CTransform4f::Scale(textScale, 1.f, 1.f));
  }
  const float printed = mBase_textpane_message->TextSupport().GetNumCharactersPrinted();
  const float charsPerSfx = gpTweakGui->GetWorldTransManagerCharsPerSfx();
  if (printed >= mLastSfxChars + charsPerSfx) {
    mLastSfxChars += charsPerSfx;
    if (!mBase_basewidget_message->GetIsVisible() || textScale == 1.f) {
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
    mBase_textpane_counter->TextSupport().SetText(rstl::string(text), false);
    mBase_textpane_counter->SetIsVisible(true);
    const float counterAlpha =
        rstl::min_val(allTextAlpha, 1.f - rstl::min_val(1.f, mMessageTextTime));
    mBase_textpane_counter->SetColor(
        CColor::White().WithAlphaOf(CMath::Clamp(0.f, counterAlpha, 1.f)));
  } else {
    mBase_textpane_counter->SetIsVisible(false);
  }
  mLoadedFrmeBaseHud->Update(dt);
  if (mLoadedSelectedHud != nullptr) {
    mLoadedSelectedHud->Update(dt);
  }
  if (!mBossEnergyIntf.null()) {
    mBossEnergyIntf->Update(dt);
  }
  if (!mEnergyIntf.null()) {
    mEnergyIntf->Update(dt, mEnergyLowPulse);
  }
  if (!mThreatIntf.null()) {
    mThreatIntf->Update(dt);
  }
  if (!mMissileIntf.null()) {
    mMissileIntf->Update(dt, mgr);
  }
  if (!mFreeLookIntf.null()) {
    mFreeLookIntf->Update(dt);
  }
  if (!mHelmetIntf.null()) {
    mHelmetIntf->Update(dt);
  }
  const CPlayerState& state = *mgr.GetPlayerState();
  const CPlayerState::EPlayerVisor visor = state.GetCurrentVisor();
  const CPlayerState::EPlayerVisor nextVisor = state.GetTransitioningVisor();
  if (mgr.GetPlayer()->GetPlayerScanState() == CPlayer::kSS_NotScanning) {
    mVisorBeamMenuAlpha = rstl::min_val(1.f, mVisorBeamMenuAlpha + 2.f * dt);
  } else {
    mVisorBeamMenuAlpha = rstl::max_val(0.f, mVisorBeamMenuAlpha - 2.f * dt);
  }
  float transitionFactor = 0.f;
  if (visor != CPlayerState::kPV_Scan) {
    if (nextVisor == CPlayerState::kPV_Scan) {
      transitionFactor = state.GetVisorTransitionFactor();
    } else {
      transitionFactor = 1.f;
    }
  }
  if (!mVisorMenu.null()) {
    mVisorMenu->UpdateHudAlpha(gpGameState->GameOptions().GetSwapBeamControls()
                                       ? transitionFactor
                                       : mVisorBeamMenuAlpha);
    mVisorMenu->Update(dt, false);
  }
  if (!mBeamMenu.null()) {
    mBeamMenu->UpdateHudAlpha(gpGameState->GameOptions().GetSwapBeamControls()
                                      ? mVisorBeamMenuAlpha
                                      : transitionFactor);
    mBeamMenu->Update(dt, false);
  }
  UpdateCameraDebugSettings();
  if (!mDecoIntf.null()) {
    mDecoIntf->Update(dt, mgr);
  }
}

void CSamusHud::Touch() const {
  if (mLoadedFrmeHelmet != nullptr) {
    mLoadedFrmeHelmet->Touch();
  }
  if (mLoadedFrmeBaseHud != nullptr) {
    mLoadedFrmeBaseHud->Touch();
  }
  if (mLoadedSelectedHud != nullptr) {
    mLoadedSelectedHud->Touch();
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
  CSfxManager::SfxVolume(mStaticSfxHi, 127);
  CSfxManager::SfxVolume(mStaticSfxLo, 127);
}

void CSamusHud::LeaveFirstPerson(const CStateManager& mgr) {
  CSfxManager::SfxVolume(mStaticSfxHi, 0);
  CSfxManager::SfxVolume(mStaticSfxLo, 0);
}

void CSamusHud::DrawHelmet(const CStateManager& mgr, float camYOff) {
  bool unmorphed = mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed;
  if (mLoadedFrmeHelmet != nullptr && unmorphed && mNextState != kHS_Ball) {
    bool leavingBall = mActiveTransState == kTS_Transitioning && mCurState == kHS_Ball;
    const float alpha = leavingBall ? mTransT : 1.f;
    const CGuiWidgetDrawParms parms(alpha, CVector3f(0.f, 15.f * camYOff, 0.f));
    mLoadedFrmeHelmet->Draw(parms);
  }
}

void CSamusHud::Draw(const CStateManager& mgr, float alpha, uint helmetVis, bool hudVis,
                     bool targetingManager) const {
  if (mNextState == kHS_None) {
    return;
  }
  const CPlayer::EPlayerMorphBallState ballState = mgr.GetPlayer()->GetMorphballTransitionState();
  mCamFilter.Draw();
  if (ballState == CPlayer::kMS_Unmorphed) {
    DrawAttachedEnemyEffect(mgr);
    mCamFilter2.Draw();
    if (targetingManager) {
      mTargetingMgr.Draw(mgr, false);
    }
  }
  CStopwatch timer;
  u64 currentTime = 0;
  for (int i = 0; i < 15; ++i) {
    mProfileInfo[i].mDrawUsec = 0;
  }
  if (helmetVis != 0) {
    const u64 previousTime = currentTime;
    currentTime = timer.GetElapsedMicros();
    mProfileInfo[11].mDrawUsec = currentTime - previousTime;
    if (helmetVis < 5) {
      if (alpha < 1.f) {
        CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_NoColor,
                                      CCameraFilterPass::kFS_CookieCutterDepthRandomStatic,
                                      CColor::White(), nullptr, 1.f - alpha);
      }
      if (mLoadedSelectedHud != nullptr) {
        if (mgr.GetPlayer()->GetDeathTime() > 0.f) {
          if (mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Unmorphed) {
            const CGuiWidgetDrawParms parms(
                mTransT *
                    CMath::Clamp(0.f, 1.f - mgr.GetPlayer()->GetDeathTime() / gkBallDeathTime, 1.f),
                CVector3f::Zero());
            mLoadedSelectedHud->Draw(parms);
          } else {
            const CGuiWidgetDrawParms parms(mTransT, CVector3f::Zero());
            mLoadedSelectedHud->Draw(parms);
          }
        } else {
          const CGuiWidgetDrawParms parms(mTransT, CVector3f::Zero());
          mLoadedSelectedHud->Draw(parms);
        }
        const u64 previousTime = currentTime;
        currentTime = timer.GetElapsedMicros();
        mProfileInfo[5].mDrawUsec = currentTime - previousTime;
      }
      if (mLoadedFrmeBaseHud != nullptr) {
        mLoadedFrmeBaseHud->Draw(CGuiWidgetDrawParms::sDefaultDrawParms);
        const u64 previousTime = currentTime;
        currentTime = timer.GetElapsedMicros();
        mProfileInfo[6].mDrawUsec = currentTime - previousTime;
      }
    }
  }
  if (!mDecoIntf.null() && mPreLoadCountdown == 0) {
    mDecoIntf->Draw();
    const u64 previousTime = currentTime;
    currentTime = timer.GetElapsedMicros();
    mProfileInfo[12].mDrawUsec = currentTime - previousTime;
  }
  bool firstPerson = mNextState >= kHS_Combat && mNextState <= kHS_Scan;
  if (firstPerson) {
    if (hudVis && helmetVis != 0 && helmetVis < 5) {
      float t = mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Combat
                    ? mgr.GetPlayerState()->GetVisorTransitionFactor()
                    : 0.f;
      mRadarIntf->Draw(mgr, alpha * t);
      const u64 previousTime = currentTime;
      currentTime = timer.GetElapsedMicros();
      mProfileInfo[0].mDrawUsec = currentTime - previousTime;
    }
    gpRender->SetDepthReadWrite(true, true);
  }
  mProfileInfo[14].mDrawUsec = timer.GetElapsedMicros();
}

void CSamusHud::ProcessControllerInput(const CFinalInput& input) {
  if (!mDecoIntf.null()) {
    mDecoIntf->ProcessInput(input);
  }
}

const CTargetingManager& CSamusHud::GetTargetingManager() const { return mTargetingMgr; }

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
  mHudDamagePracticalsGain =
      hudDamagePracticalsGainLinear * damage + gpTweakGui->GetHudDamagePracticalsGainConstant();
  const float hudDamagePracticalsInitLinear = gpTweakGui->GetHudDamagePracticalsInitLinear();
  mHudDamagePracticalsInit =
      rstl::max_val(kDamagePracticalsEpsilon, hudDamagePracticalsInitLinear * damage +
                                                  gpTweakGui->GetHudDamagePracticalsInitConstant());
  mHudDamagePracticals = mHudDamagePracticalsInit;
  if (mDamageLight != nullptr) {
    mDamageLight->SetO2PTransform(mLightTransforms[direction]);
  }
  const float hudDamageFilterGainLinear = gpTweakGui->GetHudDamageFilterGainLinear();
  mDamageFilterAmtGain =
      hudDamageFilterGainLinear * damage + gpTweakGui->GetHudDamageFilterGainConstant();
  const float hudDamageFilterInitLinear = gpTweakGui->GetHudDamageFilterInitLinear();
  mDamageFilterAmtInit =
      hudDamageFilterInitLinear * damage + gpTweakGui->GetHudDamageFilterInitConstant();
  mDamageFilterAmt = mDamageFilterAmtInit;
  if (!mDamageSfx) {
    mDamageSfx =
        CSfxManager::AddEmitter(SFXsam_r_damage_lp_00, mgr.GetPlayer()->GetTranslation(),
                                mgr.GetPlayer()->GetTransform().GetForward(), false, true,
                                CSfxManager::kMaxPriority, CSfxManager::kAllAreas);
  }
  if (camera != nullptr) {
    const CVector3f cameraToDamage = camera->GetTransform().GetQuickInverse() * position;
    mDecoShakeTranslateAmtVel =
        gpTweakGui->GetHudDecoShakeTranslateVelConstant() +
        gpTweakGui->GetHudDecoShakeTranslateVelLinear() * previousDamage;
    mDecoShakeTranslateAmt = mDecoShakeTranslateAmtVel;
    mDamagerToPlayerNorm = -1.f * cameraToDamage.AsNormalized();
    const float decoShakeGainLinear = gpTweakGui->GetDecoShakeGainLinear();
    mDecoShakeAmtGain =
        decoShakeGainLinear * previousDamage + gpTweakGui->GetDecoShakeGainConstant();
    const float decoShakeInitLinear = gpTweakGui->GetDecoShakeInitLinear();
    mDecoShakeAmtInit =
        decoShakeInitLinear * previousDamage + gpTweakGui->GetDecoShakeInitConstant();
    mDecoShakeAmt = mDecoShakeAmtInit;
  }
}

void CSamusHud::UpdateHudLag(float dt, const CStateManager& mgr) {
  if (!mDecoIntf.null()) {
    const float yaw = mgr.GetPlayer()->GetYaw();
    mDecoIntf->SetDecoRotation(yaw);
  }
  if (!gpGameState->GameOptions().GetHUDLag()) {
    if (!mHelmetIntf.null()) {
      mHelmetIntf->SetHudLagRotation(CMatrix3f::Identity());
      mHelmetIntf->SetHudLagOffset(CVector3f::Zero());
    }
    if (!mDecoIntf.null()) {
      mDecoIntf->SetReticuleTransform(CMatrix3f::Identity());
      mDecoIntf->SetHudRotation(CQuaternion::NoRotation());
      mDecoIntf->SetHudOffset(CVector3f::Zero());
    }
    mBase_basewidget_pivot->SetIdleXform(
        CTransform4f(CMatrix3f::Identity(), mBase_basewidget_pivot->GetWorldPosition()));
    mLoadedFrmeBaseHud->GetFrameCamera()->SetO2WTransform(BuildFinalCameraTransform(
        CQuaternion::NoRotation(), mBasewidgetIdlePos, mCameraPos));
    mTargetingMgr.CompoundTargetReticle().SetLeadingOrientation(CQuaternion::NoRotation());
  } else {
    CUnitVector3f cameraDirection(mFpCamDirA, CUnitVector3f::kN_No);
    if (const CFirstPersonCamera* camera =
            TCastToConstPtr< CFirstPersonCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr))) {
      const CMatrix3f rotation = camera->GetTransform().BuildMatrix3f();
      const CVector3f direction = rotation.GetColumn(1);
      cameraDirection.SetY(direction.GetY());
      cameraDirection.SetX(direction.GetX());
      cameraDirection.SetZ(direction.GetZ());
    }
    ApplyClassicLag(cameraDirection, mHudLag, mgr, dt, false);
    ApplyClassicLag(cameraDirection, mInvHudLag, mgr, dt, true);
    CQuaternion rotation =
        CQuaternion::LookAt(CUnitVector3f(mFpCamDirA), cameraDirection, CRelAngle(2.f * M_PIF));
    rotation *= rotation;
    rotation *= rotation;
    mTargetingMgr.CompoundTargetReticle().SetLeadingOrientation(rotation);
    const CVector3f bob = mgr.GetPlayer()->GetCameraBobObject()->GetHelmetBobTranslation();
    const CVector3f lagOffset = CVector3f(0.f, 0.f, bob.GetZ()) +
                                gpTweakGui->GetHudLagOffsetScale() * mDecoShakeTranslate;
    const CQuaternion lagRotation = mHudLagShakeRot * mHudLag;
    if (!mHelmetIntf.null()) {
      mHelmetIntf->SetHudLagRotation(lagRotation.BuildTransform());
      mHelmetIntf->SetHudLagOffset(lagOffset);
    }
    if (!mDecoIntf.null()) {
      mDecoIntf->SetReticuleTransform(mInvHudLag.BuildTransform());
      mDecoIntf->SetHudRotation(lagRotation);
      mDecoIntf->SetHudOffset(lagOffset);
    }
    mLoadedFrmeBaseHud->GetFrameCamera()->SetO2WTransform(
        BuildFinalCameraTransform(lagRotation, mBasewidgetIdlePos + lagOffset, mCameraPos));
    mFpCamDirA = cameraDirection;
  }
}

void CSamusHud::ApplyClassicLag(const CUnitVector3f& lookDir, CQuaternion& rotation,
                                const CStateManager& mgr, float dt, bool invert) {
  const CQuaternion lookRotation = CQuaternion::LookAt(
      CUnitVector3f(lookDir), CUnitVector3f(CVector3f::Forward()), CRelAngle(2.f * M_PIF));
  CQuaternion targetRotation =
      invert ? CQuaternion::LookAt(CUnitVector3f(lookRotation.Transform(mFpCamDirA)),
                                   CUnitVector3f(CVector3f::Forward()), CRelAngle(2.f * M_PIF))
             : CQuaternion::LookAt(CUnitVector3f(CVector3f::Forward()),
                                   CUnitVector3f(lookRotation.Transform(mFpCamDirA)),
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
  const float fov = mCamFovTweaks[gpTweakGui->GetHudCamFovTweak()];
  const float y = mCamYTweaks[gpTweakGui->GetHudCamYTweak()];
  const float z = mCamZTweaks[gpTweakGui->GetHudCamZTweak()];
  if (!mHelmetIntf.null()) {
    mHelmetIntf->UpdateCameraDebugSettings(fov, y, z);
  }
  if (!mDecoIntf.null()) {
    mDecoIntf->UpdateCameraDebugSettings(fov, y, z);
  }
  CGuiCamera* camera = mLoadedFrmeBaseHud->GetFrameCamera();
  CGuiCamera::UCameraParms parms = camera->GetParms();
  parms.perspective.fov = fov;
  camera->SetParms(parms);
  mCameraPos = CVector3f(0.f, y, z);
}

void CSamusHud::SetMessage(const rstl::wstring& text, const CHUDMemoParms& info) {
  bool visible = mBase_basewidget_message->GetIsVisible();
  if (!visible || info.IsHintMemo()) {
    if (info.IsFadeOutOnly()) {
      mMessageTextTime = 1.f;
      if (info.IsHintMemo() && visible) {
        CSfxManager::SfxStart(SFXui_x_hintoff_00, 127, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
      }
      return;
    }
    mBase_basewidget_message->SetColor(CColor::White());
    mBase_basewidget_message->SetVisibility(false, kTM_Children);
    CGuiWidget* pane =
        info.IsHintMemo() ? mBase_basewidget_message : mBase_textpane_message;
    pane->SetVisibility(true, kTM_Children);
    mBase_textpane_message->TextSupport().SetTypeWriteEffectOptions(true, 0.1f, 40.f);
    if (info.IsClearMemoWindow()) {
      mLastSfxChars = 0.f;
      mBase_textpane_message->TextSupport().SetCurTime(0.f);
      mBase_textpane_message->TextSupport().SetText(text);
    } else if (static_cast< int >(mBase_textpane_message->TextSupport().GetText().size()) ==
               0) {
      mLastSfxChars = 0.f;
      mBase_textpane_message->TextSupport().AddText(text);
    } else {
      mBase_textpane_message->TextSupport().AddText(rstl::wstring_l(L"\n") + text);
    }
    mBase_textpane_message->SetColor(CColor::White());
    mBase_basewidget_message->SetColor(CColor::White());
    mMessageTextTime = info.GetDisplayTime();
    if (info.IsHintMemo()) {
      if (!visible) {
        mAbuttonPulse = 0.f;
        mMessageTextScale = 0.f;
        CSfxManager::SfxStart(SFXui_x_hinton_00, 127, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
      }
    } else {
      mBase_basewidget_message->SetO2PTransform(mBase_basewidget_message->GetTransform());
    }
  }
}
