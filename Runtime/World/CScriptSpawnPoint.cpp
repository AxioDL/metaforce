#include "CScriptSpawnPoint.hpp"
#include "CStateManager.hpp"
#include "CWorld.hpp"
#include "CPlayer.hpp"
#include "Particle/CGenDescription.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptSpawnPoint::CScriptSpawnPoint(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                                     const rstl::reserved_vector<u32, int(CPlayerState::EItemType::Max)>& itemCounts,
                                     bool defaultSpawn, bool active, bool morphed)
: CEntity(uid, info, active, name), x34_xf(xf), x64_itemCounts(itemCounts)
{
    x64_itemCounts[int(CPlayerState::EItemType::ThermalVisor)] = 1;
    x64_itemCounts[int(CPlayerState::EItemType::XRayVisor)] = 1;
    x64_itemCounts[int(CPlayerState::EItemType::GrappleBeam)] = 1;
    x64_itemCounts[int(CPlayerState::EItemType::BoostBall)] = 1;
    x64_itemCounts[int(CPlayerState::EItemType::ChargeBeam)] = 1;
    x10c_24_firstSpawn = defaultSpawn;
    x10c_25_morphed = morphed;
}

void CScriptSpawnPoint::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptSpawnPoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr)
{
    CEntity::AcceptScriptMsg(msg, objId, stateMgr);

    if (msg == EScriptObjectMessage::SetToZero || msg == EScriptObjectMessage::Reset)
    {
        if (msg == EScriptObjectMessage::Reset)
        {
            using EPlayerItemType = CPlayerState::EItemType;
            const std::shared_ptr<CPlayerState>& plState = stateMgr.GetPlayerState();
            for (u32 i = 0; i < u32(EPlayerItemType::Max); ++i)
            {
                plState->ReInitalizePowerUp(EPlayerItemType(i), GetPowerup(EPlayerItemType(i)));
                plState->ResetAndIncrPickUp(EPlayerItemType(i), GetPowerup(EPlayerItemType(i)));
            }
        }

        if (GetActive())
        {
            CPlayer* player = stateMgr.Player();

            if (x4_areaId != stateMgr.GetNextAreaId())
            {
                CGameArea* area = stateMgr.WorldNC()->GetArea(x4_areaId);
                if (area->IsPostConstructed() && area->GetOcclusionState() == CGameArea::EOcclusionState::Occluded)
                {
                    /* while (!area->TryTakingOutOfARAM()) {} */
                    CWorld::PropogateAreaChain(CGameArea::EOcclusionState::Visible, area, stateMgr.WorldNC());
                }

                stateMgr.SetCurrentAreaId(x4_areaId);
                stateMgr.SetActorAreaId(*stateMgr.Player(), x4_areaId);
                player->Teleport(GetTransform(), stateMgr, true);
                player->SetSpawnedMorphBallState(CPlayer::EPlayerMorphBallState(x10c_25_morphed), stateMgr);

                if (area->IsPostConstructed() && area->GetOcclusionState() == CGameArea::EOcclusionState::Visible)
                    CWorld::PropogateAreaChain(CGameArea::EOcclusionState::Occluded,
                                               stateMgr.WorldNC()->GetArea(stateMgr.GetNextAreaId()),
                                               stateMgr.WorldNC());
            }
            else
            {
                player->Teleport(GetTransform(), stateMgr, true);
                player->SetSpawnedMorphBallState(CPlayer::EPlayerMorphBallState(x10c_25_morphed), stateMgr);
            }
        }

        CEntity::SendScriptMsgs(EScriptObjectState::Zero, stateMgr, EScriptObjectMessage::None);
    }
}

u32 CScriptSpawnPoint::GetPowerup(CPlayerState::EItemType item) const
{
    int idx = int(item);
    if (idx >= int(CPlayerState::EItemType::Max) || idx < 0)
        return x64_itemCounts.front();
    return x64_itemCounts[idx];
}
}
