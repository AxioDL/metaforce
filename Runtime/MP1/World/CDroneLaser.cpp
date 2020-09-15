#include "Runtime/MP1/World/CDroneLaser.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CGameLight.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
CDroneLaser::CDroneLaser(TUniqueId uid, TAreaId aId, const zeus::CTransform& xf, CAssetId particle)
: CActor(uid, true, "DroneLaser"sv, CEntityInfo(aId, CEntity::NullConnectionList), xf, CModelData::CModelDataNull(),
         CMaterialList(EMaterialTypes::NoStepLogic), CActorParameters::None().HotInThermal(true), kInvalidUniqueId)
, xf8_beamDesc(g_SimplePool->GetObj({SBIG('PART'), particle}))
, x104_beamParticle(std::make_unique<CElementGen>(xf8_beamDesc, CElementGen::EModelOrientationType::Normal,
                                                  CElementGen::EOptionalSystemFlags::One)) {}

void CDroneLaser::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CDroneLaser::Think(float dt, CStateManager& mgr) { x104_beamParticle->Update(dt); }

void CDroneLaser::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, sender, mgr);

  if (msg == EScriptObjectMessage::Deactivate) {
    SetScannerLightActive(mgr, false);
    x104_beamParticle->SetParticleEmission(false);
  } else if (msg == EScriptObjectMessage::Activate) {
    SetScannerLightActive(mgr, true);
    x104_beamParticle->SetParticleEmission(true);
  } else if (msg == EScriptObjectMessage::Deleted) {
    if (xf4_scannerLight != kInvalidUniqueId) {
      mgr.FreeScriptObject(xf4_scannerLight);
      xf4_scannerLight = kInvalidUniqueId;
    }
  } else if (msg == EScriptObjectMessage::Registered) {
    xf4_scannerLight = mgr.AllocateUniqueId();
    mgr.AddObject(new CGameLight(xf4_scannerLight, GetAreaIdAlways(), GetActive(), "LaserScanner"sv, zeus::CTransform(),
                                 GetUniqueId(), CLight::BuildPoint(zeus::skZero3f, zeus::skRed), 0, 0, 0.f));
  }
}

void CDroneLaser::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  g_Renderer->AddParticleGen(*x104_beamParticle);
  EnsureRendered(mgr);
}

void CDroneLaser::Render(CStateManager& mgr) {
  // g_Renderer->SetDepthReadWrite(true, true);
  RenderBeam(4, 0.01f, zeus::CColor(1.f, .9f, .9f, 1.f), true);
  // g_Renderer->SetDepthReadWrite(true, false);
  RenderBeam(5, 0.06f, zeus::CColor(.4f, .0f, .0f, .5f), true);
  RenderBeam(7, 0.06f, zeus::CColor(.4f, .2f, .2f, .1f), true);
}

void CDroneLaser::CalculateRenderBounds() {
  zeus::CAABox box = zeus::skInvertedBox;
  const zeus::CVector3f diff = xe8_ - GetTranslation();
  const float mag1 = 0.2f * diff.magnitude();
  box.accumulateBounds(diff);
  box.accumulateBounds(xe8_ + (mag1 * GetTransform().basis[2]));
  box.accumulateBounds(xe8_ - (mag1 * GetTransform().basis[2]));
  x9c_renderBounds = box;
}

void CDroneLaser::SetScannerLightActive(CStateManager& mgr, bool activate) {
  mgr.SendScriptMsgAlways(xf4_scannerLight, GetUniqueId(),
                          activate ? EScriptObjectMessage::Activate : EScriptObjectMessage::Deactivate);
}

void CDroneLaser::RenderBeam(u32 w, float f, const zeus::CColor& col, bool) const {
  // TODO
}

void CDroneLaser::sub_80167754(CStateManager& mgr, const zeus::CVector3f& pos, const zeus::CVector3f& look) {
  xe8_ = pos;
  if (xf4_scannerLight != kInvalidUniqueId) {
    if (TCastToPtr<CGameLight> light = mgr.ObjectById(xf4_scannerLight)) {
      light->SetTranslation(pos - 0.5f * (pos - GetTranslation()));
    }
  }
  x104_beamParticle->SetOrientation(zeus::lookAt(zeus::skZero3f, look));
  x104_beamParticle->SetTranslation(pos);
}

} // namespace urde::MP1
