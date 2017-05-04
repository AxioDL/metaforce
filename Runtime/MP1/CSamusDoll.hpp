#ifndef __URDE_CSAMUSDOLL_HPP__
#define __URDE_CSAMUSDOLL_HPP__

#include "CPlayerState.hpp"
#include "CToken.hpp"
#include "Character/CModelData.hpp"
#include "Character/CAnimCharacterSet.hpp"
#include "Particle/CElementGen.hpp"
#include "Character/CActorLights.hpp"
#include "Audio/CSfxManager.hpp"

namespace urde
{
class CDependencyGroup;
namespace MP1
{

class CSamusDoll
{
    std::vector<CToken> x0_depToks;
    zeus::CTransform x10_;
    float x40_ = 0.f;
    CPlayerState::EPlayerSuit x44_suit;
    CPlayerState::EBeamId x48_beam;
    bool x4c_intoBallComplete = false;
    bool x4d_morphballComplete = false;
    float x50_ = 1.f;
    float x54_ = 0.f;
    float x58_ = 0.f;
    float x5c_ = 0.f;
    float x60_ = 0.f;
    float x64_ = 0.f;
    float x68_ = 0.f;
    float x6c_ = 0.f;
    zeus::CQuaternion x70_;
    float x80_ = -3.6f;
    zeus::CVector3f x84_interpStartOffset = skInitialOffset;
    zeus::CQuaternion x90_interpStartRot;
    float xa0_interpStartZoom = -3.6f;
    zeus::CVector3f xa4_offset = skInitialOffset;
    zeus::CQuaternion xb0_rot;
    float xc0_zoom = -3.6f;
    float xc4_viewInterp = 0.f;
    std::experimental::optional<CModelData> xc8_suitModel1;
    rstl::reserved_vector<TCachedToken<CSkinnedModel>, 2> x118_suitModel1and2;
    std::experimental::optional<CModelData> x134_suitModelBoots;
    std::experimental::optional<CModelData> x184_ballModelData;
    bool x1d0_ = false;
    TLockedToken<CModel> x1d4_spiderBallGlass;
    u32 x1e0_ballMatIdx;
    u32 x1e4_glassMatIdx;
    u32 x1e8_ballGlowColorIdx;
    TLockedToken<CAnimCharacterSet> x1ec_itemScreenSamus;
    TLockedToken<CModel> x1f4_invBeam;
    TLockedToken<CModel> x200_invVisor;
    TLockedToken<CModel> x20c_invGrappleBeam;
    TLockedToken<CModel> x218_invFins;
    TLockedToken<CGenDescription> x224_ballInnerGlow;
    std::unique_ptr<CElementGen> x22c_ballInnerGlowGen;
    TLockedToken<CGenDescription> x230_ballTransitionFlash;
    std::vector<CLight> x23c_lights;
    std::unique_ptr<CActorLights> x24c_actorLights;
    bool x25c_ = false;
    float x260_ = 0.f;
    CSfxHandle x264_offsetSfx;
    CSfxHandle x268_rotateSfx;
    CSfxHandle x26c_zoomSfx;
    union
    {
        struct
        {
            bool x270_24_hasSpiderBall : 1;
            bool x270_25_hasGrappleBeam : 1;
            bool x270_26_ : 1;
            bool x270_27_ : 1;
            bool x270_28_ : 1;
            bool x270_29_ : 1;
            bool x270_30_ : 1;
            bool x270_31_loaded : 1;
        };
        u32 _dummy = 0;
    };

    static const zeus::CVector3f skInitialOffset;
    static CModelData BuildSuitModelData1(CPlayerState::EPlayerSuit suit);
    static CModelData BuildSuitModelDataBoots(CPlayerState::EPlayerSuit suit);
    void SetupLights();
    void SetTransitionAnimation();
    void SetRotationSfxPlaying(bool playing);
    void SetOffsetSfxPlaying(bool playing);
    void SetZoomSfxPlaying(bool playing);
public:
    CSamusDoll(const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp,
               CPlayerState::EPlayerSuit suit, CPlayerState::EBeamId beam,
               bool hasSpiderBall, bool hasGrappleBeam);
    bool IsLoaded() const;
    bool CheckLoadComplete();
    void Update(float dt, CRandom16& rand);
    void Draw(const CStateManager& mgr, float alpha);
    void Touch();
    void CheckTransition(bool morphballComplete);
    void SetRotation(float xDelta, float zDelta, float);
    void SetOffset(const zeus::CVector3f& offset, float sfxThreshold);
    void BeginViewInterpolate(bool zoomOut);
};

}
}

#endif // __URDE_CSAMUSDOLL_HPP__
