#include "CScriptEMPulse.hpp"
#include "CActorParameters.hpp"
#include "Particle/CGenDescription.hpp"
#include "Particle/CElementGen.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CStateManager.hpp"
#include "CPlayer.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "TCastTo.hpp"

namespace urde {

CScriptEMPulse::CScriptEMPulse(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                               const zeus::CTransform& xf, bool active, float initialRadius, float finalRadius,
                               float duration, float interferenceDur, float f5, float interferenceMag, float f7,
                               CAssetId partId)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Projectile),
         CActorParameters::None().HotInThermal(true), kInvalidUniqueId)
, xe8_duration(duration)
, xec_finalRadius(finalRadius)
, xf0_currentRadius(initialRadius)
, xf4_initialRadius(initialRadius)
, xf8_interferenceDur(interferenceDur)
, xfc_(f5)
, x100_interferenceMag(interferenceMag)
, x104_(f7)
, x108_particleDesc(g_SimplePool->GetObj({SBIG('PART'), partId})) {}

void CScriptEMPulse::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptEMPulse::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  xf0_currentRadius += ((xec_finalRadius - xf4_initialRadius) / xe8_duration) * dt;
  if (xf0_currentRadius < xec_finalRadius)
    mgr.FreeScriptObject(GetUniqueId());

  x114_particleGen->Update(dt);
}

void CScriptEMPulse::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);
  if (msg == EScriptObjectMessage::Activate) {
    x114_particleGen.reset(new CElementGen(x108_particleDesc, CElementGen::EModelOrientationType::Normal,
                                           CElementGen::EOptionalSystemFlags::One));

    x114_particleGen->SetOrientation(GetTransform().getRotation());
    x114_particleGen->SetGlobalTranslation(GetTranslation());
    x114_particleGen->SetParticleEmission(true);
    mgr.GetPlayerState()->GetStaticInterference().AddSource(GetUniqueId(), x100_interferenceMag, xf8_interferenceDur);
  }
}

void CScriptEMPulse::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) const {
  CActor::AddToRenderer(frustum, mgr);
  if (GetActive())
    g_Renderer->AddParticleGen(*x114_particleGen);
}

void CScriptEMPulse::CalculateRenderBounds() { x9c_renderBounds = CalculateBoundingBox(); }

rstl::optional<zeus::CAABox> CScriptEMPulse::GetTouchBounds() const { return {CalculateBoundingBox()}; }

void CScriptEMPulse::Touch(CActor& act, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (TCastToPtr<CPlayer> pl = act) {
    zeus::CVector3f posDiff = GetTranslation() - pl->GetTranslation();
    if (posDiff.magnitude() < xec_finalRadius) {
      float dur =
          ((1.f - (posDiff.magnitude() / xec_finalRadius)) * (xfc_ - xf8_interferenceDur)) + xf8_interferenceDur;
      float mag =
          ((1.f - (posDiff.magnitude() / xec_finalRadius)) * (x104_ - xf8_interferenceDur)) + x100_interferenceMag;

      if (dur > pl->GetStaticTimer())
        pl->SetHudDisable(dur, 0.5f, 2.5f);
      else
        mgr.GetPlayerState()->GetStaticInterference().AddSource(GetUniqueId(), mag, dur);
    }
  }
}

zeus::CAABox CScriptEMPulse::CalculateBoundingBox() const {
  return zeus::CAABox(GetTranslation() - xf0_currentRadius, GetTranslation() + xf0_currentRadius);
}

} // namespace urde