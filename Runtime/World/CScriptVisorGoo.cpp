#include "Runtime/World/CScriptVisorGoo.hpp"

#include "Runtime/Factory/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CHUDBillboardEffect.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

CScriptVisorGoo::CScriptVisorGoo(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                 const zeus::CTransform& xf, CAssetId particle, CAssetId electric, float minDist,
                                 float maxDist, float nearProb, float farProb, const zeus::CColor& color, int sfx,
                                 bool forceShow, bool active)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), {}, CActorParameters::None(), kInvalidUniqueId)
, xe8_particleDesc(CToken(TObjOwnerDerivedFromIObj<CGenDescription>::GetNewDerivedObject({})))
, xf0_electricDesc(CToken(TObjOwnerDerivedFromIObj<CElectricDescription>::GetNewDerivedObject({})))
, xf8_sfx(CSfxManager::TranslateSFXID(sfx))
, xfc_particleId(particle)
, x100_electricId(electric)
, x104_minDist(minDist)
, x108_maxDist(std::max(maxDist, minDist + 0.01f))
, x10c_nearProb(nearProb)
, x110_farProb(farProb)
, x114_color(color) {
  x118_24_angleTest = !forceShow;
  if (particle.IsValid()) {
    xe8_particleDesc = g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), particle});
  }
  if (electric.IsValid()) {
    xf0_electricDesc = g_SimplePool->GetObj(SObjectTag{FOURCC('ELSC'), electric});
  }
}

void CScriptVisorGoo::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptVisorGoo::Think(float, CStateManager& mgr) {
  if (GetActive()) {
    bool loaded = false;
    if (xfc_particleId.IsValid()) {
      if (xe8_particleDesc.IsLoaded()) {
        if (x100_electricId.IsValid()) {
          loaded = xf0_electricDesc.IsLoaded();
        } else {
          loaded = true;
        }
      }
    } else {
      loaded = xf0_electricDesc.IsLoaded();
    }

    if (loaded) {
      bool showGoo = false;
      if (mgr.GetPlayer().GetCameraState() == CPlayer::EPlayerCameraState::FirstPerson) {
        const zeus::CVector3f eyeToGoo = GetTranslation() - mgr.GetPlayer().GetEyePosition();
        const float eyeToGooDist = eyeToGoo.magnitude();
        if (eyeToGooDist >= x104_minDist && eyeToGooDist <= x108_maxDist) {
          if (x118_24_angleTest) {
            const float angle = zeus::radToDeg(
                std::acos(mgr.GetCameraManager()->GetCurrentCameraTransform(mgr).basis[1].normalized().dot(
                    eyeToGoo.normalized())));
            float angleThresh = 45.f;
            if (eyeToGooDist < 4.f) {
              angleThresh *= 4.f / eyeToGooDist;
              angleThresh = std::min(90.f, angleThresh);
            }
            if (angle <= angleThresh) {
              showGoo = true;
            }
          } else {
            showGoo = true;
          }
          if (showGoo) {
            const float t = (x108_maxDist - eyeToGooDist) / (x108_maxDist - x104_minDist);
            if (mgr.GetActiveRandom()->Float() * 100.f <= (1.f - t) * x110_farProb + t * x10c_nearProb) {
              mgr.AddObject(new CHUDBillboardEffect(
                  xfc_particleId.IsValid() ? std::make_optional(xe8_particleDesc) : std::nullopt,
                  x100_electricId.IsValid() ? std::make_optional(xf0_electricDesc) : std::nullopt,
                  mgr.AllocateUniqueId(), true, "VisorGoo", CHUDBillboardEffect::GetNearClipDistance(mgr),
                  CHUDBillboardEffect::GetScaleForPOV(mgr), x114_color, zeus::skOne3f, zeus::skZero3f));
              CSfxManager::SfxStart(xf8_sfx, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
            }
          }
        }
      }
      mgr.FreeScriptObject(GetUniqueId());
    }
  }
}

void CScriptVisorGoo::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Activate:
    if (xfc_particleId.IsValid()) {
      xe8_particleDesc.Lock();
    }
    if (x100_electricId.IsValid()) {
      xf0_electricDesc.Lock();
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(msg, objId, mgr);
}

void CScriptVisorGoo::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  // Empty
}

void CScriptVisorGoo::Render(CStateManager& mgr) {
  // Empty
}

std::optional<zeus::CAABox> CScriptVisorGoo::GetTouchBounds() const { return std::nullopt; }

void CScriptVisorGoo::Touch(CActor& other, CStateManager& mgr) {
  // Empty
}

} // namespace metaforce
