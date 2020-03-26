#include "Runtime/MP1/World/CShockWave.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/World/CActorParameters.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
namespace MP1 {
CShockWave::CShockWave(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                       TUniqueId parent, const SShockWaveData& data, float f1, float f2)
: CActor(uid, true, name, info, xf, CModelData::CModelDataNull(), {EMaterialTypes::Projectile},
         CActorParameters::None(), kInvalidUniqueId)
, xe8_id1(parent)
, xec_damageInfo(data.x8_damageInfo)
, x108_elementGenDesc(g_SimplePool->GetObj({SBIG('PART'), data.x4_particleDesc}))
, x110_elementGen(std::make_unique<CElementGen>(x108_elementGenDesc))
, x114_data(data)
, x150_(data.x24_)
, x154_(data.x2c_)
, x15c_(f1)
, x160_(f2) {
  if (data.x34_weaponDesc.IsValid()) {
    x974_electricDesc = g_SimplePool->GetObj({SBIG('ELSC'), data.x34_weaponDesc});
  }
  x110_elementGen->SetParticleEmission(true);
  x110_elementGen->SetOrientation(GetTransform().getRotation());
  x110_elementGen->SetGlobalTranslation(GetTranslation());
  xe6_27_thermalVisorFlags = 2;
}

void CShockWave::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CShockWave::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  // TODO
  CActor::AcceptScriptMsg(msg, uid, mgr);
}

void CShockWave::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const {
  CActor::AddToRenderer(frustum, mgr);
  g_Renderer->AddParticleGen(*x110_elementGen);
}

std::optional<zeus::CAABox> CShockWave::GetTouchBounds() const {
  // TODO
  return CActor::GetTouchBounds();
}

void CShockWave::Render(const CStateManager& mgr) const {
  CActor::Render(mgr);
  x110_elementGen->Render();
}

void CShockWave::Think(float dt, CStateManager& mgr) {
  // TODO
}

void CShockWave::Touch(CActor& actor, CStateManager& mgr) {
  // TODO
}
} // namespace MP1
} // namespace urde
