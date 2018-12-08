#include "CHudRadarInterface.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CGuiFrame.hpp"
#include "CGuiCamera.hpp"
#include "CStateManager.hpp"
#include "CGameState.hpp"
#include "World/CPlayer.hpp"
#include "Camera/CGameCamera.hpp"
#include "zeus/CEulerAngles.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "World/CWallCrawlerSwarm.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "TCastTo.hpp"

namespace urde
{

CHudRadarInterface::CHudRadarInterface(CGuiFrame& baseHud, CStateManager& stateMgr)
{
    x0_txtrRadarPaint = g_SimplePool->GetObj("TXTR_RadarPaint");
    x3c_24_visibleGame = true;
    x3c_25_visibleDebug = true;
    x40_BaseWidget_RadarStuff = baseHud.FindWidget("BaseWidget_RadarStuff");
    x44_camera = baseHud.GetFrameCamera();
    xc_radarStuffXf = x40_BaseWidget_RadarStuff->GetLocalTransform();
    x40_BaseWidget_RadarStuff->SetColor(g_tweakGuiColors->GetRadarStuffColor());
}

void CHudRadarInterface::DoDrawRadarPaint(float radius, const zeus::CColor& color) const
{
    radius *= 4.f;
    const_cast<CHudRadarInterface&>(*this).m_paintInsts.emplace_back();
    CRadarPaintShader::Instance& inst =
        const_cast<CHudRadarInterface&>(*this).m_paintInsts.back();
    inst.pos[0].assign(-radius, 0.f, radius);
    inst.uv[0].assign(0.f, 1.f);
    inst.pos[1].assign(-radius, 0.f, -radius);
    inst.uv[1].assign(0.f, 0.f);
    inst.pos[2].assign(radius, 0.f, radius);
    inst.uv[2].assign(1.f, 1.f);
    inst.pos[3].assign(radius, 0.f, -radius);
    inst.uv[3].assign(1.f, 0.f);
    inst.color = color;
}

void CHudRadarInterface::DrawRadarPaint(const zeus::CVector3f& enemyPos, float radius,
                                        float alpha, const SRadarPaintDrawParms& parms) const
{
    zeus::CVector2f playerToEnemy(enemyPos.x() - parms.x0_playerPos.x(),
                                  enemyPos.y() - parms.x0_playerPos.y());

    float zDelta = std::fabs(enemyPos.z() - parms.x0_playerPos.z());

    if (playerToEnemy.magnitude() <= parms.x78_xyRadius && zDelta <= parms.x7c_zRadius)
    {
        if (zDelta > parms.x80_ZCloseRadius)
            alpha *= 1.f - (zDelta - parms.x80_ZCloseRadius) / (parms.x7c_zRadius - parms.x80_ZCloseRadius);
        zeus::CVector2f scopeScaled = playerToEnemy * parms.x70_scopeScalar;
        zeus::CTransform modelMatrix = parms.x3c_postTranslate * zeus::CTransform::Translate(
            parms.xc_preTranslate * zeus::CVector3f(scopeScaled.x(), 0.f, scopeScaled.y()));
        CGraphics::SetModelMatrix(modelMatrix);
        zeus::CColor color = g_tweakGuiColors->GetRadarEnemyPaintColor();
        color.a() *= alpha;
        DoDrawRadarPaint(radius, color);
    }
}

void CHudRadarInterface::SetIsVisibleGame(bool v)
{
    x3c_24_visibleGame = v;
    x40_BaseWidget_RadarStuff->SetVisibility(x3c_25_visibleDebug && x3c_24_visibleGame,
                                             ETraversalMode::Children);
}

void CHudRadarInterface::Update(float dt, const CStateManager& mgr)
{
    CPlayerState& playerState = *mgr.GetPlayerState();
    float visorTransFactor = (playerState.GetCurrentVisor() == CPlayerState::EPlayerVisor::Combat) ?
                             playerState.GetVisorTransitionFactor() : 0.f;
    zeus::CColor color = g_tweakGuiColors->GetRadarStuffColor();
    color.a() *= g_GameState->GameOptions().GetHUDAlpha() / 255.f * visorTransFactor;
    x40_BaseWidget_RadarStuff->SetColor(color);
    bool tweakVis = g_tweakGui->GetHudVisMode() >= ITweakGui::EHudVisMode::Three;
    if (tweakVis != x3c_25_visibleDebug)
    {
        x3c_25_visibleDebug = tweakVis;
        x40_BaseWidget_RadarStuff->SetVisibility(x3c_25_visibleDebug && x3c_24_visibleGame,
                                                 ETraversalMode::Children);
    }
}

void CHudRadarInterface::Draw(const CStateManager& mgr, float alpha) const
{
    alpha *= g_GameState->GameOptions().GetHUDAlpha() / 255.f;
    if (g_tweakGui->GetHudVisMode() == ITweakGui::EHudVisMode::Zero || !x3c_24_visibleGame ||
        !x0_txtrRadarPaint || !x0_txtrRadarPaint.IsLoaded())
        return;

    SRadarPaintDrawParms drawParms;

    CPlayer& player = mgr.GetPlayer();
    if (player.IsOverrideRadarRadius())
    {
        drawParms.x78_xyRadius = player.GetRadarXYRadiusOverride();
        drawParms.x7c_zRadius = player.GetRadarZRadiusOverride();
        drawParms.x80_ZCloseRadius = 0.667f * drawParms.x7c_zRadius;
    }
    else
    {
        drawParms.x78_xyRadius = g_tweakGui->GetRadarXYRadius();
        drawParms.x7c_zRadius = g_tweakGui->GetRadarZRadius();
        drawParms.x80_ZCloseRadius = g_tweakGui->GetRadarZCloseRadius();
    }

    drawParms.x6c_scopeRadius = g_tweakGui->GetRadarScopeCoordRadius();
    drawParms.x70_scopeScalar = drawParms.x6c_scopeRadius / drawParms.x78_xyRadius;

    float camZ = zeus::CEulerAngles(zeus::CQuaternion(
        mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTransform().basis)).z();
    zeus::CRelAngle angleZ(camZ);
    drawParms.xc_preTranslate = zeus::CTransform::RotateY(angleZ);
    drawParms.x3c_postTranslate = x40_BaseWidget_RadarStuff->GetWorldTransform();
    float enemyRadius = g_tweakGui->GetRadarEnemyPaintRadius();

    const_cast<CHudRadarInterface&>(*this).m_paintInsts.clear();
    x44_camera->Draw(CGuiWidgetDrawParms{0.f, zeus::CVector3f{}});
    CGraphics::SetModelMatrix(drawParms.x3c_postTranslate);

    zeus::CColor playerColor = g_tweakGuiColors->GetRadarPlayerPaintColor();
    playerColor.a() *= alpha;
    DoDrawRadarPaint(g_tweakGui->GetRadarPlayerPaintRadius(), playerColor);

    zeus::CAABox radarBounds(player.GetTranslation().x() - drawParms.x78_xyRadius,
                             player.GetTranslation().y() - drawParms.x78_xyRadius,
                             player.GetTranslation().z() - drawParms.x7c_zRadius,
                             player.GetTranslation().x() + drawParms.x78_xyRadius,
                             player.GetTranslation().y() + drawParms.x78_xyRadius,
                             player.GetTranslation().z() + drawParms.x7c_zRadius);

    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, radarBounds,
                      CMaterialFilter(CMaterialList(EMaterialTypes::Target, EMaterialTypes::RadarObject),
                                      CMaterialList(EMaterialTypes::ExcludeFromRadar),
                                      CMaterialFilter::EFilterType::IncludeExclude), nullptr);

    for (TUniqueId id : nearList)
    {
        if (TCastToConstPtr<CActor> act = mgr.GetObjectById(id))
        {
            if (!act->GetActive())
                continue;
            if (TCastToConstPtr<CWallCrawlerSwarm> swarm = act.GetPtr())
            {
                float radius = enemyRadius * 0.5f;
                for (const CWallCrawlerSwarm::CBoid& boid : swarm->GetBoids())
                {
                    if (!boid.GetActive())
                        continue;
                    DrawRadarPaint(boid.GetTranslation(), radius, 0.5f, drawParms);
                }
            }
            else
            {
                DrawRadarPaint(act->GetTranslation(), enemyRadius, 1.f, drawParms);
            }
        }
    }

    const_cast<CHudRadarInterface&>(*this).m_paintShader.draw(m_paintInsts, x0_txtrRadarPaint.GetObj());
}

}
