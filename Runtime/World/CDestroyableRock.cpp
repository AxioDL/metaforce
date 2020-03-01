#include "Runtime/World/CDestroyableRock.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/CPlayerState.hpp"
#include "Runtime/CStateManager.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path
namespace urde {

CDestroyableRock::CDestroyableRock(TUniqueId id, bool active, std::string_view name, const CEntityInfo& info,
                                   const zeus::CTransform& xf, CModelData&& modelData, float mass,
                                   const CHealthInfo& health, const CDamageVulnerability& vulnerability,
                                   const CMaterialList& matList, CAssetId fsm, const CActorParameters& actParams,
                                   const CStaticRes& phazonModel, s32 w1)
: CAi(id, active, name, info, xf, std::move(modelData), modelData.GetBounds(), mass, health, vulnerability, matList,
      fsm, actParams, 0.3f, 0.8f)
, x2d8_phazonModel(phazonModel)
, x32c_thermalMag(actParams.GetThermalMag())
, x338_healthInfo(health)
, x341_(w1 != 0) {
  /* This is now in UsePhazonModel
  x2d8_phazonModel->SetSortThermal(true);
   */
  CreateShadow(false);
}

void CDestroyableRock::Accept(urde::IVisitor& visitor) { visitor.Visit(this); }
void CDestroyableRock::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  if (GetActive()) {
    if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Thermal) {
      xb4_drawFlags = CModelFlags(0, 0, 1, zeus::skWhite);
    } else {
      if (x330_.r() >= 1.f && x330_.g() >= 1.f && x330_.b() >= 1.f) {
        xb4_drawFlags = CModelFlags(0, 0, 3, zeus::skWhite);
      } else {
        x330_.a() = 1.f;
        xb4_drawFlags = CModelFlags(2, 0, 3, x330_);
      }
    }
  }

  CActor::PreRender(mgr, frustum);
}
void CDestroyableRock::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  float damageMag = x32c_thermalMag;
  if (x324_ > 0.f) {
    x324_ = zeus::max(0.f, x324_ - dt);
    x330_ = zeus::CColor::lerp(zeus::skBlack, zeus::CColor(0.5, 0.f, 0.f, 1.f), x324_);
    damageMag = ((x335_usePhazonModel ? 0.5f : 0.25f) * x324_) + damageMag;
  }
  xd0_damageMag = damageMag;
  CEntity::Think(dt, mgr);
}
} // namespace urde
