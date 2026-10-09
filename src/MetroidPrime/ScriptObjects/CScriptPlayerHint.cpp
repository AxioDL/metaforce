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

void CScriptPlayerHint::ClearSenders() { mSenders.clear(); }

void CScriptPlayerHint::AddSender(TUniqueId uid) {
  rstl::reserved_vector< TUniqueId, 8 >::iterator it =
      rstl::find(mSenders.begin(), mSenders.end(), uid);
  if (it != mSenders.end()) {
    return;
  }
  mSenders.push_back(uid);
}

void CScriptPlayerHint::RemoveSender(TUniqueId uid, CStateManager& mgr) {
  if (mSenders.empty()) {
    return;
  }

  rstl::reserved_vector< TUniqueId, 8 >::iterator it =
      rstl::find(mSenders.begin(), mSenders.end(), uid);

  if (it == mSenders.end()) {
    mSenders.erase(mSenders.begin());
  } else {
    mSenders.erase(it);
  }
}

void CScriptPlayerHint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                        CStateManager& mgr) {
  switch (msg) {
  case kSM_Deleted:
  case kSM_Deactivate: {
    RemoveSender(sender, mgr);
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
      AddSender(sender);
      player->AddToPlayerHintAddList(GetUniqueId(), mgr);
      mDeactivated = false;
      break;

    case kSM_Decrement:
      RemoveSender(sender, mgr);
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
