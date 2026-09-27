#include "MetroidPrime/ScriptObjects/CScriptRandomRelay.hpp"

#include "MetroidPrime/CStateManager.hpp"

#include "rstl/math.hpp"

CScriptRandomRelay::CScriptRandomRelay(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, int sendSetSize,
                                       const int sendSetVariance, const bool percentSize,
                                       const bool active)
: CEntity(uid, info, active, name)
, mSendSetSize(sendSetSize)
, mSendSetVariance(sendSetVariance)
, mPercentSize(percentSize) {
  if (percentSize && mSendSetSize > 100) {
    mSendSetSize = 100;
  }
}

CScriptRandomRelay::~CScriptRandomRelay() {}

void CScriptRandomRelay::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                         CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
  switch (msg) {
  case kSM_SetToZero:
    if (GetActive()) {
      SendLocalScriptMsgs(kSS_Zero, stateMgr);
    }
    break;
  default:
    break;
  }
}

void CScriptRandomRelay::SendLocalScriptMsgs(EScriptObjectState state, CStateManager& stateMgr) {
  switch (state) {
  case kSS_Zero: {
    rstl::vector< rstl::pair< CEntity*, EScriptObjectMessage > > objs;
    objs.reserve(10);

    rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
    for (; conn != GetConnectionList().end(); ++conn) {
      if (conn->mState == kSS_Zero) {
        CObjectList& objList = stateMgr.ObjectListById(kOL_All);
        CStateManager::TIdListResult list = stateMgr.GetIdListForScript(conn->mObjId);
        if (!(list.first == list.second)) {
          for (CStateManager::TIdList::const_iterator it = list.first; it != list.second; ++it) {
            CEntity* ent = objList.GetObjectById(it->second);
            if (ent && ent->GetActive()) {
              objs.push_back(rstl::pair< CEntity*, EScriptObjectMessage >(ent, conn->mMsg));
            }
          }
        }
      }
    }

    int targetSetSize = mPercentSize ? int(0.5f + (float(mSendSetSize * objs.size()) / 100.f))
                                        : mSendSetSize;
    targetSetSize +=
        int(float(mSendSetVariance) * (stateMgr.Random()->Float() * 2.0f)) - mSendSetVariance;

    targetSetSize = rstl::min_val(rstl::max_val(0, targetSetSize), 64);

    while (objs.size() > targetSetSize) {
      const int randomRemoveIdx = int(stateMgr.Random()->Float() * float(objs.size()) * 0.99f);
      rstl::vector< rstl::pair< CEntity*, EScriptObjectMessage > >::iterator it = objs.begin();
      for (int i = 0; i < randomRemoveIdx; ++i) {
        ++it;
        if (it == objs.end()) {
          break;
        }
      }
      if (it != objs.end()) {
        objs.erase(it);
      }
    }

    for (rstl::vector< rstl::pair< CEntity*, EScriptObjectMessage > >::iterator it = objs.begin();
         it != objs.end(); ++it) {
      stateMgr.DeliverScriptMsg(it->first, GetUniqueId(), it->second);
    }
    break;
  }
  default:
    SendScriptMsgs(state, stateMgr, kSM_None);
    break;
  }
}

ENTITY_ACCEPT_IMPL(CScriptRandomRelay)
