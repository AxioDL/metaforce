#include "Runtime/MP1/World/CFlyingPirate.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"

namespace urde::MP1 {
namespace {
constexpr std::array<SBurst, 6> skBurst1{{
    {4, {3, 4, 11, 12, -1, 0, 0, 0}, 0.1f, 0.05f},
    {20, {2, 3, 4, 5, -1, 0, 0, 0}, 0.1f, 0.05f},
    {20, {10, 11, 12, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {25, {15, 16, 1, 2, -1, 0, 0, 0}, 0.1f, 0.05f},
    {25, {5, 6, 7, 8, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<SBurst, 6> skBurst2{{
    {5, {3, 4, 8, 12, -1, 0, 0, 0}, 0.1f, 0.05f},
    {10, {2, 3, 4, 5, -1, 0, 0, 0}, 0.1f, 0.05f},
    {10, {10, 11, 12, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {40, {15, 16, 1, 2, -1, 0, 0, 0}, 0.1f, 0.05f},
    {35, {5, 6, 7, 8, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<SBurst, 5> skBurst3{{
    {30, {3, 4, 5, 11, 12, 4, -1, 0}, 0.1f, 0.05f},
    {20, {2, 3, 4, 5, 4, 3, -1, 0}, 0.1f, 0.05f},
    {20, {5, 4, 3, 13, 12, 11, -1, 0}, 0.1f, 0.05f},
    {30, {1, 2, 3, 4, 5, 6, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<SBurst, 5> skBurst4{{
    {10, {6, 5, 4, 14, 13, 12, -1, 0}, 0.1f, 0.05f},
    {20, {14, 13, 12, 11, 10, 9, -1, 0}, 0.1f, 0.05f},
    {20, {14, 15, 16, 11, 10, 9, -1, 0}, 0.1f, 0.05f},
    {50, {11, 10, 9, 8, 7, 6, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<const SBurst*, 5> skBursts{
    skBurst1.data(), skBurst2.data(), skBurst3.data(), skBurst4.data(), nullptr,
};
} // namespace
CFlyingPirate::CFlyingPirateData::CFlyingPirateData(CInputStream& in, u32 propCount)
: x0_(in.readFloatBig())
, x4_(in.readFloatBig())
, x8_(in.readInt32Big())
, xc_projInfo1(in)
, x34_sfx1(in.readUint16Big())
, x38_projInfo2(in)
, x60_projInfo3(in.readInt32Big(), {})
, x88_(in.readFloatBig())
, x8c_(in.readFloatBig())
, x90_particleGenDesc(g_SimplePool->GetObj({SBIG('PART'), in.readInt32Big()}))
, x9c_dInfo(in)
, xb8_(in.readFloatBig())
, xbc_(in.readFloatBig())
, xc0_(in.readFloatBig())
, xc4_(in.readFloatBig())
, xca_sfx3(in.readUint16Big())
, xcc_(in.readFloatBig())
, xd0_(in.readFloatBig())
, xd4_(in.readFloatBig())
, xd8_(in)
, xdc_(in)
, xe0_(in)
, xe4_sfx4(in.readUint16Big())
, xe6_sfx5(in.readUint16Big())
, xe8_(in.readFloatBig())
, xec_(in.readFloatBig())
, xf0_(propCount < 36 ? 0.f : in.readFloatBig()) {
  xc_projInfo1.Token().Lock();
  x38_projInfo2.Token().Lock();
  x60_projInfo3.Token().Lock();
}

CFlyingPirate::CFlyingPirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                             CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
                             CInputStream& in, u32 propCount)
: CPatterned(ECharacter::FlyingPirate, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::NewFlyer, actParms, EKnockBackVariant::Medium)
, x568_data(in, propCount)
, x6a8_pathFindSearch(nullptr, 0 /* TODO */, pInfo.GetHalfExtent(), pInfo.GetHeight(), pInfo.GetPathfindingIndex())
, x7a0_boneTracking(*GetModelData()->GetAnimationData(), "Head_1", zeus::degToRad(80.f), zeus::degToRad(180.f),
                    EBoneTrackingFlags::None)
, x7ec_burstFire(skBursts.data(), 0) {
  CModelData* modelData = GetModelData();
  CAnimData* animData = modelData->GetAnimationData();
  x798_ = animData->GetLocatorSegId("Head_1");
  x7e0_ = animData->GetLocatorSegId("L_gun_LCTR");
  x864_missileSegments.push_back(animData->GetLocatorSegId("L_Missile_LCTR"));
  x864_missileSegments.push_back(animData->GetLocatorSegId("R_Missile_LCTR"));
  x850_ = modelData->GetScale().x() * GetAnimationDistance({3, CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(1)});
  if (x568_data.xd8_.IsValid() && x568_data.xdc_.IsValid() && x568_data.xe0_.IsValid()) {
    x65c_particleGenDescs.push_back(g_SimplePool->GetObj({SBIG('PART'), x568_data.xd8_}));
    x65c_particleGenDescs.push_back(g_SimplePool->GetObj({SBIG('PART'), x568_data.xdc_}));
    x65c_particleGenDescs.push_back(g_SimplePool->GetObj({SBIG('PART'), x568_data.xe0_}));
    for (const auto& desc : x65c_particleGenDescs) {
      x684_particleGens.push_back(std::make_unique<CElementGen>(desc));
    }
  }
  x460_knockBackController.SetLocomotionDuringElectrocution(true);
}
} // namespace urde::MP1
