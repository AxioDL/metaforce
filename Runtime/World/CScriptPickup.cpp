#include "CScriptPickup.hpp"
#include "Particle/CGenDescription.hpp"
#include "GameGlobalObjects.hpp"
#include "MP1/CArtifactDoll.hpp"
#include "CExplosion.hpp"
#include "CSimplePool.hpp"
#include "TCastTo.hpp"
#include "CPlayer.hpp"
#include "CGameState.hpp"
#include "CGameOptions.hpp"
#include "MP1/CSamusHud.hpp"
#include "GuiSys/CStringTable.hpp"

namespace urde
{
CScriptPickup::CScriptPickup(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                             const zeus::CTransform& xf, CModelData&& mData, const CActorParameters& aParams,
                             const zeus::CAABox& aabb, CPlayerState::EItemType itemType, s32 amount, s32 capacity,
                             CAssetId explosionEffect, float possibility, float f2, float f3, float f4, bool active)
: CPhysicsActor(uid, active, name, info, xf, std::move(mData), CMaterialList(), aabb, SMoverData(1.f), aParams, 0.3f,
                0.1f)
, x258_itemType(itemType)
, x25c_amount(amount)
, x260_capacity(capacity)
, x264_possibility(possibility)
, x268_(f3)
, x26c_(f2)
, x278_(f4)
, x28c_24_(false)
, x28c_25_(false)
, x28c_26_(false)
{
    if (explosionEffect.IsValid())
        x27c_explosionDesc = g_SimplePool->GetObj({SBIG('PART'), explosionEffect});

    if (x64_modelData && x64_modelData->AnimationData())
        x64_modelData->AnimationData()->SetAnimation(CAnimPlaybackParms(0, -1, 1.f, true), false);

    if (x278_ != 0.f)
    {
        xb4_drawFlags = CModelFlags(5, 0, 3, zeus::CColor(1.f, 1.f, 1.f, 0.f));
        xb4_drawFlags.x2_flags &= 0xFFFC;
        xb4_drawFlags.x2_flags |= 1;
    }
}

void CScriptPickup::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptPickup::Think(float dt, CStateManager& mgr)
{
    if (!GetActive())
        return;


    if (x278_ >= 0.f)
    {
        CPhysicsActor::Stop();
        x278_ -= dt;
    }
    else
    {
        x270_ += dt;
        if (x28c_25_)
        {

        }
    }
}

void CScriptPickup::Touch(CActor& act, CStateManager& mgr)
{
    if (GetActive() && x278_ < 0.f && TCastToPtr<CPlayer>(act))
    {
        if (x258_itemType >= CPlayerState::EItemType::Truth && x258_itemType <= CPlayerState::EItemType::Newborn)
        {
            CAssetId id = MP1::CArtifactDoll::GetArtifactHeadScanFromItemType(x258_itemType);
            if (id.IsValid())
                mgr.GetPlayerState()->SetScanTime(id, 0.5f);
        }

        if (x27c_explosionDesc)
        {
            if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::Thermal)
            {
                mgr.AddObject(new CExplosion(
                    x27c_explosionDesc, mgr.AllocateUniqueId(), true,
                    CEntityInfo(GetAreaIdAlways(), CEntity::NullConnectionList, kInvalidEditorId),
                    "Explosion - Pickup Effect", x34_transform, 0, zeus::CVector3f::skOne, zeus::CColor::skWhite));
            }
        }

        mgr.GetPlayerState()->InitializePowerUp(x258_itemType, x260_capacity);
        mgr.GetPlayerState()->IncrPickup(x258_itemType, x25c_amount);
        mgr.FreeScriptObject(GetUniqueId());
        SendScriptMsgs(EScriptObjectState::Arrived, mgr, EScriptObjectMessage::None);

        if (x260_capacity > 0)
        {
            u32 total = mgr.GetPlayerState()->GetPickupTotal();
            u32 colRate = mgr.GetPlayerState()->CalculateItemCollectionRate();
            if (total == colRate)
            {
                CPersistentOptions& opts = g_GameState->SystemOptions();
                mgr.sub_80043F2C(mgr.GetHUDMessageFrameCount() + 1,
                                 g_ResFactory->GetResourceIdByName(opts.GetAllItemsCollected() ?
                                                                   "STRG_AllPickupsFound_2" :
                                                                   "STRG_AllPickupsFound_1")->id, 0.f);
                opts.SetAllItemsCollected(true);
            }
        }

        if (x258_itemType == CPlayerState::EItemType::PowerBombs &&
                g_GameState->SystemOptions().GetShowPowerBombAmmoMessage())
        {
            g_GameState->SystemOptions().IncrementPowerBombAmmoCount();
            MP1::CSamusHud::DisplayHudMemo(g_MainStringTable->GetString(109), {0.5f, true, false, false});
        }
    }
}
}
