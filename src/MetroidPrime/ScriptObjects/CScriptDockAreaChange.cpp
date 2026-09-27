#include "MetroidPrime/ScriptObjects/CScriptDockAreaChange.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"

CScriptDockAreaChange::CScriptDockAreaChange(const TUniqueId uid, const rstl::string& name,
                                             const CEntityInfo& info, const int w1, const bool active)
: CEntity(uid, info, active, name), mDockReference(w1) {}

void CScriptDockAreaChange::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                            CStateManager& stateMgr) {

  switch (msg) {
  case kSM_Action:
    if (GetActive()) {
      rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
      for (; it != GetConnectionList().end(); ++it) {
        if (it->mState != kSS_Play || it->mMsg == kSM_UNKM0) {
          continue;
        }

        CStateManager::TIdListResult search = stateMgr.GetIdListForScript(it->mObjId);
        CStateManager::TIdList::const_iterator searchItem = search.first;
        for (; searchItem != search.second; ++searchItem) {
          if (CScriptDock* dock =
                  TCastToPtr< CScriptDock >(stateMgr.ObjectById(searchItem->second))) {
            dock->SetDockReference(stateMgr, mDockReference);
          }
        }
      }

      SendScriptMsgs(kSS_Play, stateMgr, kSM_None);
    }
  }

  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

ENTITY_ACCEPT_IMPL(CScriptDockAreaChange)
