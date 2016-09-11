#include "CAnimData.hpp"
#include "CCharacterInfo.hpp"
#include "CCharLayoutInfo.hpp"
#include "Graphics/CSkinnedModel.hpp"
#include "CCharacterFactory.hpp"
#include "CAnimationManager.hpp"
#include "CTransitionManager.hpp"
#include "CAdditiveAnimPlayback.hpp"
#include "CBoolPOINode.hpp"
#include "CInt32POINode.hpp"
#include "CParticlePOINode.hpp"
#include "CSoundPOINode.hpp"
#include "CParticleGenInfo.hpp"
#include "IAnimReader.hpp"
#include "CAnimTreeNode.hpp"
#include "CAnimPerSegmentData.hpp"
#include "CSegStatementSet.hpp"
#include "CStateManager.hpp"
#include "CAnimPlaybackParms.hpp"
#include "CAnimTreeBlend.hpp"
#include "CPrimitive.hpp"
#include "CAllFormatsAnimSource.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{
rstl::reserved_vector<CBoolPOINode, 8>CAnimData::g_BoolPOINodes;
rstl::reserved_vector<CInt32POINode, 16> CAnimData::g_Int32POINodes;
rstl::reserved_vector<CParticlePOINode, 20> CAnimData::g_ParticlePOINodes;
rstl::reserved_vector<CSoundPOINode, 20> CAnimData::g_SoundPOINodes;

void CAnimData::FreeCache()
{
}

void CAnimData::InitializeCache()
{
}

CAnimData::CAnimData(ResId id,
                     const CCharacterInfo& character,
                     int defaultAnim, int charIdx, bool loop,
                     const TLockedToken<CCharLayoutInfo>& layout,
                     const TToken<CSkinnedModel>& model,
                     const rstl::optional_object<TToken<CMorphableSkinnedModel>>& iceModel,
                     const std::weak_ptr<CAnimSysContext>& ctx,
                     const std::shared_ptr<CAnimationManager>& animMgr,
                     const std::shared_ptr<CTransitionManager>& transMgr,
                     const TLockedToken<CCharacterFactory>& charFactory)
: x0_charFactory(charFactory),
  xc_charInfo(character),
  xcc_layoutData(layout),
  xd8_modelData(model),
  xfc_animCtx(ctx.lock()),
  x100_animMgr(animMgr),
  x1d8_selfId(id),
  x1fc_transMgr(transMgr),
  x204_charIdx(charIdx),
  x208_defaultAnim(defaultAnim),
  x224_pose(layout->GetSegIdList().GetList().size()),
  x2fc_poseBuilder(layout)
{
    x220_25_loop = loop;

    if (iceModel)
        xe4_iceModelData = *iceModel;

    g_BoolPOINodes.resize(8);
    g_Int32POINodes.resize(16);
    g_ParticlePOINodes.resize(20);
    g_SoundPOINodes.resize(20);

    x108_aabb = xd8_modelData->GetModel()->GetAABB();
    x120_particleDB.CacheParticleDesc(xc_charInfo.GetParticleResData());
}

ResId CAnimData::GetEventResourceIdForAnimResourceId(ResId id) const
{
    return x0_charFactory->GetEventResourceIdForAnimResourceId(id);
}

void CAnimData::AddAdditiveSegData(const CSegIdList& list, CSegStatementSet& stSet)
{
    for (std::pair<u32, CAdditiveAnimPlayback>& additive : x434_additiveAnims)
        if (additive.second.GetTargetWeight() > 0.00001f)
            additive.second.AddToSegStatementSet(list, *xcc_layoutData.GetObj(), stSet);
}

static void AdvanceAnimationTree(std::weak_ptr<CAnimTreeNode>& anim, const CCharAnimTime& dt)
{
}

SAdvancementDeltas CAnimData::AdvanceAdditiveAnims(float dt)
{
    CCharAnimTime time(dt);

    for (std::pair<u32, CAdditiveAnimPlayback>& additive : x434_additiveAnims)
    {
        if (additive.second.IsActive())
        {
            while (time.GreaterThanZero() && std::fabs(time) >= 0.00001f)
            {
                //additive.second.GetAnim()->GetInt32POIList(time, );
            }
        }
        else
        {
        }
    }

    return {};
}

SAdvancementDeltas CAnimData::UpdateAdditiveAnims(float dt)
{


    return AdvanceAdditiveAnims(dt);
}

bool CAnimData::IsAdditiveAnimation(u32 idx) const
{
    auto search = std::find_if(x434_additiveAnims.cbegin(), x434_additiveAnims.cend(),
                               [&](const std::pair<u32, CAdditiveAnimPlayback>& pair) -> bool {
        return pair.first == idx;
    });
    if (search == x434_additiveAnims.cend())
        return false;
    return true;
}

std::shared_ptr<CAnimTreeNode> CAnimData::GetAdditiveAnimationTree(u32 idx) const
{
    auto search = std::find_if(x434_additiveAnims.cbegin(), x434_additiveAnims.cend(),
                               [&](const std::pair<u32, CAdditiveAnimPlayback>& pair) -> bool {
        return pair.first == idx;
    });
    if (search == x434_additiveAnims.cend())
        return {};
    return search->second.GetAnim();
}

bool CAnimData::IsAdditiveAnimationActive(u32 idx) const
{
    auto search = std::find_if(x434_additiveAnims.cbegin(), x434_additiveAnims.cend(),
                               [&](const std::pair<u32, CAdditiveAnimPlayback>& pair) -> bool {
        return pair.first == idx;
    });
    if (search == x434_additiveAnims.cend())
        return {};
    return search->second.IsActive();
}

void CAnimData::DelAdditiveAnimation(u32)
{
}

void CAnimData::AddAdditiveAnimation(u32, float, bool, bool)
{
}

std::shared_ptr<CAnimationManager> CAnimData::GetAnimationManager()
{
    return x100_animMgr;
}

void CAnimData::SetPhase(float ph)
{
    x1f8_animRoot->VSetPhase(ph);
}

void CAnimData::Touch(const CSkinnedModel& model, int shadIdx) const
{
    const_cast<CBooModel&>(*model.GetModelInst()).Touch(shadIdx);
}

SAdvancementDeltas CAnimData::GetAdvancementDeltas(const CCharAnimTime& a,
                                                   const CCharAnimTime& b) const
{
    return x1f8_animRoot->VGetAdvancementResults(a, b).x8_deltas;
}

CCharAnimTime CAnimData::GetTimeOfUserEvent(EUserEventType, const CCharAnimTime& time) const
{
    return {};
}

void CAnimData::MultiplyPlaybackRate(float mul)
{
    x200_speedScale += mul;
}

void CAnimData::SetPlaybackRate(float set)
{
    x200_speedScale = set;
}

void CAnimData::SetRandomPlaybackRate(CRandom16&)
{
}

void CAnimData::CalcPlaybackAlignmentParms(const CAnimPlaybackParms& parms,
                                           const std::shared_ptr<CAnimTreeNode>& node)
{
}

zeus::CTransform CAnimData::GetLocatorTransform(CSegId id, const CCharAnimTime* time) const
{
    if (id == 0xFF)
        return {};

    zeus::CTransform ret;
    if (!x220_31_poseCached)
        const_cast<CAnimData*>(this)->RecalcPoseBuilder(time);

    if (!x220_30_poseBuilt)
        x2fc_poseBuilder.BuildTransform(id, ret);
    else
    {
        zeus::CMatrix3f rot = x224_pose.GetRotation(id);
        zeus::CVector3f offset = x224_pose.GetOffset(id);
        ret.setRotation(rot);
        ret.origin = offset;
    }
    return ret;
}

zeus::CTransform CAnimData::GetLocatorTransform(const std::string& name, const CCharAnimTime* time) const
{
    return GetLocatorTransform(xcc_layoutData->GetSegIdFromString(name), time);
}

bool CAnimData::IsAnimTimeRemaining(float rem, const std::string& name) const
{
    if (!x1f8_animRoot)
        return false;
    return float(x1f8_animRoot->VGetTimeRemaining()) <= rem;
}

float CAnimData::GetAnimTimeRemaining(const std::string& name) const
{
    float rem = x1f8_animRoot->VGetTimeRemaining();
    if (x200_speedScale)
        return rem / x200_speedScale;
    return rem;
}

float CAnimData::GetAnimationDuration(int animIn) const
{
    std::shared_ptr<IMetaAnim> anim = x100_animMgr->GetMetaAnimation(xc_charInfo.GetAnimationIndex(animIn));
    std::set<CPrimitive> prims;
    anim->GetUniquePrimitives(prims);

    SObjectTag tag{FOURCC('ANIM'), 0};
    float durAccum = 0.f;
    for (const CPrimitive& prim : prims)
    {
        tag.id = prim.GetAnimResId();
        TLockedToken<CAllFormatsAnimSource> animRes = xfc_animCtx->xc_store.GetObj(tag);

        CCharAnimTime dur;
        switch (animRes->GetFormat())
        {
        case EAnimFormat::Uncompressed:
        default:
        {
            const CAnimSource& src = animRes->GetAsCAnimSource();
            dur = src.GetDuration();
            break;
        }
        case EAnimFormat::BitstreamCompressed:
        case EAnimFormat::BitstreamCompressed24:
        {
            const CFBStreamedCompression& src = animRes->GetAsCFBStreamedCompression();
            dur = src.GetAnimationDuration();
            break;
        }
        }

        durAccum += dur;
    }

    if (anim->GetType() == EMetaAnimType::Random)
        return durAccum / float(prims.size());
    return durAccum;
}

std::shared_ptr<CAnimSysContext> CAnimData::GetAnimSysContext() const
{
    return xfc_animCtx;
}

std::shared_ptr<CAnimationManager> CAnimData::GetAnimationManager() const
{
    return x100_animMgr;
}

void CAnimData::RecalcPoseBuilder(const CCharAnimTime* time)
{
    if (!x1f8_animRoot)
        return;

    const CSegIdList& segIdList = GetCharLayoutInfo().GetSegIdList();
    CSegStatementSet segSet;
    if (time)
        x1f8_animRoot->VGetSegStatementSet(segIdList, segSet, *time);
    else
        x1f8_animRoot->VGetSegStatementSet(segIdList, segSet);

    AddAdditiveSegData(segIdList, segSet);
    for (const CSegId& id : segIdList.GetList())
    {
        if (id == 3)
            continue;
        CAnimPerSegmentData& segData = segSet[id];
        if (segData.x1c_hasOffset)
            x2fc_poseBuilder.Insert(id, segData.x0_rotation, segData.x10_offset);
        else
            x2fc_poseBuilder.Insert(id, segData.x0_rotation);
    }
}

void CAnimData::RenderAuxiliary(const zeus::CFrustum& frustum) const
{
    const_cast<CParticleDatabase&>(x120_particleDB).AddToRendererClipped(frustum);
}

void CAnimData::Render(CSkinnedModel& model, const CModelFlags& drawFlags,
                       const rstl::optional_object<CVertexMorphEffect>& morphEffect,
                       const float* morphMagnitudes)
{
    SetupRender(model, drawFlags, morphEffect, morphMagnitudes);
    DrawSkinnedModel(model, drawFlags);
}

void CAnimData::SetupRender(CSkinnedModel& model,
                            const CModelFlags& drawFlags,
                            const rstl::optional_object<CVertexMorphEffect>& morphEffect,
                            const float* morphMagnitudes)
{
    if (!x220_30_poseBuilt)
    {
        x2fc_poseBuilder.BuildNoScale(x224_pose);
        x220_30_poseBuilt = true;
    }
    PoseSkinnedModel(model, x224_pose, drawFlags, morphEffect, morphMagnitudes);
}

void CAnimData::DrawSkinnedModel(CSkinnedModel& model, const CModelFlags& flags)
{
    model.Draw(flags);
}

void CAnimData::PreRender()
{
    if (!x220_31_poseCached)
    {
        RecalcPoseBuilder(nullptr);
        x220_31_poseCached = true;
        x220_30_poseBuilt = false;
    }
}

void CAnimData::BuildPose()
{
    if (!x220_31_poseCached)
    {
        RecalcPoseBuilder(nullptr);
        x220_31_poseCached = true;
        x220_30_poseBuilt = false;
    }

    if (!x220_30_poseBuilt)
    {
        x2fc_poseBuilder.BuildNoScale(x224_pose);
        x220_30_poseBuilt = true;
    }
}

void CAnimData::PrimitiveSetToTokenVector(const std::set<CPrimitive>& primSet,
                                          std::vector<CToken>& tokensOut, bool preLock)
{
    tokensOut.reserve(primSet.size());

    SObjectTag tag{FOURCC('ANIM'), 0};
    for (const CPrimitive& prim : primSet)
    {
        tag.id = prim.GetAnimResId();
        tokensOut.push_back(g_SimplePool->GetObj(tag));
        if (preLock)
            tokensOut.back().Lock();
    }
}

void CAnimData::GetAnimationPrimitives(const CAnimPlaybackParms& parms, std::set<CPrimitive>& primsOut) const
{
    std::shared_ptr<IMetaAnim> animA =
        x100_animMgr->GetMetaAnimation(xc_charInfo.GetAnimationIndex(parms.x0_animA));
    animA->GetUniquePrimitives(primsOut);

    if (parms.x4_animB != -1)
    {
        std::shared_ptr<IMetaAnim> animB =
            x100_animMgr->GetMetaAnimation(xc_charInfo.GetAnimationIndex(parms.x4_animB));
        animB->GetUniquePrimitives(primsOut);
    }
}

void CAnimData::SetAnimation(const CAnimPlaybackParms& parms, bool noTrans)
{
    if (parms.x0_animA == x40c_playbackParms.x0_animA ||
        (parms.x4_animB == x40c_playbackParms.x4_animB &&
         parms.x8_blendWeight == x40c_playbackParms.x8_blendWeight &&
         parms.x8_blendWeight != 1.f) ||
        parms.x4_animB == -1)
    {
        if (x220_29_animationJustStarted)
            return;
    }

    x40c_playbackParms.x0_animA = parms.x0_animA;
    x40c_playbackParms.x4_animB = parms.x4_animB;
    x40c_playbackParms.x8_blendWeight = parms.x8_blendWeight;
    x200_speedScale = 1.f;
    x208_defaultAnim = parms.x0_animA;

    u32 animIdxA = xc_charInfo.GetAnimationIndex(parms.x0_animA);

    ResetPOILists();

    std::shared_ptr<CAnimTreeNode> blendNode;
    if (parms.x4_animB != -1)
    {
        u32 animIdxB = xc_charInfo.GetAnimationIndex(parms.x4_animB);

        std::shared_ptr<CAnimTreeNode> treeA =
            x100_animMgr->GetAnimationTree(animIdxA, CMetaAnimTreeBuildOrders::NoSpecialOrders());
        std::shared_ptr<CAnimTreeNode> treeB =
            x100_animMgr->GetAnimationTree(animIdxB, CMetaAnimTreeBuildOrders::NoSpecialOrders());

        blendNode = std::make_shared<CAnimTreeBlend>(false, treeA, treeB, parms.x8_blendWeight,
                                                     CAnimTreeBlend::CreatePrimitiveName(treeA, treeB,
                                                                                         parms.x8_blendWeight));
    }
    else
    {
        blendNode = x100_animMgr->GetAnimationTree(animIdxA, CMetaAnimTreeBuildOrders::NoSpecialOrders());
    }

    if (!noTrans && x1f8_animRoot)
        x1f8_animRoot = x1fc_transMgr->GetTransitionTree(x1f8_animRoot, blendNode);
    else
        x1f8_animRoot = blendNode;

    x220_24_animating = parms.xc_animating;
    CalcPlaybackAlignmentParms(parms, blendNode);
    ResetPOILists();
    x220_29_animationJustStarted = true;
}

SAdvancementDeltas CAnimData::DoAdvance(float dt, bool& b1, CRandom16& random, bool advTree)
{
    b1 = false;

    zeus::CVector3f offsetPre, offsetPost;
    zeus::CQuaternion quatPre, quatPost;

    ResetPOILists();
    float scaleDt = dt * x200_speedScale;
    if (x2fc_poseBuilder.HasRoot())
    {
        SAdvancementDeltas deltas = UpdateAdditiveAnims(scaleDt);
        offsetPre = deltas.x0_posDelta;
        quatPre = deltas.xc_rotDelta;
    }

    if (!x220_24_animating)
    {
        b1 = true;
        return {};
    }

    if (x220_29_animationJustStarted)
    {
        x220_29_animationJustStarted = false;
        b1 = true;
    }

    if (advTree && x1f8_animRoot)
    {
        SetRandomPlaybackRate(random);
        CCharAnimTime time(scaleDt);
        if (x220_25_loop)
        {
            while (time.GreaterThanZero() && !time.EpsilonZero())
            {
                x210_passedIntCount += x1f8_animRoot->GetInt32POIList(time, g_Int32POINodes.data(), 16, x210_passedIntCount, 0);
                x20c_passedBoolCount += x1f8_animRoot->GetBoolPOIList(time, g_BoolPOINodes.data(), 16, x20c_passedBoolCount, 0);
                x214_passedParticleCount += x1f8_animRoot->GetParticlePOIList(time, g_ParticlePOINodes.data(), 16, x214_passedParticleCount, 0);
                x218_passedSoundCount += x1f8_animRoot->GetSoundPOIList(time, g_SoundPOINodes.data(), 16, x218_passedSoundCount, 0);
                AdvanceAnim(time, offsetPost, quatPost);
            }
        }
        else
        {
            CCharAnimTime remTime = x1f8_animRoot->VGetTimeRemaining();
            while (remTime.GreaterThanZero() && !remTime.EpsilonZero())
            {
                x210_passedIntCount += x1f8_animRoot->GetInt32POIList(time, g_Int32POINodes.data(), 16, x210_passedIntCount, 0);
                x20c_passedBoolCount += x1f8_animRoot->GetBoolPOIList(time, g_BoolPOINodes.data(), 16, x20c_passedBoolCount, 0);
                x214_passedParticleCount += x1f8_animRoot->GetParticlePOIList(time, g_ParticlePOINodes.data(), 16, x214_passedParticleCount, 0);
                x218_passedSoundCount += x1f8_animRoot->GetSoundPOIList(time, g_SoundPOINodes.data(), 16, x218_passedSoundCount, 0);
                AdvanceAnim(time, offsetPost, quatPost);
                remTime = x1f8_animRoot->VGetTimeRemaining();
                time = std::max(0.f, std::min(float(remTime), float(time)));
                if (remTime.EpsilonZero())
                {
                    x220_24_animating = false;
                    x1dc_alignPos = zeus::CVector3f::skZero;
                    x220_28_ = false;
                    x220_26_ = false;
                }
            }
        }

        x220_31_poseCached = false;
        x220_30_poseBuilt = false;
    }

    return {offsetPost + offsetPre, quatPost * quatPre};
}

SAdvancementDeltas CAnimData::Advance(float dt, const zeus::CVector3f& scale,
                                      CStateManager& stateMgr, TAreaId aid, bool advTree)
{
    bool b2;
    return DoAdvance(dt, b2, *stateMgr.GetActiveRandom(), advTree);
    if (b2)
        x120_particleDB.SuspendAllActiveEffects(stateMgr);

    for (CParticlePOINode& node : g_ParticlePOINodes)
    {
        if (node.GetCharacterIndex() == -1 || node.GetCharacterIndex() == x204_charIdx)
        {
            x120_particleDB.StartEffect(node.GetName(), node.GetFlags(), node.GetParticleData(),
                                        scale, stateMgr, aid, x21c_);
        }
    }
}

SAdvancementDeltas CAnimData::AdvanceIgnoreParticles(float dt, CRandom16& random, bool advTree)
{
    bool b2;
    return DoAdvance(dt, b2, random, advTree);
}

void CAnimData::AdvanceAnim(CCharAnimTime& time, zeus::CVector3f& offset, zeus::CQuaternion& quat)
{
    SAdvancementResults results;
    std::shared_ptr<IAnimReader> simplified;

    if (!x104_)
    {
        results = x1f8_animRoot->VAdvanceView(time);
        simplified = x1f8_animRoot->VSimplified();
    }

    if (simplified)
    {
        if (simplified->IsCAnimTreeNode())
        {
            if (x1f8_animRoot != simplified)
                x1f8_animRoot = std::static_pointer_cast<CAnimTreeNode>(std::move(simplified));
        }
        else
            x1f8_animRoot.reset();
    }

    if ((x220_28_ || x220_27_) && x210_passedIntCount > 0)
    {
        for (CInt32POINode& node : g_Int32POINodes)
        {
            if (node.GetPoiType() == EPOIType::UserEvent)
            {
                switch (EUserEventType(node.GetValue()))
                {
                case EUserEventType::AlignTargetPosStart:
                {
                    x220_26_ = true;
                    break;
                }
                case EUserEventType::AlignTargetPos:
                {
                    x1dc_alignPos = zeus::CVector3f::skZero;
                    x220_28_ = false;
                    x220_26_ = false;
                    break;
                }
                case EUserEventType::AlignTargetRot:
                {
                    x1e8_alignRot = zeus::CQuaternion::skNoRotation;
                    x220_27_ = false;
                    break;
                }
                default: break;
                }
            }
        }
    }

    offset += results.x8_deltas.x0_posDelta;
    if (x220_26_)
        offset += x1dc_alignPos * time;

    zeus::CQuaternion rot = results.x8_deltas.xc_rotDelta * x1e8_alignRot;
    quat = quat * rot;
    x1dc_alignPos = rot.transform(x1dc_alignPos);
    time = results.x0_remTime;
}

void CAnimData::SetXRayModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules)
{
    xf4_xrayModel = std::make_shared<CSkinnedModel>(model, skinRules, xd8_modelData->GetLayoutInfo(), 0);
}

void CAnimData::SetInfraModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules)
{
    xf8_infraModel = std::make_shared<CSkinnedModel>(model, skinRules, xd8_modelData->GetLayoutInfo(), 0);
}

void CAnimData::PoseSkinnedModel(CSkinnedModel& model, const CPoseAsTransforms& pose,
                                 const CModelFlags& drawFlags,
                                 const rstl::optional_object<CVertexMorphEffect>& morphEffect,
                                 const float* morphMagnitudes)
{
    model.Calculate(pose, drawFlags, morphEffect, morphMagnitudes);
}

void CAnimData::AdvanceParticles(const zeus::CTransform& xf, float dt,
                                 const zeus::CVector3f& vec, CStateManager& stateMgr)
{
    x120_particleDB.Update(dt, x224_pose, *xcc_layoutData, xf, vec, stateMgr);
}

float CAnimData::GetAverageVelocity(int animIn) const
{
    std::shared_ptr<IMetaAnim> anim = x100_animMgr->GetMetaAnimation(xc_charInfo.GetAnimationIndex(animIn));
    std::set<CPrimitive> prims;
    anim->GetUniquePrimitives(prims);

    SObjectTag tag{FOURCC('ANIM'), 0};
    float velAccum = 0.f;
    float durAccum = 0.f;
    for (const CPrimitive& prim : prims)
    {
        tag.id = prim.GetAnimResId();
        TLockedToken<CAllFormatsAnimSource> animRes = xfc_animCtx->xc_store.GetObj(tag);

        CCharAnimTime dur;
        float avgVel;
        switch (animRes->GetFormat())
        {
        case EAnimFormat::Uncompressed:
        default:
        {
            const CAnimSource& src = animRes->GetAsCAnimSource();
            dur = src.GetDuration();
            avgVel = src.GetAverageVelocity();
            break;
        }
        case EAnimFormat::BitstreamCompressed:
        case EAnimFormat::BitstreamCompressed24:
        {
            const CFBStreamedCompression& src = animRes->GetAsCFBStreamedCompression();
            dur = src.GetAnimationDuration();
            avgVel = src.GetAverageVelocity();
            break;
        }
        }

        velAccum += dur * avgVel;
        durAccum += dur;
    }

    if (durAccum > 0.f)
        return velAccum / durAccum;
    return 0.f;
}

void CAnimData::ResetPOILists()
{
    x20c_passedBoolCount = 0;
    x210_passedIntCount = 0;
    x214_passedParticleCount = 0;
    x218_passedSoundCount = 0;
}

CSegId CAnimData::GetLocatorSegId(const std::string& name) const
{
    return xcc_layoutData->GetSegIdFromString(name);
}

zeus::CAABox CAnimData::GetBoundingBox(const zeus::CTransform& xf) const
{
    return GetBoundingBox().getTransformedAABox(xf);
}

zeus::CAABox CAnimData::GetBoundingBox() const
{
    auto aabbList = xc_charInfo.GetAnimBBoxList();
    if (aabbList.empty())
        return x108_aabb;

    CAnimTreeEffectiveContribution contrib = x1f8_animRoot->GetContributionOfHighestInfluence();
    auto search = std::find_if(aabbList.cbegin(), aabbList.cend(),
                               [&](const std::pair<std::string, zeus::CAABox>& other) -> bool {
        return contrib.x4_name == other.first;
    });
    if (search == aabbList.cend())
        return x108_aabb;

    return search->second;
}

}
