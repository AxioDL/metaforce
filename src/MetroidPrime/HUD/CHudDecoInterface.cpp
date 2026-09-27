#include "MetroidPrime/HUD/CHudDecoInterface.hpp"

#include "GuiSys/CAuiEnergyBarT01.hpp"
#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidget.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/math.hpp"

static const char skDecoWidgetName[] = "basewidget_deco";
static const char skBarDecoWidgetName[] = "basewidget_tickdeco0";
static const char skPivotWidgetName[] = "basewidget_pivot";
static const char skFrameNormalGroupWidgetName[] = "basewidget_frame";

static const char skSeekerWidgetNameXRay[] = "basewidget_seeker";

static const char skScanGroupLeftName[] = "basewidget_leftside";
static const char skScanPaneLeftName[] = "basewidget_leftguages";
static const char skScanPaneDecoLeftName[] = "basewidget_databankl";
static const char skScanGroupRightName[] = "basewidget_rightside";
static const char skScanPaneRightName[] = "basewidget_rightguages";
static const char skScanPaneDecoRightName[] = "basewidget_databankr";
static const char skScanLeftSideDecoName[] = "basewidget_leftsidedeco";
static const char skScanRightSideDecoName[] = "basewidget_rightsidedeco";
static const char skScanMessageName[] = "textpane_scanning";
static const char skScanBarName[] = "energybart01_scanbar";
static const char skScanGroupName[] = "basewidget_scanguage";
static const char skScanMessageGroupBaseName[] = "basewidget_textgroup";
static const char skScanTextPaneBaseName[] = "textpane_message";
static const char skScanScrollMessageName[] = "textpane_scrollmessage";
static const char skScanXMarkName[] = "model_xmark";
static const char skScanAButtonName[] = "model_abutton";
static const char skScanDashName[] = "model_dash";

CHudDecoInterfaceCombat::~CHudDecoInterfaceCombat() {}

CHudDecoInterfaceXRay::~CHudDecoInterfaceXRay() {}

CHudDecoInterfaceScan::~CHudDecoInterfaceScan() {}

CHudDecoInterfaceCombat::CHudDecoInterfaceCombat(CGuiFrame& hud)
: mRotation(CQuaternion::NoRotation())
, mPivotPosition(CVector3f::Zero())
, mOffset(CVector3f::Zero())
, mCamPos(CVector3f::Zero())
, mBasePosition(CVector3f::Zero())
, mBaseRotation(CMatrix3f::Identity())
, mVisDebug(true)
, mVisGame(true) {
  mCamera = hud.GetFrameCamera();
  mCamPos = mCamera->GetLocalPosition();
  mBasewidget_pivot = hud.FindWidget(skPivotWidgetName);
  mBasewidget_deco = hud.FindWidget(skDecoWidgetName);
  mBasewidget_tickdeco0 = hud.FindWidget(skBarDecoWidgetName);
  mBasewidget_frame = hud.FindWidget(skFrameNormalGroupWidgetName);
  mPivotPosition = mBasewidget_pivot->GetIdleXform().GetTranslation();
  mBasewidget_tickdeco0->SetColor(gpTweakGuiColors->GetTickDecoColor());
  mBasePosition = mBasewidget_frame->GetO2PTransform().GetTranslation();
  mBaseRotation = mBasewidget_frame->GetO2PTransform().BuildMatrix3f();
  UpdateHudAlpha();
}

void CHudDecoInterfaceCombat::SetIsVisibleDebug(const bool visible) {
  mVisDebug = visible;
  UpdateVisibility();
}

void CHudDecoInterfaceCombat::SetIsVisibleGame(bool visible) {
  mVisGame = visible;
  UpdateVisibility();
}

void CHudDecoInterfaceCombat::UpdateHudAlpha() {
  mBasewidget_pivot->SetColor(
      CColor(uchar(255), uchar(255), uchar(255), uchar(gpGameState->GameOptions().GetHUDAlpha())));
}

void CHudDecoInterfaceCombat::UpdateVisibility() {
  const bool visible = mVisDebug && mVisGame;
  mBasewidget_deco->SetVisibility(visible, kTM_Children);
  mBasewidget_tickdeco0->SetVisibility(visible, kTM_Children);
}

void CHudDecoInterfaceCombat::SetHudRotation(const CQuaternion& rotation) {
  mRotation = rotation;
}

void CHudDecoInterfaceCombat::SetHudOffset(const CVector3f& offset) { mOffset = offset; }

void CHudDecoInterfaceCombat::SetDamageTransform(const CMatrix3f& rotation,
                                                 const CVector3f& position) {
  mBasewidget_frame->SetO2PTransform(
      CTransform4f(rotation * mBaseRotation, mBasePosition + position));
}

void CHudDecoInterfaceCombat::SetFrameColorValue(float value) {
  const CColor color = value > 0.f ? CColor::White() : gpTweakGuiColors->GetHudFrameColor();
  mBasewidget_frame->SetColor(color);
}

void CHudDecoInterfaceCombat::Update(float dt, const CStateManager& mgr) {
  mCamera->SetO2WTransform(CSamusHud::BuildFinalCameraTransform(
      mRotation, mPivotPosition + mOffset, mCamPos));
}

void CHudDecoInterfaceCombat::UpdateCameraDebugSettings(float fov, float y, float z) {
  CGuiCamera::UCameraParms parms = mCamera->GetParms();
  parms.perspective.fov = fov;
  mCamera->SetParms(parms);
  mCamPos = CVector3f(0.f, y, z);
}

CHudDecoInterfaceXRay::CHudDecoInterfaceXRay(CGuiFrame& hud)
: mSeekerScale(1.f)
, mRotation(CQuaternion::NoRotation())
, mPivotPosition(CVector3f::Zero())
, mOffset(CVector3f::Zero())
, mCamPos(CVector3f::Zero())
, mReticuleXf(CMatrix3f::Identity())
, mSeekerPosition(CVector3f::Zero())
, x6c_(CVector3f::Zero())
, x78_(CMatrix3f::Identity())
, mVisDebug(true)
, mVisGame(true) {
  mCamera = hud.GetFrameCamera();
  mCamPos = mCamera->GetLocalPosition();
  mBasewidget_pivot = hud.FindWidget(skPivotWidgetName);
  mBasewidget_seeker = hud.FindWidget(skSeekerWidgetNameXRay);
  mBasewidget_rotate = hud.FindWidget("basewidget_rotate");
  mPivotPosition = mBasewidget_pivot->GetIdleXform().GetTranslation();
  mSeekerPosition = mBasewidget_seeker->GetIdleXform().GetTranslation();
  if (CGuiWidget* widget = hud.FindWidget("basewidget_energydeco")) {
    widget->SetColor(gpTweakGuiColors->GetXRayEnergyDecoColor());
  }
  if (CGuiWidget* widget = hud.FindWidget("model_frame")) {
    widget->SetIsAlwaysDepthWrite(true);
  }
  if (CGuiWidget* widget = hud.FindWidget("model_frame1")) {
    widget->SetIsAlwaysDepthWrite(true);
  }
  if (CGuiWidget* widget = hud.FindWidget("model_frame2")) {
    widget->SetIsAlwaysDepthWrite(true);
  }
  if (CGuiWidget* widget = hud.FindWidget("model_frame3")) {
    widget->SetIsAlwaysDepthWrite(true);
  }
  if (CGuiWidget* widget = hud.FindWidget("model_misslieslider")) {
    widget->SetIsAlwaysDepthWrite(true);
  }
  if (CGuiWidget* widget = hud.FindWidget("model_threatslider")) {
    widget->SetIsAlwaysDepthWrite(true);
  }
  UpdateHudAlpha();
}

void CHudDecoInterfaceXRay::UpdateHudAlpha() {
  mBasewidget_pivot->SetColor(
      CColor(uchar(255), uchar(255), uchar(255), uchar(gpGameState->GameOptions().GetHUDAlpha())));
}

void CHudDecoInterfaceXRay::SetIsVisibleDebug(const bool visible) {
  mVisDebug = visible;
  UpdateVisibility();
}

void CHudDecoInterfaceXRay::SetIsVisibleGame(bool visible) {
  mVisGame = visible;
  UpdateVisibility();
}

void CHudDecoInterfaceXRay::UpdateVisibility() {}

void CHudDecoInterfaceXRay::SetHudRotation(const CQuaternion& rotation) { mRotation = rotation; }

void CHudDecoInterfaceXRay::SetReticuleTransform(const CMatrix3f& transform) {
  mReticuleXf = transform;
}

void CHudDecoInterfaceXRay::SetDecoRotation(float angle) {
  mBasewidget_rotate->SetO2PTransform(
      CTransform4f(CMatrix3f::RotateY(CRelAngle::FromRadians(angle)),
                   mBasewidget_rotate->GetLocalPosition()));
}

void CHudDecoInterfaceXRay::SetHudOffset(const CVector3f& offset) { mOffset = offset; }

void CHudDecoInterfaceXRay::SetDamageTransform(const CMatrix3f& rotation,
                                               const CVector3f& position) {}

void CHudDecoInterfaceXRay::SetFrameColorValue(float value) {}

void CHudDecoInterfaceXRay::Update(float dt, const CStateManager& mgr) {
  static const float kMinSeekerScale = 0.35f;
  if (mgr.GetPlayer()->GetOrbitState() == CPlayer::kOS_OrbitObject) {
    mSeekerScale = rstl::max_val(kMinSeekerScale, mSeekerScale - 3.f * dt);
  } else {
    mSeekerScale = rstl::min_val(1.f, 3.f * dt + mSeekerScale);
  }
  mCamera->SetO2WTransform(CSamusHud::BuildFinalCameraTransform(
      mRotation, mPivotPosition + mOffset, mCamPos));
  mBasewidget_seeker->SetO2PTransform(
      CTransform4f(CMatrix3f::Scale(mSeekerScale) * mReticuleXf, mSeekerPosition));
}

void CHudDecoInterfaceXRay::UpdateCameraDebugSettings(float fov, float y, float z) {
  CGuiCamera::UCameraParms parms = mCamera->GetParms();
  parms.perspective.fov = fov;
  mCamera->SetParms(parms);
  mCamPos = CVector3f(0.f, y, z);
}

CScanDisplay::~CScanDisplay() {}

CHudDecoInterfaceScan::CHudDecoInterfaceScan(CGuiFrame& hud)
: mScanHudFlat(gpSimplePool->GetObj("FRME_ScanHudFlat"))
, mLoadedScanHudFlat(nullptr)
, mSelHud(hud)
, mScanDisplay(&hud)
, mLatestHudPoi(kInvalidUniqueId)
, mLatestScanningObject(kInvalidUniqueId)
, mLatestScanState(CPlayer::kSS_NotScanning)
, mScanningTime(0.f)
, x1dc_(0.f)
, x1e0_(1.f)
, mRotation(CQuaternion::NoRotation())
, mPivotPosition(CVector3f::Zero())
, mOffset(CVector3f::Zero())
, mCamPos(CVector3f::Zero())
, mLeftsidePosition(CVector3f::Zero())
, mRightsidePosition(CVector3f::Zero())
, mSidesTimer(0.f)
, mSidesPositioner(gpTweakGui->GetScanSidesPositionStart())
, mScanningTextAlpha(0.f)
, mScanBarAlpha(0.f)
, mVisDebug(true)
, mVisGame(true) {
  mScanHudFlat.Lock();
  mCamera = hud.GetFrameCamera();
  mCamPos = mCamera->GetLocalPosition();
  mBasewidget_pivot = hud.FindWidget(skPivotWidgetName);
  mBasewidget_leftside = hud.FindWidget(skScanGroupLeftName);
  mBasewidget_rightside = hud.FindWidget(skScanGroupRightName);
  mPivotPosition = mBasewidget_pivot->GetIdleXform().GetTranslation();
  if (CGuiWidget* widget = hud.FindWidget("basewidget_deco")) {
    widget->SetColor(gpTweakGuiColors->GetHudFrameColor());
  }
  mLeftsidePosition = mBasewidget_leftside->GetO2PTransform().GetTranslation();
  mBasewidget_leftside->SetO2PTransform(
      CTransform4f(CMatrix3f::RotateZ(CRelAngle::FromRadians(gpTweakGui->GetScanSidesAngle())),
                   mLeftsidePosition));
  if (CGuiWidget* widget = hud.FindWidget(skScanPaneDecoLeftName)) {
    const CVector3f position = widget->GetO2PTransform().GetTranslation();
    widget->SetO2PTransform(CTransform4f(
        CMatrix3f::RotateZ(CRelAngle::FromRadians(-1.f * gpTweakGui->GetScanSidesAngle())),
        position));
  }
  if (CGuiWidget* widget = hud.FindWidget(skScanPaneLeftName)) {
    const CVector3f position = widget->GetO2PTransform().GetTranslation();
    widget->SetO2PTransform(
        CTransform4f(CMatrix3f::Scale(gpTweakGui->GetScanSidesXScale(), 1.f, 1.f), position));
  }
  mRightsidePosition = mBasewidget_rightside->GetO2PTransform().GetTranslation();
  mBasewidget_rightside->SetO2PTransform(CTransform4f(
      CMatrix3f::RotateZ(CRelAngle::FromRadians(-1.f * gpTweakGui->GetScanSidesAngle())),
      mRightsidePosition));
  if (CGuiWidget* widget = hud.FindWidget(skScanPaneDecoRightName)) {
    const CVector3f position = widget->GetO2PTransform().GetTranslation();
    widget->SetO2PTransform(CTransform4f(
        CMatrix3f::RotateZ(CRelAngle::FromRadians(gpTweakGui->GetScanSidesAngle())), position));
  }
  if (CGuiWidget* widget = hud.FindWidget(skScanPaneRightName)) {
    const CVector3f position = widget->GetO2PTransform().GetTranslation();
    widget->SetO2PTransform(
        CTransform4f(CMatrix3f::Scale(gpTweakGui->GetScanSidesXScale(), 1.f, 1.f), position));
  }
  const CVector3f sidesPos(mSidesPositioner, 0.f, 0.f);
  mBasewidget_leftside->SetLocalPosition(mLeftsidePosition +
                                             mBasewidget_leftside->RotateO2P(sidesPos));
  mBasewidget_rightside->SetLocalPosition(mRightsidePosition -
                                              mBasewidget_rightside->RotateO2P(sidesPos));
  UpdateHudAlpha();
}

void CHudDecoInterfaceScan::UpdateHudAlpha() {
  mBasewidget_pivot->SetColor(
      CColor::White().WithAlphaOf(gpGameState->GameOptions().GetHudAlpha()));
}

void CHudDecoInterfaceScan::SetIsVisibleDebug(const bool visible) {
  mVisDebug = visible;
  UpdateVisibility();
}

void CHudDecoInterfaceScan::SetIsVisibleGame(bool visible) {
  mVisGame = visible;
  UpdateVisibility();
}

void CHudDecoInterfaceScan::UpdateVisibility() {}

void CHudDecoInterfaceScan::SetHudRotation(const CQuaternion& rotation) {
  mRotation = rotation;
}

void CHudDecoInterfaceScan::SetReticuleTransform(const CMatrix3f& transform) {}

void CHudDecoInterfaceScan::SetHudOffset(const CVector3f& offset) { mOffset = offset; }

void CHudDecoInterfaceScan::SetDamageTransform(const CMatrix3f& rotation,
                                               const CVector3f& position) {}

void CHudDecoInterfaceScan::SetFrameColorValue(float value) {}

void CHudDecoInterfaceScan::InitializeFlatFrame() {
  mLoadedScanHudFlat = mScanHudFlat.GetObject();
  CGuiCamera* camera = mLoadedScanHudFlat->GetFrameCamera();
  CGuiCamera::UCameraParms parms = camera->GetParms();
  parms.perspective.fov = mCamera->GetParms().perspective.fov;
  camera->SetParms(parms);
  camera->SetO2WTransform(CTransform4f::Translate(mCamPos));
  mFlat_basewidget_scanguage = mLoadedScanHudFlat->FindWidget(skScanGroupName);
  mFlat_basewidget_scanguage->SetVisibility(false, kTM_Children);
  mFlat_textpane_scanning =
      static_cast< CGuiTextPane* >(mLoadedScanHudFlat->FindWidget(skScanMessageName));
  mFlat_energybart01_scanbar =
      static_cast< CAuiEnergyBarT01* >(mLoadedScanHudFlat->FindWidget(skScanBarName));
  mFlat_textpane_message =
      static_cast< CGuiTextPane* >(mLoadedScanHudFlat->FindWidget(skScanTextPaneBaseName));
  mFlat_textpane_scrollmessage =
      static_cast< CGuiTextPane* >(mLoadedScanHudFlat->FindWidget(skScanScrollMessageName));
  mFlat_basewidget_textgroup = mLoadedScanHudFlat->FindWidget(skScanMessageGroupBaseName);
  mFlat_model_xmark =
      static_cast< CGuiModel* >(mLoadedScanHudFlat->FindWidget(skScanXMarkName));
  mFlat_model_abutton =
      static_cast< CGuiModel* >(mLoadedScanHudFlat->FindWidget(skScanAButtonName));
  mFlat_model_dash =
      static_cast< CGuiModel* >(mLoadedScanHudFlat->FindWidget(skScanDashName));
  mFlat_basewidget_textgroup->SetVisibility(false, kTM_Children);
  mFlat_textpane_scanning->SetIsVisible(false);
  mFlat_textpane_scanning->TextSupport().SetFontColor(gpTweakGuiColors->GetHudMessageFill());
  mFlat_textpane_scanning->TextSupport().SetOutlineColor(
      gpTweakGuiColors->GetHudMessageOutline());
  mFlat_energybart01_scanbar->SetCoordFunc(CAuiEnergyBarT01::DownloadBarCoordFunc);
  mFlat_energybart01_scanbar->SetTesselation(1.f);
  mFlat_energybart01_scanbar->SetMaxEnergy(1.f);
  mFlat_energybart01_scanbar->SetFilledColor(
      CColor(uchar(103), uchar(174), uchar(225), uchar(255)));
  mFlat_energybart01_scanbar->SetShadowColor(CColor(0));
  mFlat_energybart01_scanbar->SetEmptyColor(CColor(0));
  mFlat_energybart01_scanbar->SetFilledDrainSpeed(999.f);
  mFlat_energybart01_scanbar->SetShadowDrainSpeed(999.f);
  mFlat_energybart01_scanbar->SetShadowDrainDelay(0.f);
  mFlat_energybart01_scanbar->SetIsAlwaysResetTimer(false);
  mFlat_model_xmark->SetVisibility(false, kTM_Children);
  mFlat_model_abutton->SetVisibility(false, kTM_Children);
  mFlat_model_dash->SetVisibility(false, kTM_Children);
}

void CHudDecoInterfaceScan::Update(float dt, const CStateManager& mgr) {
  const CPlayer* player = mgr.GetPlayer();
  const CPlayer::EPlayerScanState scanState = player->GetPlayerScanState();
  if (scanState != CPlayer::kSS_NotScanning) {
    mScanningTime = player->GetScanTimer();
  }
  if (scanState == CPlayer::kSS_Scanning || scanState == CPlayer::kSS_ScanComplete) {
    mSidesTimer = rstl::min_val(gpTweakGui->GetScanSidesEndTime(), mSidesTimer + dt);
  } else {
    mSidesTimer = rstl::max_val(0.f, mSidesTimer - dt);
  }
  const float oldSidesPositioner = mSidesPositioner;
  const float sidesTime = mSidesTimer;
  const float sidesStart = gpTweakGui->GetScanSidesStartTime();
  const float sidesDuration = gpTweakGui->GetScanSidesDuration();
  const float sidesT = sidesTime < sidesStart ? 0.f : (sidesTime - sidesStart) / sidesDuration;
  mSidesPositioner = (1.f - sidesT) * gpTweakGui->GetScanSidesPositionStart() +
                         sidesT * gpTweakGui->GetScanSidesPositionEnd();
  if (oldSidesPositioner != mSidesPositioner) {
    const CVector3f sidesPos(mSidesPositioner, 0.f, 0.f);
    mBasewidget_leftside->SetLocalPosition(mLeftsidePosition +
                                               mBasewidget_leftside->RotateO2P(sidesPos));
    mBasewidget_rightside->SetLocalPosition(mRightsidePosition -
                                                mBasewidget_rightside->RotateO2P(sidesPos));
  }
  mCamera->SetO2WTransform(CSamusHud::BuildFinalCameraTransform(
      mRotation, mPivotPosition + mOffset, mCamPos));
  if (mLoadedScanHudFlat == nullptr) {
    if (mScanHudFlat.TryCache() && mScanHudFlat.GetObject()->GetIsFinishedLoading()) {
      InitializeFlatFrame();
    } else {
      return;
    }
  }
  mLoadedScanHudFlat->Update(dt);
  UpdateScanDisplay(mgr, dt);
}

void CHudDecoInterfaceScan::Draw() const {
  mScanDisplay.Draw();
  if (mLoadedScanHudFlat != nullptr) {
    mLoadedScanHudFlat->Draw(CGuiWidgetDrawParms::Default());
  }
}

void CHudDecoInterfaceScan::ProcessInput(const CFinalInput& input) {
  mScanDisplay.ProcessInput(input);
}

void CHudDecoInterfaceScan::UpdateCameraDebugSettings(float fov, float y, float z) {
  CGuiCamera::UCameraParms parms = mCamera->GetParms();
  parms.perspective.fov = fov;
  mCamera->SetParms(parms);
  mCamPos = CVector3f(0.f, y, z);
}

void CHudDecoInterfaceScan::UpdateScanDisplay(const CStateManager& mgr, float dt) {
  const CPlayer* player = mgr.GetPlayer();
  const CPlayer::EPlayerScanState scanState = player->GetPlayerScanState();
  const TUniqueId orbitTarget = player->GetOrbitTargetId();
  const TUniqueId scanningObject = player->GetScanningObjectId();
  const float scanningTime = player->GetScanTimer();
  if (scanState != mLatestScanState) {
    if (player->IsNewScanScanning()) {
      if (scanState == CPlayer::kSS_ScanComplete) {
        if (mLatestScanState == CPlayer::kSS_Scanning) {
          mFlat_textpane_scanning->TextSupport().SetText(
              rstl::wstring_l(gpStringTable->GetString(15)));
          mFlat_textpane_scanning->TextSupport().SetTypeWriteEffectOptions(false, 0.f, 40.f);
          mScanningTextAlpha = 2.f;
        }
      } else if (scanState == CPlayer::kSS_Scanning) {
        mFlat_textpane_scanning->TextSupport().SetText(
            rstl::wstring_l(gpStringTable->GetString(14)));
        mFlat_textpane_scanning->TextSupport().SetTypeWriteEffectOptions(false, 0.f, 40.f);
        mScanningTextAlpha = 1.f;
      }
    }
    mLatestScanState = scanState;
  }
  if (scanningObject != mLatestScanningObject) {
    mLatestScanningObject = scanningObject;
  }
  if (orbitTarget != mLatestHudPoi) {
    mLatestHudPoi = orbitTarget;
    if (orbitTarget != kInvalidUniqueId) {
      if (!const_cast< CPlayer* >(mgr.GetPlayer())->ObjectInScanningRange(orbitTarget, mgr)) {
        mFlat_textpane_scanning->TextSupport().SetText(rstl::wstring_l(L""));
        mFlat_textpane_scanning->TextSupport().SetText(
            rstl::wstring_l(gpStringTable->GetString(16)));
        mFlat_textpane_scanning->TextSupport().SetTypeWriteEffectOptions(true, 0.f, 40.f);
        mScanningTextAlpha = 1.f;
      }
    }
  }
  const CScannableObjectInfo* scanInfo = GetCurrScanInfo(mgr);
  if (mLatestScanningObject != mScanDisplay.GetScanTarget() || scanInfo == nullptr) {
    mScanDisplay.StopScan();
    if (mScanDisplay.GetScanState() == CScanDisplay::kSS_Inactive && scanInfo != nullptr) {
      mScanDisplay.StartScan(mLatestScanningObject, *scanInfo, mFlat_textpane_message,
                                mFlat_textpane_scrollmessage, mFlat_basewidget_textgroup,
                                mFlat_model_xmark, mFlat_model_abutton,
                                mFlat_model_dash, scanningTime);
    }
  }
  mScanDisplay.Update(dt, scanningTime);
  if (mLatestScanningObject != kInvalidUniqueId && GetCurrScanInfo(mgr) != nullptr) {
    if (const CActor* actor =
            TCastToConstPtr< CActor >(mgr.GetObjectById(mLatestScanningObject))) {
      if (const CScannableObjectInfo* actorScan = actor->GetScannableObjectInfo()) {
        mFlat_energybart01_scanbar->SetCurrEnergy(
            mScanningTime / actorScan->GetTotalDownloadTime(), CAuiEnergyBarT01::kSM_Normal);
      }
    }
  }
  if (mLatestScanState != CPlayer::kSS_Scanning) {
    if (mLatestHudPoi == kInvalidUniqueId ||
        const_cast< CPlayer* >(mgr.GetPlayer())->ObjectInScanningRange(mLatestHudPoi, mgr)) {
      mScanningTextAlpha = rstl::max_val(0.f, mScanningTextAlpha - dt);
    }
  }
  if (mScanningTextAlpha > 0.f) {
    mFlat_textpane_scanning->SetColor(
        CColor::White().WithAlphaOf(rstl::min_val(1.f, mScanningTextAlpha)));
    mFlat_textpane_scanning->SetIsVisible(true);
  } else {
    mFlat_textpane_scanning->SetIsVisible(false);
  }
  if (GetCurrScanInfo(mgr) != nullptr) {
    mScanBarAlpha = rstl::min_val(1.f, mScanBarAlpha + 2.f * dt);
  } else {
    mScanBarAlpha = rstl::max_val(0.f, mScanBarAlpha - 2.f * dt);
  }
  if (mScanBarAlpha > 0.f) {
    mFlat_basewidget_scanguage->SetColor(
        CColor::White().WithAlphaOf(rstl::min_val(1.f, mScanBarAlpha)));
    mFlat_basewidget_scanguage->SetVisibility(true, kTM_Children);
  } else {
    mFlat_basewidget_scanguage->SetVisibility(false, kTM_Children);
  }
}

const CScannableObjectInfo* CHudDecoInterfaceScan::GetCurrScanInfo(const CStateManager& mgr) const {
  if (mLatestScanState == CPlayer::kSS_NotScanning) {
    return nullptr;
  }
  if (const CActor* actor =
          TCastToConstPtr< CActor >(mgr.GetObjectById(mLatestScanningObject))) {
    return actor->GetScannableObjectInfo();
  }
  return nullptr;
}

float CHudDecoInterfaceScan::GetMessageTextAlpha() const {
  const float scanningAlpha = rstl::min_val(1.f, mScanningTextAlpha);
  return 1.f - rstl::max_val(scanningAlpha, mScanDisplay.GetBodyAlpha());
}

CHudDecoInterfaceThermal::SLockWidget::SLockWidget(CGuiWidget* widget)
: mWidget(widget), mTransform(widget->GetO2PTransform()) {}

CHudDecoInterfaceThermal::CHudDecoInterfaceThermal(CGuiFrame& hud)
: mRotation(CQuaternion::NoRotation())
, mPivotPosition(CVector3f::Zero())
, mOffset(CVector3f::Zero())
, mCamPos(CVector3f::Zero())
, mReticuleXf(CMatrix3f::Identity())
, mReticulePosition(CVector3f::Zero())
, mLockonScale(5.f)
, mRetflashTimer(0.f)
, mVisDebug(true)
, mVisGame(true) {
  mCamera = hud.GetFrameCamera();
  mCamPos = mCamera->GetLocalPosition();
  mBasewidget_pivot = hud.FindWidget(skPivotWidgetName);
  const char* const reticleName = "basewidget_reticle";
  mBasewidget_reticle = hud.FindWidget(reticleName);
  mModel_retflash = static_cast< CGuiModel* >(hud.FindWidget("model_retflash"));
  const char* const lockName = "basewidget_lock";
  const char* const lockonName = "basewidget_lockon";
  mPivotPosition = mBasewidget_pivot->GetIdleXform().GetTranslation();
  mReticulePosition = mBasewidget_reticle->GetIdleXform().GetTranslation();
  if (CGuiWidget* widget = hud.FindWidget("basewidget_deco")) {
    widget->SetColor(gpTweakGuiColors->GetThermalDecoColor());
  }
  if (CGuiWidget* widget = hud.FindWidget("basewidget_oultlinesa")) {
    widget->SetColor(gpTweakGuiColors->GetThermalOutlinesColor());
  }
  if (CGuiWidget* widget = hud.FindWidget(lockName)) {
    widget->SetColor(gpTweakGuiColors->GetThermalLockColor());
  }
  if (CGuiWidget* widget = hud.FindWidget(reticleName)) {
    widget->SetColor(gpTweakGuiColors->GetThermalOutlinesColor());
  }
  if (CGuiWidget* widget = hud.FindWidget(lockonName)) {
    widget->SetColor(gpTweakGuiColors->GetThermalOutlinesColor());
  }
  if (CGuiWidget* widget = hud.FindWidget("model_threaticon")) {
    widget->SetColor(gpTweakGuiColors->GetThermalOutlinesColor());
  }
  if (CGuiWidget* widget = hud.FindWidget("model_missileicon")) {
    widget->SetColor(gpTweakGuiColors->GetThermalOutlinesColor());
  }
  if (CGuiWidget* widget = hud.FindWidget(lockName)) {
    for (CGuiWidget* child = static_cast< CGuiWidget* >(widget->ChildObject()); child != nullptr;
         child = static_cast< CGuiWidget* >(child->NextSibling())) {
      mLockonWidgets.reserve(mLockonWidgets.size() + 1);
      mLockonWidgets.push_back(SLockWidget(child));
      child->SetO2PTransform(child->GetO2PTransform() * CTransform4f::Scale(mLockonScale));
    }
  }
  mPivotPosition = mBasewidget_pivot->GetIdleXform().GetTranslation();
  UpdateHudAlpha();
}

void CHudDecoInterfaceThermal::SetIsVisibleDebug(const bool visible) {
  mVisDebug = visible;
  UpdateVisibility();
}

void CHudDecoInterfaceThermal::SetIsVisibleGame(bool visible) {
  mVisGame = visible;
  UpdateVisibility();
}

void CHudDecoInterfaceThermal::UpdateHudAlpha() {
  mBasewidget_pivot->SetColor(
      CColor(uchar(255), uchar(255), uchar(255), uchar(gpGameState->GameOptions().GetHUDAlpha())));
}

void CHudDecoInterfaceThermal::UpdateVisibility() {}

void CHudDecoInterfaceThermal::SetHudRotation(const CQuaternion& rotation) {
  mRotation = rotation;
}

void CHudDecoInterfaceThermal::SetReticuleTransform(const CMatrix3f& transform) {
  mReticuleXf = transform;
}

void CHudDecoInterfaceThermal::SetHudOffset(const CVector3f& offset) { mOffset = offset; }

void CHudDecoInterfaceThermal::SetDamageTransform(const CMatrix3f& rotation,
                                                  const CVector3f& position) {}

void CHudDecoInterfaceThermal::Update(float dt, const CStateManager& mgr) {
  static const float kMaxLockonScale = 5.f;
  const float oldLockonScale = mLockonScale;
  if (mgr.GetPlayer()->GetOrbitTargetId() != kInvalidUniqueId) {
    mLockonScale = rstl::max_val(1.f, mLockonScale - 15.f * dt);
  } else {
    mLockonScale = rstl::min_val(kMaxLockonScale, mLockonScale + 15.f * dt);
  }
  if (oldLockonScale != mLockonScale) {
    for (AUTO(it, mLockonWidgets.begin()); it != mLockonWidgets.end(); ++it) {
      it->mWidget->SetO2PTransform(it->mTransform * CTransform4f::Scale(mLockonScale));
    }
  }
  mRetflashTimer += dt;
  if (mRetflashTimer > 1.f) {
    mRetflashTimer -= 2.f;
  }
  mModel_retflash->SetColor(
      CColor::White().WithAlphaOf(CMath::AbsF(mRetflashTimer) * 0.5f + 0.5f));
  mCamera->SetO2WTransform(CSamusHud::BuildFinalCameraTransform(
      mRotation, mPivotPosition + mOffset, mCamPos));
  mBasewidget_reticle->SetO2PTransform(CTransform4f(mReticuleXf, mReticulePosition));
}

void CHudDecoInterfaceThermal::UpdateCameraDebugSettings(float fov, float y, float z) {
  CGuiCamera::UCameraParms parms = mCamera->GetParms();
  parms.perspective.fov = fov;
  mCamera->SetParms(parms);
  mCamPos = CVector3f(0.f, y, z);
}

CHudDecoInterfaceThermal::~CHudDecoInterfaceThermal() {}
