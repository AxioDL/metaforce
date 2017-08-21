//
// Created by antidote on 8/19/17.
//

#include "CMetaree.hpp"

namespace urde
{
namespace MP1
{
CMetaree::CMetaree(TUniqueId uid, const std::string& name, EFlavorType flavor, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, const CDamageInfo& dInfo,
                   float f1, const zeus::CVector3f& v1, float f2, EBodyType bodyType, float f3, float f4,
                   const CActorParameters& aParms)
: CPatterned(ECharacter::Metaree, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             EColliderType::Zero, bodyType, aParms, 0)
, x568_(f3)
, x56c_(f4)
, x570_(f1)
, x574_(v1)
, x580_(f2)
, x5ca_24_(true)
, x5ca_25_(false)
, x5ca_26_(false)
{
}

void CMetaree::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CMetaree::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CPatterned::AcceptScriptMsg(msg, uid, mgr);

    if (msg == EScriptObjectMessage::Start)
        x5ca_25_ = true;
    else if (msg == EScriptObjectMessage::Registered)
        x450_bodyController->Activate(mgr);
}
}
}
