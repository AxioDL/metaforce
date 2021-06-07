#include "Runtime/World/CScriptSpawnPoint.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

CScriptSpawnPoint::CScriptSpawnPoint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                     const zeus::CTransform& xf,
                                     const rstl::reserved_vector<u32, int(CPlayerState::EItemType::Max)>& itemCounts,
                                     bool defaultSpawn, bool active, bool morphed)
: CEntity(uid, info, active, name)
, x34_xf(xf)
, x64_itemCounts(itemCounts)
, x10c_24_firstSpawn(defaultSpawn)
, x10c_25_morphed(morphed) {}

void CScriptSpawnPoint::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptSpawnPoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);

  if (msg == EScriptObjectMessage::SetToZero || msg == EScriptObjectMessage::Reset) {
    if (msg == EScriptObjectMessage::Reset) {
      using EPlayerItemType = CPlayerState::EItemType;
      const std::shared_ptr<CPlayerState>& plState = stateMgr.GetPlayerState();
      for (u32 i = 0; i < u32(EPlayerItemType::Max); ++i) {
        plState->ReInitializePowerUp(EPlayerItemType(i), GetPowerup(EPlayerItemType(i)));
        plState->ResetAndIncrPickUp(EPlayerItemType(i), GetPowerup(EPlayerItemType(i)));
      }
    }

    if (GetActive()) {
      CPlayer* player = stateMgr.Player();

      if (x4_areaId != stateMgr.GetNextAreaId()) {
        CGameArea* area = stateMgr.GetWorld()->GetArea(x4_areaId);
        if (area->IsPostConstructed() && area->GetOcclusionState() == CGameArea::EOcclusionState::Occluded) {
          /* while (!area->TryTakingOutOfARAM()) {} */
          CWorld::PropogateAreaChain(CGameArea::EOcclusionState::Visible, area, stateMgr.GetWorld());
        }

        stateMgr.SetCurrentAreaId(x4_areaId);
        stateMgr.SetActorAreaId(*stateMgr.Player(), x4_areaId);
        player->Teleport(GetTransform(), stateMgr, true);
        player->SetSpawnedMorphBallState(CPlayer::EPlayerMorphBallState(x10c_25_morphed), stateMgr);

        if (area->IsPostConstructed() && area->GetOcclusionState() == CGameArea::EOcclusionState::Visible) {
          CWorld::PropogateAreaChain(CGameArea::EOcclusionState::Occluded,
                                     stateMgr.GetWorld()->GetArea(stateMgr.GetNextAreaId()), stateMgr.GetWorld());
        }
      } else {
        player->Teleport(GetTransform(), stateMgr, true);
        player->SetSpawnedMorphBallState(CPlayer::EPlayerMorphBallState(x10c_25_morphed), stateMgr);
      }
    }

    CEntity::SendScriptMsgs(EScriptObjectState::Zero, stateMgr, EScriptObjectMessage::None);
  }
}

u32 CScriptSpawnPoint::GetPowerup(CPlayerState::EItemType item) const {
  const auto idx = static_cast<size_t>(item);
  if (item >= CPlayerState::EItemType::Max) {
    return x64_itemCounts.front();
  }
  return x64_itemCounts[idx];
}
} // namespace metaforce
