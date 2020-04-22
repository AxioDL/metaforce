#include "Runtime/MP1/CGameCubeDoll.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"

namespace urde::MP1 {

CGameCubeDoll::CGameCubeDoll() {
  x0_model = g_SimplePool->GetObj("CMDL_GameCube");
  x8_lights.push_back(CLight::BuildDirectional(zeus::skForward, zeus::skWhite));
  x18_actorLights = std::make_unique<CActorLights>(8, zeus::skZero3f, 4, 4, false, false, false, 0.1f);
}

void CGameCubeDoll::UpdateActorLights() {
  x8_lights[0] = CLight::BuildDirectional(
      (zeus::skForward + zeus::skRight * 0.25f + zeus::skDown * 0.1f).normalized(),
      zeus::skWhite);
  x18_actorLights->BuildFakeLightList(x8_lights, zeus::CColor(0.25f, 1.f));
}

void CGameCubeDoll::Update(float dt) {
  if (!CheckLoadComplete())
    return;
  x1c_fader = std::min(2.f * dt + x1c_fader, 1.f);
  UpdateActorLights();
}

void CGameCubeDoll::Draw(float alpha) {
  if (!IsLoaded())
    return;
  SCOPED_GRAPHICS_DEBUG_GROUP("CGameCubeDoll::Draw", zeus::skPurple);

  g_Renderer->SetPerspective(55.f, g_Viewport.x8_width, g_Viewport.xc_height, 0.2f, 4096.f);
  CGraphics::SetViewPointMatrix(zeus::CTransform::Translate(0.f, -2.f, 0.f));
  x18_actorLights->ActivateLights(x0_model->GetInstance());
  CGraphics::SetModelMatrix(zeus::CTransform::RotateZ(zeus::degToRad(360.f * CGraphics::GetSecondsMod900() * -0.25f)) *
                            zeus::CTransform::Scale(0.2f));
  CModelFlags flags(5, 0, 3, zeus::CColor(1.f, alpha * x1c_fader));
  x0_model->Draw(flags);
}

void CGameCubeDoll::Touch() {
  if (!CheckLoadComplete())
    return;
  x0_model->Touch(0);
}

bool CGameCubeDoll::CheckLoadComplete() {
  if (IsLoaded())
    return true;
  if (x0_model.IsLoaded()) {
    x20_24_loaded = true;
    return true;
  }
  return false;
}

} // namespace urde::MP1
