#ifndef __URDE_CANIMDATA_HPP__
#define __URDE_CANIMDATA_HPP__

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "CCharacterInfo.hpp"
#include "CParticleDatabase.hpp"
#include "CPoseAsTransforms.hpp"
#include "CHierarchyPoseBuilder.hpp"
#include "CAdditiveAnimPlayback.hpp"
#include "CCharLayoutInfo.hpp"
#include "CAnimPlaybackParms.hpp"
#include <set>

enum class EUserEventType
{
    Projectile,
    EggLay,
    LoopedSoundStop,
    AlignTargetPos,
    AlignTargetRot,
    ChangeMaterial,
    Delete,
    GenerateEnd,
    DamageOn,
    DamageOff,
    AlignTargetPosStart,
    DeGenerate,
    Landing,
    TakeOff,
    FadeIn,
    FadeOut,
    ScreenShake,
    BeginAction,
    EndAction,
    BecomeRagDoll,
    IkLock,
    IkRelease,
    BreakLockOn,
    BecomeShootThrough,
    RemoveCollision,
    ObjectPickUp,
    ObjectDrop,
    EventStart,
    EventStop,
    Activate,
    Deactivate,
    SoundPlay,
    SoundStop,
    EffectOn,
    EffectOff
};

namespace urde
{
class CCharLayoutInfo;
class CSkinnedModel;
class CMorphableSkinnedModel;
struct CAnimSysContext;
class CAnimationManager;
class CTransitionManager;
class CCharacterFactory;
class IMetaAnim;
struct CModelFlags;
class CVertexMorphEffect;
class CPrimitive;
class CRandom16;
class CStateManager;
class CCharAnimTime;
class CModel;
class CSkinRules;
class CAnimTreeNode;
class CSegIdList;
class CSegStatementSet;
class CBoolPOINode;
class CInt32POINode;
class CParticlePOINode;
class CSoundPOINode;
class IAnimReader;
struct SAdvancementDeltas;

class CAnimData
{
    friend class CModelData;
    TLockedToken<CCharacterFactory> x0_charFactory;
    CCharacterInfo xc_charInfo;
    TLockedToken<CCharLayoutInfo> xcc_layoutData;
    TCachedToken<CSkinnedModel> xd8_modelData;
    TLockedToken<CMorphableSkinnedModel> xe4_iceModelData;
    std::shared_ptr<CSkinnedModel> xf4_xrayModel;
    std::shared_ptr<CSkinnedModel> xf8_infraModel;
    std::shared_ptr<CAnimSysContext> xfc_animCtx;
    std::shared_ptr<CAnimationManager> x100_animMgr;
    u32 x104_ = 0;
    zeus::CAABox x108_aabb;
    CParticleDatabase x120_particleDB;
    ResId x1d8_selfId;
    zeus::CVector3f x1dc_alignPos;
    zeus::CQuaternion x1e8_alignRot;
    std::shared_ptr<CAnimTreeNode> x1f8_animRoot;
    std::shared_ptr<CTransitionManager> x1fc_transMgr;

    float x200_speedScale = 1.f;
    u32 x204_charIdx;
    u16 x208_defaultAnim;
    u32 x20c_passedBoolCount = 0;
    u32 x210_passedIntCount = 0;
    u32 x214_passedParticleCount = 0;
    u32 x218_passedSoundCount = 0;
    u32 x21c_ = 0;

    union
    {
        u32 x220_flags = 0;
        struct
        {
            bool x220_24_animating : 1;
            bool x220_25_loop : 1;
            bool x220_26_ : 1;
            bool x220_27_ : 1;
            bool x220_28_ : 1;
            bool x220_29_animationJustStarted : 1;
            bool x220_30_poseBuilt : 1;
            bool x220_31_poseCached : 1;
        };
    };

    CPoseAsTransforms x224_pose;
    CHierarchyPoseBuilder x2fc_poseBuilder;

    CAnimPlaybackParms x40c_playbackParms;
    rstl::reserved_vector<std::pair<u32, CAdditiveAnimPlayback>, 8> x434_additiveAnims;

    static rstl::reserved_vector<CBoolPOINode, 8> g_BoolPOINodes;
    static rstl::reserved_vector<CInt32POINode, 16> g_Int32POINodes;
    static rstl::reserved_vector<CParticlePOINode, 20> g_ParticlePOINodes;
    static rstl::reserved_vector<CSoundPOINode, 20> g_SoundPOINodes;

    int m_drawInstCount;

public:
    CAnimData(ResId,
              const CCharacterInfo& character,
              int defaultAnim, int charIdx, bool loop,
              const TLockedToken<CCharLayoutInfo>& layout,
              const TToken<CSkinnedModel>& model,
              const std::experimental::optional<TToken<CMorphableSkinnedModel>>& iceModel,
              const std::weak_ptr<CAnimSysContext>& ctx,
              const std::shared_ptr<CAnimationManager>& animMgr,
              const std::shared_ptr<CTransitionManager>& transMgr,
              const TLockedToken<CCharacterFactory>& charFactory,
              int drawInstCount);

    ResId GetEventResourceIdForAnimResourceId(ResId) const;
    void AddAdditiveSegData(const CSegIdList& list, CSegStatementSet& stSet);
    SAdvancementDeltas AdvanceAdditiveAnims(float);
    SAdvancementDeltas UpdateAdditiveAnims(float);
    bool IsAdditiveAnimation(u32) const;
    std::shared_ptr<CAnimTreeNode> GetAdditiveAnimationTree(u32) const;
    bool IsAdditiveAnimationActive(u32) const;
    void DelAdditiveAnimation(u32);
    void AddAdditiveAnimation(u32, float, bool, bool);
    std::shared_ptr<CAnimationManager> GetAnimationManager();
    const CCharacterInfo& GetCharacterInfo() const { return xc_charInfo; }
    const CCharLayoutInfo& GetCharLayoutInfo() const { return *xcc_layoutData.GetObj(); }
    void SetPhase(float);
    void Touch(const CSkinnedModel& model, int shaderIdx) const;
    SAdvancementDeltas GetAdvancementDeltas(const CCharAnimTime& a, const CCharAnimTime& b) const;
    CCharAnimTime GetTimeOfUserEvent(EUserEventType, const CCharAnimTime& time) const;
    void MultiplyPlaybackRate(float);
    void SetPlaybackRate(float);
    void SetRandomPlaybackRate(CRandom16&);
    void CalcPlaybackAlignmentParms(const CAnimPlaybackParms& parms,
                                    const std::shared_ptr<CAnimTreeNode>& node);
    zeus::CTransform GetLocatorTransform(CSegId id, const CCharAnimTime* time) const;
    zeus::CTransform GetLocatorTransform(const std::string& name, const CCharAnimTime* time) const;
    bool IsAnimTimeRemaining(float, const std::string& name) const;
    float GetAnimTimeRemaining(const std::string& name) const;
    float GetAnimationDuration(int) const;
    bool GetIsLoop() const {return x220_25_loop;}
    void EnableLooping(bool val) {x220_25_loop = val; x220_24_animating = true;}
    bool IsAnimating() const {return x220_24_animating;}
    std::shared_ptr<CAnimSysContext> GetAnimSysContext() const;
    std::shared_ptr<CAnimationManager> GetAnimationManager() const;
    void RecalcPoseBuilder(const CCharAnimTime*);
    void RenderAuxiliary(const zeus::CFrustum& frustum) const;
    void Render(CSkinnedModel& model, const CModelFlags& drawFlags,
                const std::experimental::optional<CVertexMorphEffect>& morphEffect,
                const float* morphMagnitudes);
    void SetupRender(CSkinnedModel& model,
                     const CModelFlags& drawFlags,
                     const std::experimental::optional<CVertexMorphEffect>& morphEffect,
                     const float* morphMagnitudes);
    static void DrawSkinnedModel(CSkinnedModel& model, const CModelFlags& flags);
    void PreRender();
    void BuildPose();
    static void PrimitiveSetToTokenVector(const std::set<CPrimitive>& primSet,
                                          std::vector<CToken>& tokensOut, bool preLock);
    void GetAnimationPrimitives(const CAnimPlaybackParms& parms, std::set<CPrimitive>& primsOut) const;
    void SetAnimation(const CAnimPlaybackParms& parms, bool);
    SAdvancementDeltas DoAdvance(float, bool&, CRandom16&, bool advTree);
    SAdvancementDeltas Advance(float, const zeus::CVector3f&, CStateManager& stateMgr, TAreaId aid, bool advTree);
    SAdvancementDeltas AdvanceIgnoreParticles(float, CRandom16&, bool advTree);
    void AdvanceAnim(CCharAnimTime& time, zeus::CVector3f&, zeus::CQuaternion&);
    void SetXRayModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules);
    void SetInfraModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules);

    static void PoseSkinnedModel(CSkinnedModel& model, const CPoseAsTransforms& pose,
                                 const CModelFlags& drawFlags,
                                 const std::experimental::optional<CVertexMorphEffect>& morphEffect,
                                 const float* morphMagnitudes);
    void AdvanceParticles(const zeus::CTransform& xf, float dt,
                          const zeus::CVector3f&, CStateManager& stateMgr);
    float GetAverageVelocity(int animIn) const;
    void ResetPOILists();
    CSegId GetLocatorSegId(const std::string& name) const;
    zeus::CAABox GetBoundingBox(const zeus::CTransform& xf) const;
    zeus::CAABox GetBoundingBox() const;
    static void FreeCache();
    static void InitializeCache();
    const CHierarchyPoseBuilder& GetPoseBuilder() const { return x2fc_poseBuilder; }
    const CParticleDatabase& GetParticleDB() const { return x120_particleDB; }
};

}

#endif // __URDE_CANIMDATA_HPP__
