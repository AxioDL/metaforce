#include "CSpacePirate.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "Character/CPASAnimParmData.hpp"
#include "World/CScriptWaypoint.hpp"
#include "World/CPatternedInfo.hpp"
#include "TCastTo.hpp"

namespace urde::MP1
{
CSpacePirate::CSpacePirateData::CSpacePirateData(urde::CInputStream& in, u32 propCount)
: x0_(in.readFloatBig()), x4_(in.readFloatBig()), x8_(in.readFloatBig()), xc_(in.readFloatBig())
, x10_(in.readFloatBig()), x14_(in.readFloatBig()), x18_(in.readUint32Big()), x1c_(in.readBool()), x20_(in)
, x48_(CSfxManager::TranslateSFXID(in.readUint32Big())), x4c_(in), x68_(in.readFloatBig()), x6c_(in)
, x94_(in.readFloatBig()), x98_(CSfxManager::TranslateSFXID(in.readUint32Big())), x9c_(in.readFloatBig())
, xa0_(in.readFloatBig()), xa4_(CSfxManager::TranslateSFXID(in.readUint32Big())), xa8_(in.readFloatBig())
, xac_firstBurstCount(in.readUint32Big()), xb0_(in.readFloatBig()), xb4_(in.readFloatBig()), xb8_(in.readFloatBig())
, xbc_(in.readFloatBig()), xc0_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xc2_(CSfxManager::TranslateSFXID(in.readUint32Big())), xc4_(propCount > 35 ? in.readFloatBig() : 0.2f)
, xc8_(propCount > 36 ? in.readFloatBig() : 8.f)
{}

static const std::string_view skParts[] =
{
    "Collar"sv,
    "Neck_1"sv,
    "R_shoulder"sv,
    "R_elbow"sv,
    "R_wrist"sv,
    "L_shoulder"sv,
    "L_elbow"sv,
    "L_wrist"sv,
    "R_hip"sv,
    "R_knee"sv,
    "R_ankle"sv,
    "L_hip"sv,
    "L_knee"sv,
    "L_ankle"sv
};

static const float skRadii[] =
{
    0.45f,
    0.52f,
    0.35f,
    0.1f,
    0.15f,
    0.35f,
    0.1f,
    0.15f,
    0.15f,
    0.15f,
    0.15f,
    0.15f,
    0.15f,
    0.15f
};

CPirateRagDoll::CPirateRagDoll(CStateManager& mgr, CSpacePirate* sp, u16 s1, u32 flags)
: CRagDoll(-sp->GetGravityConstant(), -3.f, 8.f, flags), x6c_spacePirate(sp), x70_s1(s1)
{
    xb0_24_ = true;
    x6c_spacePirate->RemoveMaterial(EMaterialTypes::Solid, EMaterialTypes::AIBlock,
                                    EMaterialTypes::GroundCollider, mgr);
    x6c_spacePirate->HealthInfo(mgr)->SetHP(-1.f);
    SetNumParticles(14);
    SetNumLengthConstraints(47);
    SetNumJointConstraints(4);
    zeus::CVector3f scale = x6c_spacePirate->GetModelData()->GetScale();
    CAnimData* aData = x6c_spacePirate->ModelData()->AnimationData();
    aData->BuildPose();
    zeus::CVector3f center = x6c_spacePirate->GetBoundingBox().center();
    for (int i = 0; i < 14; ++i)
    {
        CSegId id = aData->GetLocatorSegId(skParts[i]);
        AddParticle(id, center, x6c_spacePirate->GetTransform() * (aData->GetPose().GetOffset(id) * scale),
                    skRadii[i] * scale.z);
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
    AddLengthConstraint(2, 8);
    AddLengthConstraint(2, 11);
    AddLengthConstraint(5, 8);
    AddLengthConstraint(5, 11);
    AddLengthConstraint(8, 11);
    AddLengthConstraint(8, 9);
    AddLengthConstraint(9, 10);
    AddLengthConstraint(11, 12);
    AddLengthConstraint(12, 13);
    AddMinLengthConstraint(1, 8, x14_lengthConstraints[2].GetLength());
    AddMinLengthConstraint(1, 11, x14_lengthConstraints[3].GetLength());
    AddMinLengthConstraint(1, 2, x14_lengthConstraints[1].GetLength() * 0.9f);
    AddMinLengthConstraint(1, 5, x14_lengthConstraints[4].GetLength() * 0.9f);
    AddMinLengthConstraint(1, 4, x14_lengthConstraints[0].GetLength() * 2.5f);
    AddMinLengthConstraint(1, 7, x14_lengthConstraints[0].GetLength() * 2.5f);
    AddMinLengthConstraint(4, 2, x14_lengthConstraints[5].GetLength());
    AddMinLengthConstraint(7, 5, x14_lengthConstraints[7].GetLength());
    AddMinLengthConstraint(3, 5, x14_lengthConstraints[5].GetLength() * 0.5f + x14_lengthConstraints[9].GetLength());
    AddMinLengthConstraint(6, 2, x14_lengthConstraints[7].GetLength() * 0.5f + x14_lengthConstraints[9].GetLength());
    AddMinLengthConstraint(4, 5, x14_lengthConstraints[5].GetLength() * 0.5f + x14_lengthConstraints[9].GetLength());
    AddMinLengthConstraint(7, 2, x14_lengthConstraints[7].GetLength() * 0.5f + x14_lengthConstraints[9].GetLength());
    AddMinLengthConstraint(4, 7, x14_lengthConstraints[9].GetLength());
    AddMinLengthConstraint(4, 8, x14_lengthConstraints[14].GetLength());
    AddMinLengthConstraint(7, 11, x14_lengthConstraints[14].GetLength());
    AddMinLengthConstraint(10, 8, x14_lengthConstraints[15].GetLength());
    AddMinLengthConstraint(13, 11, x14_lengthConstraints[17].GetLength());
    AddMinLengthConstraint(9, 2, x14_lengthConstraints[15].GetLength() * 0.707f + x14_lengthConstraints[10].GetLength());
    AddMinLengthConstraint(12, 5, x14_lengthConstraints[17].GetLength() * 0.707f + x14_lengthConstraints[13].GetLength());
    AddMinLengthConstraint(9, 11, x14_lengthConstraints[15].GetLength());
    AddMinLengthConstraint(12, 8, x14_lengthConstraints[17].GetLength());
    AddMinLengthConstraint(10, 0, x14_lengthConstraints[2].GetLength() + x14_lengthConstraints[15].GetLength());
    AddMinLengthConstraint(13, 0, x14_lengthConstraints[3].GetLength() + x14_lengthConstraints[17].GetLength());
    AddMinLengthConstraint(10, 13, x14_lengthConstraints[14].GetLength());
    AddMinLengthConstraint(9, 12, x14_lengthConstraints[14].GetLength());
    AddMinLengthConstraint(10, 12, x14_lengthConstraints[14].GetLength());
    AddMinLengthConstraint(13, 9, x14_lengthConstraints[14].GetLength());
    AddMaxLengthConstraint(10, 13, x14_lengthConstraints[14].GetLength() * 5.f);
    AddJointConstraint(8, 2, 5, 8, 9, 10); // R_hip, R_shoulder, L_shoulder, R_hip, R_knee, R_ankle
    AddJointConstraint(11, 2, 5, 11, 12, 13); // L_hip, R_shoulder, L_shoulder, L_hip, L_knee, L_ankle
    AddJointConstraint(2, 11, 5, 2, 3, 4); // R_shoulder, L_hip, L_shoulder, R_shoulder, R_elbow, R_wrist
    AddJointConstraint(5, 2, 8, 5, 6, 7); // L_shoulder, R_shoulder, R_hip, L_shoulder, L_elbow, R_wrist
    for (const auto& conn : x6c_spacePirate->GetConnectionList())
    {
        if (conn.x0_state == EScriptObjectState::Modify && conn.x4_msg == EScriptObjectMessage::Follow)
        {
            TUniqueId wpId = mgr.GetIdForScript(conn.x8_objId);
            if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(wpId))
            {
                x90_waypoints.push_back(wpId);
                x9c_wpParticleIdxs.push_back(wp->GetAnimation());
                if (x90_waypoints.size() == 4)
                    break;
            }
        }
    }
}

void CPirateRagDoll::PreRender(const zeus::CVector3f& v, CModelData& mData)
{
    if (!x68_25_ || x68_27_)
    {
        CAnimData* aData = mData.AnimationData();
        for (CSegId id : aData->GetCharLayoutInfo().GetSegIdList().GetList())
            if (aData->GetCharLayoutInfo().GetRootNode()->GetBoneMap()[id].x10_children.size() > 1)
                aData->PoseBuilder().GetTreeMap()[id].x4_rotation = zeus::CQuaternion::skNoRotation;
        CSegId rootId = aData->GetLocatorSegId("Skeleton_Root"sv);
        aData->PoseBuilder().GetTreeMap()[rootId].x14_offset =
            (0.5f * (x4_particles[8].GetPosition() + x4_particles[11].GetPosition()) - v) / mData.GetScale();
        zeus::CVector3f _7c = x4_particles[2].GetPosition() - x4_particles[5].GetPosition();
        zeus::CVector3f _88 =
        (x4_particles[0].GetPosition() -
        (x4_particles[8].GetPosition() + x4_particles[11].GetPosition()) * 0.5f).normalized();
        zeus::CVector3f _x94 = _88.cross(_7c).normalized();
        zeus::CQuaternion _a4(zeus::CMatrix3f(_x94.cross(_88), _x94, _88).transposed());
        aData->PoseBuilder().GetTreeMap()[rootId].x4_rotation = _a4;
        if (x6c_spacePirate->x7b4_ == kInvalidUniqueId)
        {
            zeus::CVector3f _b0 = aData->GetCharLayoutInfo().GetFromParentUnrotated(x4_particles[1].GetBone());
            aData->PoseBuilder().GetTreeMap()[x4_particles[1].GetBone()] = zeus::CQuaternion::shortestRotationArc(_b0,
            _a4.inverse().transform(x4_particles[1].GetPosition() - x4_particles[0].GetPosition()));
        }
        BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 3, 4,
                  _a4 * BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 2, 3, _a4));
        BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 6, 7,
                  _a4 * BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 5, 6, _a4));
        BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 9, 10,
                  _a4 * BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 8, 9, _a4));
        BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 12, 13,
                  _a4 * BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 11, 12, _a4));
        zeus::CQuaternion q;
        q.rotateX(zeus::degToRad(-70.f));
        aData->PoseBuilder().GetTreeMap()[x4_particles[10].GetBone()].x4_rotation = q;
        aData->PoseBuilder().GetTreeMap()[x4_particles[13].GetBone()].x4_rotation = q;
        aData->MarkPoseDirty();
    }
}

void CPirateRagDoll::Update(CStateManager& mgr, float dt, float f2in)
{
    if (!x68_25_ || x68_27_)
    {
        if (x6c_spacePirate->x7b4_ != kInvalidUniqueId)
        {
            float f2 = x4_particles[2].GetPosition().z - x4_particles[5].GetPosition().z;
            if (f2 * f2 > 0.0625f)
            {
                zeus::CVector3f vec(0.f, 0.f, ((f2 > 0.f) ? f2 - 0.25f : f2 + 0.25f) * 0.1f);
                x4_particles[2].Position() -= vec;
                x4_particles[5].Position() += vec;
            }
            f2 = x4_particles[0].GetPosition().z -
                (x4_particles[8].GetPosition().z + x4_particles[11].GetPosition().z) * 0.5f;
            if (f2 * f2 > 0.0625f)
            {
                zeus::CVector3f vec(0.f, 0.f, ((f2 > 0.f) ? f2 - 0.25f : f2 + 0.25f) * 0.1f);
                x4_particles[0].Position() -= vec;
                x4_particles[8].Position() += vec;
                x4_particles[11].Position() += vec;
            }
        }
        zeus::CVector3f _54 =
        x4_particles[8].GetPosition() * 0.25f +
        x4_particles[11].GetPosition() * 0.25f + x4_particles[0].GetPosition() * 0.5f;
        _54.z =
        std::min(x4_particles[0].GetPosition().z - x4_particles[0].GetRadius(),
            std::min(x4_particles[8].GetPosition().z - x4_particles[8].GetRadius(),
                     x4_particles[11].GetPosition().z - x4_particles[11].GetRadius()));
        if (_54.z < 0.5f + f2in)
            x84_ = x84_ * 1000.f;
        zeus::CVector3f vec = x84_ * 0.333f * (1.f / x6c_spacePirate->GetMass());
        x4_particles[11].Acceleration() += vec;
        x4_particles[8].Acceleration() += vec;
        x4_particles[0].Acceleration() += vec;
        x84_ = zeus::CVector3f::skZero;
        CRagDoll::Update(mgr, dt, f2in);
        auto particleIdxIt = x9c_wpParticleIdxs.begin();
        for (TUniqueId id : x90_waypoints)
        {
            if (const CScriptWaypoint* wp = static_cast<const CScriptWaypoint*>(mgr.GetObjectById(id)))
                if (wp->GetActive())
                    x4_particles[*particleIdxIt].Position() = wp->GetTranslation();
            ++particleIdxIt;
        }
        zeus::CVector3f _60 =
        x4_particles[8].GetPosition() * 0.25f +
        x4_particles[11].GetPosition() * 0.25f + x4_particles[0].GetPosition() * 0.5f;
        _60.z =
        std::min(x4_particles[0].GetPosition().z - x4_particles[0].GetRadius(),
            std::min(x4_particles[8].GetPosition().z - x4_particles[8].GetRadius(),
                     x4_particles[11].GetPosition().z - x4_particles[11].GetRadius()));
        x6c_spacePirate->SetTransform({});
        x6c_spacePirate->SetTranslation(_60);
        x6c_spacePirate->SetVelocityWR((_60 - _54) * (1.f / dt));
        x74_ -= dt;
        if (x54_ > 2.5f && x74_ < 0.f &&
            (xb0_24_ || (x6c_spacePirate->GetTranslation() - x78_).magSquared() > 0.1f))
        {
            CSfxManager::AddEmitter(x70_s1, x6c_spacePirate->GetTranslation(), zeus::CVector3f::skZero,
                std::min(25.f * x54_, 127.f) / 127.f, true, false, 0x7f, kInvalidAreaId);
            x74_ = mgr.GetActiveRandom()->Float() * 0.222f + 0.222f;
            xb0_24_ = false;
            x78_ = x6c_spacePirate->GetTranslation();
        }
    }
    else
    {
        x6c_spacePirate->SetMomentumWR(zeus::CVector3f::skZero);
        x6c_spacePirate->Stop();
    }
}

void CPirateRagDoll::Prime(CStateManager& mgr, const zeus::CTransform& xf, CModelData& mData)
{
    const auto& aabb = x6c_spacePirate->GetBaseBoundingBox();
    zeus::CVector3f newMax = aabb.max;
    newMax.z = (aabb.max.z - aabb.min.z) * 0.5f + aabb.min.z;
    x6c_spacePirate->SetBoundingBox({aabb.min, newMax});
    CRagDoll::Prime(mgr, xf, mData);
}

const SBurst CSpacePirate::skBurstsQuick[] =
{
    {20, {3, 4, 5, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {20, {2, 3, 4, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {20, {6, 5, 4, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {20, {1, 2, 3, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {20, {7, 6, 5, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};
const SBurst CSpacePirate::skBurstsStandard[] =
{
    {15, {5, 3, 2, 1, -1, 0, 0, 0}, 0.100000, 0.050000},
    {20, {1, 2, 3, 4, -1, 0, 0, 0}, 0.100000, 0.050000},
    {20, {7, 6, 5, 4, -1, 0, 0, 0}, 0.100000, 0.050000},
    {15, {3, 4, 5, 6, -1, 0, 0, 0}, 0.100000, 0.050000},
    {15, {6, 5, 4, 3, -1, 0, 0, 0}, 0.100000, 0.050000},
    {15, {2, 3, 4, 5, -1, 0, 0, 0}, 0.100000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};
const SBurst CSpacePirate::skBurstsFrenzied[] =
{
    {40, {1, 2, 3, 4, 5, 6, -1, 0}, 0.100000, 0.050000},
    {40, {7, 6, 5, 4, 3, 2, -1, 0}, 0.100000, 0.050000},
    {10, {2, 3, 4, 5, 4, 3, -1, 0}, 0.100000, 0.050000},
    {10, {6, 5, 4, 3, 4, 5, -1, 0}, 0.100000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};
const SBurst CSpacePirate::skBurstsJumping[] =
{
    {20, {16, 4, -1, 0, 0, 0, 0, 0}, 0.100000, 0.050000},
    {40, {5, 7, -1, 0, 0, 0, 0, 0}, 0.100000, 0.050000},
    {40, {1, 10, -1, 0, 0, 0, 0, 0}, 0.100000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};
const SBurst CSpacePirate::skBurstsInjured[] =
{
    {15, {16, 1, 3, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {20, {3, 4, 6, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {25, {7, 5, 4, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {25, {2, 6, 4, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {15, {7, 5, 3, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};
const SBurst CSpacePirate::skBurstsSeated[] =
{
    {35, {7, 13, -1, 0, 0, 0, 0, 0}, 0.100000, 0.050000},
    {35, {9, 1, -1, 0, 0, 0, 0, 0}, 0.100000, 0.050000},
    {30, {16, 12, -1, 0, 0, 0, 0, 0}, 0.100000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};
const SBurst CSpacePirate::skBurstsQuickOOV[] =
{
    {10, {16, 15, 13, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {20, {13, 12, 10, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {30, {9, 11, 12, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {30, {14, 10, 12, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {10, {9, 11, 13, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};
const SBurst CSpacePirate::skBurstsStandardOOV[] =
{
    {26, {16, 8, 11, 14, -1, 0, 0, 0}, 0.100000, 0.050000},
    {26, {16, 13, 11, 12, -1, 0, 0, 0}, 0.100000, 0.050000},
    {16, {9, 11, 13, 10, -1, 0, 0, 0}, 0.100000, 0.050000},
    {16, {14, 13, 12, 11, -1, 0, 0, 0}, 0.100000, 0.050000},
    {8, {10, 11, 12, 13, -1, 0, 0, 0}, 0.100000, 0.050000},
    {8, {6, 8, 11, 13, -1, 0, 0, 0}, 0.100000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};
const SBurst CSpacePirate::skBurstsFrenziedOOV[] =
{
    {40, {1, 16, 14, 12, 10, 11, -1, 0}, 0.100000, 0.050000},
    {40, {9, 11, 12, 13, 11, 7, -1, 0}, 0.100000, 0.050000},
    {10, {8, 10, 11, 12, 13, 12, -1, 0}, 0.100000, 0.050000},
    {10, {15, 13, 12, 10, 12, 9, -1, 0}, 0.100000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};
const SBurst CSpacePirate::skBurstsJumpingOOV[] =
{
    {40, {7, 13, -1, 0, 0, 0, 0, 0}, 0.100000, 0.050000},
    {40, {9, 1, -1, 0, 0, 0, 0, 0}, 0.100000, 0.050000},
    {20, {16, 12, -1, 0, 0, 0, 0, 0}, 0.100000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};
const SBurst CSpacePirate::skBurstsInjuredOOV[] =
{
    {30, {9, 11, 13, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {10, {13, 12, 10, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {15, {9, 11, 12, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {15, {14, 10, 12, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {30, {16, 15, 13, -1, 0, 0, 0, 0}, 0.100000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};
const SBurst CSpacePirate::skBurstsSeatedOOV[] =
{
    {35, {7, 13, -1, 0, 0, 0, 0, 0}, 0.100000, 0.050000},
    {35, {9, 1, -1, 0, 0, 0, 0, 0}, 0.100000, 0.050000},
    {30, {16, 12, -1, 0, 0, 0, 0, 0}, 0.100000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};

const SBurst* CSpacePirate::skBursts[] =
{
    skBurstsQuick,
    skBurstsStandard,
    skBurstsFrenzied,
    skBurstsJumping,
    skBurstsInjured,
    skBurstsSeated,
    skBurstsQuickOOV,
    skBurstsStandardOOV,
    skBurstsFrenziedOOV,
    skBurstsJumpingOOV,
    skBurstsInjuredOOV,
    skBurstsSeatedOOV,
    nullptr
};

CSpacePirate::CSpacePirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CActorParameters& aParams, const CPatternedInfo& pInfo,
                           CInputStream& in, u32 propCount)
: CPatterned(ECharacter::SpacePirate, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, aParams, EKnockBackVariant::Medium)
, x568_pirateData(in, propCount), x660_(nullptr, 0x1, pInfo.GetPathfindingIndex(), 1.f, 1.f),
  x750_(pInfo.GetHealthInfo().GetHP()),
  x764_(*x64_modelData->AnimationData(), "Head_1"sv, 1.22173f, 3.14159f, false),
  x7c4_(skBursts, x568_pirateData.xac_firstBurstCount),
  x8b8_(x568_pirateData.xb0_), x8bc_(x568_pirateData.xb4_),
  x8c0_(x568_pirateData.xb8_), x8c4_(x568_pirateData.xa8_)
{
    x634_24_ = bool(x568_pirateData.x18_ & 0x1);
    x634_25_ = bool(x568_pirateData.x18_ & 0x2);
    x634_26_ = bool(x568_pirateData.x18_ & 0x4);
    x634_27_ = bool(x568_pirateData.x18_ & 0x8);
    x634_28_ = bool(x568_pirateData.x18_ & 0x10);
    x634_29_ = bool(x568_pirateData.x18_ & 0x20);
    x634_30_ = bool(x568_pirateData.x18_ & 0x40);
    x634_31_ = bool(x568_pirateData.x18_ & 0x80);
    x635_24_ = bool(x568_pirateData.x18_ & 0x200);
    x635_25_ = bool(x568_pirateData.x18_ & 0x400);
    x635_26_ = bool(x568_pirateData.x18_ & 0x1000);
    x635_27_ = bool(x568_pirateData.x18_ & 0x2000);
    x635_28_ = bool(x568_pirateData.x18_ & 0x4000);
    x635_29_ = bool(x568_pirateData.x18_ & 0x8000);
    x635_30_ = bool(x568_pirateData.x18_ & 0x10000);
    x635_31_ = bool(x568_pirateData.x18_ & 0x20000);
    x636_24_trooper = bool(x568_pirateData.x18_ & 0x40000);

    x758_headSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("Head_1"sv);
    x7b6_gunSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("R_gun_LCTR"sv);
    x7b7_elbowSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("R_elbow"sv);
    x7b8_wristSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("R_wrist"sv);
    x7b9_swooshSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("Swoosh_LCTR"sv);

    if (!x634_29_)
    {
        x7a4_intoJumpDist = GetAnimationDistance(CPASAnimParmData(13,
            CPASAnimParm::FromEnum(0), CPASAnimParm::FromEnum(0)));
        x848_dodgeDist = GetAnimationDistance(CPASAnimParmData(3,
            CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(1)));
        x84c_breakDodgeDist = GetAnimationDistance(CPASAnimParmData(3,
            CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(2)));
    }
    else
    {
        x450_bodyController->BodyStateInfo().SetLocoAnimChangeAtEndOfAnimOnly(true);
    }

    const auto& baseAABB = GetBaseBoundingBox();
    x7a8_ = (baseAABB.max.z - baseAABB.min.z) * 0.6f;

    if (x90_actorLights)
        x90_actorLights->SetAmbienceGenerated(false);

    x460_knockBackController.sub80233d40(3, 3.f, FLT_MAX);
    x460_knockBackController.SetLocomotionDuringElectrocution(true);

    if (x634_29_)
        x460_knockBackController.SetKnockBackVariant(EKnockBackVariant::Small);
    else if (x636_24_trooper && GetDamageVulnerability()->WeaponHurts(CWeaponMode(EWeaponType::Plasma), false))
        x460_knockBackController.SetKnockBackVariant(EKnockBackVariant::Large);

    if (!x450_bodyController->HasBodyState(pas::EAnimationState::AdditiveAim))
        x634_27_ = true;

    if (x636_24_trooper)
    {
        if (GetDamageVulnerability()->WeaponHurts(CWeaponMode(EWeaponType::Plasma), false))
            x8cc_trooperColor = zeus::CColor(0.996f, 0.f, 0.157f, 1.f);
        else if (GetDamageVulnerability()->WeaponHurts(CWeaponMode(EWeaponType::Ice), false))
            x8cc_trooperColor = zeus::CColor::skWhite;
        else if (GetDamageVulnerability()->WeaponHurts(CWeaponMode(EWeaponType::Power), false))
            x8cc_trooperColor = zeus::CColor(0.992f, 0.937f, 0.337f, 1.f);
        else if (GetDamageVulnerability()->WeaponHurts(CWeaponMode(EWeaponType::Wave), false))
            x8cc_trooperColor = zeus::CColor(0.776f, 0.054f, 1.f, 1.f);
    }
}

void CSpacePirate::Accept(IVisitor &visitor)
{
    visitor.Visit(this);
}

void CSpacePirate::Think(float dt, CStateManager& mgr)
{
    if (!GetActive())
        return;

    if (!x450_bodyController->GetActive())
        x450_bodyController->Activate(mgr);

#if 0
    bool inCineCam = mgr.GetCameraManager()->IsInCinematicCamera();
    if (inCineCam && !x637_31_prevInCineCam)
        sub_801283a8(mgr);
    else if (!inCineCam && x637_31_prevInCineCam && !x635_31_)
        CreateCollisionActor(mgr);
    x637_31_prevInCineCam = inCineCam;

    float steeringSpeed = x748_ == 0.f ? x644_ : 0.f;
    x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(steeringSpeed, steeringSpeed);

    x744_ = std::max(x744_ - dt, 0.f);

    if (x400_25_alive)
    {
        x850_ += dt;
        x854_ += dt;
        if (x637_27_)
        {
            x854_ = 0.f;
            x637_27_ = false;
        }
        if (x400_24_hitByPlayerProjectile)
        {
            x850_ = 0.f;
            x400_24_hitByPlayerProjectile = false;
        }
    }

    sub_8012169c(dt, mgr);

    if (!x450_bodyController->IsFrozen())
    {
        if (x400_25_alive)
        {
            x748_ = std::max(x748_ - dt, 0.f);
            if (x637_28_)
                x7ac_ += dt;
            else
                x7ac_ = 0.f;
            x838_ = std::max(x838_ - dt, 0.f);
            x8c0_ = std::max(x8c0_ - dt, 0.f);
            CheckForProjectiles(mgr);
        }
        sub_80121214(dt, mgr);
        sub_8012102c(dt, mgr);
        x860_ikChain.Update(dt);
    }

    if (x634_24_)
    {
        x634_24_ = false;
        if (x634_25_)
            x450_bodyController->SetLocomotionType(pas::ELocomotionType::Internal6);
        else
            x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
        x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "Ambushing"sv);
    }

    bool r29 = x85c_ragDoll == 0;
    if (r29 || )
    {

    }
#endif
    CPatterned::Think(dt, mgr);
}

}
