#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"

#include "MetroidPrime/CActorParameters.hpp"

#include "Kyoto/Math/CUnitVector3f.hpp"

CScriptCoverPoint::CScriptCoverPoint(TUniqueId uid, const rstl::string& name,
                                     const CEntityInfo& info, const CTransform4f& xf,
                                     const bool active, uint flags, bool crouch,
                                     float horizontalAngle, float verticalAngle, float coverTime)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_flags(flags)
, xec_cosHorizontalAngle(cosf(horizontalAngle * 0.008726646f))
, xf0_sinVerticalAngle(sinf(verticalAngle * 0.008726646f))
, xf4_coverTime(coverTime)
, xf8_24_crouch(crouch)
, xf8_25_inUse(false)
, xfa_occupant(kInvalidUniqueId)
, xfc_retreating(kInvalidUniqueId)
, x100_touchBounds(CAABox(xf.GetTranslation(), xf.GetTranslation()))
, x11c_timeLeft(0.f) {}

ENTITY_ACCEPT_IMPL(CScriptCoverPoint)

void CScriptCoverPoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                        CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_InitializedInArea:
    for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
         conn != GetConnectionList().end(); ++conn) {
      if (conn->x0_state == kSS_Retreat) {
        xfc_retreating = mgr.GetIdForScript(conn->x8_objId);
        break;
      }
    }
  default:
    break;
  }
}

pas::ECoverDirection CScriptCoverPoint::GetAttackDirection() const {
  return pas::ECoverDirection(xe8_flags);
}

bool CScriptCoverPoint::ShouldCrouch() const { return xf8_24_crouch; }

bool CScriptCoverPoint::ShouldStay() const { return xe8_flags >> 3 & 1; }

bool CScriptCoverPoint::ShouldWallHang() const { return xe8_flags >> 4 & 1; }

bool CScriptCoverPoint::ShouldLandHere() const { return xe8_flags >> 5 & 1; }

float CScriptCoverPoint::GetCosHorizontalAngle() const { return xec_cosHorizontalAngle; }

float CScriptCoverPoint::GetSinSqVerticalAngle() const {
  return xf0_sinVerticalAngle * xf0_sinVerticalAngle;
}

const bool CScriptCoverPoint::Blown(const CVector3f& point) const {
  bool result = true;

  if (GetActive()) {
    if (ShouldWallHang()) {
      result = false;
    } else {
      CVector3f posDif = point - GetTransform().GetTranslation();
      float magnitude = posDif.Magnitude();
      posDif /= magnitude;
      if (magnitude > 8.0f) {
        CUnitVector3f normDif(CVector3f(posDif.GetX(), posDif.GetY(), 0.f), CUnitVector3f::kN_Yes);
        CUnitVector3f frontVec(CVector3f(GetTransform().GetColumn(kDY).GetX(),
                               GetTransform().GetColumn(kDY).GetY(), 0.f), CUnitVector3f::kN_Yes);
        if (CVector3f::Dot(frontVec, normDif) > GetCosHorizontalAngle() &&
            (posDif.GetZ() * posDif.GetZ()) < GetSinSqVerticalAngle())
          result = false;
      }
    }
  }
  return result;
}

bool CScriptCoverPoint::GetInUse(TUniqueId uid) const {
  return xf8_25_inUse || x11c_timeLeft > 0.f ||
         (xfa_occupant != kInvalidUniqueId && uid != kInvalidUniqueId && uid != xfa_occupant);
}

void CScriptCoverPoint::SetInUse(bool inUse) {
  xf8_25_inUse = inUse;
  if (!xf8_25_inUse)
    x11c_timeLeft = xf4_coverTime;
}

void CScriptCoverPoint::Think(float delta, CStateManager&) {
  if (x11c_timeLeft > 0.f)
    x11c_timeLeft -= delta;
}

void CScriptCoverPoint::AddToRenderer(const CFrustumPlanes&, const CStateManager&) const {}

void CScriptCoverPoint::Render(const CStateManager&) const {}

rstl::optional_object< CAABox > CScriptCoverPoint::GetTouchBounds() const {
  return x100_touchBounds;
}
