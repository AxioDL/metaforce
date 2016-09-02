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
      x220_25_loop(loop),
      x224_pose(layout->GetSegIdList().GetList().size()),
      x2fc_poseBuilder(layout)
{
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
    for (std::pair<u32, CAdditiveAnimPlayback>& additive : x1048_additiveAnims)
        if (additive.second.GetTargetWeight() > 0.00001f)
            additive.second.AddToSegStatementSet(list, *xcc_layoutData.GetObj(), stSet);
}

static void AdvanceAnimationTree(std::weak_ptr<CAnimTreeNode>& anim, const CCharAnimTime& dt)
{
}

SAdvancementDeltas CAnimData::AdvanceAdditiveAnims(float dt)
{
    CCharAnimTime time(dt);

    for (std::pair<u32, CAdditiveAnimPlayback>& additive : x1048_additiveAnims)
    {
        if (additive.second.GetA())
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

bool CAnimData::IsAdditiveAnimation(u32) const
{
    return false;
}

std::shared_ptr<CAnimTreeNode> CAnimData::GetAdditiveAnimationTree(u32) const
{
    return {};
}

bool CAnimData::IsAdditiveAnimationActive(u32) const
{
    return false;
}

void CAnimData::DelAdditiveAnimation(u32)
{
}

void CAnimData::AddAdditiveAnimation(u32, float, bool, bool)
{
}

std::shared_ptr<CAnimationManager> CAnimData::GetAnimationManager()
{
    return {};
}

void CAnimData::SetPhase(float)
{
}

void CAnimData::Touch(const CSkinnedModel& model, int) const
{
}

SAdvancementDeltas CAnimData::GetAdvancementDeltas(const CCharAnimTime& a,
                                                   const CCharAnimTime& b) const
{
    return {};
}

CCharAnimTime CAnimData::GetTimeOfUserEvent(EUserEventType, const CCharAnimTime& time) const
{
    return {};
}

void CAnimData::MultiplyPlaybackRate(float)
{
}

void CAnimData::SetPlaybackRate(float)
{
}

void CAnimData::SetRandomPlaybackRate(CRandom16&)
{
}

void CAnimData::CalcPlaybackAlignmentParms(const CAnimPlaybackParms& parms,
                                           const std::weak_ptr<CAnimTreeNode>& node)
{
}

zeus::CTransform CAnimData::GetLocatorTransform(CSegId id, const CCharAnimTime* time) const
{
    return {};
}

zeus::CTransform CAnimData::GetLocatorTransform(const std::string& name, const CCharAnimTime* time) const
{
    return {};
}

bool CAnimData::IsAnimTimeRemaining(float, const std::string& name) const
{
    return false;
}

float CAnimData::GetAnimTimeRemaining(const std::string& name) const
{
    return 0.f;
}

float CAnimData::GetAnimationDuration(int) const
{
    return 0.f;
}

std::shared_ptr<CAnimSysContext> CAnimData::GetAnimSysContext() const
{
    return {};
}

std::shared_ptr<CAnimationManager> CAnimData::GetAnimationManager() const
{
    return {};
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
        x2fc_poseBuilder.Insert(id, segData.x0_rotation, segData.x10_offset);
    }
}

void CAnimData::RenderAuxiliary(const CFrustumPlanes& frustum) const
{
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
}

void CAnimData::PrimitiveSetToTokenVector(const std::set<CPrimitive>& primSet, std::vector<CToken>& tokensOut)
{
}

void CAnimData::GetAnimationPrimitives(const CAnimPlaybackParms& parms, std::set<CPrimitive>& primsOut) const
{
}

void CAnimData::SetAnimation(const CAnimPlaybackParms& parms, bool)
{
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

    if (x220_29_)
    {
        x220_29_ = false;
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
                    x1dc_ = zeus::CVector3f::skZero;
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

    if (x104_)
    {
        results = x1f8_animRoot->VAdvanceView(time);
        simplified = x1f8_animRoot->VSimplified();
    }

    if (simplified)
    {
        if (simplified->IsCAnimTreeNode())
        {
            if (x1f8_animRoot != simplified)
                x1f8_animRoot = std::move(simplified);
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
                    x1dc_ = zeus::CVector3f::skZero;
                    x220_28_ = false;
                    x220_26_ = false;
                    break;
                }
                case EUserEventType::AlignTargetRot:
                {
                    x1e8_ = zeus::CQuaternion::skNoRotation;
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
        offset += x1dc_ * time;

    zeus::CQuaternion rot = results.x8_deltas.xc_rotDelta * x1e8_;
    quat = quat * rot;
    x1dc_ = rot.transform(x1dc_);
    time = results.x0_remTime;
}

void CAnimData::SetXRayModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules)
{
}

void CAnimData::SetInfraModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules)
{
}

void CAnimData::PoseSkinnedModel(CSkinnedModel& model, const CPoseAsTransforms& pose,
                                 const CModelFlags& drawFlags,
                                 const rstl::optional_object<CVertexMorphEffect>& morphEffect,
                                 const float* morphMagnitudes)
{
    model.Calculate(pose, drawFlags, morphEffect, morphMagnitudes);
}

void CAnimData::AdvanceParticles(const zeus::CTransform& xf, float,
                                 const zeus::CVector3f&, CStateManager& stateMgr)
{
}

void CAnimData::GetAverageVelocity(int) const
{
}

void CAnimData::ResetPOILists()
{
}

CSegId CAnimData::GetLocatorSegId(const std::string& name) const
{
    return {};
}

zeus::CAABox CAnimData::GetBoundingBox(const zeus::CTransform& xf) const
{
    return {};
}

zeus::CAABox CAnimData::GetBoundingBox() const
{
    return {};
}

}
