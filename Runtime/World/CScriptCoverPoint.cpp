#include "Runtime/World/CScriptCoverPoint.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CActorParameters.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CScriptCoverPoint::CScriptCoverPoint(TUniqueId uid, std::string_view name, const CEntityInfo& info, zeus::CTransform xf,
                                     bool active, u32 flags, bool crouch, float horizontalAngle, float verticalAngle,
                                     float coverTime)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_26_landHere((flags & 0x20) != 0u)
, xe8_27_wallHang((flags & 0x10) != 0u)
, xe8_28_stay((flags & 0x8) != 0u)
, xe8_29_((flags & 0x4) != 0u)
, xe8_30_attackDirection((flags & 0x2) != 0u)
, xf4_coverTime(coverTime)
, xf8_24_crouch(crouch) {
  xec_cosHorizontalAngle = std::cos(zeus::degToRad(horizontalAngle) * 0.5f);
  xf0_sinVerticalAngle = std::sin(zeus::degToRad(verticalAngle) * 0.5f);
  x100_touchBounds.emplace(xf.origin, xf.origin);
}

void CScriptCoverPoint::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptCoverPoint::Think(float delta, CStateManager&) {
  if (x11c_timeLeft <= 0.f)
    return;
  x11c_timeLeft -= delta;
}

std::optional<zeus::CAABox> CScriptCoverPoint::GetTouchBounds() const {
  if (x100_touchBounds) {
    return x100_touchBounds;
  }

  return std::nullopt;
}

void CScriptCoverPoint::SetInUse(bool inUse) {
  xf8_25_inUse = inUse;
  if (inUse)
    x11c_timeLeft = xf4_coverTime;
}

bool CScriptCoverPoint::GetInUse(TUniqueId uid) const {
  if (xf8_25_inUse || x11c_timeLeft > 0.f)
    return true;

  return !(xfa_occupant == kInvalidUniqueId || uid == kInvalidUniqueId || xfa_occupant == uid);
}

bool CScriptCoverPoint::Blown(const zeus::CVector3f& point) const {
  if (!x30_24_active)
    return true;

  if (ShouldWallHang()) {
    zeus::CVector3f posDif = point - x34_transform.origin;
    posDif *= zeus::CVector3f(1.f / posDif.magnitude());
    zeus::CVector3f normDif = posDif.normalized();

    zeus::CVector3f frontVec = x34_transform.frontVector();
    frontVec.normalize();

    if (frontVec.dot(normDif) <= GetCosHorizontalAngle() || (posDif.z() * posDif.z()) >= GetSinSqVerticalAngle())
      return true;
  }
  return false;
}

float CScriptCoverPoint::GetSinSqVerticalAngle() const { return xf0_sinVerticalAngle * xf0_sinVerticalAngle; }

void CScriptCoverPoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  if (msg == EScriptObjectMessage::WorldInitialized) {
    for (const SConnection& con : x20_conns)
      if (con.x0_state == EScriptObjectState::Retreat) {
        xfc_retreating = mgr.GetIdForScript(con.x8_objId);
        break;
      }
  }
}

} // namespace urde
