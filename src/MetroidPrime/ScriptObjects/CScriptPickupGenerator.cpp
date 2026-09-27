#include "MetroidPrime/ScriptObjects/CScriptPickupGenerator.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CWallCrawlerSwarm.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickup.hpp"

#include "rstl/math.hpp"

CScriptPickupGenerator::CScriptPickupGenerator(const TUniqueId uid, const rstl::string& name,
                                               const CEntityInfo& info, const CVector3f& pos,
                                               const float frequency, const bool active)
: CEntity(uid, info, active, name)
, mPosition(pos)
, mFrequency(frequency)
, mDelayTimer(0.0f) {
  ResetSpawnNothingCounter();
}

CScriptPickupGenerator::~CScriptPickupGenerator() {}

void CScriptPickupGenerator::GetTargets(CStateManager& mgr, TUniqueId sender,
                                        rstl::vector< TUniqueId >& idsOut) const {
  idsOut.reserve(rstl::max_val(1, GetConnectionList().size()));
  rstl::vector< SConnection >::const_iterator iter = GetConnectionList().begin();
  for (; iter != GetConnectionList().end(); ++iter) {
    if (iter->mState == kSS_Zero && iter->mMsg == kSM_Follow) {
      const TUniqueId id = mgr.GetIdForScript(iter->mObjId);
      if (id != kInvalidUniqueId) {
        if (const CEntity* ent = mgr.GetObjectById(id)) {
          if (ent->GetActive()) {
            idsOut.push_back(id);
          }
        }
      }
    }
  }
  if (idsOut.empty()) {
    idsOut.push_back(sender);
  }
}

float CScriptPickupGenerator::GetSpawnablePickups(
    CStateManager& mgr, rstl::vector< rstl::pair< float, TEditorId > >& idsOut) const {
  float totalPossibility = 0.f;
  CPlayerState& pState = *mgr.PlayerState();
  idsOut.reserve(GetConnectionList().size());
  rstl::vector< SConnection >::const_iterator iter = GetConnectionList().begin();
  for (; iter != GetConnectionList().end(); ++iter) {
    if (iter->mState == kSS_Zero && iter->mMsg == kSM_Activate) {
      const TUniqueId id = mgr.GetIdForScript(iter->mObjId);
      if (id != kInvalidUniqueId) {
        if (const CScriptPickup* const pickup =
                TCastToConstPtr< CScriptPickup >(mgr.GetObjectById(id))) {
          const CPlayerState::EItemType item = pickup->GetItem();
          float possibility = pickup->GetPossibility();
          float multiplier = 1.f;
          bool doAlways = false;
          bool doThirtyPerc = false;
          switch (item) {
          case CPlayerState::kIT_Missiles:
            if (pState.HasPowerUp(CPlayerState::kIT_Missiles)) {
              if (pState.GetItemAmount(CPlayerState::kIT_Missiles) <
                  pState.GetItemCapacity(CPlayerState::kIT_Missiles)) {
                doAlways = true;
              } else {
                doThirtyPerc = true;
              }
            }
            break;
          case CPlayerState::kIT_PowerBombs:
            if (pState.HasPowerUp(CPlayerState::kIT_PowerBombs)) {
              if (pState.GetItemAmount(CPlayerState::kIT_PowerBombs) <
                  pState.GetItemCapacity(CPlayerState::kIT_PowerBombs)) {
                doAlways = true;
                if (pState.GetItemAmount(CPlayerState::kIT_PowerBombs) < 2 && possibility >= 10.f &&
                    possibility < 25.f) {
                  multiplier = 2.f;
                }
              } else {
                doThirtyPerc = true;
              }
            }
            break;
          case CPlayerState::kIT_HealthRefill: {
            CHealthInfo* healthInfo = pState.HealthInfo();
            if (healthInfo && healthInfo->GetHP() < pState.CalculateHealth()) {
              doAlways = true;
            } else {
              doThirtyPerc = true;
            }
          } break;
          default:
            doAlways = true;
            break;
          }
          const bool thirtyPercTest = mgr.Random()->Float() < 0.3f;
          if ((doAlways || (doThirtyPerc && thirtyPercTest)) && possibility > 0.f) {
            totalPossibility += possibility * multiplier;
            idsOut.push_back(rstl::pair< float, TEditorId >(possibility, iter->mObjId));
          }
        }
      }
    }
  }
  return totalPossibility;
}

void CScriptPickupGenerator::SpawnPickup(CStateManager& mgr, TEditorId templateId,
                                         TUniqueId generatorId) const {
  TUniqueId templateUniqueId = mgr.GetIdForScript(templateId);
  CEntity* pickupTempl = mgr.ObjectById(templateUniqueId);
  CEntity* generator = mgr.ObjectById(generatorId);

  if (pickupTempl && generator) {
    bool oldGeneratingObject = mgr.IsGeneratingObject();
    mgr.SetIsGeneratingObject(true);
    TUniqueId p = mgr.GenerateObject(templateId).second;
    mgr.SetIsGeneratingObject(oldGeneratingObject);

    if (p == kInvalidUniqueId) {
      return;
    }

    CEntity* newObj = mgr.ObjectById(p);
    CActor* const newAct = TCastToPtr< CActor >(newObj);
    CScriptPickup* newPickup = TCastToPtr< CScriptPickup >(newObj);
    const CActor* const generatorAct = TCastToConstPtr< CActor >(generator);
    const CWallCrawlerSwarm* swarmAct = TCastToConstPtr< CWallCrawlerSwarm >(generator);

    if (newAct && swarmAct) {
      newAct->SetTranslation(swarmAct->GetLastKilledOffset() + mPosition);
    } else if (newAct && generatorAct) {
      newAct->SetTranslation(generatorAct->GetTranslation() + mPosition);
    }

    if (newPickup) {
      newPickup->SetWasGenerated();
    }

    mgr.DeliverScriptMsg(newObj, GetUniqueId(), kSM_Activate);
  }
}

void CScriptPickupGenerator::ResetSpawnNothingCounter() {
  if (mFrequency > 0.f) {
    mDelayTimer += 100.f / mFrequency;
  } else {
    mDelayTimer = FLT_MAX;
  }
}

void CScriptPickupGenerator::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                             CStateManager& stateMgr) {
  switch (msg) {
  case kSM_SetToZero:
    if (GetActive() && mFrequency != 100.f) {
      mDelayTimer -= 1.f;
      if (mDelayTimer < 0.00001f) {
        ResetSpawnNothingCounter();
      } else {
        rstl::vector< TUniqueId > generatorIds;
        GetTargets(stateMgr, sender, generatorIds);
        rstl::vector< rstl::pair< float, TEditorId > > pickupTemplates;
        const float totalProb = GetSpawnablePickups(stateMgr, pickupTemplates);
        if (pickupTemplates.empty()) {
          break;
        } else {
          int count = 0;
          const float r = stateMgr.Random()->Range(0.f, totalProb);
          float f2 = 0.f;

          for (; count < pickupTemplates.size(); ++count) {
            const float probability = pickupTemplates[count].first;
            if (r >= f2 && r <= f2 + probability) {
              break;
            }
            f2 += probability;
          }
          if (count == pickupTemplates.size()) {
            break;
          } else {
            const rstl::pair< float, TEditorId > pickup = pickupTemplates[count];
            SpawnPickup(stateMgr, pickup.second,
                        generatorIds[stateMgr.Random()->Float() * generatorIds.size() * 0.99f]);
          }
        }
      }
    }
    break;
  default:
    break;
  }

  CEntity::AcceptScriptMsg(msg, sender, stateMgr);
}

ENTITY_ACCEPT_IMPL(CScriptPickupGenerator)
