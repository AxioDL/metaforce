#include "Runtime/GuiSys/CHudRadarInterface.hpp"

#include "Runtime/CGameState.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/GuiSys/CGuiCamera.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWallCrawlerSwarm.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <zeus/CEulerAngles.hpp>

namespace urde {

CHudRadarInterface::CHudRadarInterface(CGuiFrame& baseHud, CStateManager& stateMgr) {
  x0_txtrRadarPaint = g_SimplePool->GetObj("TXTR_RadarPaint");
  x40_BaseWidget_RadarStuff = baseHud.FindWidget("BaseWidget_RadarStuff");
  x44_camera = baseHud.GetFrameCamera();
  xc_radarStuffXf = x40_BaseWidget_RadarStuff->GetLocalTransform();
  x40_BaseWidget_RadarStuff->SetColor(g_tweakGuiColors->GetRadarStuffColor());
}

void CHudRadarInterface::DoDrawRadarPaint(const zeus::CVector3f& translate, float radius, const zeus::CColor& color) {
  radius *= 4.f;

  CRadarPaintShader::Instance& inst = m_paintInsts.emplace_back();
  inst.pos[0] = translate + zeus::CVector3f(-radius, 0.f, radius);
  inst.uv[0].assign(0.f, 1.f);
  inst.pos[1] = translate + zeus::CVector3f(-radius, 0.f, -radius);
  inst.uv[1].assign(0.f, 0.f);
  inst.pos[2] = translate + zeus::CVector3f(radius, 0.f, radius);
  inst.uv[2].assign(1.f, 1.f);
  inst.pos[3] = translate + zeus::CVector3f(radius, 0.f, -radius);
  inst.uv[3].assign(1.f, 0.f);
  inst.color = color;
}

void CHudRadarInterface::DrawRadarPaint(const zeus::CVector3f& enemyPos, float radius, float alpha,
                                        const SRadarPaintDrawParms& parms) {
  const zeus::CVector2f playerToEnemy = enemyPos.toVec2f() - parms.x0_playerPos.toVec2f();
  const float zDelta = std::fabs(enemyPos.z() - parms.x0_playerPos.z());

  if (playerToEnemy.magnitude() > parms.x78_xyRadius || zDelta > parms.x7c_zRadius) {
    return;
  }

  if (zDelta > parms.x80_ZCloseRadius) {
    alpha *= 1.f - (zDelta - parms.x80_ZCloseRadius) / (parms.x7c_zRadius - parms.x80_ZCloseRadius);
  }

  const zeus::CVector2f scopeScaled = playerToEnemy * parms.x70_scopeScalar;
  zeus::CColor color = g_tweakGuiColors->GetRadarEnemyPaintColor();
  color.a() *= alpha;
  color.a() *= parms.x74_alpha;
  DoDrawRadarPaint(parms.xc_preTranslate * zeus::CVector3f(scopeScaled.x(), 0.f, scopeScaled.y()), radius, color);
}

void CHudRadarInterface::SetIsVisibleGame(bool v) {
  x3c_24_visibleGame = v;
  x40_BaseWidget_RadarStuff->SetVisibility(x3c_25_visibleDebug && x3c_24_visibleGame, ETraversalMode::Children);
}

void CHudRadarInterface::Update(float dt, const CStateManager& mgr) {
  const CPlayerState& playerState = *mgr.GetPlayerState();
  const float visorTransFactor = (playerState.GetCurrentVisor() == CPlayerState::EPlayerVisor::Combat)
                                     ? playerState.GetVisorTransitionFactor()
                                     : 0.f;
  zeus::CColor color = g_tweakGuiColors->GetRadarStuffColor();
  color.a() *= g_GameState->GameOptions().GetHUDAlpha() / 255.f * visorTransFactor;
  x40_BaseWidget_RadarStuff->SetColor(color);
  const bool tweakVis = g_tweakGui->GetHudVisMode() >= ITweakGui::EHudVisMode::Three;

  if (tweakVis == x3c_25_visibleDebug) {
    return;
  }

  x3c_25_visibleDebug = tweakVis;
  x40_BaseWidget_RadarStuff->SetVisibility(x3c_25_visibleDebug && x3c_24_visibleGame, ETraversalMode::Children);
}

void CHudRadarInterface::Draw(const CStateManager& mgr, float alpha) {
  alpha *= g_GameState->GameOptions().GetHUDAlpha() / 255.f;
  if (g_tweakGui->GetHudVisMode() == ITweakGui::EHudVisMode::Zero || !x3c_24_visibleGame || !x0_txtrRadarPaint ||
      !x0_txtrRadarPaint.IsLoaded()) {
    return;
  }

  SRadarPaintDrawParms drawParms;

  const CPlayer& player = mgr.GetPlayer();
  if (player.IsOverrideRadarRadius()) {
    drawParms.x78_xyRadius = player.GetRadarXYRadiusOverride();
    drawParms.x7c_zRadius = player.GetRadarZRadiusOverride();
    drawParms.x80_ZCloseRadius = 0.667f * drawParms.x7c_zRadius;
  } else {
    drawParms.x78_xyRadius = g_tweakGui->GetRadarXYRadius();
    drawParms.x7c_zRadius = g_tweakGui->GetRadarZRadius();
    drawParms.x80_ZCloseRadius = g_tweakGui->GetRadarZCloseRadius();
  }

  drawParms.x6c_scopeRadius = g_tweakGui->GetRadarScopeCoordRadius();
  drawParms.x70_scopeScalar = drawParms.x6c_scopeRadius / drawParms.x78_xyRadius;

  const float camZ =
      zeus::CEulerAngles(zeus::CQuaternion(mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTransform().basis)).z();
  zeus::CRelAngle angleZ(camZ);
  angleZ.makeRel();
  drawParms.xc_preTranslate = zeus::CTransform::RotateY(angleZ);
  drawParms.x3c_postTranslate = x40_BaseWidget_RadarStuff->GetWorldTransform();
  const float enemyRadius = g_tweakGui->GetRadarEnemyPaintRadius();

  m_paintInsts.clear();
  x44_camera->Draw(CGuiWidgetDrawParms{0.f, zeus::CVector3f{}});
  CGraphics::SetModelMatrix(drawParms.x3c_postTranslate);

  zeus::CColor playerColor = g_tweakGuiColors->GetRadarPlayerPaintColor();
  playerColor.a() *= alpha;
  DoDrawRadarPaint(zeus::skZero3f, g_tweakGui->GetRadarPlayerPaintRadius(), playerColor);

  const zeus::CAABox radarBounds(
      player.GetTranslation().x() - drawParms.x78_xyRadius, player.GetTranslation().y() - drawParms.x78_xyRadius,
      player.GetTranslation().z() - drawParms.x7c_zRadius, player.GetTranslation().x() + drawParms.x78_xyRadius,
      player.GetTranslation().y() + drawParms.x78_xyRadius, player.GetTranslation().z() + drawParms.x7c_zRadius);

  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildNearList(nearList, radarBounds,
                    CMaterialFilter(CMaterialList(EMaterialTypes::Target, EMaterialTypes::RadarObject),
                                    CMaterialList(EMaterialTypes::ExcludeFromRadar),
                                    CMaterialFilter::EFilterType::IncludeExclude),
                    nullptr);
  drawParms.x0_playerPos = mgr.GetPlayer().GetTranslation();
  drawParms.x74_alpha = alpha;

  for (const TUniqueId id : nearList) {
    if (const TCastToConstPtr<CActor> act = mgr.GetObjectById(id)) {
      if (!act->GetActive()) {
        continue;
      }
      if (const TCastToConstPtr<CWallCrawlerSwarm> swarm = act.GetPtr()) {
        const float radius = enemyRadius * 0.5f;
        for (const CWallCrawlerSwarm::CBoid& boid : swarm->GetBoids()) {
          if (!boid.GetActive()) {
            continue;
          }
          DrawRadarPaint(boid.GetTranslation(), radius, 0.5f, drawParms);
        }
      } else {
        DrawRadarPaint(act->GetTranslation(), enemyRadius, 1.f, drawParms);
      }
    }
  }

  m_paintShader.draw(m_paintInsts, x0_txtrRadarPaint.GetObj());
}

} // namespace urde
