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
      x21c_25_loop(loop),
      x220_pose(layout->GetSegIdList().GetList().size()),
      x2f8_poseBuilder(layout)
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
    for (std::pair<u32, CAdditiveAnimPlayback>& additive : x1044_additiveAnims)
        if (additive.second.GetTargetWeight() > 0.00001f)
            additive.second.AddToSegStatementSet(list, *xcc_layoutData.GetObj(), stSet);
}

static void AdvanceAnimationTree(std::weak_ptr<CAnimTreeNode>& anim, const CCharAnimTime& dt)
{
}

void CAnimData::AdvanceAdditiveAnims(float dt)
{
    CCharAnimTime time(dt);

    for (std::pair<u32, CAdditiveAnimPlayback>& additive : x1044_additiveAnims)
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
}

void CAnimData::UpdateAdditiveAnims(float)
{
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

void CAnimData::RecalcPoseBuilder(const CCharAnimTime*) const
{
}

void CAnimData::RenderAuxiliary(const CFrustumPlanes& frustum) const
{
}

void CAnimData::Render(const CSkinnedModel& model, const CModelFlags& drawFlags,
                       const rstl::optional_object<CVertexMorphEffect>& morphEffect,
                       const float* morphMagnitudes) const
{
}

void CAnimData::SetupRender(const CSkinnedModel& model,
                            const rstl::optional_object<CVertexMorphEffect>& morphEffect,
                            const float* morphMagnitudes) const
{
}

void CAnimData::PreRender()
{
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

void CAnimData::DoAdvance(float, bool&, CRandom16&, bool)
{
}

SAdvancementDeltas CAnimData::Advance(float, const zeus::CVector3f&, CStateManager& stateMgr, bool)
{
    return {};
}

SAdvancementDeltas CAnimData::AdvanceIgnoreParticles(float, CRandom16&, bool)
{
    return {};
}

void CAnimData::AdvanceAnim(CCharAnimTime& time, zeus::CVector3f&, zeus::CQuaternion&)
{
}

void CAnimData::SetXRayModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules)
{
}

void CAnimData::SetInfraModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules)
{
}

void CAnimData::PoseSkinnedModel(const CSkinnedModel& model, const CPoseAsTransforms& pose,
                                 const rstl::optional_object<CVertexMorphEffect>& morphEffect,
                                 const float* morphMagnitudes) const
{
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
