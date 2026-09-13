#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"

CScriptAiJumpPoint::CScriptAiJumpPoint(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, const CTransform4f& xf,
                                       const bool active, float apex)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_apex(apex)
, xec_touchBounds(CAABox(xf.GetTranslation(), xf.GetTranslation()))
, x108_24_inUse(false)
, x10a_occupant(kInvalidUniqueId)
, x10c_currentWaypoint(kInvalidUniqueId)
, x10e_nextWaypoint(kInvalidUniqueId)
, x110_timeRemaining(0.f) {}

ENTITY_ACCEPT_IMPL(CScriptAiJumpPoint)

void CScriptAiJumpPoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other,
                                         CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, other, mgr);

  switch (msg) {
  case kSM_InitializedInArea:
    rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
    for (; conn != GetConnectionList().end(); ++conn) {
      if (conn->x0_state != kSS_Arrived || conn->x4_msg != kSM_Next) {
        continue;
      }
      TUniqueId id = mgr.GetIdForScript(conn->x8_objId);
      if (const CScriptWaypoint* wpnt = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
        x10c_currentWaypoint = id;
        x10e_nextWaypoint = wpnt->NextWaypoint(mgr);
        return;
      }
    }
  }
}

bool CScriptAiJumpPoint::GetInUse(TUniqueId uid) const {
  return x108_24_inUse || x110_timeRemaining > 0.f ||
         (x10a_occupant != kInvalidUniqueId && uid != kInvalidUniqueId && uid != x10a_occupant);
}

void CScriptAiJumpPoint::Think(float dt, CStateManager&) {
  if (x110_timeRemaining > 0) {
    x110_timeRemaining -= dt;
  }
}

void CScriptAiJumpPoint::AddToRenderer(const CFrustumPlanes&, const CStateManager&) const {}

void CScriptAiJumpPoint::Render(const CStateManager&) const {}

rstl::optional_object< CAABox > CScriptAiJumpPoint::GetTouchBounds() const {
  return xec_touchBounds;
}

CScriptAiJumpPoint::~CScriptAiJumpPoint() {}
