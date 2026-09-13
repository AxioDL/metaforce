#include "MetroidPrime/Enemies/CPuffer.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CFire.hpp"

static const char* skGasJetLocators[] = {
    "GasJet01", "GasJet02", "GasJet03", "GasJet04", "GasJet05", "GasJet06", "GasJet07",
    "GasJet08", "GasJet09", "GasJet10", "GasJet11", "GasJet12", "GasJet13", "GasJet14",
};

static const char* skGasLocators[] = {
    "Gas_01_LCTR", "Gas_02_LCTR", "Gas_03_LCTR", "Gas_04_LCTR", "Gas_05_LCTR",
    "Gas_06_LCTR", "Gas_07_LCTR", "Gas_08_LCTR", "Gas_09_LCTR", "Gas_10_LCTR",
    "Gas_11_LCTR", "Gas_12_LCTR", "Gas_13_LCTR", "Gas_14_LCTR",
};

CPuffer::CPuffer(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                 const CTransform4f& xf, const CModelData& modelData,
                 const CActorParameters& actorParameters, const CPatternedInfo& patternedInfo,
                 float hoverSpeed, CAssetId cloudEffect, const CDamageInfo& cloudDamage,
                 CAssetId cloudSteam, float f2, bool b1, bool b2, bool b3,
                 const CDamageInfo& explosionDamage, ushort sfxId)
: CPatterned(kC_Puffer, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Flyer, kCT_One,
             kBT_RestrictedFlyer, actorParameters, kCS_Small)
, x568_face(xf.GetColumn(kDY))
, x574_cloudEffect(gpSimplePool->GetObj(SObjectTag('PART', cloudEffect)))
, x57c_cloudDamage(cloudDamage)
, x598_24_(b1)
, x598_25_(b3)
, x598_26_(b2)
, x59a_(CSfxManager::TranslateSFXID(sfxId))
, x59c_explosionDamage(explosionDamage)
, x5b8_(f2)
, x5bc_cloudSteam(cloudSteam)
, x5c0_move(CVector3f::Zero())
, x5cc_(kInvalidUniqueId)
, x5d0_enabledParticles(0) {
  SetDrawShadow(false);
  KnockBackCtrl().SetImpulseDurationIdx(1);
  x574_cloudEffect.Lock();
  BodyCtrl()->SetRestrictedFlyerMoveSpeed(hoverSpeed);
}

CPuffer::~CPuffer() {}

ENTITY_ACCEPT_IMPL(CPuffer)

void CPuffer::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Registered:
    BodyCtrl()->Activate(mgr);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Player),
                                                          CMaterialList(kMT_NoStaticCollision)));
    break;
  case kSM_Action:
    if (GetActive()) {
      SetPendingDeath(true);
    }
    break;
  default:
    break;
  }
}

void CPuffer::Touch(CActor& act, CStateManager& mgr) {
  CPatterned::Touch(act, mgr);

  if (IsAlive() && mgr.GetPlayer()->GetUniqueId() == act.GetUniqueId()) {
    SetPendingDeath(true);
  }
}

rstl::optional_object< CAABox > CPuffer::GetTouchBounds() const {
  rstl::optional_object< CAABox > touchBounds = CPatterned::GetTouchBounds();
  if (touchBounds.valid()) {
    CAABox box = *touchBounds;
    box.AccumulateBounds(box.GetMinPoint() - CVector3f(0.5f, 0.5f, 0.5f));
    box.AccumulateBounds(box.GetMaxPoint() + CVector3f(0.5f, 0.5f, 0.5f));
    return box;
  }
  return touchBounds;
}

void CPuffer::Death(CStateManager& mgr, const CVector3f& vec, EScriptObjectState state) {
  CPatterned::Death(mgr, vec, state);

  mgr.ApplyDamageToWorld(
      GetUniqueId(), *this, GetTranslation(), x59c_explosionDamage,
      CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()));

  TUniqueId uid = mgr.AllocateUniqueId();
  CAABox aabb =
      CAABox(CVector3f(-1.f, -1.f, -1.f), CVector3f(1.f, 1.f, 1.f))
          .GetTransformedAABox(GetTransform() * CTransform4f::Scale(x57c_cloudDamage.GetRadius()));
  mgr.AddObject(rs_new CFire(x574_cloudEffect, uid, GetCurrentAreaId(), true, GetUniqueId(),
                             GetTransform(), x57c_cloudDamage, aabb, CVector3f(1.f, 1.f, 1.f), true,
                             x5bc_cloudSteam, x598_24_, x598_26_, x598_25_, 1.f, x5b8_, 1.f, 1.f));
}

void CPuffer::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);

  UpdateJets(mgr);
  CVector3f moveVector = BodyCtrl()->GetCommandMgr().GetMoveVector();

  if (x5cc_ != GetDestObj()) {
    x5cc_ = GetDestObj();
    CSfxManager::AddEmitter(x59a_, GetTranslation(), CVector3f::Zero(), true, false);
  }

  BodyCtrl()->CommandMgr().ClearLocomotionCmds();
  if (moveVector.CanBeNormalized()) {
    x5c0_move = CVector3f::Lerp(x5c0_move, moveVector, dt / 0.5f).AsNormalized();
    BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(x5c0_move, x568_face, 1.f));
  }
}

void CPuffer::UpdateJets(CStateManager& mgr) {
  CVector3f moveVector = BodyCtrl()->GetCommandMgr().GetMoveVector();

  if (x5d4_gasLocators.empty()) {
    for (int i = 0; i < ARRAY_SIZE(skGasLocators); ++i) {
      x5d4_gasLocators.push_back(
          GetScaledLocatorTransform(rstl::string_l(skGasLocators[i])).GetColumn(kDY));
    }
  }

  if (moveVector.CanBeNormalized()) {
    CVector3f moveNorm = -moveVector.AsNormalized();
    for (int i = 0; i < ARRAY_SIZE(skGasJetLocators); ++i) {
      CVector3f tmp = GetTransform().Rotate(x5d4_gasLocators[i]);
      float ang = CMath::FastCosR(CAbsAngle::FromDegrees(45.f).AsRadians());
      bool enable = CVector3f::Dot(moveNorm, tmp) > ang;
      if (IsParticleEnabled(i) != enable) {
        AnimationData()->SetParticleEffectState(rstl::string_l(skGasJetLocators[i]), enable, mgr);
      }

      SetParticleEnabled(i, enable);
    }
  } else {
    for (int i = 0; i < ARRAY_SIZE(skGasJetLocators); ++i) {
      if (IsParticleEnabled(i)) {
        AnimationData()->SetParticleEffectState(rstl::string_l(skGasJetLocators[i]), false, mgr);
      }
    }
    x5d0_enabledParticles = 0;
  }
}
