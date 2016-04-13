#ifndef __URDE_CANIMDATA_HPP__
#define __URDE_CANIMDATA_HPP__

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "CCharacterInfo.hpp"
#include "CParticleDatabase.hpp"
#include "CPoseAsTransforms.hpp"
#include "CHierarchyPoseBuilder.hpp"

namespace urde
{
class CCharLayoutInfo;
class CSkinnedModel;
class CAnimSysContext;
class CAnimationManager;
class CTransitionManager;
class CCharacterFactory;
class IMetaAnim;

class CAnimData
{
    TLockedToken<CCharacterFactory> x0_charFactory;
    CCharacterInfo xc_charInfo;
    TLockedToken<CCharLayoutInfo> xcc_layoutData;
    TCachedToken<CSkinnedModel> xd8_modelData;
    // TLockedToken<CSkinnedModelWithAvgNormals> xe4_modelAvgNormalData;
    std::shared_ptr<CSkinnedModel> xf4_xrayModel;
    std::shared_ptr<CSkinnedModel> xf8_infraModel;
    std::shared_ptr<CAnimSysContext> xfc_animCtx;
    std::shared_ptr<CAnimationManager> x100_animMgr;
    u32 x104_ = 0;
    zeus::CAABox x108_aabb;
    CParticleDatabase x120_particleDB;
    TResId x1d8_selfId;
    zeus::CVector3f x1dc_;
    zeus::CQuaternion x1e8_;
    std::shared_ptr<IMetaAnim> x1f8_animRoot;
    std::shared_ptr<CTransitionManager> x1fc_transMgr;

    float x200_ = 1.f;
    u32 x204_b;
    u16 x208_a;
    u32 x20c_passedBoolCount = 0;
    u32 x210_passedIntCount = 0;
    u32 x214_passedParticleCount = 0;
    u32 x218_passedSoundCount = 0;

    union
    {
        u32 x21c_flags = 0;
        struct
        {
            bool x21c_24_ : 1;
            bool x21c_25_c : 1;
            bool x21c_26_ : 1;
            bool x21c_27_ : 1;
            bool x21c_28_ : 1;
            bool x21c_29_ : 1;
            bool x21c_30_ : 1;
            bool x21c_31_ : 1;
        };
    };

    CPoseAsTransforms x220_pose;
    CHierarchyPoseBuilder x2f8_poseBuilder;

    u32 x101c_ = -1;
    u32 x1020_ = -1;
    float x1024_ = 1.f;
    bool x1028_ = true;
    u32 x102c_ = 0;
    u32 x1030_ = 0;
    bool x1034_ = false;
    u32 x1038_ = 0;
    u32 x103c_ = 0;
    u32 x1040_ = 0;
    u32 x1044_ = 0;

public:
    CAnimData(TResId, const CCharacterInfo& character, int a, int b, bool c,
              const TLockedToken<CCharLayoutInfo>& layout,
              const TToken<CSkinnedModel>& model,
              const std::weak_ptr<CAnimSysContext>& ctx,
              const std::shared_ptr<CAnimationManager>& animMgr,
              const std::shared_ptr<CTransitionManager>& transMgr,
              const TLockedToken<CCharacterFactory>& charFactory);
    static void InitializeCache()
    {
    }
};

}

#endif // __URDE_CANIMDATA_HPP__
