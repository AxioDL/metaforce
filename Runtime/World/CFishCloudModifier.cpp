#include "CFishCloudModifier.hpp"
#include "CFishCloud.hpp"
#include "CStateManager.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"

namespace urde
{
CFishCloudModifier::CFishCloudModifier(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& eInfo,
                                                   const zeus::CVector3f& pos, bool b2, bool b3, float f1, float f2)
    : CActor(uid, active, name, eInfo, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(), {EMaterialTypes::NoStepLogic},
             CActorParameters::None(), kInvalidUniqueId)
    , xe8_(f1)
    , xec_(f2)
    , xf0_isRepulsor(b2)
    , xf1_(b3)
{
}

void CFishCloudModifier::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CFishCloudModifier::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CActor::AcceptScriptMsg(msg, uid, mgr);

    if ((msg == EScriptObjectMessage::Activate || msg == EScriptObjectMessage::InitializedInArea) && GetActive())
        AddSelf(mgr);
    else if (msg == EScriptObjectMessage::Deactivate || msg == EScriptObjectMessage::Deleted)
        RemoveSelf(mgr);
}

void CFishCloudModifier::AddSelf(CStateManager& mgr)
{
    for (const SConnection& conn : GetConnectionList())
    {
        if (conn.x0_state != EScriptObjectState::Modify || conn.x4_msg != EScriptObjectMessage::Follow)
            continue;

        if (TCastToPtr<CFishCloud> fishCloud = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId)))
        {
            if (xf0_isRepulsor)
                fishCloud->AddRepulsor(GetUniqueId(), xe8_, xec_);
            else
                fishCloud->AddAttractor(GetUniqueId(), xe8_, xec_);
        }
    }
}

void CFishCloudModifier::RemoveSelf(CStateManager& mgr)
{
    for (const SConnection& conn : GetConnectionList())
    {
        if (conn.x0_state != EScriptObjectState::Modify || conn.x4_msg != EScriptObjectMessage::Follow)
            continue;

        if (TCastToPtr<CFishCloud> fishCloud = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId)))
        {
            if (xf0_isRepulsor)
                fishCloud->RemoveRepulsor(GetUniqueId());
            else
                fishCloud->RemoveAttractor(GetUniqueId());
        }
    }
}
}
