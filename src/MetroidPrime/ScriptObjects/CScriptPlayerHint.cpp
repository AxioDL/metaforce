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
, mDeactivated(false)
, mPriority(priority)
, mOverrideFlags(overrideFlags)
, mMpId(kInvalidUniqueId) {}

void CScriptPlayerHint::ClearObjectList() { mObjectList.clear(); }

void CScriptPlayerHint::AddToObjectList(TUniqueId uid) {
  rstl::reserved_vector< TUniqueId, 8 >::iterator it =
      rstl::find(mObjectList.begin(), mObjectList.end(), uid);
  if (it != mObjectList.end()) {
    return;
  }
  mObjectList.push_back(uid);
}

void CScriptPlayerHint::RemoveFromObjectList(TUniqueId uid, CStateManager& mgr) {
  if (mObjectList.empty()) {
    return;
  }

  rstl::reserved_vector< TUniqueId, 8 >::iterator it =
      rstl::find(mObjectList.begin(), mObjectList.end(), uid);

  if (it == mObjectList.end()) {
    mObjectList.erase(mObjectList.begin());
  } else {
    mObjectList.erase(it);
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
    mDeactivated = true;
    break;
  }
  case kSM_Increment:
    mMpId = kInvalidUniqueId;
    if ((mOverrideFlags & 0x4000) != 0) {
      rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
      for (; it != GetConnectionList().end(); ++it) {
        if (it->mState != kSS_Play) {
          continue;
        }
        mMpId = mgr.GetIdForScript(it->mObjId);
        if (const CMetroidPrimeRelay* mpRelay =
                TCastToConstPtr< CMetroidPrimeRelay >(mgr.GetObjectById(mMpId))) {
          mMpId = mpRelay->GetMetroidPrimeExoId();
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
      mDeactivated = false;
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
