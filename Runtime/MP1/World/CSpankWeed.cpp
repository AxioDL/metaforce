#include "CSpankWeed.hpp"
#include "World/CPatternedInfo.hpp"
#include <logvisor/logvisor.hpp>

namespace urde::MP1
{
logvisor::Module SpankLog("urde::MP1::SpankWeed");
CSpankWeed::CSpankWeed(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                       CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo, float f1,
                       float f2, float f3, float f4)
: CPatterned(ECharacter::SpankWeed, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Restricted, actParms, EKnockBackVariant::Medium)
, x568_(f1)
, x56c_(pInfo.GetDetectionHeightRange())
, x570_(f2)
, x574_(f3)
, x578_(f4)
, x584_(xf.origin)
{
    SetCallTouch(false);
    CreateShadow(false);

    zeus::CVector3f modelScale = GetModelData()->GetScale();
    if (modelScale.x != modelScale.y || modelScale.x != modelScale.z)
    {
        float scale = modelScale.magnitude() / std::sqrt(3.f);

        ModelData()->SetScale(zeus::CVector3f(scale));
        SpankLog.report(logvisor::Level::Warning, "WARNING: Non-uniform scale (%.2f, %.2f, %.2f) applied to Spank Weed"
                                                  "...changing scale to (%.2f, %.2f, %.2f)\n",
                                                  modelScale.x, modelScale.y, modelScale.z,
                                                  scale, scale, scale);
    }
    CMaterialList list = GetMaterialFilter().GetExcludeList();
    list.Add(EMaterialTypes::Character);
    list.Add(EMaterialTypes::Player);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(GetMaterialFilter().GetIncludeList(), list));

    CSegId segId = GetModelData()->GetAnimationData()->GetLocatorSegId("lockon_target_LCTR"sv);
    if (segId != 0xFF)
    {
        zeus::CTransform locatorXf = GetTransform() * zeus::CTransform::Scale(GetModelData()->GetScale()) *
                                     GetModelData()->GetAnimationData()->GetLocatorTransform(segId, nullptr);
        x5a8_ = locatorXf.origin;
        x59c_ = locatorXf.origin - GetTranslation();
    }
    x460_knockBackController.SetAutoResetImpulse(false);
}
}