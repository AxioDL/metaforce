#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CMetroidPrimeRelay.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

#include "rstl/algorithm.hpp"

CScriptPlayerHint::CScriptPlayerHint(TUniqueId uid, const rstl::string& name,
                                     const CEntityInfo& info, const CTransform4f& xf,
                                     const bool active, int priority, int overrideFlags)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, xfc_deactivated(false)
, x100_priority(priority)
, x104_overrideFlags(overrideFlags)
, x108_mpId(kInvalidUniqueId) {}

void CScriptPlayerHint::ClearObjectList() { xe8_objectList.clear(); }

void CScriptPlayerHint::AddToObjectList(TUniqueId uid) {
  rstl::reserved_vector< TUniqueId, 8 >::iterator it =
      rstl::find(xe8_objectList.begin(), xe8_objectList.end(), uid);
  if (it != xe8_objectList.end()) {
    return;
  }
  xe8_objectList.push_back(uid);
}

void CScriptPlayerHint::RemoveFromObjectList(TUniqueId uid, CStateManager& mgr) {
  if (xe8_objectList.empty()) {
    return;
  }

  rstl::reserved_vector< TUniqueId, 8 >::iterator it =
      rstl::find(xe8_objectList.begin(), xe8_objectList.end(), uid);

  if (it == xe8_objectList.end()) {
    xe8_objectList.erase(xe8_objectList.begin());
  } else {
    xe8_objectList.erase(it);
  }
}

void CScriptPlayerHint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                        CStateManager& mgr) {

  switch (msg) {
  case kSM_Deactivate:
  case kSM_Deleted: {
    RemoveFromObjectList(sender, mgr);
    CPlayer* player = mgr.Player();
    player->AddToPlayerHintRemoveList(GetUniqueId(), mgr);
    xfc_deactivated = true;
    break;
  }
  case kSM_Increment:
    x108_mpId = kInvalidUniqueId;
    if ((x104_overrideFlags & 0x4000) != 0) {
      rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
      for (; it != GetConnectionList().end(); ++it) {
        if (it->x0_state != kSS_Play) {
          continue;
        }
        x108_mpId = mgr.GetIdForScript(it->x8_objId);
        if (const CMetroidPrimeRelay* mpRelay =
                TCastToConstPtr< CMetroidPrimeRelay >(mgr.GetObjectById(x108_mpId))) {
          x108_mpId = mpRelay->GetMetroidPrimeExoId();
          break;
        }
      }
    }
    break;
  default:
    break;
  }
  if (GetActive()) {
    CPlayer* player = mgr.Player();

    switch (msg) {
    case kSM_Increment:
      AddToObjectList(sender);
      player->AddToPlayerHintAddList(GetUniqueId(), mgr);
      xfc_deactivated = false;
      break;

    case kSM_Decrement:
      RemoveFromObjectList(sender, mgr);
      player->AddToPlayerHintRemoveList(GetUniqueId(), mgr);
      break;

    default:
      break;
    }
  }

  CActor::AcceptScriptMsg(msg, sender, mgr);
}

ENTITY_ACCEPT_IMPL(CScriptPlayerHint)

CScriptPlayerHint::~CScriptPlayerHint() {}
