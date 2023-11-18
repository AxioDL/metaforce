#include "Runtime/World/CMorphBallShadow.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"

namespace metaforce {

void CMorphBallShadow::GatherAreas(const CStateManager& mgr) {
  x18_areas.clear();
  for (const CGameArea& area : *mgr.GetWorld()) {
    CGameArea::EOcclusionState occState = CGameArea::EOcclusionState::Occluded;
    if (area.IsPostConstructed())
      occState = area.GetPostConstructed()->x10dc_occlusionState;
    if (occState == CGameArea::EOcclusionState::Visible)
      x18_areas.push_back(area.GetAreaId());
  }
}

void CMorphBallShadow::RenderIdBuffer(const zeus::CAABox& aabb, const CStateManager& mgr, CPlayer& player) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CMorphBallShadow::RenderIdBuffer", zeus::skBlue);
  xb8_shadowVolume = aabb;
  x0_actors.clear();
  x18_areas.clear();
  x30_worldModelBits.clear();
  g_Renderer->x318_26_requestRGBA6 = true;

  if (true /* TODO */ || !g_Renderer->x318_27_currentRGBA6) {
    xd0_hasIds = false;
    return;
  }

  GatherAreas(mgr);

  SViewport backupVp = CGraphics::g_Viewport;
  // g_Renderer->BindBallShadowIdTarget();
  // CGraphics::g_BooMainCommandQueue->clearTarget();

  zeus::CTransform backupViewMtx = CGraphics::g_ViewMatrix;
  CGraphics::CProjectionState backupProjection = CGraphics::g_Proj;
  zeus::CVector2f backupDepth = CGraphics::g_CachedDepthRange;
  zeus::CTransform viewMtx(
      zeus::skRight, zeus::skDown, zeus::skForward,
      zeus::CVector3f((aabb.min.x() + aabb.max.x()) * 0.5f, (aabb.min.y() + aabb.max.y()) * 0.5f, aabb.max.z()));

  CGraphics::SetDepthRange(DEPTH_NEAR, DEPTH_FAR);
  float vpX = (aabb.max.x() - aabb.min.x()) * 0.5f;
  float vpY = (aabb.max.y() - aabb.min.y()) * 0.5f;
  float vpZ = (aabb.max.z() - aabb.min.z()) + FLT_EPSILON;
  CGraphics::SetOrtho(-vpX, vpX, vpY, -vpY, 0.f, vpZ);

  EntityList nearItems;
  mgr.BuildNearList(nearItems, aabb, CMaterialFilter::skPassEverything, &player);

  CGraphics::SetViewPointMatrix(viewMtx);

  int alphaVal = 4;
  for (TUniqueId id : nearItems) {
    if (alphaVal > 255)
      break;

    CActor* actor = static_cast<CActor*>(mgr.ObjectById(id));
    if (!actor || !actor->CanDrawStatic())
      continue;

    x0_actors.push_back(actor);

    auto* modelData = actor->GetModelData();
    zeus::CTransform modelXf = actor->GetTransform() * zeus::CTransform::Scale(modelData->GetScale());
    CGraphics::SetModelMatrix(modelXf);

    CModelFlags flags(0, 0, 3, zeus::CColor{1.f, 1.f, 1.f, alphaVal / 255.f});
    // flags.m_extendedShader = EExtendedShader::SolidColor; // Do solid color draw
    auto& model = *modelData->PickStaticModel(CModelData::EWhichModel::Normal);
    model.VerifyCurrentShader(flags.x1_matSetIdx);
    model.DrawUnsortedParts(flags);
    alphaVal += 4;
  }

  CGraphics::SetModelMatrix(zeus::CTransform());

  g_Renderer->FindOverlappingWorldModels(x30_worldModelBits, aabb);
  alphaVal = g_Renderer->DrawOverlappingWorldModelIDs(alphaVal, x30_worldModelBits, aabb);

  // g_Renderer->ResolveBallShadowIdTarget();

  // g_Renderer->BindMainDrawTarget();
  CGraphics::SetViewPointMatrix(backupViewMtx);
  CGraphics::SetProjectionState(backupProjection);
  g_Renderer->SetViewport(backupVp.x0_left, backupVp.x4_top, backupVp.x8_width, backupVp.xc_height);
  CGraphics::SetDepthRange(backupDepth[0], backupDepth[1]);

  xd0_hasIds = alphaVal != 4;
}

bool CMorphBallShadow::AreasValid(const CStateManager& mgr) const {
  auto it = x18_areas.begin();
  for (const CGameArea& area : *mgr.GetWorld()) {
    CGameArea::EOcclusionState occState = CGameArea::EOcclusionState::Occluded;
    if (area.IsPostConstructed())
      occState = area.GetPostConstructed()->x10dc_occlusionState;
    if (occState != CGameArea::EOcclusionState::Visible)
      continue;
    if (it == x18_areas.end())
      return false;
    if (*it != area.GetAreaId())
      return false;
    ++it;
  }
  return true;
}

void CMorphBallShadow::Render(const CStateManager& mgr, float alpha) {
  if (!xd0_hasIds || !AreasValid(mgr))
    return;

  CModelFlags flags;
  flags.x4_color.a() = alpha;
  // flags.m_extendedShader = EExtendedShader::MorphBallShadow;
  // flags.mbShadowBox = xb8_shadowVolume;

  int alphaVal = 4;
  for (auto* actor : x0_actors) {
    auto* modelData = actor->GetModelData();
    zeus::CTransform modelXf = actor->GetTransform() * zeus::CTransform::Scale(modelData->GetScale());
    CGraphics::SetModelMatrix(modelXf);

    flags.x4_color.r() = alphaVal / 255.f;
    auto& model = *modelData->PickStaticModel(CModelData::EWhichModel::Normal);
    model.VerifyCurrentShader(flags.x1_matSetIdx);
    model.DrawUnsortedParts(flags);
    alphaVal += 4;
  }

  CGraphics::SetModelMatrix(zeus::CTransform());
  g_Renderer->DrawOverlappingWorldModelShadows(alphaVal, x30_worldModelBits, xb8_shadowVolume);
}

} // namespace metaforce
