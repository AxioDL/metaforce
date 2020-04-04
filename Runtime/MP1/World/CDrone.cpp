#include "Runtime/MP1/World/CDrone.hpp"

#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
CDrone::CDrone(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
               const zeus::CTransform& xf, float f1, CModelData&& mData, const CPatternedInfo& pInfo,
               const CActorParameters& actParms, EMovementType movement, EColliderType colliderType, EBodyType bodyType,
               const CDamageInfo& dInfo1, CAssetId aId1, const CDamageInfo& dInfo2, CAssetId aId2,
               std::vector<CVisorFlare::CFlareDef> flares, float f2, float f3, float f4, float f5, float f6, float f7,
               float f8, float f9, float f10, float f11, float f12, float f13, float f14, float f15, float f16,
               float f17, float f18, float f19, float f20, CAssetId crscId, float f21, float f22, float f23, float f24,
               s32 sId, bool b1)
: CPatterned(ECharacter::Drone, uid, name, flavor, info, xf, std::move(mData), pInfo, movement, colliderType, bodyType,
             actParms, EKnockBackVariant(flavor == EFlavorType::Zero))
, x568_(aId1)
, x56c_(g_SimplePool->GetObj({SBIG('CRSC'), crscId}))
, x57c_flares(std::move(flares))
, x590_(dInfo1)
, x5ac_(dInfo2)
, x5e4_(f23)
, x5ec_(f1)
, x5f0_(f2)
, x5f4_(f3)
, x5f8_(f4)
, x5fc_(f5)
, x600_(f11)
, x608_(f6)
, x60c_(f7)
, x610_(f8)
, x614_(f9)
, x618_(f10)
, x61c_(f12)
, x620_(f20)
, x63c_(f13)
, x640_(f14)
, x648_(f15)
, x64c_(f16)
, x650_(f17)
, x654_(f18)
, x658_(f19)
, x65c_(f21)
, x660_(f22)
, x664_(f24)
, x690_(zeus::CSphere({0.f, 0.f, 1.8f}, 1.1f), CActor::GetMaterialList())
, x6b0_pathFind(nullptr, 3 + int(b1), pInfo.GetPathfindingIndex(), 1.f, 2.4f)
, x7cc_(CSfxManager::TranslateSFXID(sId))
, x82c_(std::make_unique<CModelData>(CStaticRes{aId2, zeus::skOne3f}))
, x830_13_(0)
, x830_10_(0)
, x834_24_(false)
, x834_25_(false)
, x834_26_(false)
, x834_27_(false)
, x834_28_(false)
, x834_29_(false)
, x834_30_(false)
, x834_31_(false)
, x835_24_(false)
, x835_25_(b1)
, x835_26_(false) {
  UpdateTouchBounds(pInfo.GetHalfExtent());
  x460_knockBackController.SetEnableShock(true);
  x460_knockBackController.SetAvailableState(EKnockBackAnimationState::Hurled, false);
  x460_knockBackController.SetLocomotionDuringElectrocution(true);
  MakeThermalColdAndHot();
  CreateShadow(flavor != EFlavorType::One);
}

void CDrone::Accept(IVisitor& visitor) { visitor.Visit(this); }
void CDrone::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, sender, mgr);

  switch (msg) {
  case EScriptObjectMessage::Registered:
    x450_bodyController->Activate(mgr);
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    x450_bodyController->BodyStateInfo().SetMaximumPitch(0.f);
    break;
  default:
    break;
  }
}

void CDrone::UpdateTouchBounds(float radius) {
  const zeus::CTransform xf = GetLctrTransform("Skeleton_Root"sv);
  const zeus::CVector3f diff = xf.origin - GetTranslation();
  SetBoundingBox(zeus::CAABox{diff - radius, diff + radius});
  x6b0_pathFind.SetRadius(0.25f + radius);
}

} // namespace urde::MP1
