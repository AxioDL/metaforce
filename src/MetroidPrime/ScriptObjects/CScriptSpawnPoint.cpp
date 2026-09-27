#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

CScriptSpawnPoint::CScriptSpawnPoint(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const rstl::reserved_vector< int, int(CPlayerState::kIT_Max) >& itemCounts,
    const bool defaultSpawn, const bool active, const bool morphed)
: CEntity(uid, info, active, name)
, mXf(xf)
, mItemCounts(itemCounts)
, mFirstSpawn(defaultSpawn)
, mMorphed(morphed) {}

CScriptSpawnPoint::~CScriptSpawnPoint() {}

const CTransform4f& CScriptSpawnPoint::GetTransform() const { return mXf; }

int CScriptSpawnPoint::GetPowerup(const CPlayerState::EItemType& type) const {
  if (CPlayerState::kIT_Max <= type || type < 0) {
    return mItemCounts.front();
  }
  return mItemCounts[type];
}

void CScriptSpawnPoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                        CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);

  switch (msg) {
  case kSM_Reset:
    for (int i = 0; i < CPlayerState::kIT_Max; ++i) {
      const CPlayerState::EItemType e = static_cast< CPlayerState::EItemType >(i);
      stateMgr.PlayerState()->SetPowerUp(e, GetPowerup(e));
      stateMgr.PlayerState()->SetPickup(e, GetPowerup(e));
    }
  case kSM_SetToZero:
    if (GetActive()) {
      CPlayer* player = stateMgr.Player();
      TAreaId thisAreaId = GetCurrentAreaId();
      TAreaId nextAreaId = stateMgr.GetNextAreaId();

      if (nextAreaId != thisAreaId) {
        bool propagateAgain = false;

        CGameArea* area = stateMgr.World()->Area(thisAreaId);
        if (area->GetOcclusionState() == CGameArea::kOS_Occluded) {
          while (!area->TryTakingOutOfARAM()) {
          }
          CWorld::PropogateAreaChain(CGameArea::kOS_Visible, area, stateMgr.World());
          propagateAgain = true;
        }

        stateMgr.SetCurrentAreaId(thisAreaId);
        stateMgr.SetActorAreaId(*player, thisAreaId);
        player->Teleport(mXf, stateMgr, true);
        player->SetSpawnedMorphBallState(
            mMorphed ? CPlayer::kMS_Morphed : CPlayer::kMS_Unmorphed, stateMgr);

        if (propagateAgain) {
          CWorld::PropogateAreaChain(CGameArea::kOS_Occluded, stateMgr.World()->Area(nextAreaId),
                                     stateMgr.World());
        }

      } else {
        player->Teleport(mXf, stateMgr, true);
        player->SetSpawnedMorphBallState(
            mMorphed ? CPlayer::kMS_Morphed : CPlayer::kMS_Unmorphed, stateMgr);
      }
      CEntity::SendScriptMsgs(kSS_Zero, stateMgr, kSM_None);
    }
  default:
    break;
  }
}

ENTITY_ACCEPT_IMPL(CScriptSpawnPoint)
