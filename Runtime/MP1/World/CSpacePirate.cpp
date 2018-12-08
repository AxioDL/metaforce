#include "CSpacePirate.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "Character/CPASAnimParmData.hpp"
#include "World/CScriptWaypoint.hpp"
#include "World/CPatternedInfo.hpp"
#include "World/CScriptWater.hpp"
#include "World/CPlayer.hpp"
#include "World/CTeamAiMgr.hpp"
#include "Weapon/CGameProjectile.hpp"
#include "CTimeProvider.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "World/CScriptCoverPoint.hpp"
#include "World/CScriptTargetingPoint.hpp"
#include "World/CWorld.hpp"
#include "TCastTo.hpp"

namespace urde::MP1
{
CSpacePirate::CSpacePirateData::CSpacePirateData(urde::CInputStream& in, u32 propCount)
: x0_(in.readFloatBig()), x4_(in.readFloatBig()), x8_(in.readFloatBig()), xc_(in.readFloatBig())
, x10_(in.readFloatBig()), x14_hearNoiseRange(in.readFloatBig()), x18_flags(in.readUint32Big()), x1c_(in.readBool()), x20_mainProjectileInfo(in)
, x48_(CSfxManager::TranslateSFXID(in.readUint32Big())), x4c_(in), x68_(in.readFloatBig()), x6c_(in)
, x94_(in.readFloatBig()), x98_ragdollThudSfx(CSfxManager::TranslateSFXID(in.readUint32Big())), x9c_averageNextShotTime(in.readFloatBig())
, xa0_nextShotTimeVariation(in.readFloatBig()), xa4_(CSfxManager::TranslateSFXID(in.readUint32Big())), xa8_aimDelayTime(in.readFloatBig())
, xac_firstBurstCount(in.readUint32Big()), xb0_minCloakAlpha(in.readFloatBig()), xb4_maxCloakAlpha(in.readFloatBig()), xb8_(in.readFloatBig())
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

CPirateRagDoll::CPirateRagDoll(CStateManager& mgr, CSpacePirate* sp, u16 thudSfx, u32 flags)
: CRagDoll(-sp->GetGravityConstant(), -3.f, 8.f, flags), x6c_spacePirate(sp), x70_thudSfx(thudSfx)
{
    xb0_24_initSfx = true;
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
                    skRadii[i] * scale.z());
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
    if (!x68_25_over || x68_27_continueSmallMovements)
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

void CPirateRagDoll::Update(CStateManager& mgr, float dt, float waterTop)
{
    if (!x68_25_over || x68_27_continueSmallMovements)
    {
        if (x6c_spacePirate->x7b4_ != kInvalidUniqueId)
        {
            // Shoulder height delta
            float f2 = x4_particles[2].GetPosition().z() - x4_particles[5].GetPosition().z();
            if (f2 * f2 > 0.0625f)
            {
                zeus::CVector3f vec(0.f, 0.f, ((f2 > 0.f) ? f2 - 0.25f : f2 + 0.25f) * 0.1f);
                x4_particles[2].Position() -= vec;
                x4_particles[5].Position() += vec;
            }
            // Collar-hips height delta
            f2 = x4_particles[0].GetPosition().z() -
                (x4_particles[8].GetPosition().z() + x4_particles[11].GetPosition().z()) * 0.5f;
            if (f2 * f2 > 0.0625f)
            {
                zeus::CVector3f vec(0.f, 0.f, ((f2 > 0.f) ? f2 - 0.25f : f2 + 0.25f) * 0.1f);
                x4_particles[0].Position() -= vec;
                x4_particles[8].Position() += vec;
                x4_particles[11].Position() += vec;
            }
        }
        // Collar-hips weighted center
        zeus::CVector3f oldTorsoCenter =
        x4_particles[8].GetPosition() * 0.25f +
        x4_particles[11].GetPosition() * 0.25f + x4_particles[0].GetPosition() * 0.5f;
        oldTorsoCenter.z() =
        std::min(x4_particles[0].GetPosition().z() - x4_particles[0].GetRadius(),
            std::min(x4_particles[8].GetPosition().z() - x4_particles[8].GetRadius(),
                     x4_particles[11].GetPosition().z() - x4_particles[11].GetRadius()));
        if (oldTorsoCenter.z() < 0.5f + waterTop)
            x84_torsoImpulse = x84_torsoImpulse * 1000.f;
        zeus::CVector3f accDelta = x84_torsoImpulse * 0.333f * (1.f / x6c_spacePirate->GetMass());
        x4_particles[11].Velocity() += accDelta;
        x4_particles[8].Velocity() += accDelta;
        x4_particles[0].Velocity() += accDelta;
        x84_torsoImpulse = zeus::CVector3f::skZero;
        CRagDoll::Update(mgr, dt, waterTop);
        auto particleIdxIt = x9c_wpParticleIdxs.begin();
        for (TUniqueId id : x90_waypoints)
        {
            if (const CScriptWaypoint* wp = static_cast<const CScriptWaypoint*>(mgr.GetObjectById(id)))
                if (wp->GetActive())
                    x4_particles[*particleIdxIt].Position() = wp->GetTranslation();
            ++particleIdxIt;
        }
        // Collar-hips weighted center
        zeus::CVector3f newTorsoCenter =
        x4_particles[8].GetPosition() * 0.25f +
        x4_particles[11].GetPosition() * 0.25f + x4_particles[0].GetPosition() * 0.5f;
        newTorsoCenter.z() =
        std::min(x4_particles[0].GetPosition().z() - x4_particles[0].GetRadius(),
            std::min(x4_particles[8].GetPosition().z() - x4_particles[8].GetRadius(),
                     x4_particles[11].GetPosition().z() - x4_particles[11].GetRadius()));
        x6c_spacePirate->SetTransform({});
        x6c_spacePirate->SetTranslation(newTorsoCenter);
        x6c_spacePirate->SetVelocityWR((newTorsoCenter - oldTorsoCenter) * (1.f / dt));
        x74_sfxTimer -= dt;
        if (x54_impactVel > 2.5f && x74_sfxTimer < 0.f &&
            (xb0_24_initSfx || (x6c_spacePirate->GetTranslation() - x78_lastSFXPos).magSquared() > 0.1f))
        {
            CSfxManager::AddEmitter(x70_thudSfx, x6c_spacePirate->GetTranslation(), zeus::CVector3f::skZero,
                std::min(25.f * x54_impactVel, 127.f) / 127.f, true, false, 0x7f, kInvalidAreaId);
            x74_sfxTimer = mgr.GetActiveRandom()->Float() * 0.222f + 0.222f;
            xb0_24_initSfx = false;
            x78_lastSFXPos = x6c_spacePirate->GetTranslation();
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
    newMax.z() = (aabb.max.z() - aabb.min.z()) * 0.5f + aabb.min.z();
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

std::list<TUniqueId> CSpacePirate::mChargePlayerList;

CSpacePirate::CSpacePirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CActorParameters& aParams, const CPatternedInfo& pInfo,
                           CInputStream& in, u32 propCount)
: CPatterned(ECharacter::SpacePirate, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, aParams, EKnockBackVariant::Medium)
, x568_pirateData(in, propCount), x660_pathFindSearch(nullptr, 0x1, pInfo.GetPathfindingIndex(), 1.f, 1.f),
  x750_initialHP(pInfo.GetHealthInfo().GetHP()),
  x764_boneTracking(*x64_modelData->AnimationData(), "Head_1"sv, 1.22173f, 3.14159f, false),
  x7c4_burstFire(skBursts, x568_pirateData.xac_firstBurstCount),
  x8b8_minCloakAlpha(x568_pirateData.xb0_minCloakAlpha), x8bc_maxCloakAlpha(x568_pirateData.xb4_maxCloakAlpha),
  x8c0_(x568_pirateData.xb8_), x8c4_aimDelayTimer(x568_pirateData.xa8_aimDelayTime)
{
    x634_24_pendingAmbush = bool(x568_pirateData.x18_flags & 0x1);
    x634_25_ceilingAmbush = bool(x568_pirateData.x18_flags & 0x2);
    x634_26_ = bool(x568_pirateData.x18_flags & 0x4);
    x634_27_melee = bool(x568_pirateData.x18_flags & 0x8);
    x634_28_ = bool(x568_pirateData.x18_flags & 0x10);
    x634_29_onlyAttackInRange = bool(x568_pirateData.x18_flags & 0x20);
    x634_30_ = bool(x568_pirateData.x18_flags & 0x40);
    x634_31_ = bool(x568_pirateData.x18_flags & 0x80);
    x635_24_ = bool(x568_pirateData.x18_flags & 0x200);
    x635_25_ = bool(x568_pirateData.x18_flags & 0x400);
    x635_26_ = bool(x568_pirateData.x18_flags & 0x1000);
    x635_27_shadowPirate = bool(x568_pirateData.x18_flags & 0x2000);
    x635_28_ = bool(x568_pirateData.x18_flags & 0x4000);
    x635_29_ = bool(x568_pirateData.x18_flags & 0x8000);
    x635_30_floatingCorpse = bool(x568_pirateData.x18_flags & 0x10000);
    x635_31_ragdollNoAiCollision = bool(x568_pirateData.x18_flags & 0x20000);
    x636_24_trooper = bool(x568_pirateData.x18_flags & 0x40000);

    x758_headSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("Head_1"sv);
    x7b6_gunSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("R_gun_LCTR"sv);
    x7b7_elbowSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("R_elbow"sv);
    x7b8_wristSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("R_wrist"sv);
    x7b9_swooshSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("Swoosh_LCTR"sv);

    if (!x634_29_onlyAttackInRange)
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
    x7a8_ = (baseAABB.max.z() - baseAABB.min.z()) * 0.6f;

    if (x90_actorLights)
        x90_actorLights->SetAmbienceGenerated(false);

    x460_knockBackController.sub80233d40(3, 3.f, FLT_MAX);
    x460_knockBackController.SetLocomotionDuringElectrocution(true);

    if (x634_29_onlyAttackInRange)
        x460_knockBackController.SetKnockBackVariant(EKnockBackVariant::Small);
    else if (x636_24_trooper && GetDamageVulnerability()->WeaponHurts(CWeaponMode(EWeaponType::Plasma), false))
        x460_knockBackController.SetKnockBackVariant(EKnockBackVariant::Large);

    if (!x450_bodyController->HasBodyState(pas::EAnimationState::AdditiveAim))
        x634_27_melee = true;

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

void CSpacePirate::UpdateCloak(float dt, CStateManager& mgr)
{
    if (x635_27_shadowPirate)
    {
        if (x400_25_alive)
        {
            if (x8a8_cloakDelayTimer > 0.f)
            {
                x8a8_cloakDelayTimer -= dt;
                if (x8a8_cloakDelayTimer <= 0.f)
                    x3e8_alphaDelta = -0.4f;
            }
        }
        else
        {
            x8b8_minCloakAlpha = 0.f;
            x8bc_maxCloakAlpha = 1.f;
        }

        if (x8ac_electricParticleTimer > 0.f)
        {
            x8ac_electricParticleTimer -= dt;
            if (x8ac_electricParticleTimer <= 0.f && !x450_bodyController->IsElectrocuting())
                mgr.GetActorModelParticles()->StopElectric(*this);
        }

        if (x450_bodyController->IsFrozen())
            x3e8_alphaDelta = 2.f;

        if (x3e8_alphaDelta < 0.f && x42c_color.a() < x8b8_minCloakAlpha)
        {
            x42c_color.a() = x8b8_minCloakAlpha;
            x3e8_alphaDelta = 0.f;
            RemoveMaterial(EMaterialTypes::Target, mgr);
        }

        if (x3e8_alphaDelta > 0.f && x42c_color.a() > x8bc_maxCloakAlpha)
        {
            x42c_color.a() = x8bc_maxCloakAlpha;
            AddMaterial(EMaterialTypes::Target, mgr);
        }

        x8b0_cloakStepTime -= dt;
        if (x8b0_cloakStepTime < 0.f)
        {
            x8b0_cloakStepTime = (1.f - mgr.GetActiveRandom()->Float()) * 0.08f;
            if (x3e8_alphaDelta < 0.f)
            {
                x8b4_shadowPirateAlpha = x42c_color.a();
                if (x400_25_alive)
                    x8b4_shadowPirateAlpha -= (x42c_color.a() - x8b8_minCloakAlpha) * x8b0_cloakStepTime;
            }
            else if (x3e8_alphaDelta > 0.f)
            {
                x8b4_shadowPirateAlpha = x42c_color.a() + x8b0_cloakStepTime * (x8bc_maxCloakAlpha - x42c_color.a());
            }
            else
            {
                x8b4_shadowPirateAlpha = x42c_color.a();
            }
        }
    }
}

bool CSpacePirate::ShouldFrenzy(CStateManager& mgr)
{
    bool reset = false;
    if (x638_24_pendingFrenzyChance)
    {
        x638_24_pendingFrenzyChance = false;
        if (mgr.GetActiveRandom()->Next() % 100 < 25)
            reset = true;
    }

    if (!mChargePlayerList.empty())
        reset = true;

    if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed)
        reset = true;

    if (HealthInfo(mgr)->GetHP() < 0.3f * x750_initialHP &&
        mgr.GetActiveRandom()->Next() % 100 < 60 && x854_lowHealthFrenzyTimer < 0.5f)
        reset = true;

    if (reset)
        x63c_frenzyFrames = mgr.GetActiveRandom()->Range(2, 4);
    x63c_frenzyFrames -= 1;
    return x63c_frenzyFrames >= 0;
}

void CSpacePirate::ResetTeamAiRole(CStateManager& mgr)
{
    CTeamAiMgr::ResetTeamAiRole(!x634_27_melee ? CTeamAiMgr::EAttackType::Projectile :
                                                 CTeamAiMgr::EAttackType::Melee,
                                mgr, x8c8_teamAiMgrId, GetUniqueId(), true);
}

void CSpacePirate::AssignTeamAiRole(CStateManager& mgr)
{
    if (x8c8_teamAiMgrId == kInvalidUniqueId)
        x8c8_teamAiMgrId = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
    if (x8c8_teamAiMgrId != kInvalidUniqueId)
    {
        if (TCastToPtr<CTeamAiMgr> aimgr = mgr.ObjectById(x8c8_teamAiMgrId))
        {
            aimgr->AssignTeamAiRole(*this, x634_27_melee ? CTeamAiRole::ETeamAiRole::Melee :
                                           CTeamAiRole::ETeamAiRole::Projectile, CTeamAiRole::ETeamAiRole::Unknown, CTeamAiRole::ETeamAiRole::Invalid);
        }
    }
}

void CSpacePirate::RemoveTeamAiRole(CStateManager& mgr)
{
    if (x8c8_teamAiMgrId != kInvalidUniqueId)
    {
        if (TCastToPtr<CTeamAiMgr> aimgr = mgr.ObjectById(x8c8_teamAiMgrId))
        {
            if (aimgr->IsPartOfTeam(GetUniqueId()))
            {
                aimgr->RemoveTeamAiRole(GetUniqueId());
                x8c8_teamAiMgrId = kInvalidUniqueId;
            }
        }
    }
}

bool CSpacePirate::CheckTargetable(CStateManager& mgr)
{
    return GetModelAlphau8(mgr) > 127;
}

void CSpacePirate::FireProjectile(float dt, CStateManager& mgr)
{
    zeus::CTransform gunXf = GetLctrTransform(x7b6_gunSeg);
    if (!x400_25_alive)
    {
        LaunchProjectile(gunXf, mgr, 6, EProjectileAttrib::None, false, {}, 0xffff,
                         false, zeus::CVector3f::skOne);
    }
}

void CSpacePirate::UpdateAttacks(float dt, CStateManager& mgr)
{
    bool reset = true;
    if ((!x400_25_alive || (x450_bodyController->GetBodyStateInfo().GetCurrentState()->CanShoot() &&
        x637_29_ && !x634_27_melee && !x634_25_ceilingAmbush && !x639_26_started &&
        !x450_bodyController->IsElectrocuting())) && x7c4_burstFire.GetBurstType() != -1)
    {
        if (x400_25_alive)
        {
            if (!x634_29_onlyAttackInRange || (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() <
                                              x3c8_leashRadius * x3c8_leashRadius)
            {
                reset = false;
                x7bc_ -= dt;
                if (x7bc_ < 0.f)
                {
                    const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x8c8_teamAiMgrId, GetUniqueId());
                    if (!role || role->GetTeamAiRole() == CTeamAiRole::ETeamAiRole::Projectile)
                    {
                        if (x8c8_teamAiMgrId == kInvalidUniqueId ||
                            CTeamAiMgr::AddAttacker(CTeamAiMgr::EAttackType::Projectile, mgr,
                                                    x8c8_teamAiMgrId, GetUniqueId()))
                        {
                            if (ShouldFrenzy(mgr))
                                x7c4_burstFire.SetBurstType(2);
                            if (x635_26_)
                                x7c4_burstFire.SetBurstType(5);
                            if (!PlayerSpot(mgr, 0.f) && x7c4_burstFire.GetBurstType() < 6)
                                x7c4_burstFire.SetBurstType(x7c4_burstFire.GetBurstType() + 6);

                            x7c4_burstFire.Start(mgr);
                            x7bc_ = mgr.GetActiveRandom()->Float() * x308_attackTimeVariation + x304_averageAttackTime;
                            if ((GetGunEyePos() - mgr.GetPlayer().GetAimPosition(mgr, 0.f)).normalized().
                                dot(mgr.GetPlayer().GetTransform().basis[1]) < 0.9f)
                            {
                                for (CEntity* ent : mgr.GetListeningAiObjectList())
                                {
                                    if (CSpacePirate* otherPirate = CPatterned::CastTo<CSpacePirate>(ent))
                                    {
                                        if (otherPirate != this && otherPirate->x637_25_ &&
                                            otherPirate->GetAreaIdAlways() == GetAreaIdAlways())
                                            x7bc_ += 0.2f;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        x7c4_burstFire.Update(mgr, dt);

        if (x7c4_burstFire.ShouldFire())
        {
            if (mgr.GetPlayer().IsSidewaysDashing() && mgr.GetActiveRandom()->Float() < 0.5f)
                x7c4_burstFire.SetAvoidAccuracy(true);
            FireProjectile(dt, mgr);
            x7c4_burstFire.SetAvoidAccuracy(false);
            float nextShotTime =
                x568_pirateData.xa0_nextShotTimeVariation *
                (mgr.GetActiveRandom()->Float() - 0.5f) + x568_pirateData.x9c_averageNextShotTime;
            if (x7c4_burstFire.GetTimeToNextShot() > 0.f)
                x7c4_burstFire.SetTimeToNextShot(nextShotTime);
        }
        else if (!x7c4_burstFire.IsBurstSet())
        {
            reset = true;
        }
    }

    if (reset)
        ResetTeamAiRole(mgr);

    xe7_31_targetable = CheckTargetable(mgr);
}

zeus::CVector3f CSpacePirate::GetTargetPos(CStateManager& mgr)
{
    if (x7c0_targetId != mgr.GetPlayer().GetUniqueId())
    {
        if (TCastToConstPtr<CActor> act = mgr.GetObjectById(x7c0_targetId))
            if (act->GetActive())
                return act->GetTranslation();
        x764_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
        x7c0_targetId = mgr.GetPlayer().GetUniqueId();
    }
    return mgr.GetPlayer().GetTranslation();
}

void CSpacePirate::UpdateAimBodyState(float dt, CStateManager& mgr)
{
    if (x400_25_alive && x637_25_ && !x637_29_ && !x450_bodyController->IsFrozen() &&
        !x634_27_melee && !x85c_ragDoll && (!x635_26_ || x639_28_) && x31c_faceVec.z() <= 0.f)
    {
        x8c4_aimDelayTimer = std::max(0.f, x8c4_aimDelayTimer - dt);
        if (x8c4_aimDelayTimer == 0.f)
        {
            x450_bodyController->GetCommandMgr().DeliverCmd(CBCAdditiveAimCmd());
            x450_bodyController->GetCommandMgr().DeliverAdditiveTargetVector(
                x34_transform.transposeRotate(GetTargetPos(mgr) - GetTranslation()));
        }
    }
    else if (x637_25_ && !x634_27_melee)
    {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::AdditiveIdle));
    }
}

void CSpacePirate::SetCinematicCollision(CStateManager& mgr)
{
    RemoveMaterial(EMaterialTypes::AIBlock, mgr);
    CMaterialFilter filter = GetMaterialFilter();
    filter.IncludeList().Remove(EMaterialTypes::AIBlock);
    SetMaterialFilter(filter);
}

void CSpacePirate::SetNonCinematicCollision(CStateManager& mgr)
{
    AddMaterial(EMaterialTypes::AIBlock, mgr);
    CMaterialFilter filter = GetMaterialFilter();
    filter.IncludeList().Add(EMaterialTypes::AIBlock);
    SetMaterialFilter(filter);
}

void CSpacePirate::CheckForProjectiles(CStateManager& mgr)
{
    if (x637_26_hearPlayerFire)
    {
        zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
        zeus::CAABox aabb(aimPos - 5.f, aimPos + 5.f);
        rstl::reserved_vector<TUniqueId, 1024> nearList;
        mgr.BuildNearList(nearList, aabb, CMaterialFilter::MakeInclude({EMaterialTypes::Projectile}), nullptr);
        for (TUniqueId id : nearList)
        {
            if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(id))
            {
                zeus::CVector3f delta = GetBoundingBox().center() - proj->GetTranslation();
                if (delta.isMagnitudeSafe())
                {
                    if (x34_transform.basis[1].dot(delta) < 0.f)
                    {
                        delta.normalize();
                        zeus::CVector3f projDelta = proj->GetTranslation() - proj->GetPreviousPos();
                        if (projDelta.isMagnitudeSafe())
                        {
                            projDelta.normalize();
                            if (projDelta.dot(delta) > 0.939f)
                                x637_27_inProjectilePath = true;
                        }
                    }
                }
                else
                {
                    x637_27_inProjectilePath = true;
                }
                if (x637_27_inProjectilePath)
                    break;
            }
        }
        x637_26_hearPlayerFire = false;
    }
}

void CSpacePirate::Think(float dt, CStateManager& mgr)
{
    if (!GetActive())
        return;

    if (!x450_bodyController->GetActive())
        x450_bodyController->Activate(mgr);

    bool inCineCam = mgr.GetCameraManager()->IsInCinematicCamera();
    if (inCineCam && !x637_31_prevInCineCam)
        SetCinematicCollision(mgr);
    else if (!inCineCam && x637_31_prevInCineCam && !x635_31_ragdollNoAiCollision)
        SetNonCinematicCollision(mgr);
    x637_31_prevInCineCam = inCineCam;

    float steeringSpeed = x748_ == 0.f ? x644_ : 0.f;
    x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(steeringSpeed, steeringSpeed);

    x744_ = std::max(x744_ - dt, 0.f);

    if (x400_25_alive)
    {
        x850_timeSinceHitByPlayer += dt;
        x854_lowHealthFrenzyTimer += dt;
        if (x637_27_inProjectilePath)
        {
            x854_lowHealthFrenzyTimer = 0.f;
            x637_27_inProjectilePath = false;
        }
        if (x400_24_hitByPlayerProjectile)
        {
            x850_timeSinceHitByPlayer = 0.f;
            x400_24_hitByPlayerProjectile = false;
        }
    }

    UpdateCloak(dt, mgr);

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
        UpdateAttacks(dt, mgr);
        UpdateAimBodyState(dt, mgr);
        x860_ikChain.Update(dt);
    }

    if (x634_24_pendingAmbush)
    {
        x634_24_pendingAmbush = false;
        if (x634_25_ceilingAmbush)
            x450_bodyController->SetLocomotionType(pas::ELocomotionType::Internal6);
        else
            x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
        x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "Ambushing"sv);
    }

    if (!x85c_ragDoll || !x85c_ragDoll->IsPrimed())
    {
        CPatterned::Think(dt, mgr);
        if (!x450_bodyController->IsFrozen())
            x764_boneTracking.Update(dt);
    }
    else
    {
        UpdateAlphaDelta(dt, mgr);
        UpdateDamageColor(dt);
        if (CSfxHandle hnd = GetSfxHandle())
            CSfxManager::UpdateEmitter(hnd, GetTranslation(), zeus::CVector3f::skZero, 1.f);
    }
    if (x85c_ragDoll)
    {
        if (!x85c_ragDoll->IsPrimed())
        {
            x85c_ragDoll->Prime(mgr, GetTransform(), *x64_modelData);
            zeus::CVector3f trans = GetTranslation();
            SetTransform({});
            SetTranslation(trans);
            x450_bodyController->SetPlaybackRate(0.f);
        }
        else
        {
            float waterTop = -FLT_MAX;
            if (xc4_fluidId != kInvalidUniqueId)
                if (TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(xc4_fluidId))
                    if (water->GetActive())
                        waterTop = water->GetTriggerBoundsWR().max.z();
            x85c_ragDoll->Update(mgr, dt * CalcDyingThinkRate(), waterTop);
            x64_modelData->AdvanceParticles(x34_transform, dt, mgr);
        }
        if (x85c_ragDoll->IsOver() && !x85c_ragDoll->WillContinueSmallMovements() && !x400_27_fadeToDeath)
        {
            /* Ragdoll has finished animating */
            x400_27_fadeToDeath = true;
            AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
            x3e8_alphaDelta = -0.333333f;
            x638_30_ragdollOver = true;
            SetMomentumWR(zeus::CVector3f::skZero);
            CPhysicsActor::Stop();
        }
    }
    if (x858_ragdollDelayTimer > 0.f)
    {
        x858_ragdollDelayTimer -= dt;
        if (x858_ragdollDelayTimer <= 0.f)
        {
            if (!x85c_ragDoll)
            {
                x85c_ragDoll = std::make_unique<CPirateRagDoll>(
                    mgr, this, x568_pirateData.x98_ragdollThudSfx,
                    (x635_30_floatingCorpse ? 3 : 0) |
                    (x635_31_ragdollNoAiCollision ? 4 : 0));
                RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
            }
            x858_ragdollDelayTimer = 0.f;
        }
    }
}

void CSpacePirate::SetEyeParticleActive(CStateManager& mgr, bool active)
{
    if (!x636_24_trooper)
    {
        if (!x634_29_onlyAttackInRange || x635_26_)
        {
            if (!x635_27_shadowPirate)
                x64_modelData->AnimationData()->SetParticleEffectState("TwoEyes"sv, active, mgr);
        }
        else
        {
            x64_modelData->AnimationData()->SetParticleEffectState("OneEye"sv, active, mgr);
        }
    }
}

void CSpacePirate::SetVelocityForJump()
{

}

void CSpacePirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr)
{
    if (x637_29_ || x634_25_ceilingAmbush)
    {
        switch (msg)
        {
        case EScriptObjectMessage::Falling:
            if (!x637_29_ || x450_bodyController->GetCurrentStateId() != pas::EAnimationState::WallHang ||
                x450_bodyController->GetBodyStateInfo().GetCurrentState()->ApplyGravity())
            {
                if (x634_25_ceilingAmbush)
                {
                    if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Locomotion ||
                        (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Jump &&
                         !x450_bodyController->GetBodyStateInfo().GetCurrentState()->IsMoving()))
                    {
                        CPhysicsActor::Stop();
                        SetMomentumWR(zeus::CVector3f::skZero);
                        return;
                    }
                }
            }
            break;
        case EScriptObjectMessage::OnFloor:
            x850_timeSinceHitByPlayer = FLT_MAX;
            break;
        default:
            break;
        }
    }
    switch (msg)
    {
    case EScriptObjectMessage::OnNormalSurface:
    case EScriptObjectMessage::Activate:
        if (GetActive())
        {
            if (x634_29_onlyAttackInRange)
                x638_31_ = true;
            else
                x400_24_hitByPlayerProjectile = true;
            AssignTeamAiRole(mgr);
        }
        else if (x634_25_ceilingAmbush)
        {
            RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
            x328_27_onGround = false;
        }
        break;
    default:
        break;
    }
    CPatterned::AcceptScriptMsg(msg, sender, mgr);
    switch (msg)
    {
    case EScriptObjectMessage::InitializedInArea:
        for (const auto& conn : GetConnectionList())
        {
            if (conn.x0_state == EScriptObjectState::Retreat && conn.x4_msg == EScriptObjectMessage::Next)
            {
                TUniqueId id = mgr.GetIdForScript(conn.x8_objId);
                if (TCastToPtr<CScriptCoverPoint> cp = mgr.ObjectById(id))
                    cp->Reserve(GetUniqueId());
            }
            else if (conn.x0_state == EScriptObjectState::Patrol && conn.x4_msg == EScriptObjectMessage::Follow)
            {
                x637_24_ = true;
            }
        }
        x660_pathFindSearch.SetArea(
            mgr.GetWorld()->GetAreaAlways(x4_areaId)->GetPostConstructed()->x10bc_pathArea);
        if (x635_30_floatingCorpse)
        {
            x858_ragdollDelayTimer = 0.01f;
            RemoveMaterial(EMaterialTypes::Character, mgr);
            x400_25_alive = false;
            HealthInfo(mgr)->SetHP(-1.f);
        }
        else
        {
            SetEyeParticleActive(mgr, true);
        }
        break;
    case EScriptObjectMessage::Decrement:
        if (x85c_ragDoll)
        {
            x85c_ragDoll->SetNoOverTimer(false);
            x85c_ragDoll->SetContinueSmallMovements(false);
        }
        break;
    case EScriptObjectMessage::Registered:
    {
        if (x634_25_ceilingAmbush)
        {
            x634_24_pendingAmbush = true;
            if (x635_27_shadowPirate)
            {
                x42c_color.a() = x568_pirateData.xb0_minCloakAlpha;
                x3e8_alphaDelta = -1.f;
            }
        }
        x75c_ = mgr.GetActiveRandom()->Next() % 6;
        CMaterialFilter filter = GetMaterialFilter();
        filter.IncludeList().Remove(EMaterialTypes::AIPassthrough);
        filter.ExcludeList().Add(EMaterialTypes::AIPassthrough);
        SetMaterialFilter(filter);
        break;
    }
    case EScriptObjectMessage::SetToZero:
        if (x30_24_active)
            x636_29_ = true;
        break;
    case EScriptObjectMessage::Falling:
        if (!x450_bodyController->IsFrozen())
        {
            float zMom = GetGravityConstant() * xe8_mass;
            if (x634_25_ceilingAmbush)
                zMom *= 3.f;
            SetMomentumWR({0.f, 0.f, -zMom});
        }
        if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Step)
            SetVelocityWR({0.f, 0.f, x138_velocity.z()});
        x7c4_burstFire.SetBurstType(3);
        break;
    case EScriptObjectMessage::Jumped:
        CPatterned::AcceptScriptMsg(msg, sender, mgr);
        SetMomentumWR({0.f, 0.f, -GetGravityConstant() * xe8_mass});
        SetVelocityForJump();
        break;
    case EScriptObjectMessage::OnFloor:
        if (!x634_29_onlyAttackInRange)
            x7c4_burstFire.SetBurstType(1);
        else
            x7c4_burstFire.SetBurstType(4);
        x637_30_ = false;
        if (x635_27_shadowPirate && x138_velocity.z() < -1.f)
        {
            x3e8_alphaDelta = 1.f;
            x8a8_cloakDelayTimer += -0.05f * x138_velocity.z();
            x8a8_cloakDelayTimer = zeus::clamp(0.f, x8a8_cloakDelayTimer, 1.f);
            x8bc_maxCloakAlpha = 0.5f;
            if (x400_25_alive)
            {
                mgr.GetActorModelParticles()->sub_801e51d0(*this);
                x8ac_electricParticleTimer = 1.f + x8a8_cloakDelayTimer;
            }
        }
        break;
    case EScriptObjectMessage::Action:
        if (TCastToPtr<CScriptTargetingPoint> tp = mgr.ObjectById(sender))
        {
            if (tp->GetActive())
            {
                x764_boneTracking.SetTarget(sender);
                x7c0_targetId = sender;
                x400_24_hitByPlayerProjectile = true;
            }
            else
            {
                x764_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
                x7c0_targetId = mgr.GetPlayer().GetUniqueId();
            }
            x7bc_ = 0.f;
        }
        break;
    case EScriptObjectMessage::Deactivate:
    case EScriptObjectMessage::Deleted:
        RemoveTeamAiRole(mgr);
        mChargePlayerList.remove(GetUniqueId());
        break;
    case EScriptObjectMessage::Start:
        x639_26_started = false;
        break;
    case EScriptObjectMessage::Stop:
        x639_26_started = true;
        break;
    default:
        break;
    }
}

void CSpacePirate::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum)
{
    if (x85c_ragDoll && x85c_ragDoll->IsPrimed())
        x85c_ragDoll->PreRender(GetTranslation(), *x64_modelData);
    CPatterned::PreRender(mgr, frustum);
    if (!x85c_ragDoll || !x85c_ragDoll->IsPrimed())
    {
        x764_boneTracking.PreRender(mgr, *x64_modelData->AnimationData(), x34_transform,
                                    x64_modelData->GetScale(), *x450_bodyController);
        x860_ikChain.PreRender(*x64_modelData->AnimationData(), x34_transform, x64_modelData->GetScale());
    }
}

void CSpacePirate::Render(const CStateManager& mgr) const
{
    float time = x400_25_alive ? CGraphics::GetSecondsMod900() : 0.f;
    CTimeProvider prov(time);
    g_Renderer->SetGXRegister1Color(x8cc_trooperColor);
    CPatterned::Render(mgr);
}

void CSpacePirate::CalculateRenderBounds()
{
    if (x85c_ragDoll && x85c_ragDoll->IsPrimed())
    {
        zeus::CVector3f margin = x64_modelData->GetScale() * 0.2f;
        zeus::CAABox ragdollBounds = x85c_ragDoll->CalculateRenderBounds();
        x9c_renderBounds = zeus::CAABox(ragdollBounds.min - margin, ragdollBounds.max + margin);
    }
    else
    {
        CActor::CalculateRenderBounds();
    }
}

void CSpacePirate::Touch(CActor& other, CStateManager& mgr)
{
    CPatterned::Touch(other, mgr);
    if (x85c_ragDoll && x85c_ragDoll->IsPrimed())
    {
        if (TCastToPtr<CScriptTrigger> trig = other)
        {
            if (trig->GetActive() &&
                (trig->GetTriggerFlags() & ETriggerFlags::DetectAI) != ETriggerFlags::None &&
                trig->GetForceMagnitude() > 0.f)
            {
                x85c_ragDoll->TorsoImpulse() += trig->GetForceVector();
            }
        }
    }
}

zeus::CAABox CSpacePirate::GetSortingBounds(const CStateManager& mgr) const
{
    zeus::CAABox aabb = x64_modelData->GetBounds();
    zeus::CVector3f radius = aabb.extents() * 0.5f;
    zeus::CVector3f center = aabb.center();
    return zeus::CAABox(center - radius, center + radius);
}

void CSpacePirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt)
{
    bool handled = false;
    switch (type)
    {
    case EUserEventType::BeginAction:
        RemoveMaterial(EMaterialTypes::Solid, mgr);
        x638_30_ragdollOver = true;
        handled = true;
        break;
    case EUserEventType::EndAction:
        x639_30_ = false;
        handled = true;
        break;
    case EUserEventType::Projectile:
    case EUserEventType::BecomeRagDoll:
        if (x634_29_onlyAttackInRange || HealthInfo(mgr)->GetHP() <= 0.f)
            x858_ragdollDelayTimer = mgr.GetActiveRandom()->Float() * 0.05f + 0.001f;
        handled = true;
        break;
    case EUserEventType::IkLock:
        if (!x860_ikChain.GetActive())
        {
            CSegId lctrId = x64_modelData->GetAnimationData()->GetLocatorSegId(node.GetLocatorName());
            if (lctrId != 3)
            {
                zeus::CTransform xf = GetLctrTransform(lctrId);
                x860_ikChain.Activate(*x64_modelData->AnimationData(), lctrId, xf);
                x639_28_ = true;
            }
        }
        handled = true;
        break;
    case EUserEventType::IkRelease:
        x860_ikChain.Deactivate();
        handled = true;
        break;
    case EUserEventType::ScreenShake:
        SendScriptMsgs(EScriptObjectState::Play, mgr, EScriptObjectMessage::None);
        handled = true;
        break;
    case EUserEventType::FadeOut:
        x3e8_alphaDelta = -0.8f;
        mgr.GetActorModelParticles()->sub_801e51d0(*this);
        x8ac_electricParticleTimer = 1.f;
        handled = true;
        break;
    default:
        break;
    }
    if (!handled)
        CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CSpacePirate::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state)
{

}

void CSpacePirate::KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info,
                             EKnockBackType type, bool inDeferred, float magnitude)
{

}

bool CSpacePirate::IsListening() const
{
    return true;
}

bool CSpacePirate::Listen(const zeus::CVector3f& pos, EListenNoiseType type)
{
    bool ret = false;
    if (x400_25_alive)
    {
        zeus::CVector3f delta = pos - GetTranslation();
        if (delta.magSquared() < x568_pirateData.x14_hearNoiseRange * x568_pirateData.x14_hearNoiseRange &&
            (x3c0_detectionHeightRange == 0.f ||
            delta.z() * delta.z() < x3c0_detectionHeightRange * x3c0_detectionHeightRange))
            x636_25_hearNoise = true;
        if (type == EListenNoiseType::PlayerFire)
            x637_26_hearPlayerFire = true;
    }
    return ret;
}

zeus::CVector3f CSpacePirate::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role) const
{
    return GetTranslation();
}

void CSpacePirate::Patrol(CStateManager& mgr, EStateMsg msg, float dt)
{

}

void CSpacePirate::Dead(CStateManager& mgr, EStateMsg msg, float dt)
{

}

void CSpacePirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt)
{

}

void CSpacePirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::TargetCover(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Halt(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Run(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Generate(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Deactivate(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Attack(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::JumpBack(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Shuffle(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::TurnAround(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Skid(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::CoverAttack(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Crouch(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::GetUp(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Taunt(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Flee(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Lurk(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Jump(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Dodge(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Cover(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Approach(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::WallHang(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::WallDetach(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Enraged(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::Bounce(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

void CSpacePirate::PathFindEx(CStateManager& mgr, EStateMsg msg, float dt)
{
    
}

bool CSpacePirate::Leash(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::OffLine(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::Attacked(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::InRange(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::SpotPlayer(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::PatternOver(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::PatternShagged(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::AnimOver(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::ShouldAttack(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::ShouldJumpBack(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::Stuck(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::Landed(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::HearShot(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::HearPlayer(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::CoverCheck(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::CoverFind(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::CoverBlown(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::CoverNearlyBlown(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::CoveringFire(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::LineOfSight(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::AggressionCheck(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::ShouldDodge(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::ShouldRetreat(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::ShouldCrouch(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::ShouldMove(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::ShotAt(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::HasTargetingPoint(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::ShouldWallHang(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::StartAttack(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::BreakAttack(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::ShouldStrafe(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::ShouldSpecialAttack(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::LostInterest(CStateManager& mgr, float arg)
{
    return false;
}

bool CSpacePirate::BounceFind(CStateManager& mgr, float arg)
{
    return false;
}

CPathFindSearch* CSpacePirate::GetSearchPath()
{
    return &x660_pathFindSearch;
}

u8 CSpacePirate::GetModelAlphau8(const CStateManager& mgr) const
{
    if ((mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::XRay &&
         mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::Thermal) ||
         !x400_25_alive)
    {
        if (!x635_27_shadowPirate)
            return u8(x42c_color.a() * 255.f);
        else
            return u8(x8b4_shadowPirateAlpha * 255.f);
    }
    return 255;
}

float CSpacePirate::GetGravityConstant() const
{
    return 50.f;
}

CProjectileInfo* CSpacePirate::GetProjectileInfo()
{
    return &x568_pirateData.x20_mainProjectileInfo;
}

}
