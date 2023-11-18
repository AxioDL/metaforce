#include "Runtime/World/CScriptEMPulse.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

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
  if (!GetActive()) {
    return;
  }

  xf0_currentRadius += ((xec_finalRadius - xf4_initialRadius) / xe8_duration) * dt;
  if (xf0_currentRadius < xec_finalRadius) {
    mgr.FreeScriptObject(GetUniqueId());
  }

  x114_particleGen->Update(dt);
}

void CScriptEMPulse::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  if (msg != EScriptObjectMessage::Activate) {
    return;
  }

  x114_particleGen = std::make_unique<CElementGen>(x108_particleDesc, CElementGen::EModelOrientationType::Normal,
                                                   CElementGen::EOptionalSystemFlags::One);

  x114_particleGen->SetOrientation(GetTransform().getRotation());
  x114_particleGen->SetGlobalTranslation(GetTranslation());
  x114_particleGen->SetParticleEmission(true);
  mgr.GetPlayerState()->GetStaticInterference().AddSource(GetUniqueId(), x100_interferenceMag, xf8_interferenceDur);
}

void CScriptEMPulse::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  CActor::AddToRenderer(frustum, mgr);
  if (GetActive()) {
    g_Renderer->AddParticleGen(*x114_particleGen);
  }
}

void CScriptEMPulse::CalculateRenderBounds() { x9c_renderBounds = CalculateBoundingBox(); }

std::optional<zeus::CAABox> CScriptEMPulse::GetTouchBounds() const { return {CalculateBoundingBox()}; }

void CScriptEMPulse::Touch(CActor& act, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (const TCastToPtr<CPlayer> pl = act) {
    const zeus::CVector3f posDiff = GetTranslation() - pl->GetTranslation();
    const float diffMagnitude = posDiff.magnitude();
    if (diffMagnitude < xec_finalRadius) {
      const float multi = 1.f - (diffMagnitude / xec_finalRadius);
      const float dur = (multi * (xfc_ - xf8_interferenceDur)) + xf8_interferenceDur;
      const float mag = (multi * (x104_ - x100_interferenceMag)) + x100_interferenceMag;

      if (dur > pl->GetStaticTimer()) {
        pl->SetHudDisable(dur, 0.5f, 2.5f);
        pl->SetOrbitRequestForTarget(mgr.GetPlayer().GetOrbitTargetId(),
                                     CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
      }
      mgr.GetPlayerState()->GetStaticInterference().AddSource(GetUniqueId(), mag, dur);
    }
  }
}

zeus::CAABox CScriptEMPulse::CalculateBoundingBox() const {
  return zeus::CAABox(GetTranslation() - xf0_currentRadius, GetTranslation() + xf0_currentRadius);
}

} // namespace metaforce
