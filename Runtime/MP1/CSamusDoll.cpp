#include "CSamusDoll.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CDependencyGroup.hpp"
#include "zeus/CEulerAngles.hpp"
#include "Collision/CollisionUtil.hpp"

namespace urde
{
namespace MP1
{

const zeus::CVector3f CSamusDoll::skInitialOffset = {0.f, 0.f, 0.8f};

static const std::pair<const char*, u32> SpiderBallGlassModels[] =
{
    {"SamusSpiderBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 1},
    {"SamusPhazonBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 1},
    {"SamusPhazonBallGlassCMDL", 0},
};

static const std::pair<const char*, u32> SpiderBallCharacters[] =
{
    {"SamusSpiderBallANCS", 0},
    {"SamusSpiderBallANCS", 0},
    {"SamusSpiderBallANCS", 1},
    {"SamusPhazonBallANCS", 0},
    {"SamusFusionBallANCS", 0},
    {"SamusFusionBallANCS", 2},
    {"SamusFusionBallANCS", 1},
    {"SamusFusionBallANCS", 3},
};

static const std::pair<const char*, u32> BallCharacters[] =
{
    {"SamusBallANCS", 0},
    {"SamusBallANCS", 0},
    {"SamusBallANCS", 1},
    {"SamusBallANCS", 0},
    {"SamusFusionBallANCS", 0},
    {"SamusFusionBallANCS", 2},
    {"SamusFusionBallANCS", 1},
    {"SamusFusionBallANCS", 3},
};

static const u32 SpiderBallGlowColorIdxs[] =
{
    3, 3, 2, 4, 5, 7, 6, 8
};

static const u32 BallGlowColorIdxs[] =
{
    0, 0, 1, 0, 5, 7, 6, 8
};

static const char* BeamModels[] =
{
    "CMDL_InvPowerBeam",
    "CMDL_InvIceBeam",
    "CMDL_InvWaveBeam",
    "CMDL_InvPlasmaBeam",
    "CMDL_InvPowerBeam"
};

static const char* VisorModels[] =
{
    "CMDL_InvVisor",
    "CMDL_InvGravityVisor",
    "CMDL_InvVisor",
    "CMDL_InvPhazonVisor",
    "CMDL_InvFusionVisor",
    "CMDL_InvFusionVisor",
    "CMDL_InvFusionVisor",
    "CMDL_InvFusionVisor"
};

static const char* FinModels[] =
{
    "CMDL_InvPowerFins",
    "CMDL_InvPowerFins",
    "CMDL_InvPowerFins",
    "CMDL_InvPowerFins",
    "CMDL_InvPowerFins",
    "CMDL_InvVariaFins",
    "CMDL_InvGravityFins",
    "CMDL_InvPhazonFins"
};

static const u32 Character1Idxs[8] =
{
    0, 6, 2, 10, 16, 24, 20, 28
};

static const u32 CharacterBootsIdxs[8] =
{
    1, 7, 3, 11, 17, 25, 21, 29
};

static const u32 Character2and3Idxs[8][2] =
{
    {14, 15},
    {8, 9},
    {4, 5},
    {12, 13},
    {18, 19},
    {26, 27},
    {22, 23},
    {30, 31}
};

CSamusDoll::CSamusDoll(const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp,
                       CPlayerState::EPlayerSuit suit, CPlayerState::EBeamId beam,
                       bool hasSpiderBall, bool hasGrappleBeam)
: x44_suit(suit), x48_beam(beam)
{
    x1d4_spiderBallGlass = g_SimplePool->GetObj(SpiderBallGlassModels[int(suit)].first);
    x1e0_ballMatIdx = hasSpiderBall ? SpiderBallCharacters[int(suit)].second : BallCharacters[int(suit)].second;
    x1e4_glassMatIdx = SpiderBallGlassModels[int(suit)].second;
    x1e8_ballGlowColorIdx = hasSpiderBall ? SpiderBallGlowColorIdxs[int(suit)] : BallGlowColorIdxs[int(suit)];
    x1ec_itemScreenSamus = g_SimplePool->GetObj("ANCS_ItemScreenSamus");
    x1f4_invBeam = g_SimplePool->GetObj(BeamModels[int(beam)]);
    x200_invVisor = g_SimplePool->GetObj(VisorModels[int(suit)]);
    x20c_invGrappleBeam = g_SimplePool->GetObj("CMDL_InvGrappleBeam");
    x218_invFins = g_SimplePool->GetObj(FinModels[int(suit)]);
    x224_ballInnerGlow = g_SimplePool->GetObj("BallInnerGlow");
    x22c_ballInnerGlowGen = std::make_unique<CElementGen>(x224_ballInnerGlow,
                                                          CElementGen::EModelOrientationType::Normal,
                                                          CElementGen::EOptionalSystemFlags::One);
    x230_ballTransitionFlash = g_SimplePool->GetObj("MorphBallTransitionFlash");
    x23c_lights.push_back(CLight::BuildDirectional(zeus::CVector3f::skForward, zeus::CColor::skWhite));
    x24c_actorLights = std::make_unique<CActorLights>(8, zeus::CVector3f::skZero, 4, 4, false, false, false, 0.1f);
    x270_24_hasSpiderBall = hasSpiderBall;
    x270_25_hasGrappleBeam = hasGrappleBeam;
    x22c_ballInnerGlowGen->SetGlobalScale(zeus::CVector3f(0.625f));
    x0_depToks.reserve(suitDgrp.GetObjectTagVector().size() + ballDgrp.GetObjectTagVector().size());
    for (const SObjectTag& tag : suitDgrp.GetObjectTagVector())
    {
        x0_depToks.push_back(g_SimplePool->GetObj(tag));
        x0_depToks.back().Lock();
    }
    for (const SObjectTag& tag : ballDgrp.GetObjectTagVector())
    {
        x0_depToks.push_back(g_SimplePool->GetObj(tag));
        x0_depToks.back().Lock();
    }
}

bool CSamusDoll::IsLoaded() const
{
    if (x270_31_loaded)
        return true;
    if (!x1ec_itemScreenSamus.IsLoaded())
        return false;
    if (!x1f4_invBeam.IsLoaded())
        return false;
    if (!x200_invVisor.IsLoaded())
        return false;
    if (!x20c_invGrappleBeam.IsLoaded())
        return false;
    if (!x1d4_spiderBallGlass.IsLoaded())
        return false;
    if (x218_invFins && !x218_invFins.IsLoaded())
        return false;
    return xc8_suitModel1.operator bool();
}

CModelData CSamusDoll::BuildSuitModelData1(CPlayerState::EPlayerSuit suit)
{
    CModelData ret(CAnimRes(g_ResFactory->GetResourceIdByName("ANCS_ItemScreenSamus")->id,
                            Character1Idxs[int(suit)], zeus::CVector3f::skOne, 2, true));
    CAnimPlaybackParms parms(2, -1, 1.f, true);
    ret.AnimationData()->SetAnimation(parms, false);
    return ret;
}

CModelData CSamusDoll::BuildSuitModelDataBoots(CPlayerState::EPlayerSuit suit)
{
    CModelData ret(CAnimRes(g_ResFactory->GetResourceIdByName("ANCS_ItemScreenSamus")->id,
                            CharacterBootsIdxs[int(suit)], zeus::CVector3f::skOne, 2, true));
    CAnimPlaybackParms parms(2, -1, 1.f, true);
    ret.AnimationData()->SetAnimation(parms, false);
    return ret;
}

bool CSamusDoll::CheckLoadComplete()
{
    if (IsLoaded())
        return true;

    for (const CToken& tok : x0_depToks)
        if (!tok.IsLoaded())
            return false;

    xc8_suitModel1.emplace(BuildSuitModelData1(x44_suit));
    for (int i=0 ; i<2 ; ++i)
    {
        CAnimRes res(g_ResFactory->GetResourceIdByName("ANCS_ItemScreenSamus")->id,
                     Character2and3Idxs[int(x44_suit)][i],
                     zeus::CVector3f::skOne, 2, true);
        CModelData mData(res);
        x118_suitModel1and2.push_back(mData.GetAnimationData()->GetModelData());
        x118_suitModel1and2.back().Lock();
    }
    x134_suitModelBoots.emplace(BuildSuitModelDataBoots(x44_suit));

    CAnimRes res(g_ResFactory->GetResourceIdByName(x270_24_hasSpiderBall ?
                                                   SpiderBallCharacters[int(x44_suit)].first :
                                                   BallCharacters[int(x44_suit)].first)->id,
                 0, zeus::CVector3f::skOne, 0, true);
    x184_ballModelData.emplace(res);
    x1e0_ballMatIdx = x270_24_hasSpiderBall ?
                      SpiderBallCharacters[int(x44_suit)].second :
                      BallCharacters[int(x44_suit)].second;
    x270_31_loaded = true;
    return true;
}

void CSamusDoll::Update(float dt, CRandom16& rand)
{

}

void CSamusDoll::Draw(const CStateManager& mgr, float alpha)
{

}

void CSamusDoll::Touch()
{
    if (!CheckLoadComplete())
        return;
    xc8_suitModel1->AnimationData()->PreRender();
    x134_suitModelBoots->AnimationData()->PreRender();
    x184_ballModelData->AnimationData()->PreRender();
    xc8_suitModel1->Touch(CModelData::EWhichModel::Normal, 0);
    x134_suitModelBoots->Touch(CModelData::EWhichModel::Normal, 0);
    x184_ballModelData->Touch(CModelData::EWhichModel::Normal, 0);
}

void CSamusDoll::SetupLights()
{
    x23c_lights[0] = CLight::BuildDirectional(xb0_rot.toTransform().basis[1], zeus::CColor(0.75f, 1.f));
    x24c_actorLights->BuildFakeLightList(x23c_lights, zeus::CColor::skBlack);
}

void CSamusDoll::CheckTransition(bool morphballComplete)
{
    if (x54_ > 0.f)
        return;
    if (x4d_morphballComplete == morphballComplete)
        return;
    x4d_morphballComplete = morphballComplete;
    SetTransitionAnimation();
}

void CSamusDoll::SetTransitionAnimation()
{
    if (!x4c_intoBallComplete)
    {
        xc8_suitModel1->AnimationData()->SetAnimation(CAnimPlaybackParms{0, -1, 1.f, true}, false);
        x134_suitModelBoots->AnimationData()->SetAnimation(CAnimPlaybackParms{0, -1, 1.f, true}, false);
        x50_ = x54_ = xc8_suitModel1->GetAnimationData()->GetAnimationDuration(0);
    }
    else if (!x4d_morphballComplete)
    {
        xc8_suitModel1->AnimationData()->SetAnimation(CAnimPlaybackParms{1, -1, 1.f, true}, false);
        x134_suitModelBoots->AnimationData()->SetAnimation(CAnimPlaybackParms{1, -1, 1.f, true}, false);
        x50_ = x54_ = xc8_suitModel1->GetAnimationData()->GetAnimationDuration(1);
    }
}

void CSamusDoll::SetRotationSfxPlaying(bool playing)
{
    if (playing)
    {
        if (x268_rotateSfx)
            return;
        x268_rotateSfx = CSfxManager::SfxStart(1375, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
    }
    else
    {
        CSfxManager::SfxStop(x268_rotateSfx);
        x268_rotateSfx.reset();
    }
}

void CSamusDoll::SetOffsetSfxPlaying(bool playing)
{
    if (playing)
    {
        if (x264_offsetSfx)
            return;
        x264_offsetSfx = CSfxManager::SfxStart(1406, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
    }
    else
    {
        CSfxManager::SfxStop(x264_offsetSfx);
        x264_offsetSfx.reset();
    }
}

void CSamusDoll::SetZoomSfxPlaying(bool playing)
{
    if (playing)
    {
        if (x26c_zoomSfx)
            return;
        x26c_zoomSfx = CSfxManager::SfxStart(1376, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
    }
    else
    {
        CSfxManager::SfxStop(x26c_zoomSfx);
        x26c_zoomSfx.reset();
    }
}

void CSamusDoll::SetRotation(float xDelta, float zDelta, float f3)
{
    if (xc4_viewInterp != 0.f && xc4_viewInterp != 1.f)
        return;
    SetRotationSfxPlaying(xDelta != 0.f || zDelta != 0.f);
    zeus::CEulerAngles angles(xb0_rot);

    zeus::CRelAngle angX(angles.x);
    zeus::CRelAngle angZ(angles.z);

    angX += xDelta;
    angZ += zDelta;

    float angXCenter = angX;
    if (angXCenter > M_PIF)
        angXCenter -= 2.f * M_PIF;
    angX = zeus::clamp(-M_PIF / 2.f, angXCenter, M_PIF / 2.f);

    float angZCenter = angZ;
    if (angZCenter > M_PIF)
        angZCenter -= 2.f * M_PIF;
    angZ = zeus::clamp(-M_PIF / 2.f, angZCenter, M_PIF / 2.f);

    zeus::CQuaternion quat;
    quat.rotateZ(angZ);
    quat.rotateX(angX);
    xb0_rot = quat;
}

void CSamusDoll::SetOffset(const zeus::CVector3f& offset, float sfxThreshold)
{
    if (xc4_viewInterp != 0.f && xc4_viewInterp != 1.f)
        return;
    zeus::CVector3f oldOffset = xa4_offset;
    zeus::CMatrix3f rotMtx = xb0_rot.toTransform().basis;
    xa4_offset += rotMtx * zeus::CVector3f(offset.x, 0.f, offset.z);
    SetOffsetSfxPlaying((oldOffset - xa4_offset).magnitude() > sfxThreshold);
    float oldZoom = xc0_zoom;
    xc0_zoom = zeus::clamp(-4.f, xc0_zoom + offset.y, -2.2f);
    bool zoomSfx = std::fabs(xc0_zoom - oldZoom) > sfxThreshold;
    float zoomDelta = offset.y - (xc0_zoom - oldZoom);
    zeus::CVector3f newOffset = rotMtx[1] * zoomDelta + xa4_offset;
    zeus::CVector3f delta = newOffset - xa4_offset;
    oldOffset = xa4_offset;
    if (!delta.isZero())
    {
        zeus::CSphere sphere(skInitialOffset, 1.f);
        float T;
        zeus::CVector3f point;
        if (CollisionUtil::RaySphereIntersection(sphere, xa4_offset, delta.normalized(), 0.f, T, point))
        {
            if ((point - xa4_offset).magnitude() < std::fabs(zoomDelta))
                xa4_offset = point;
            else
                xa4_offset = newOffset;
        }
        else
        {
            xa4_offset = newOffset;
        }
    }
    if ((oldOffset - xa4_offset).magnitude() > sfxThreshold)
        zoomSfx = true;
    SetZoomSfxPlaying(zoomSfx);
    delta = xa4_offset - skInitialOffset;
    if (delta.magnitude() > 1.f)
        xa4_offset = delta.normalized() + skInitialOffset;
}

void CSamusDoll::BeginViewInterpolate(bool zoomOut)
{
    if (xc4_viewInterp == 0.f)
    {
        CSfxManager::SfxStart(1440, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    }
    else
    {
        SetOffsetSfxPlaying(false);
        SetZoomSfxPlaying(false);
        SetRotationSfxPlaying(false);
        CSfxManager::SfxStart(1441, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    }

    xc4_viewInterp = zoomOut ? FLT_EPSILON : (-1.f + FLT_EPSILON);
    x84_interpStartOffset = xa4_offset;
    x90_interpStartRot = xb0_rot;
    xa0_interpStartZoom = xc0_zoom;

    x80_ = zoomOut ? -2.2f : -3.6f;
}

}
}
