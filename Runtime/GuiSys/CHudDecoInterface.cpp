#include "Runtime/GuiSys/CHudDecoInterface.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/GuiSys/CAuiEnergyBarT01.hpp"
#include "Runtime/GuiSys/CGuiCamera.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiModel.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"
#include "Runtime/MP1/CSamusHud.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

void IHudDecoInterface::SetReticuleTransform(const zeus::CMatrix3f& xf) {}
void IHudDecoInterface::SetDecoRotation(float angle) {}
void IHudDecoInterface::SetFrameColorValue(float v) {}
void IHudDecoInterface::Draw() {}
void IHudDecoInterface::ProcessInput(const CFinalInput& input) {}
float IHudDecoInterface::GetHudTextAlpha() const { return 1.f; }

CHudDecoInterfaceCombat::CHudDecoInterfaceCombat(CGuiFrame& selHud) {
  x6c_camera = selHud.GetFrameCamera();
  x2c_camPos = x6c_camera->GetLocalPosition();
  x70_basewidget_pivot = selHud.FindWidget("basewidget_pivot");
  x74_basewidget_deco = selHud.FindWidget("basewidget_deco");
  x78_basewidget_tickdeco0 = selHud.FindWidget("basewidget_tickdeco0");
  x7c_basewidget_frame = selHud.FindWidget("basewidget_frame");
  x14_pivotPosition = x70_basewidget_pivot->GetIdlePosition();
  x78_basewidget_tickdeco0->SetColor(g_tweakGuiColors->GetTickDecoColor());
  x38_basePosition = x7c_basewidget_frame->GetLocalPosition();
  x44_baseRotation = x7c_basewidget_frame->GetLocalTransform().buildMatrix3f();
  CHudDecoInterfaceCombat::UpdateHudAlpha();
}

void CHudDecoInterfaceCombat::UpdateVisibility() {
  bool vis = x68_24_visDebug && x68_25_visGame;
  x74_basewidget_deco->SetVisibility(vis, ETraversalMode::Children);
  x78_basewidget_tickdeco0->SetVisibility(vis, ETraversalMode::Children);
}

void CHudDecoInterfaceCombat::SetIsVisibleDebug(bool v) {
  x68_24_visDebug = v;
  UpdateVisibility();
}

void CHudDecoInterfaceCombat::SetIsVisibleGame(bool v) {
  x68_25_visGame = v;
  UpdateVisibility();
}

void CHudDecoInterfaceCombat::SetHudRotation(const zeus::CQuaternion& rot) { x4_rotation = rot; }

void CHudDecoInterfaceCombat::SetHudOffset(const zeus::CVector3f& off) { x20_offset = off; }

void CHudDecoInterfaceCombat::SetDamageTransform(const zeus::CMatrix3f& rotation, const zeus::CVector3f& position) {
  x7c_basewidget_frame->SetLocalTransform(zeus::CTransform(rotation * x44_baseRotation, position + x38_basePosition));
}

void CHudDecoInterfaceCombat::SetFrameColorValue(float v) {
  zeus::CColor color = v > 0.f ? zeus::skWhite : g_tweakGuiColors->GetHudFrameColor();
  x7c_basewidget_frame->SetColor(color);
}

void CHudDecoInterfaceCombat::Update(float dt, const CStateManager& stateMgr) {
  x6c_camera->SetO2WTransform(
      MP1::CSamusHud::BuildFinalCameraTransform(x4_rotation, x14_pivotPosition + x20_offset, x2c_camPos));
}

void CHudDecoInterfaceCombat::UpdateCameraDebugSettings(float fov, float y, float z) {
  x6c_camera->SetFov(fov);
  x2c_camPos.y() = y;
  x2c_camPos.z() = z;
}

void CHudDecoInterfaceCombat::UpdateHudAlpha() {
  zeus::CColor color = zeus::skWhite;
  color.a() = g_GameState->GameOptions().GetHUDAlpha() / 255.f;
  x70_basewidget_pivot->SetColor(color);
}

CHudDecoInterfaceScan::CHudDecoInterfaceScan(CGuiFrame& selHud) : x14_selHud(selHud), x18_scanDisplay(selHud) {
  x4_scanHudFlat = g_SimplePool->GetObj("FRME_ScanHudFlat");
  x234_sidesPositioner = g_tweakGui->GetScanSidesPositionStart();
  x244_camera = selHud.GetFrameCamera();
  x248_basewidget_pivot = selHud.FindWidget("basewidget_pivot");
  x24c_basewidget_leftside = selHud.FindWidget("basewidget_leftside");
  x250_basewidget_rightside = selHud.FindWidget("basewidget_rightside");
  x1f4_pivotPosition = x248_basewidget_pivot->GetIdlePosition();
  if (CGuiWidget* deco = selHud.FindWidget("basewidget_deco"))
    deco->SetColor(g_tweakGuiColors->GetHudFrameColor());

  x218_leftsidePosition = x24c_basewidget_leftside->GetLocalPosition();
  zeus::CTransform leftXf(zeus::CMatrix3f::RotateZ(g_tweakGui->GetScanSidesAngle()), x218_leftsidePosition);
  x24c_basewidget_leftside->SetLocalTransform(leftXf);
  if (CGuiWidget* w = selHud.FindWidget("basewidget_databankl")) {
    zeus::CTransform xf(zeus::CMatrix3f::RotateZ(g_tweakGui->GetScanSidesAngle() * -1.f), w->GetLocalPosition());
    w->SetLocalTransform(xf);
  }
  if (CGuiWidget* w = selHud.FindWidget("basewidget_leftguages")) {
    zeus::CTransform xf(zeus::CMatrix3f(zeus::CVector3f{g_tweakGui->GetScanSidesXScale(), 1.f, 1.f}),
                        w->GetLocalPosition());
    w->SetLocalTransform(xf);
  }

  x224_rightsidePosition = x250_basewidget_rightside->GetLocalPosition();
  zeus::CTransform rightXf(zeus::CMatrix3f::RotateZ(g_tweakGui->GetScanSidesAngle() * -1.f), x224_rightsidePosition);
  x250_basewidget_rightside->SetLocalTransform(rightXf);
  if (CGuiWidget* w = selHud.FindWidget("basewidget_databankr")) {
    zeus::CTransform xf(zeus::CMatrix3f::RotateZ(g_tweakGui->GetScanSidesAngle()), w->GetLocalPosition());
    w->SetLocalTransform(xf);
  }
  if (CGuiWidget* w = selHud.FindWidget("basewidget_rightguages")) {
    zeus::CTransform xf(zeus::CMatrix3f(zeus::CVector3f{g_tweakGui->GetScanSidesXScale(), 1.f, 1.f}),
                        w->GetLocalPosition());
    w->SetLocalTransform(xf);
  }

  zeus::CVector3f sidesPos(x234_sidesPositioner, 0.f, 0.f);
  x24c_basewidget_leftside->SetLocalPosition(x24c_basewidget_leftside->RotateO2P(x218_leftsidePosition + sidesPos));
  x250_basewidget_rightside->SetLocalPosition(x250_basewidget_rightside->RotateO2P(x224_rightsidePosition - sidesPos));
  x234_sidesPositioner = FLT_MAX;

  CHudDecoInterfaceScan::UpdateHudAlpha();
}

void CHudDecoInterfaceScan::UpdateVisibility() {
  // Empty
}

void CHudDecoInterfaceScan::SetIsVisibleDebug(bool v) {
  x240_24_visDebug = v;
  UpdateVisibility();
}

void CHudDecoInterfaceScan::SetIsVisibleGame(bool v) {
  x240_25_visGame = v;
  UpdateVisibility();
}

void CHudDecoInterfaceScan::SetHudRotation(const zeus::CQuaternion& rot) { x1e4_rotation = rot; }

void CHudDecoInterfaceScan::SetHudOffset(const zeus::CVector3f& off) { x200_offset = off; }

void CHudDecoInterfaceScan::SetReticuleTransform(const zeus::CMatrix3f& xf) {
  // Empty
}

void CHudDecoInterfaceScan::SetDamageTransform(const zeus::CMatrix3f& rotation, const zeus::CVector3f& position) {
  // Empty
}

void CHudDecoInterfaceScan::SetFrameColorValue(float v) {
  // Empty
}

void CHudDecoInterfaceScan::InitializeFlatFrame() {
  x10_loadedScanHudFlat = x4_scanHudFlat.GetObj();
  x10_loadedScanHudFlat->SetMaxAspect(1.33f);
  x10_loadedScanHudFlat->GetFrameCamera()->SetO2WTransform(zeus::CTransform::Translate(x20c_camPos));
  x258_flat_basewidget_scanguage = x10_loadedScanHudFlat->FindWidget("basewidget_scanguage");
  x258_flat_basewidget_scanguage->SetVisibility(false, ETraversalMode::Children);
  x254_flat_textpane_scanning = static_cast<CGuiTextPane*>(x10_loadedScanHudFlat->FindWidget("textpane_scanning"));
  x25c_flat_energybart01_scanbar =
      static_cast<CAuiEnergyBarT01*>(x10_loadedScanHudFlat->FindWidget("energybart01_scanbar"));
  x264_flat_textpane_message = static_cast<CGuiTextPane*>(x10_loadedScanHudFlat->FindWidget("textpane_message"));
  x268_flat_textpane_scrollmessage =
      static_cast<CGuiTextPane*>(x10_loadedScanHudFlat->FindWidget("textpane_scrollmessage"));
  x260_flat_basewidget_textgroup = x10_loadedScanHudFlat->FindWidget("basewidget_textgroup");
  x26c_flat_model_xmark = static_cast<CGuiModel*>(x10_loadedScanHudFlat->FindWidget("model_xmark"));
  x270_flat_model_abutton = static_cast<CGuiModel*>(x10_loadedScanHudFlat->FindWidget("model_abutton"));
  x274_flat_model_dash = static_cast<CGuiModel*>(x10_loadedScanHudFlat->FindWidget("model_dash"));
  x260_flat_basewidget_textgroup->SetVisibility(false, ETraversalMode::Children);
  x254_flat_textpane_scanning->SetIsVisible(false);
  x254_flat_textpane_scanning->TextSupport().SetFontColor(g_tweakGuiColors->GetHudMessageFill());
  x254_flat_textpane_scanning->TextSupport().SetOutlineColor(g_tweakGuiColors->GetHudMessageOutline());
  x25c_flat_energybart01_scanbar->SetCoordFunc(CAuiEnergyBarT01::DownloadBarCoordFunc);
  x25c_flat_energybart01_scanbar->ResetMaxEnergy();
  x25c_flat_energybart01_scanbar->SetFilledColor(zeus::CColor(0.4f, 0.68f, 0.88f, 1.f));
  x25c_flat_energybart01_scanbar->SetShadowColor(zeus::skClear);
  x25c_flat_energybart01_scanbar->SetEmptyColor(zeus::skClear);
  x25c_flat_energybart01_scanbar->SetFilledDrainSpeed(999.f);
  x25c_flat_energybart01_scanbar->SetShadowDrainSpeed(999.f);
  x25c_flat_energybart01_scanbar->SetShadowDrainDelay(0.f);
  x25c_flat_energybart01_scanbar->SetIsAlwaysResetTimer(false);
  x26c_flat_model_xmark->SetVisibility(false, ETraversalMode::Children);
  x26c_flat_model_xmark->SetVisibility(false, ETraversalMode::Children);
  x270_flat_model_abutton->SetVisibility(false, ETraversalMode::Children);
  x274_flat_model_dash->SetVisibility(false, ETraversalMode::Children);
}

const CScannableObjectInfo* CHudDecoInterfaceScan::GetCurrScanInfo(const CStateManager& stateMgr) const {
  if (x1d4_latestScanState == CPlayer::EPlayerScanState::NotScanning)
    return nullptr;

  if (TCastToConstPtr<CActor> act = stateMgr.GetObjectById(x1d2_latestScanningObject))
    return act->GetScannableObjectInfo();

  return nullptr;
}

void CHudDecoInterfaceScan::UpdateScanDisplay(const CStateManager& stateMgr, float dt) {
  CPlayer& player = stateMgr.GetPlayer();
  CPlayer::EPlayerScanState scanState = player.GetScanningState();
  if (scanState != x1d4_latestScanState) {
    if (player.IsNewScanScanning()) {
      if (scanState == CPlayer::EPlayerScanState::ScanComplete) {
        if (x1d4_latestScanState == CPlayer::EPlayerScanState::Scanning) {
          // Scan complete
          x254_flat_textpane_scanning->TextSupport().SetText(g_MainStringTable->GetString(15));
          x254_flat_textpane_scanning->TextSupport().SetTypeWriteEffectOptions(false, 0.f, 40.f);
          x238_scanningTextAlpha = 2.f;
        }
      } else if (scanState == CPlayer::EPlayerScanState::Scanning) {
        // Scanning
        x254_flat_textpane_scanning->TextSupport().SetText(g_MainStringTable->GetString(14));
        x254_flat_textpane_scanning->TextSupport().SetTypeWriteEffectOptions(false, 0.f, 40.f);
        x238_scanningTextAlpha = 2.f;
      }
    }
    x1d4_latestScanState = scanState;
  }

  if (player.GetScanningObjectId() != x1d2_latestScanningObject)
    x1d2_latestScanningObject = player.GetScanningObjectId();

  if (player.GetOrbitTargetId() != x1d0_latestHudPoi) {
    x1d0_latestHudPoi = player.GetOrbitTargetId();
    if (x1d0_latestHudPoi != kInvalidUniqueId) {
      if (!player.ObjectInScanningRange(x1d0_latestHudPoi, stateMgr)) {
        // Object out of scanning range
        x254_flat_textpane_scanning->TextSupport().SetText(g_MainStringTable->GetString(16));
        x254_flat_textpane_scanning->TextSupport().SetTypeWriteEffectOptions(true, 0.f, 40.f);
        x238_scanningTextAlpha = 1.f;
      }
    }
  }

  const CScannableObjectInfo* scanInfo = GetCurrScanInfo(stateMgr);
  if (x1d2_latestScanningObject != x18_scanDisplay.x10_objId || !scanInfo) {
    x18_scanDisplay.StopScan();
    if (x18_scanDisplay.xc_state == CScanDisplay::EScanState::Inactive && scanInfo) {
      x18_scanDisplay.StartScan(x1d2_latestScanningObject, *scanInfo, x264_flat_textpane_message,
                                x268_flat_textpane_scrollmessage, x260_flat_basewidget_textgroup, x26c_flat_model_xmark,
                                x270_flat_model_abutton, x274_flat_model_dash, player.GetScanningTime());
    }
  }

  x18_scanDisplay.Update(dt, player.GetScanningTime());

  if (x1d2_latestScanningObject != kInvalidUniqueId && GetCurrScanInfo(stateMgr))
    if (TCastToConstPtr<CActor> act = stateMgr.GetObjectById(x1d2_latestScanningObject))
      if (const CScannableObjectInfo* actScan = act->GetScannableObjectInfo())
        x25c_flat_energybart01_scanbar->SetCurrEnergy(x1d8_scanningTime / actScan->GetTotalDownloadTime(),
                                                      CAuiEnergyBarT01::ESetMode::Normal);

  if (x1d4_latestScanState != CPlayer::EPlayerScanState::Scanning)
    if (x1d0_latestHudPoi == kInvalidUniqueId || player.ObjectInScanningRange(x1d0_latestHudPoi, stateMgr))
      x238_scanningTextAlpha = std::max(0.f, x238_scanningTextAlpha - dt);

  if (x238_scanningTextAlpha > 0.f) {
    zeus::CColor color = zeus::skWhite;
    color.a() = std::min(x238_scanningTextAlpha, 1.f);
    x254_flat_textpane_scanning->SetColor(color);
    x254_flat_textpane_scanning->SetIsVisible(true);
  } else {
    x254_flat_textpane_scanning->SetIsVisible(false);
  }

  if (GetCurrScanInfo(stateMgr))
    x23c_scanBarAlpha = std::min(x23c_scanBarAlpha + 2.f * dt, 1.f);
  else
    x23c_scanBarAlpha = std::max(0.f, x23c_scanBarAlpha - 2.f * dt);

  if (x23c_scanBarAlpha > 0.f) {
    zeus::CColor color = zeus::skWhite;
    color.a() = std::min(x23c_scanBarAlpha, 1.f);
    x258_flat_basewidget_scanguage->SetColor(color);
    x258_flat_basewidget_scanguage->SetVisibility(true, ETraversalMode::Children);
  } else {
    x258_flat_basewidget_scanguage->SetVisibility(false, ETraversalMode::Children);
  }
}

void CHudDecoInterfaceScan::Update(float dt, const CStateManager& stateMgr) {
  CPlayer& player = stateMgr.GetPlayer();
  CPlayer::EPlayerScanState scanState = player.GetScanningState();
  if (scanState != CPlayer::EPlayerScanState::NotScanning)
    x1d8_scanningTime = player.GetScanningTime();

  if (scanState == CPlayer::EPlayerScanState::Scanning || scanState == CPlayer::EPlayerScanState::ScanComplete)
    x230_sidesTimer = std::min(x230_sidesTimer + dt, g_tweakGui->GetScanSidesEndTime());
  else
    x230_sidesTimer = std::max(0.f, x230_sidesTimer - dt);

  float sidesT = x230_sidesTimer < g_tweakGui->GetScanSidesStartTime()
                     ? 0.f
                     : (x230_sidesTimer - g_tweakGui->GetScanSidesStartTime()) / g_tweakGui->GetScanSidesDuration();
  float oldSidesPositioner = x234_sidesPositioner;
  x234_sidesPositioner =
      (1.f - sidesT) * g_tweakGui->GetScanSidesPositionStart() + sidesT * g_tweakGui->GetScanSidesPositionEnd();
  if (oldSidesPositioner != x234_sidesPositioner) {
    zeus::CVector3f sidesPos(x234_sidesPositioner, 0.f, 0.f);
    x24c_basewidget_leftside->SetLocalPosition(x218_leftsidePosition + x24c_basewidget_leftside->RotateO2P(sidesPos));
    x250_basewidget_rightside->SetLocalPosition(x224_rightsidePosition -
                                                x250_basewidget_rightside->RotateO2P(sidesPos));
  }

  x244_camera->SetO2WTransform(
      MP1::CSamusHud::BuildFinalCameraTransform(x1e4_rotation, x1f4_pivotPosition + x200_offset, x20c_camPos));

  if (!x10_loadedScanHudFlat) {
    if (!x4_scanHudFlat.IsLoaded() || !x4_scanHudFlat->GetIsFinishedLoading())
      return;
    InitializeFlatFrame();
  }

  x10_loadedScanHudFlat->Update(dt);
  UpdateScanDisplay(stateMgr, dt);
}

void CHudDecoInterfaceScan::Draw() {
  x18_scanDisplay.Draw();
  if (x10_loadedScanHudFlat) {
    x10_loadedScanHudFlat->Draw(CGuiWidgetDrawParms::Default);
  }
}

void CHudDecoInterfaceScan::ProcessInput(const CFinalInput& input) { x18_scanDisplay.ProcessInput(input); }

void CHudDecoInterfaceScan::UpdateCameraDebugSettings(float fov, float y, float z) {
  x244_camera->SetFov(fov);
  x20c_camPos.y() = y;
  x20c_camPos.z() = z;
}

void CHudDecoInterfaceScan::UpdateHudAlpha() {
  zeus::CColor color = zeus::skWhite;
  color.a() = g_GameState->GameOptions().GetHUDAlpha() / 255.f;
  x248_basewidget_pivot->SetColor(color);
}

float CHudDecoInterfaceScan::GetHudTextAlpha() const {
  return 1.f - std::max(std::min(x238_scanningTextAlpha, 1.f), x18_scanDisplay.x1a8_bodyAlpha);
}

CHudDecoInterfaceXRay::CHudDecoInterfaceXRay(CGuiFrame& selHud) {
  xa0_camera = selHud.GetFrameCamera();
  x30_camPos = xa0_camera->GetLocalPosition();

  xa4_basewidget_pivot = selHud.FindWidget("basewidget_pivot");
  xa8_basewidget_seeker = selHud.FindWidget("basewidget_seeker");
  xac_basewidget_rotate = selHud.FindWidget("basewidget_rotate");

  if (CGuiWidget* w = selHud.FindWidget("basewidget_energydeco"))
    w->SetColor(g_tweakGuiColors->GetXRayEnergyDecoColor());

  if (CGuiWidget* w = selHud.FindWidget("model_frame"))
    w->SetDepthWrite(true);
  if (CGuiWidget* w = selHud.FindWidget("model_frame1"))
    w->SetDepthWrite(true);
  if (CGuiWidget* w = selHud.FindWidget("model_frame2"))
    w->SetDepthWrite(true);
  if (CGuiWidget* w = selHud.FindWidget("model_frame3"))
    w->SetDepthWrite(true);
  if (CGuiWidget* w = selHud.FindWidget("model_misslieslider"))
    w->SetDepthWrite(true);
  if (CGuiWidget* w = selHud.FindWidget("model_threatslider"))
    w->SetDepthWrite(true);

  CHudDecoInterfaceXRay::UpdateHudAlpha();
}

void CHudDecoInterfaceXRay::UpdateVisibility() {
  // Empty
}

void CHudDecoInterfaceXRay::SetIsVisibleDebug(bool v) {
  x9c_24_visDebug = v;
  UpdateVisibility();
}

void CHudDecoInterfaceXRay::SetIsVisibleGame(bool v) {
  x9c_25_visGame = v;
  UpdateVisibility();
}

void CHudDecoInterfaceXRay::SetHudRotation(const zeus::CQuaternion& rot) { x8_rotation = rot; }

void CHudDecoInterfaceXRay::SetHudOffset(const zeus::CVector3f& off) { x24_offset = off; }

void CHudDecoInterfaceXRay::SetReticuleTransform(const zeus::CMatrix3f& xf) { x3c_reticuleXf = xf; }

void CHudDecoInterfaceXRay::SetDecoRotation(float angle) {
  xac_basewidget_rotate->SetLocalTransform(
      zeus::CTransform(zeus::CMatrix3f::RotateY(angle), xac_basewidget_rotate->GetLocalPosition()));
}

void CHudDecoInterfaceXRay::SetDamageTransform(const zeus::CMatrix3f& rotation, const zeus::CVector3f& position) {
  // Empty
}

void CHudDecoInterfaceXRay::SetFrameColorValue(float v) {
  // Empty
}

void CHudDecoInterfaceXRay::Update(float dt, const CStateManager& stateMgr) {
  if (stateMgr.GetPlayer().GetOrbitState() == CPlayer::EPlayerOrbitState::OrbitObject)
    x4_seekerScale = std::max(x4_seekerScale - 3.f * dt, 0.35f);
  else
    x4_seekerScale = std::min(3.f * dt + x4_seekerScale, 1.f);

  xa0_camera->SetO2WTransform(
      MP1::CSamusHud::BuildFinalCameraTransform(x8_rotation, x18_pivotPosition + x24_offset, x30_camPos));

  xa8_basewidget_seeker->SetLocalTransform(
      zeus::CTransform(zeus::CMatrix3f(x4_seekerScale) * x3c_reticuleXf, x60_seekerPosition));
}

void CHudDecoInterfaceXRay::UpdateCameraDebugSettings(float fov, float y, float z) {
  xa0_camera->SetFov(fov);
  x30_camPos.y() = y;
  x30_camPos.z() = z;
}

void CHudDecoInterfaceXRay::UpdateHudAlpha() {
  zeus::CColor color = zeus::skWhite;
  color.a() = g_GameState->GameOptions().GetHUDAlpha() / 255.f;
  xa4_basewidget_pivot->SetColor(color);
}

CHudDecoInterfaceThermal::CHudDecoInterfaceThermal(CGuiFrame& selHud) {
  x74_camera = selHud.GetFrameCamera();
  x2c_camPos = x74_camera->GetLocalPosition();

  x78_basewidget_pivot = selHud.FindWidget("basewidget_pivot");
  x7c_basewidget_reticle = selHud.FindWidget("basewidget_reticle");
  x80_model_retflash = static_cast<CGuiModel*>(selHud.FindWidget("model_retflash"));

  x14_pivotPosition = x78_basewidget_pivot->GetIdlePosition();
  x5c_reticulePosition = x7c_basewidget_reticle->GetIdlePosition();

  if (CGuiWidget* w = selHud.FindWidget("basewidget_deco"))
    w->SetColor(g_tweakGuiColors->GetThermalDecoColor());

  if (CGuiWidget* w = selHud.FindWidget("basewidget_oultlinesa"))
    w->SetColor(g_tweakGuiColors->GetThermalOutlinesColor());

  if (CGuiWidget* w = selHud.FindWidget("basewidget_lock"))
    w->SetColor(g_tweakGuiColors->GetThermalLockColor());

  if (CGuiWidget* w = selHud.FindWidget("basewidget_reticle"))
    w->SetColor(g_tweakGuiColors->GetThermalOutlinesColor());

  if (CGuiWidget* w = selHud.FindWidget("basewidget_lockon"))
    w->SetColor(g_tweakGuiColors->GetThermalOutlinesColor());

  if (CGuiWidget* w = selHud.FindWidget("model_threaticon"))
    w->SetColor(g_tweakGuiColors->GetThermalOutlinesColor());

  if (CGuiWidget* w = selHud.FindWidget("model_missileicon"))
    w->SetColor(g_tweakGuiColors->GetThermalOutlinesColor());

  if (CGuiWidget* w = selHud.FindWidget("basewidget_lock")) {
    for (CGuiWidget* c = static_cast<CGuiWidget*>(w->GetChildObject()); c;
         c = static_cast<CGuiWidget*>(c->GetNextSibling())) {
      x84_lockonWidgets.emplace_back(c, c->GetLocalTransform());
      c->SetLocalTransform(c->GetLocalTransform() * zeus::CTransform::Scale(x68_lockonScale));
    }
  }

  x14_pivotPosition = x78_basewidget_pivot->GetIdlePosition();
  CHudDecoInterfaceThermal::UpdateHudAlpha();
}

void CHudDecoInterfaceThermal::UpdateVisibility() {
  // Empty
}

void CHudDecoInterfaceThermal::SetIsVisibleDebug(bool v) {
  x70_24_visDebug = v;
  UpdateVisibility();
}

void CHudDecoInterfaceThermal::SetIsVisibleGame(bool v) {
  x70_25_visGame = v;
  UpdateVisibility();
}

void CHudDecoInterfaceThermal::SetHudRotation(const zeus::CQuaternion& rot) { x4_rotation = rot; }

void CHudDecoInterfaceThermal::SetHudOffset(const zeus::CVector3f& off) { x20_offset = off; }

void CHudDecoInterfaceThermal::SetReticuleTransform(const zeus::CMatrix3f& xf) { x38_reticuleXf = xf; }

void CHudDecoInterfaceThermal::SetDamageTransform(const zeus::CMatrix3f& rotation, const zeus::CVector3f& position) {
  // Empty
}

void CHudDecoInterfaceThermal::Update(float dt, const CStateManager& stateMgr) {
  float oldLockonScale = x68_lockonScale;
  if (stateMgr.GetPlayer().GetOrbitTargetId() != kInvalidUniqueId)
    x68_lockonScale = std::max(x68_lockonScale - 15.f * dt, 1.f);
  else
    x68_lockonScale = std::min(x68_lockonScale + 15.f * dt, 5.f);

  if (oldLockonScale != x68_lockonScale)
    for (auto& lockWidget : x84_lockonWidgets)
      lockWidget.first->SetLocalTransform(lockWidget.second * zeus::CTransform::Scale(x68_lockonScale));

  x6c_retflashTimer += dt;
  if (x6c_retflashTimer > 1.f)
    x6c_retflashTimer -= 2.f;

  zeus::CColor flashColor = zeus::skWhite;
  flashColor.a() = std::fabs(x6c_retflashTimer) * 0.5f + 0.5f;
  x80_model_retflash->SetColor(flashColor);

  x74_camera->SetO2WTransform(
      MP1::CSamusHud::BuildFinalCameraTransform(x4_rotation, x14_pivotPosition + x20_offset, x2c_camPos));

  x7c_basewidget_reticle->SetLocalTransform(zeus::CTransform(x38_reticuleXf, x5c_reticulePosition));
}

void CHudDecoInterfaceThermal::UpdateCameraDebugSettings(float fov, float y, float z) {
  x74_camera->SetFov(fov);
  x2c_camPos.y() = y;
  x2c_camPos.z() = z;
}

void CHudDecoInterfaceThermal::UpdateHudAlpha() {
  zeus::CColor color = zeus::skWhite;
  color.a() = g_GameState->GameOptions().GetHUDAlpha() / 255.f;
  x78_basewidget_pivot->SetColor(color);
}

} // namespace urde
