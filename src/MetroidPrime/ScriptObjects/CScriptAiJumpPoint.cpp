#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"

CScriptAiJumpPoint::CScriptAiJumpPoint(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, const CTransform4f& xf,
                                       const bool active, float apex)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mApex(apex)
, mTouchBounds(CAABox(xf.GetTranslation(), xf.GetTranslation()))
, mInUse(false)
, mOccupant(kInvalidUniqueId)
, mCurrentWaypoint(kInvalidUniqueId)
, mNextWaypoint(kInvalidUniqueId)
, mTimeRemaining(0.f) {}

ENTITY_ACCEPT_IMPL(CScriptAiJumpPoint)

void CScriptAiJumpPoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other,
                                         CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, other, mgr);

  switch (msg) {
  case kSM_InitializedInArea:
    rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
    for (; conn != GetConnectionList().end(); ++conn) {
      if (conn->mState != kSS_Arrived || conn->mMsg != kSM_Next) {
        continue;
      }
      TUniqueId id = mgr.GetIdForScript(conn->mObjId);
      if (const CScriptWaypoint* wpnt = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
        mCurrentWaypoint = id;
        mNextWaypoint = wpnt->NextWaypoint(mgr);
        return;
      }
    }
  }
}

bool CScriptAiJumpPoint::GetInUse(TUniqueId uid) const {
  return mInUse || mTimeRemaining > 0.f ||
         (mOccupant != kInvalidUniqueId && uid != kInvalidUniqueId && uid != mOccupant);
}

void CScriptAiJumpPoint::Think(float dt, CStateManager&) {
  if (mTimeRemaining > 0) {
    mTimeRemaining -= dt;
  }
}

void CScriptAiJumpPoint::AddToRenderer(const CFrustumPlanes&, const CStateManager&) const {}

void CScriptAiJumpPoint::Render(const CStateManager&) const {}

rstl::optional_object< CAABox > CScriptAiJumpPoint::GetTouchBounds() const {
  return mTouchBounds;
}

CScriptAiJumpPoint::~CScriptAiJumpPoint() {}
