#include "CScriptPlayerHint.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"
#include "CStateManager.hpp"
#include "CPlayer.hpp"
#include "MP1/World/CMetroidPrimeRelay.hpp"

namespace urde
{

CScriptPlayerHint::CScriptPlayerHint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                     const zeus::CTransform& xf, bool active, u32 priority, u32 overrideFlags)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), {EMaterialTypes::NoStepLogic},
         CActorParameters::None(), kInvalidUniqueId), x100_priority(priority), x104_overrideFlags(overrideFlags)
{}

void CScriptPlayerHint::Accept(IVisitor& visit)
{
    visit.Visit(this);
}

void CScriptPlayerHint::AddToObjectList(TUniqueId uid)
{
    for (TUniqueId existId : xe8_objectList)
        if (uid == existId)
            return;
    if (xe8_objectList.size() != 8)
        xe8_objectList.push_back(uid);
}

void CScriptPlayerHint::RemoveFromObjectList(TUniqueId uid)
{
    for (auto it = xe8_objectList.begin() ; it != xe8_objectList.end() ; ++it)
    {
        if (*it == uid)
        {
            xe8_objectList.erase(it);
            return;
        }
    }
}

void CScriptPlayerHint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr)
{
    switch (msg)
    {
    case EScriptObjectMessage::Deactivate:
    case EScriptObjectMessage::Deleted:
        RemoveFromObjectList(sender);
        mgr.GetPlayer().AddToPlayerHintRemoveList(GetUniqueId(), mgr);
        xfc_deactivated = true;
        break;
    case EScriptObjectMessage::Increment:
        x108_mpId = kInvalidUniqueId;
        if ((x104_overrideFlags & 0x4000) != 0)
        {
            for (const SConnection& conn : x20_conns)
            {
                if (conn.x0_state != EScriptObjectState::Play)
                    continue;
                x108_mpId = mgr.GetIdForScript(conn.x8_objId);
                if (TCastToConstPtr<MP1::CMetroidPrimeRelay> mpRelay = mgr.GetObjectById(x108_mpId))
                {
                    x108_mpId = mpRelay->GetMetroidPrimeExoId();
                    break;
                }
            }
        }
        break;
    default:
        break;
    }

    if (x30_24_active)
    {
        switch (msg)
        {
        case EScriptObjectMessage::Increment:
            AddToObjectList(sender);
            mgr.GetPlayer().AddToPlayerHintAddList(GetUniqueId(), mgr);
            xfc_deactivated = false;
            break;
        case EScriptObjectMessage::Decrement:
            RemoveFromObjectList(sender);
            mgr.GetPlayer().AddToPlayerHintRemoveList(GetUniqueId(), mgr);
            break;
        default:
            break;
        }
    }

    CActor::AcceptScriptMsg(msg, sender, mgr);
}

}