#include "Runtime/MP1/World/CPuffer.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CFire.hpp"
#include "Runtime/World/CKnockBackController.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {

CPuffer::CPuffer(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                 CModelData&& modelData, const CActorParameters& actorParameters, const CPatternedInfo& patternedInfo,
                 float hoverSpeed, CAssetId cloudEffect, const CDamageInfo& cloudDamage, CAssetId cloudSteam, float f2,
                 bool b1, bool b2, bool b3, const CDamageInfo& explosionDamage, s16 sfxId)
: CPatterned(ECharacter::Puffer, uid, name, EFlavorType::Zero, info, xf, std::move(modelData), patternedInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::RestrictedFlyer, actorParameters,
             EKnockBackVariant::Small)
, x568_face(xf.frontVector())
, x574_cloudEffect(g_SimplePool->GetObj({SBIG('PART'), cloudEffect}))
, x57c_cloudDamage(cloudDamage)
, x598_24_(b1)
, x598_25_(b3)
, x598_26_(b2)
, x59a_(CSfxManager::TranslateSFXID(sfxId))
, x59c_explosionDamage(explosionDamage)
, x5b8_(f2)
, x5bc_cloudSteam(cloudSteam) {
  CreateShadow(false);
  x460_knockBackController.SetImpulseDurationIdx(1);
  x574_cloudEffect.Lock();
}

void CPuffer::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CPuffer::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case EScriptObjectMessage::Registered:
    x450_bodyController->Activate(mgr);
    SetMaterialFilter(
        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Player}, {EMaterialTypes::NoStaticCollision}));
    break;
  case EScriptObjectMessage::Action:
    if (GetActive())
      x401_30_pendingDeath = true;
    break;
  default:
    break;
  }
}

void CPuffer::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  sub8025bfa4(mgr);
  zeus::CVector3f moveVector = x450_bodyController->GetCommandMgr().GetMoveVector();

  if (x5cc_ != x2dc_destObj) {
    x5cc_ = x2dc_destObj;
    CSfxManager::AddEmitter(x59a_, GetTranslation(), {}, true, false, 127, -1);
  }

  x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
  if (moveVector.canBeNormalized()) {
    //zeus::CVector3f vec = x5c0_move * (1.f - (dt / 0.5f)) + (moveVector * (dt / 0.5f));
    x5c0_move = moveVector.normalized();
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(x5c0_move, x568_face, 1.f));
  }
}

std::optional<zeus::CAABox> CPuffer::GetTouchBounds() const {
  auto touchBounds = CPatterned::GetTouchBounds();
  if (touchBounds) {
    touchBounds->accumulateBounds(touchBounds->min - 0.5f);
    touchBounds->accumulateBounds(touchBounds->max + 0.5f);
  }

  return touchBounds;
}

void CPuffer::Touch(CActor& act, CStateManager& mgr) {
  CPatterned::Touch(act, mgr);

  if (x400_25_alive && act.GetUniqueId() == mgr.GetPlayer().GetUniqueId())
    x401_30_pendingDeath = true;
}

void CPuffer::Death(CStateManager& mgr, const zeus::CVector3f& vec, EScriptObjectState state) {
  CPatterned::Death(mgr, vec, state);
  mgr.ApplyDamageToWorld(GetUniqueId(), *this, GetTranslation(), x59c_explosionDamage,
                         CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}));
  zeus::CTransform xf = GetTransform() * zeus::CTransform::Scale(x57c_cloudDamage.GetRadius());
  zeus::CAABox aabox(-1.f, 1.f);
  mgr.AddObject(new CFire(x574_cloudEffect, mgr.AllocateUniqueId(), GetAreaIdAlways(), true, GetUniqueId(),
                          GetTransform(), x57c_cloudDamage, aabox.getTransformedAABox(xf), {1.f, 1.f, 1.f}, true,
                          x5bc_cloudSteam, x598_24_, x598_26_, x598_25_, 1.f, x5b8_, 1.f, 1.f));
}

static const char* GasLocators[14] = {
    "Gas_01_LCTR", "Gas_02_LCTR", "Gas_03_LCTR", "Gas_04_LCTR", "Gas_05_LCTR", "Gas_06_LCTR", "Gas_07_LCTR",
    "Gas_08_LCTR", "Gas_09_LCTR", "Gas_10_LCTR", "Gas_11_LCTR", "Gas_12_LCTR", "Gas_13_LCTR", "Gas_14_LCTR",
};

static const char* GesJetLocators[14] = {
    "GasJet01", "GasJet02", "GasJet03", "GasJet04", "GasJet05", "GasJet06", "GasJet07",
    "GasJet08", "GasJet09", "GasJet10", "GasJet11", "GasJet12", "GasJet13", "GasJet14",
};

void CPuffer::sub8025bfa4(CStateManager& mgr) {
  zeus::CVector3f moveVector = x450_bodyController->GetCommandMgr().GetMoveVector();

  if (x5d4_gasLocators.empty()) {
    for (u32 i = 0; i < 14; ++i)
      x5d4_gasLocators.push_back(GetScaledLocatorTransform(GasLocators[i]).frontVector());
  }

  if (moveVector.canBeNormalized()) {}
}
} // namespace urde::MP1
