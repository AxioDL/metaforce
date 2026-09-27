#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"

#include "MetroidPrime/CActorParameters.hpp"

#include "Kyoto/Math/CUnitVector3f.hpp"

CScriptCoverPoint::CScriptCoverPoint(TUniqueId uid, const rstl::string& name,
                                     const CEntityInfo& info, const CTransform4f& xf,
                                     const bool active, uint flags, bool crouch,
                                     float horizontalAngle, float verticalAngle, float coverTime)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mFlags(flags)
, mCosHorizontalAngle(cosf(horizontalAngle * 0.008726646f))
, mSinVerticalAngle(sinf(verticalAngle * 0.008726646f))
, mCoverTime(coverTime)
, mCrouch(crouch)
, mInUse(false)
, mOccupant(kInvalidUniqueId)
, mRetreating(kInvalidUniqueId)
, mTouchBounds(CAABox(xf.GetTranslation(), xf.GetTranslation()))
, mTimeLeft(0.f) {}

ENTITY_ACCEPT_IMPL(CScriptCoverPoint)

void CScriptCoverPoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                        CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_InitializedInArea:
    for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
         conn != GetConnectionList().end(); ++conn) {
      if (conn->mState == kSS_Retreat) {
        mRetreating = mgr.GetIdForScript(conn->mObjId);
        break;
      }
    }
  default:
    break;
  }
}

pas::ECoverDirection CScriptCoverPoint::GetAttackDirection() const {
  return pas::ECoverDirection(mFlags);
}

bool CScriptCoverPoint::ShouldCrouch() const { return mCrouch; }

bool CScriptCoverPoint::ShouldStay() const { return mFlags >> 3 & 1; }

bool CScriptCoverPoint::ShouldWallHang() const { return mFlags >> 4 & 1; }

bool CScriptCoverPoint::ShouldLandHere() const { return mFlags >> 5 & 1; }

float CScriptCoverPoint::GetCosHorizontalAngle() const { return mCosHorizontalAngle; }

float CScriptCoverPoint::GetSinSqVerticalAngle() const {
  return mSinVerticalAngle * mSinVerticalAngle;
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
  return mInUse || mTimeLeft > 0.f ||
         (mOccupant != kInvalidUniqueId && uid != kInvalidUniqueId && uid != mOccupant);
}

void CScriptCoverPoint::SetInUse(bool inUse) {
  mInUse = inUse;
  if (!mInUse)
    mTimeLeft = mCoverTime;
}

void CScriptCoverPoint::Think(float delta, CStateManager&) {
  if (mTimeLeft > 0.f)
    mTimeLeft -= delta;
}

void CScriptCoverPoint::AddToRenderer(const CFrustumPlanes&, const CStateManager&) const {}

void CScriptCoverPoint::Render(const CStateManager&) const {}

rstl::optional_object< CAABox > CScriptCoverPoint::GetTouchBounds() const {
  return mTouchBounds;
}
