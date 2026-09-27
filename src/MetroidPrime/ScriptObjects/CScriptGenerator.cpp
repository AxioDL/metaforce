#include "MetroidPrime/ScriptObjects/CScriptGenerator.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CWallCrawlerSwarm.hpp"

#include "rstl/algorithm.hpp"

CScriptGenerator::CScriptGenerator(const TUniqueId uid, const rstl::string& name,
                                   const CEntityInfo& info, const int spawnCount,
                                   const bool noReuseFollowers, const CVector3f& vec1,
                                   const bool noInheritXf, const bool active, const float minScale,
                                   const float maxScale)
: CEntity(uid, info, active, name)
, mSpawnCount(spawnCount)
, mNoReuseFollowers(noReuseFollowers)
, mNoInheritTransform(noInheritXf)
, mOffset(vec1)
, mMinScale(minScale)
, mMaxScale(maxScale) {}

CScriptGenerator::~CScriptGenerator() {}

void CScriptGenerator::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                       CStateManager& stateMgr) {
  switch (msg) {
  case kSM_SetToZero: {
    if (!GetActive()) {
      break;
    }

    rstl::vector< TUniqueId > follows;
    follows.reserve(!GetConnectionList().empty() ? GetConnectionList().size() : 1);
    rstl::vector< SConnection >::const_iterator iter = GetConnectionList().begin();
    for (; iter != GetConnectionList().end(); ++iter) {
      if (iter->mState != kSS_Zero || iter->mMsg != kSM_Follow) {
        continue;
      }

      const TUniqueId uid = stateMgr.GetIdForScript(iter->mObjId);
      if (uid != kInvalidUniqueId) {
        const CEntity* entity = stateMgr.GetObjectById(uid);
        if (entity && entity->GetActive()) {
          follows.push_back(uid);
        }
      }
    }

    if (follows.empty()) {
      follows.push_back(sender);
    }

    rstl::vector< rstl::pair< TUniqueId, TEditorId > > activates;
    activates.reserve(GetConnectionList().size());

    for (iter = GetConnectionList().begin(); iter != GetConnectionList().end(); ++iter) {
      if (iter->mState != kSS_Zero) {
        continue;
      }

      TUniqueId uid = stateMgr.GetIdForScript(iter->mObjId);
      if (uid == kInvalidUniqueId) {
        continue;
      }

      if (iter->mMsg == kSM_Activate) {
        if (!stateMgr.GetObjectById(uid)) {
          continue;
        }
        activates.push_back(rstl::pair< TUniqueId, TEditorId >(uid, iter->mObjId));
      } else {
        stateMgr.SendScriptMsgAlways(uid, GetUniqueId(), iter->mMsg);
      }
    }

    if (activates.empty()) {
      break;
    }

    for (int i = 0; i < mSpawnCount; ++i) {
      if (activates.size() == 0 || follows.size() == 0) {
        break;
      }

      int activatesRand = 0.99f * (stateMgr.Random()->Float() * activates.size());
      const int followsRand = 0.99f * (stateMgr.Random()->Float() * follows.size());

      for (int j = 0; j < activates.size(); ++j) {
        if (TCastToConstPtr< CScriptSound >(stateMgr.GetObjectById(activates[j].first))) {
          activatesRand = j;
          break;
        }
      }

      const rstl::pair< TUniqueId, TEditorId > idPair = activates[activatesRand];
      CEntity* activate = stateMgr.ObjectById(idPair.first);
      CEntity* follow = stateMgr.ObjectById(follows[followsRand]);

      if (!activate || !follow) {
        break;
      }

      const bool oldGeneratingObject = stateMgr.IsGeneratingObject();
      stateMgr.SetIsGeneratingObject(true);
      TUniqueId objId = stateMgr.GenerateObject(idPair.second).second;
      stateMgr.SetIsGeneratingObject(oldGeneratingObject);

      if (objId != kInvalidUniqueId) {
        CEntity* genObj = stateMgr.ObjectById(objId);
        CActor* activateActor = TCastToPtr< CActor >(genObj);
        const CActor* followActor = TCastToConstPtr< CActor >(follow);
        const CWallCrawlerSwarm* wallCrawlerSwarm = TCastToConstPtr< CWallCrawlerSwarm >(follow);

        if (activateActor && wallCrawlerSwarm) {
          if (!mNoInheritTransform) {
            activateActor->SetTransform(wallCrawlerSwarm->GetTransform());
          }
          activateActor->SetTranslation(wallCrawlerSwarm->GetLastKilledOffset() + mOffset);
        } else if (activateActor && followActor) {
          if (!mNoInheritTransform) {
            activateActor->SetTransform(followActor->GetTransform());
          }
          activateActor->SetTranslation(followActor->GetTranslation() + mOffset);
        }
        if (genObj) {
          CEntity* genObj = stateMgr.ObjectById(objId);
          CActor* activateActor = TCastToPtr< CActor >(genObj);
          const CActor* followActor = TCastToConstPtr< CActor >(follow);
          const CWallCrawlerSwarm* wallCrawlerSwarm = TCastToConstPtr< CWallCrawlerSwarm >(follow);

          if (activateActor) {
            if (activateActor && wallCrawlerSwarm) {
              if (!mNoInheritTransform) {
                activateActor->SetTransform(wallCrawlerSwarm->GetTransform());
              }
              activateActor->SetTranslation(wallCrawlerSwarm->GetLastKilledOffset() + mOffset);
            } else if (activateActor && followActor) {
              if (!mNoInheritTransform) {
                activateActor->SetTransform(followActor->GetTransform());
              }
              activateActor->SetTranslation(followActor->GetTranslation() + mOffset);
            }
          }

          const float rnd = stateMgr.Random()->Range(mMinScale, mMaxScale);
          if (activateActor->HasModelData()) {
            activateActor->ModelData()->SetScale(rnd * activateActor->ModelData()->GetScale());
          }

          stateMgr.DeliverScriptMsg(genObj, GetUniqueId(), kSM_Activate);
        }
      }

      activates.erase(activates.begin() + activatesRand);
      if (mNoReuseFollowers) {
        follows.erase(follows.begin() + followsRand);
      }
    }
    break;
  }
  default:
    break;
  }

  CEntity::AcceptScriptMsg(msg, sender, stateMgr);
}

ENTITY_ACCEPT_IMPL(CScriptGenerator)
