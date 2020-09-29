#include "Runtime/World/CHUDBillboardEffect.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

u32 CHUDBillboardEffect::g_IndirectTexturedBillboardCount = 0;
u32 CHUDBillboardEffect::g_BillboardCount = 0;

CHUDBillboardEffect::CHUDBillboardEffect(const std::optional<TToken<CGenDescription>>& particle,
                                         const std::optional<TToken<CElectricDescription>>& electric,
                                         TUniqueId uid, bool active, std::string_view name, float dist,
                                         const zeus::CVector3f& scale0, const zeus::CColor& color,
                                         const zeus::CVector3f& scale1, const zeus::CVector3f& translation)
: CEffect(uid, CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList), active, name, zeus::CTransform()) {
  xec_translation = translation;
  xec_translation.y() += dist;
  xf8_localScale = scale1 * scale0;

  if (particle) {
    x104_26_isElementGen = true;
    xe8_generator = std::make_unique<CElementGen>(*particle);
    if (static_cast<CElementGen&>(*xe8_generator).IsIndirectTextured())
      ++g_IndirectTexturedBillboardCount;
  } else {
    xe8_generator = std::make_unique<CParticleElectric>(*electric);
  }
  ++g_BillboardCount;
  xe8_generator->SetModulationColor(color);
  xe8_generator->SetLocalScale(xf8_localScale);
}

CHUDBillboardEffect::~CHUDBillboardEffect() {
  --g_BillboardCount;
  if (xe8_generator->Get4CharId() == FOURCC('PART'))
    if (static_cast<CElementGen&>(*xe8_generator).IsIndirectTextured())
      --g_IndirectTexturedBillboardCount;
}

void CHUDBillboardEffect::Accept(IVisitor& visitor) { visitor.Visit(this); }

float CHUDBillboardEffect::CalcGenRate() {
  float f1;
  if (g_BillboardCount + g_IndirectTexturedBillboardCount <= 4)
    f1 = 0.f;
  else
    f1 = g_BillboardCount * 0.2f + g_IndirectTexturedBillboardCount * 0.1f;
  return 1.f - std::min(f1, 0.8f);
}

void CHUDBillboardEffect::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    mgr.SetActorAreaId(*this, mgr.GetWorld()->GetCurrentAreaId());
    float oldGenRate = xe8_generator->GetGeneratorRate();
    xe8_generator->SetGeneratorRate(oldGenRate * CalcGenRate());
    xe8_generator->Update(dt);
    xe8_generator->SetGeneratorRate(oldGenRate);
    if (!x104_27_runIndefinitely) {
      x108_timeoutTimer += dt;
      if (x108_timeoutTimer > 30.f) {
        mgr.FreeScriptObject(GetUniqueId());
        return;
      }
    }
    if (xe8_generator->IsSystemDeletable())
      mgr.FreeScriptObject(GetUniqueId());
  }
}

void CHUDBillboardEffect::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  if (x104_25_enableRender && x104_24_renderAsParticleGen) {
    g_Renderer->AddParticleGen(*xe8_generator);
  }
}

void CHUDBillboardEffect::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  if (mgr.GetPlayer().GetCameraState() == CPlayer::EPlayerCameraState::FirstPerson) {
    zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
    xe8_generator->SetGlobalTranslation(camXf * xec_translation);
    xe8_generator->SetGlobalOrientation(camXf);
    x104_25_enableRender = true;
  } else {
    x104_25_enableRender = false;
  }
  x104_24_renderAsParticleGen = !mgr.RenderLast(GetUniqueId());
}

void CHUDBillboardEffect::Render(CStateManager& mgr) {
  if (x104_25_enableRender && !x104_24_renderAsParticleGen) {
    SCOPED_GRAPHICS_DEBUG_GROUP("CHUDBillboardEffect::Render", zeus::skPurple);
    xe8_generator->Render();
  }
}

float CHUDBillboardEffect::GetNearClipDistance(CStateManager& mgr) {
  return mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetNearClipDistance() + 0.01f;
}

zeus::CVector3f CHUDBillboardEffect::GetScaleForPOV(CStateManager& mgr) { return {0.155f, 1.f, 0.155f}; }

} // namespace urde
