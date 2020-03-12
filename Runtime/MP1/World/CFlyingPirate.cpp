#include "Runtime/MP1/World/CFlyingPirate.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"

namespace urde::MP1 {
namespace {
constexpr std::array<const SBurst, 6> skBurst1{{
    {4, {3, 4, 11, 12, -1, 0, 0, 0}, 0.1f, 0.05f},
    {20, {2, 3, 4, 5, -1, 0, 0, 0}, 0.1f, 0.05f},
    {20, {10, 11, 12, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {25, {15, 16, 1, 2, -1, 0, 0, 0}, 0.1f, 0.05f},
    {25, {5, 6, 7, 8, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<const SBurst, 6> skBurst2{{
    {5, {3, 4, 8, 12, -1, 0, 0, 0}, 0.1f, 0.05f},
    {10, {2, 3, 4, 5, -1, 0, 0, 0}, 0.1f, 0.05f},
    {10, {10, 11, 12, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {40, {15, 16, 1, 2, -1, 0, 0, 0}, 0.1f, 0.05f},
    {35, {5, 6, 7, 8, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<const SBurst, 5> skBurst3{{
    {30, {3, 4, 5, 11, 12, 4, -1, 0}, 0.1f, 0.05f},
    {20, {2, 3, 4, 5, 4, 3, -1, 0}, 0.1f, 0.05f},
    {20, {5, 4, 3, 13, 12, 11, -1, 0}, 0.1f, 0.05f},
    {30, {1, 2, 3, 4, 5, 6, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<const SBurst, 5> skBurst4{{
    {10, {6, 5, 4, 14, 13, 12, -1, 0}, 0.1f, 0.05f},
    {20, {14, 13, 12, 11, 10, 9, -1, 0}, 0.1f, 0.05f},
    {20, {14, 15, 16, 11, 10, 9, -1, 0}, 0.1f, 0.05f},
    {50, {11, 10, 9, 8, 7, 6, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<const SBurst* const, 5> skBursts{
    skBurst1.data(), skBurst2.data(), skBurst3.data(), skBurst4.data(), nullptr,
};

constexpr std::array<const std::string_view, 14> skParts{
    "s_Head",  "s_R_shoulder", "s_R_elbow", "s_R_wrist", "s_L_shoulder", "s_L_elbow", "s_L_wrist",
    "s_R_hip", "s_R_knee",     "s_R_ankle", "s_L_hip",   "s_L_knee",     "s_L_ankle", "s_rocket_LCTR",
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

CFlyingPirate::CFlyingPirateRagDoll::CFlyingPirateRagDoll(CStateManager& mgr, CFlyingPirate* actor, u16 w1, u16 w2)
: CRagDoll(-actor->GetGravityConstant(), 3.f, 8.f, 0)
, x88_(w1)
, x9c_(w2)
, xa4_(actor->GetDestPos() - actor->GetTranslation()) {
  actor->RemoveMaterial(EMaterialTypes::Solid, EMaterialTypes::AIBlock, EMaterialTypes::GroundCollider, mgr);
  actor->HealthInfo(mgr)->SetHP(-1.f);
  SetNumParticles(15);
  SetNumLengthConstraints(45);
  SetNumJointConstraints(4);
  CAnimData* animData = actor->GetModelData()->GetAnimationData();
  animData->BuildPose();
  const zeus::CVector3f& center = actor->GetBoundingBox().center();
  for (const auto& part : skParts) {
    const CSegId& id = animData->GetLocatorSegId(part);
    AddParticle(id, center, center * animData->GetPose().GetOffset(id), 0.45f * center.z());
  }
  SatisfyWorldConstraintsOnConstruction(mgr);
  AddLengthConstraint(0, 1);
  AddLengthConstraint(0, 2);
  AddLengthConstraint(0, 8);
  AddLengthConstraint(0, 11);
  AddLengthConstraint(0, 5);
  AddLengthConstraint(2, 3);
  AddLengthConstraint(3, 4);
  AddLengthConstraint(5, 6);
  AddLengthConstraint(6, 7);
  AddLengthConstraint(2, 5);
  AddLengthConstraint(2, 11);
  AddLengthConstraint(5, 8);
  AddLengthConstraint(5, 11);
  AddLengthConstraint(8, 11);
  AddLengthConstraint(8, 9);
  AddLengthConstraint(9, 10);
  AddLengthConstraint(11, 12);
  AddLengthConstraint(12, 13);
  AddLengthConstraint(14, 0);
  AddLengthConstraint(14, 2);
  AddLengthConstraint(14, 5);
  AddLengthConstraint(14, 8);
  AddLengthConstraint(14, 11);
  AddMinLengthConstraint(1, 8, x14_lengthConstraints[2].GetLength());
  AddMinLengthConstraint(1, 11, x14_lengthConstraints[3].GetLength());
  AddMinLengthConstraint(4, 2, x14_lengthConstraints[5].GetLength());
  AddMinLengthConstraint(7, 5, x14_lengthConstraints[7].GetLength());
  AddMinLengthConstraint(3, 5, 0.5f * x14_lengthConstraints[5].GetLength() + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(6, 2, 0.5f * x14_lengthConstraints[7].GetLength() + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(4, 5, 0.5f * x14_lengthConstraints[5].GetLength() + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(7, 2, 0.5f * x14_lengthConstraints[7].GetLength() + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(10, 8, x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(11, 11, x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(9, 2, 0.707f * x14_lengthConstraints[15].GetLength() + x14_lengthConstraints[10].GetLength());
  AddMinLengthConstraint(12, 5, 0.707f * x14_lengthConstraints[17].GetLength() + x14_lengthConstraints[13].GetLength());
  AddMinLengthConstraint(9, 11, x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(12, 8, x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(10, 0, x14_lengthConstraints[2].GetLength() + x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(13, 0, x14_lengthConstraints[3].GetLength() + x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(9, 12, 0.5f * x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(10, 12, 0.5f * x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(13, 9, 0.5f * x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(10, 13, 0.5f * x14_lengthConstraints[14].GetLength());
  AddJointConstraint(8, 2, 5, 8, 9, 10);
  AddJointConstraint(11, 2, 5, 11, 12, 13);
  AddJointConstraint(2, 11, 5, 2, 3, 4);
  AddJointConstraint(5, 2, 8, 5, 6, 7);
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
  const CModelData* modelData = GetModelData();
  const CAnimData* animData = modelData->GetAnimationData();
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
