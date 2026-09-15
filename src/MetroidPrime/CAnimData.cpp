#include "MetroidPrime/CAnimData.hpp"

#include "Kyoto/Animation/CAnimTreeBlend.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CAllFormatsAnimSource.hpp"
#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeTweenBase.hpp"
#include "Kyoto/Animation/CAnimationManager.hpp"
#include "Kyoto/Animation/CFBStreamedCompression.hpp"
#include "Kyoto/Animation/CPrimitive.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/CSegStatementSet.hpp"
#include "Kyoto/Animation/CTransitionManager.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/Factories/CCharacterFactory.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include <math.h>

#include "Kyoto/Math/CloseEnough.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

typedef rstl::vector< rstl::pair< rstl::string, CAABox > > TAabbList;
typedef rstl::vector< rstl::pair< rstl::string, rstl::vector< CEffectComponent > > > TEffectList;

rstl::reserved_vector< CBoolPOINode, 8 > CAnimData::mBoolPOINodes;
rstl::reserved_vector< CInt32POINode, 16 > CAnimData::mInt32POINodes;
rstl::reserved_vector< CParticlePOINode, 20 > CAnimData::mParticlePOINodes;
rstl::reserved_vector< CSoundPOINode, 20 > CAnimData::mSoundPOINodes;
rstl::reserved_vector< CInt32POINode, 16 > sInt32TransientCache;
static int skPOICacheReferenceCount;
static CInt32POINode* sInt32TransientCacheData;

CAnimData::CAnimData(
    uint selfId, const CCharacterInfo& charInfo, int defaultAnim, int charIdx, const bool loop,
    const TLockedToken< CCharLayoutInfo >& layoutData, const TToken< CSkinnedModel >& modelData,
    const rstl::optional_object< TLockedToken< CSkinnedModelWithAvgNormals > >& iceModelData,
    const rstl::ncrc_ptr< CAnimSysContext >& animCtx,
    const rstl::rc_ptr< CAnimationManager >& animMgr,
    const rstl::rc_ptr< CTransitionManager >& transMgr,
    const TLockedToken< CCharacterFactory >& charFactory)
: x0_charFactory(charFactory)
, xc_charInfo(charInfo)
, xcc_layoutData(layoutData)
, xd8_modelData(modelData)
, xe4_iceModelData(iceModelData)
, xf4_xrayModel(nullptr)
, xf8_infraModel(nullptr)
, xfc_animCtx(animCtx)
, x100_animMgr(animMgr)
, x104_animDir(kAD_Forward)
, x108_aabb(CAABox::MakeMaxInvertedBox())
, x120_particleDB()
, x1d8_selfId(selfId)
, x1dc_alignPos(CVector3f::Zero())
, x1e8_alignRot(CQuaternion::NoRotation())
, x1f8_animRoot()
, x1fc_transMgr(transMgr)
, x200_speedScale(1.f)
, x204_charIdx(charIdx)
, x208_currentAnim(defaultAnim)
, x20c_passedBoolCount(0)
, x210_passedIntCount(0)
, x214_passedParticleCount(0)
, x218_passedSoundCount(0)
, x21c_particleLightIdx(0)
, x220_24_animating(false)
, x220_25_loop(loop)
, x220_26_aligningPos(false)
, x220_27_(false)
, x220_28_(false)
, x220_29_animationJustStarted(false)
, x220_30_poseBuilt(false)
, x220_31_poseCached(false)
, x224_pose(static_cast< uchar >(layoutData->GetBodyPartSegIds().size()))
, x2fc_poseBuilder(CLayoutDescription(layoutData))
, x40c_playbackParms(-1, -1, 1.f, true)
, x434_additiveAnims() {
  if (skPOICacheReferenceCount == 0) {
    mBoolPOINodes.resize(8);
    mInt32POINodes.resize(16);
    mParticlePOINodes.resize(20);
    mSoundPOINodes.resize(20);
  }
  ++skPOICacheReferenceCount;

  xd8_modelData->CalculateDefault();
  const CVector3f* pointItr =
      reinterpret_cast< const CVector3f* >(xd8_modelData->GetModel()->GetPositions());
  for (int i = 0; i < xd8_modelData->GetNumPoints(); ++i) {
    x108_aabb.AccumulateBounds(pointItr[i]);
  }

  x120_particleDB.CacheParticleDesc(charInfo.GetParticleResData());

  CLayoutDescription layoutDesc(xcc_layoutData);
  CHierarchyPoseBuilder pb(layoutDesc);
  pb.BuildNoScale(x224_pose);
  x220_30_poseBuilt = true;

  int initialAnim = defaultAnim;
  if (initialAnim == -1) {
    initialAnim = 0;
    rstl::string warning =
        rstl::string_l("Character ") + charInfo.GetCharacterName() +
        rstl::string_l(" has invalid initial animation, so defaulting to first.\n");
  }

  const uint animRes = charInfo.GetAnimationIndexList()[initialAnim];
  x1f8_animRoot =
      GetAnimationManager()->GetAnimationTree(animRes, CMetaAnimTreeBuildOrders::NoSpecialOrders());
}

CAnimData::~CAnimData() {
  if (--skPOICacheReferenceCount == 0) {
    mBoolPOINodes.clear();
    mInt32POINodes.clear();
    mParticlePOINodes.clear();
    mSoundPOINodes.clear();
  }
}

CAABox CAnimData::GetBoundingBox() const {
  const rstl::vector< rstl::pair< rstl::string, CAABox > >& aabbList =
      xc_charInfo.GetAnimBBoxList();

  if (aabbList.size() > 0) {
    CAnimTreeEffectiveContribution contrib = x1f8_animRoot->GetContributionOfHighestInfluence();
    rstl::string name = contrib.GetPrimitiveName();

    rstl::vector< rstl::pair< rstl::string, CAABox > >::const_iterator search =
        rstl::find_by_key(aabbList, rstl::string(name));
    if (search != aabbList.end()) {
      return search->second;
    }
  }
  return x108_aabb;
}

CAABox CAnimData::GetBoundingBox(const CTransform4f& xf) const {
  return GetBoundingBox().GetTransformedAABox(xf);
}

CSegId CAnimData::GetLocatorSegId(const rstl::string& name) const {
  return xcc_layoutData->GetSegIdFromString(name);
}

void CAnimData::ResetPOILists() {
  x20c_passedBoolCount = 0;
  x210_passedIntCount = 0;
  x214_passedParticleCount = 0;
  x218_passedSoundCount = 0;
}

float CAnimData::GetAverageVelocity(int animIn) const {
  const uint animRes = xc_charInfo.GetAnimationIndexList()[animIn];
  rstl::rc_ptr< IMetaAnim > anim = x100_animMgr->GetMetaAnimation(animRes);

  rstl::set< CPrimitive > primitiveSet;
  anim->GetUniquePrimitives(primitiveSet);

  float ret;
  float weightedVel = 0.f;
  float totalDur = 0.f;
  rstl::set< CPrimitive >::const_iterator it = primitiveSet.begin();
  rstl::set< CPrimitive >::const_iterator end = primitiveSet.end();
  while (it != end) {
    const SObjectTag animTag('ANIM', it->GetAnimResId());

    TLockedToken< CAllFormatsAnimSource > animData = xfc_animCtx->GetSimplePool().GetObj(animTag);

    weightedVel += animData->GetAverageVelocity() * animData->GetAnimationDuration().GetSeconds();
    totalDur += animData->GetAnimationDuration().GetSeconds();
    ++it;
  }

  ret = 0.f;
  if (totalDur > 0.f) {
    ret = weightedVel / totalDur;
  }

  return ret;
}

void CAnimData::AdvanceParticles(const CTransform4f& xf, float dt, const CVector3f& scale,
                                 CStateManager& mgr) {
  x120_particleDB.Update(dt, x224_pose, **xcc_layoutData, xf, scale, mgr);
}

void CAnimData::PoseSkinnedModel(const CSkinnedModel& model, const CPoseAsTransforms& pose,
                                 const rstl::optional_object< CVertexMorphEffect >& morphEffect,
                                 const float* avgNormals) const {
  const_cast< CSkinnedModel& >(model).Calculate(pose, morphEffect, avgNormals, nullptr);
}

void CAnimData::DrawSkinnedModel(const CSkinnedModel& model, const CModelFlags& flags) const {
  GXLightID light = static_cast< GXLightID >(CGraphics::GetLightMask());

  GXAttnFn attnFn = GX_AF_NONE;
  if (static_cast< uint >(light) != 0) {
    attnFn = GX_AF_SPOT;
  }

  GXDiffuseFn diffFn = GX_DF_NONE;
  if (static_cast< uint >(light) != 0) {
    diffFn = GX_DF_CLAMP;
  }

  CGX::SetChanCtrl(CGX::Channel0, static_cast< uint >(light) != 0, GX_SRC_REG, GX_SRC_REG, light,
                   diffFn, attnFn);
  model.Draw(flags);
}

void CAnimData::InitializeCache() {
  sInt32TransientCache.clear();
  sInt32TransientCache.resize(16);
  sInt32TransientCacheData = sInt32TransientCache.data();
}

void CAnimData::FreeCache() {
  sInt32TransientCache.clear();
  sInt32TransientCacheData = nullptr;
}

void CAnimData::SubstituteModelData(const TLockedToken< CSkinnedModel >& model) {
  xd8_modelData = model;

  xd8_modelData->CalculateDefault();
  x108_aabb = CAABox::MakeMaxInvertedBox();

  const CVector3f* pointItr =
      reinterpret_cast< const CVector3f* >(xd8_modelData->GetModel()->GetPositions());
  for (int i = 0; i < xd8_modelData->GetNumPoints(); ++i) {
    x108_aabb.AccumulateBounds(pointItr[i]);
  }
}

void CAnimData::SetInfraModel(const TLockedToken< CModel >& model,
                              const TLockedToken< CSkinRules >& skinRules) {
  CSkinnedModel* skinnedModel = rs_new CSkinnedModel(
      model, skinRules, xd8_modelData->GetLayoutInfo(), CSkinnedModel::kDO_Owned);
  skinnedModel->CalculateDefault();
  xf8_infraModel = rstl::rc_ptr< CSkinnedModel >(skinnedModel);
}

void CAnimData::SetXRayModel(const TLockedToken< CModel >& model,
                             const TLockedToken< CSkinRules >& skinRules) {
  CSkinnedModel* skinnedModel = rs_new CSkinnedModel(
      model, skinRules, xd8_modelData->GetLayoutInfo(), CSkinnedModel::kDO_Owned);
  skinnedModel->CalculateDefault();
  xf4_xrayModel = rstl::rc_ptr< CSkinnedModel >(skinnedModel);
}

void CAnimData::AdvanceAnim(CCharAnimTime& time, CVector3f& offset, CQuaternion& rotation) {
  const float dt = time.GetSeconds();
  CAdvancementResults results(CCharAnimTime(0.f), CAdvancementDeltas());
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified;

  if (x104_animDir == kAD_Forward) {
    results = x1f8_animRoot->VAdvanceView(time);
    simplified = x1f8_animRoot->Simplified();
  }

  if (simplified.valid()) {
    x1f8_animRoot = Cast(simplified.data());
  }

  if (x220_28_ || x220_27_) {
    const int count = x210_passedIntCount;
    const CInt32POINode* node = mInt32POINodes.data();
    if (count > 0) {
      for (int i = 0; i < count; ++i, ++node) {
        if (node->GetPoiType() == kPT_UserEvent) {
          switch (node->GetValue()) {
          case kUE_AlignTargetPosStart:
            x220_26_aligningPos = true;
            break;
          case kUE_AlignTargetPos:
            x1dc_alignPos = CVector3f::Zero();
            x220_28_ = false;
            x220_26_aligningPos = false;
            break;
          case kUE_AlignTargetRot:
            x1e8_alignRot = CQuaternion::NoRotation();
            x220_27_ = false;
            break;
          }
        }
      }
    }
  }
  const CAdvancementDeltas deltas = results.GetAdvancementDeltas();
  const CVector3f& deltaPos = deltas.GetOffsetDelta();
  const CQuaternion& deltaRot = deltas.GetOrientationDelta();

  offset += deltaPos;
  if (x220_26_aligningPos) {
    offset += x1dc_alignPos * dt;
  }

  CQuaternion alignRot = deltaRot * x1e8_alignRot;
  rotation *= alignRot;
  x1dc_alignPos = alignRot.BuildInverted().Transform(x1dc_alignPos);

  time = results.x0_remTime;
}

CAdvancementDeltas CAnimData::AdvanceIgnoreParticles(float dt, CRandom16& random, bool advTree) {
  bool suspendParticles;
  return DoAdvance(dt, suspendParticles, random, advTree);
}

CAdvancementDeltas CAnimData::Advance(float dt, const CVector3f& scale, CStateManager& mgr,
                                      TAreaId aid, bool advTree) {
  bool suspendParticles;
  CAdvancementDeltas deltas = DoAdvance(dt, suspendParticles, *mgr.Random(), advTree);

  if (suspendParticles) {
    x120_particleDB.SuspendAllActiveEffects(mgr);
  }

  const int passedParticleCount = x214_passedParticleCount;
  for (int i = 0; i < passedParticleCount; ++i) {
    const CParticlePOINode& node = mParticlePOINodes[i];
    const int charIdx = node.GetCharacterIndex();
    if (charIdx == -1 || charIdx == x204_charIdx) {
      x120_particleDB.AddParticleEffect(node.GetString(), node.GetFlags(), node.GetParticleData(),
                                        scale, mgr, aid, false, x21c_particleLightIdx);
    }
  }

  return deltas;
}

CAdvancementDeltas CAnimData::DoAdvance(float dt, bool& suspendParticles, CRandom16& random,
                                        bool advTree) {
  suspendParticles = false;

  CVector3f offset(0.f, 0.f, 0.f);
  CQuaternion rotation(CQuaternion::NoRotation());

  const float scaledDt = dt * x200_speedScale;
  CVector3f additiveOffset(0.f, 0.f, 0.f);
  CQuaternion additiveRotation(CQuaternion::NoRotation());

  ResetPOILists();

  if (x434_additiveAnims.size() > 0) {
    const CAdvancementDeltas additiveDeltas = UpdateAdditiveAnims(scaledDt);
    additiveOffset = additiveDeltas.GetOffsetDelta();
    additiveRotation = additiveDeltas.GetOrientationDelta();
    x220_31_poseCached = false;
    x220_30_poseBuilt = false;
  }

  const bool animating = IsAnimating() == true;
  if (!animating) {
    suspendParticles = true;
    return CAdvancementDeltas(offset, rotation);
  }

  if (x220_29_animationJustStarted) {
    x220_29_animationJustStarted = false;
    suspendParticles = true;
  }

  if (advTree) {
    SetRandomPlaybackRate(random);

    CCharAnimTime time(scaledDt);

    if (x220_25_loop) {
      while (time.GreaterThanZero() && !close_enough(time.GetSeconds(), 0.f)) {
        x210_passedIntCount +=
            x1f8_animRoot->GetInt32POIList(time, mInt32POINodes.data(), 16, x210_passedIntCount, 0);
        x20c_passedBoolCount +=
            x1f8_animRoot->GetBoolPOIList(time, mBoolPOINodes.data(), 8, x20c_passedBoolCount, 0);
        x214_passedParticleCount += x1f8_animRoot->GetParticlePOIList(
            time, mParticlePOINodes.data(), 20, x214_passedParticleCount, 0);
        x218_passedSoundCount += x1f8_animRoot->GetSoundPOIList(time, mSoundPOINodes.data(), 20,
                                                                x218_passedSoundCount, 0);

        AdvanceAnim(time, offset, rotation);
      }
    } else {
      CCharAnimTime remTime = x1f8_animRoot->VGetTimeRemaining();

      while (!close_enough(remTime.GetSeconds(), 0.f) && !close_enough(time.GetSeconds(), 0.f)) {
        x210_passedIntCount +=
            x1f8_animRoot->GetInt32POIList(time, mInt32POINodes.data(), 16, x210_passedIntCount, 0);
        x20c_passedBoolCount +=
            x1f8_animRoot->GetBoolPOIList(time, mBoolPOINodes.data(), 8, x20c_passedBoolCount, 0);
        x214_passedParticleCount += x1f8_animRoot->GetParticlePOIList(
            time, mParticlePOINodes.data(), 20, x214_passedParticleCount, 0);
        x218_passedSoundCount += x1f8_animRoot->GetSoundPOIList(time, mSoundPOINodes.data(), 20,
                                                                x218_passedSoundCount, 0);

        AdvanceAnim(time, offset, rotation);

        remTime = x1f8_animRoot->VGetTimeRemaining();
        time = CCharAnimTime(
            rstl::max_val(0.f, rstl::min_val(time.GetSeconds(), remTime.GetSeconds())));

        if (close_enough(remTime.GetSeconds(), 0.f)) {
          x220_24_animating = false;
          x1dc_alignPos = CVector3f::Zero();
          x220_28_ = false;
          x220_26_aligningPos = false;
        }
      }
    }

    x220_31_poseCached = false;
    x220_30_poseBuilt = false;
  }

  return CAdvancementDeltas(offset + additiveOffset, rotation * additiveRotation);
}

void CAnimData::SetAnimation(const CAnimPlaybackParms& parms, bool noTrans) {
  if (parms.GetAnimationId() == x40c_playbackParms.GetAnimationId() ||
      (parms.GetSecondAnimationId() == x40c_playbackParms.GetSecondAnimationId() &&
       parms.GetSecondAnimationId() != -1) ||
      (parms.GetBlendFactor() == x40c_playbackParms.GetBlendFactor() &&
       parms.GetBlendFactor() != 1.f)) {
    if (x220_29_animationJustStarted)
      return;
  }

  x40c_playbackParms.SetAnimationId(parms.GetAnimationId());
  x40c_playbackParms.SetSecondAnimationId(parms.GetSecondAnimationId());
  x40c_playbackParms.SetBlendFactor(parms.GetBlendFactor());

  const int animA = parms.GetAnimationId();
  const bool animating = parms.GetIsPlayAnimation();
  x200_speedScale = 1.f;
  x208_currentAnim = animA;

  const int animB = parms.GetSecondAnimationId();
  const float blendFactor = parms.GetBlendFactor();
  const uint animResA = xc_charInfo.GetAnimationIndexList()[animA];
  ResetPOILists();

  rstl::ncrc_ptr< CAnimTreeNode > newAnimTree;

  if (animB != -1) {
    const uint animResB = xc_charInfo.GetAnimationIndexList()[animB];

    rstl::ncrc_ptr< CAnimTreeNode > treeA(GetAnimationManager()->GetAnimationTree(
        animResA, CMetaAnimTreeBuildOrders::NoSpecialOrders()));
    rstl::ncrc_ptr< CAnimTreeNode > treeB(GetAnimationManager()->GetAnimationTree(
        animResB, CMetaAnimTreeBuildOrders::NoSpecialOrders()));

    newAnimTree = rstl::ncrc_ptr< CAnimTreeNode >(
        rs_new CAnimTreeBlend(false, treeA, treeB, blendFactor,
                              CAnimTreeBlend::CreatePrimitiveName(treeA, treeB, blendFactor)));
  } else {
    newAnimTree = GetAnimationManager()->GetAnimationTree(
        animResA, CMetaAnimTreeBuildOrders::NoSpecialOrders());
  }

  if (!noTrans) {
    x1f8_animRoot = x1fc_transMgr->GetTransitionTree(x1f8_animRoot, newAnimTree);
  } else {
    x1f8_animRoot = newAnimTree;
  }

  x220_24_animating = animating;
  CalcPlaybackAlignmentParms(parms, newAnimTree);
  ResetPOILists();
  x220_29_animationJustStarted = true;
}

void CAnimData::GetAnimationPrimitives(const CAnimPlaybackParms& parms,
                                       rstl::set< CPrimitive >& primsOut) const {
  const int animB = parms.GetSecondAnimationId();

  const uint animResA = xc_charInfo.GetAnimationIndexList()[parms.GetAnimationId()];
  GetAnimationManager()->GetMetaAnimation(animResA)->GetUniquePrimitives(primsOut);

  if (animB != -1) {
    const uint animResB = xc_charInfo.GetAnimationIndexList()[animB];
    GetAnimationManager()->GetMetaAnimation(animResB)->GetUniquePrimitives(primsOut);
  }
}

void CAnimData::PrimitiveSetToTokenVector(const rstl::set< CPrimitive >& primSet,
                                          rstl::vector< CToken >& tokensOut, bool preLock) {
  tokensOut = rstl::vector< CToken >();
  tokensOut.reserve(primSet.size());

  rstl::set< CPrimitive >::const_iterator it = primSet.begin();
  rstl::set< CPrimitive >::const_iterator end = primSet.end();
  for (; it != end; ++it) {
    CToken token = gpSimplePool->GetObj(SObjectTag('ANIM', it->GetAnimResId()));
    if (preLock) {
      token.Lock();
    }
    tokensOut.push_back(token);
  }
}

void CAnimData::BuildPose() {
  if (!x220_31_poseCached) {
    RecalcPoseBuilder(nullptr);
    x220_31_poseCached = true;
    x220_30_poseBuilt = false;
  }

  if (!x220_30_poseBuilt) {
    x2fc_poseBuilder.BuildNoScale(x224_pose);
    x220_30_poseBuilt = true;
  }
}

void CAnimData::PreRender() {
  if (!x220_31_poseCached) {
    RecalcPoseBuilder(nullptr);
    x220_31_poseCached = true;
    x220_30_poseBuilt = false;
  }
}

void CAnimData::SetupRender(const CSkinnedModel& model,
                            const rstl::optional_object< CVertexMorphEffect >& morphEffect,
                            const float* avgNormals) const {
  if (!x220_30_poseBuilt) {
    CAnimData* self = const_cast< CAnimData* >(this);
    self->x2fc_poseBuilder.BuildNoScale(self->x224_pose);
    self->x220_30_poseBuilt = true;
  }

  PoseSkinnedModel(model, x224_pose, morphEffect, avgNormals);
}

void CAnimData::Render(const CSkinnedModel& model, const CModelFlags& flags,
                       const rstl::optional_object< CVertexMorphEffect >& morphEffect,
                       const float* avgNormals) const {
  SetupRender(model, morphEffect, avgNormals);
  DrawSkinnedModel(model, flags);
}

void CAnimData::RenderAuxiliary(const CFrustumPlanes& frustum) const {
  x120_particleDB.AddToRendererClipped(frustum);
}

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
void CHierarchyPoseBuilder::Insert(const CSegId& id, const CQuaternion& rot) {
  x38_treeMap[id].SetRotation(rot);
}

void CHierarchyPoseBuilder::Insert(const CSegId& id, const CVector3f& off) {
  x38_treeMap[id].SetOffset(off);
}
#endif

void CAnimData::RecalcPoseBuilder(const CCharAnimTime* time) const {
  const CSegIdList* segIdList = &xcc_layoutData->GetBodyPartSegIds();

  CStackSegStatementSet statementSet;
  CHierarchyPoseBuilder& poseBuilder = x2fc_poseBuilder;

  if (time == nullptr) {
    x1f8_animRoot->VGetSegStatementSet(*segIdList, statementSet);
  } else {
    x1f8_animRoot->VGetSegStatementSet(*segIdList, statementSet, *time);
  }

  AddAdditiveSegData(*segIdList, statementSet);

  int i = 0;
  const int segCount = segIdList->size();
  while (i < segCount) {
    const CSegId& seg = (*segIdList)[i];
    if (seg.val() != 3) {
      poseBuilder.Insert(seg, statementSet.GetData(seg).Orientation());
      if (statementSet.GetData(seg).OffsetValid()) {
        poseBuilder.Insert(seg, statementSet.GetData(seg).Offset());
      }
    }
    ++i;
  }
}

rstl::rc_ptr< CAnimationManager > CAnimData::GetAnimationManager() const { return x100_animMgr; }

float CAnimData::GetAnimationDuration(int animIn) const {
  const uint animRes = xc_charInfo.GetAnimationIndexList()[animIn];
  rstl::rc_ptr< IMetaAnim > anim = GetAnimationManager()->GetMetaAnimation(animRes);

  rstl::set< CPrimitive > primitiveSet;
  anim->GetUniquePrimitives(primitiveSet);

  float duration = 0.f;
  rstl::set< CPrimitive >::const_iterator it = primitiveSet.begin();
  rstl::set< CPrimitive >::const_iterator end = primitiveSet.end();
  while (it != end) {
    const SObjectTag animTag('ANIM', it->GetAnimResId());

    TLockedToken< CAllFormatsAnimSource > animData =
        GetAnimSysContext()->GetSimplePool().GetObj(animTag);

    duration += animData->GetAnimationDuration().GetSeconds();
    ++it;
  }

  if (anim->GetType() == kMAT_Random) {
    duration /= primitiveSet.size();
  }

  return duration;
}

inline rstl::ncrc_ptr< CAnimSysContext > CAnimData::GetAnimSysContext() const { return xfc_animCtx; }

float CAnimData::GetAnimTimeRemaining(const rstl::string&) const {
  float remTime = x1f8_animRoot->VGetTimeRemaining().GetSeconds();
  if (x200_speedScale > 0.f) {
    remTime /= x200_speedScale;
  }
  return remTime;
}

bool CAnimData::IsAnimTimeRemaining(float rem, const rstl::string&) const {
  if (x1f8_animRoot.GetPtr() != 0) {
    const float remTime = x1f8_animRoot->VGetTimeRemaining().GetSeconds();
    return !close_enough(remTime, 0.f, rem);
  }

  return false;
}

CTransform4f CAnimData::GetLocatorTransform(const rstl::string& name,
                                            const CCharAnimTime* time) const {
  CSegId seg = xcc_layoutData->GetSegIdFromString(name);
  CSegId segCopy = seg;
  return GetLocatorTransform(segCopy, time);
}

CTransform4f CAnimData::GetLocatorTransform(CSegId seg, const CCharAnimTime* time) const {
  CTransform4f xf = CTransform4f::Identity();

  if (seg.val() != 0xFF) {
    if (time != nullptr || !x220_31_poseCached) {
      CAnimData* self = const_cast< CAnimData* >(this);
      self->RecalcPoseBuilder(time);
      self->x220_31_poseCached = !time;
    }

    if (!x220_30_poseBuilt) {
      x2fc_poseBuilder.BuildTransform(seg, xf);
    } else {
      const CMatrix3f& rot = x224_pose.GetTransformMinusOffset(seg);
      const CVector3f& offset = x224_pose.GetOffset(seg);
      xf.SetRotation(rot);
      xf.SetTranslation(offset);
    }
  }

  return xf;
}

CMatrix3f CMatrix3f::Inverse() const {
  const float detScale = 1.f / Determinant();
  return CMatrix3f((m11 * m22 - m12 * m21) * detScale, (-(m01 * m22 - m02 * m21)) * detScale,
                   (m01 * m12 - m02 * m11) * detScale, (-(m10 * m22 - m12 * m20)) * detScale,
                   (m00 * m22 - m02 * m20) * detScale, (-(m00 * m12 - m02 * m10)) * detScale,
                   (m10 * m21 - m11 * m20) * detScale, (-(m00 * m21 - m01 * m20)) * detScale,
                   (m00 * m11 - m01 * m10) * detScale);
}

void CAnimData::CalcPlaybackAlignmentParms(const CAnimPlaybackParms& parms,
                                           const rstl::ncrc_ptr< CAnimTreeNode >& node) {
  const CQuaternion* deltaOrient = parms.GetDeltaOrient();
  const CTransform4f* objectXf = parms.GetObjectXform();

  CQuaternion alignRot = CQuaternion::NoRotation();
  x1e8_alignRot = alignRot;
  x220_27_ = false;

  if (deltaOrient != nullptr && objectXf != nullptr) {
    ResetPOILists();
    x210_passedIntCount += node->GetInt32POIList(CCharAnimTime::Infinity(), mInt32POINodes.data(),
                                                 16, x210_passedIntCount, 64);

    const int count = x210_passedIntCount;
    if (count > 0) {
      for (int i = 0; i < count; ++i) {
        const CInt32POINode* poi = &mInt32POINodes[i];
        if (poi->GetPoiType() == kPT_UserEvent && poi->GetValue() == kUE_AlignTargetRot) {
          const CCharAnimTime& poiTime = poi->GetTime();
          const CAdvancementResults adv =
              node->GetAdvancementResults(poiTime, CCharAnimTime::ZeroFlat());
          const CMatrix3f invObjRot = objectXf->BuildMatrix3f().Inverse();
          const CQuaternion targetRot = (*deltaOrient) * CQuaternion::FromMatrix(invObjRot);
          const CQuaternion fullRot =
              targetRot * adv.GetAdvancementDeltas().GetOrientationDelta().BuildInverted();

          alignRot = CQuaternion::Slerp(CQuaternion::NoRotation(), fullRot,
                                        1.f / (60.f * poiTime.GetSeconds()));
          x1e8_alignRot = alignRot;
          x220_27_ = true;
          break;
        }
      }
    }
  }

  if (!x220_27_) {
    const CVector3f* targetPos = parms.GetTargetPos();
    bool foundStart = false;
    bool foundAlign = false;
    CVector3f startPos = CVector3f::Zero();
    CVector3f alignPos = CVector3f::Zero();
    CCharAnimTime startTime = CCharAnimTime::ZeroPlus();
    CCharAnimTime alignTime = CCharAnimTime::ZeroPlus();

    if (targetPos != nullptr && objectXf != nullptr) {
      ResetPOILists();
      x210_passedIntCount += node->GetInt32POIList(CCharAnimTime::Infinity(), mInt32POINodes.data(),
                                                   16, x210_passedIntCount, 64);

      const int count = x210_passedIntCount;
      if (count > 0) {
        for (int i = 0; i < count; ++i) {
          const CInt32POINode* poi = &mInt32POINodes[i];
          if (poi->GetPoiType() == kPT_UserEvent) {
            const rstl::string& locator = poi->GetLocatorName();
            if (poi->GetValue() == kUE_AlignTargetPosStart) {
              startTime = poi->GetTime();
              foundStart = true;

              const CAdvancementResults adv =
                  node->GetAdvancementResults(startTime, CCharAnimTime::ZeroFlat());
              startPos = adv.GetAdvancementDeltas().GetOffsetDelta();

              if (parms.GetIsUseLocator()) {
                const CTransform4f xf = GetLocatorTransform(locator, &startTime);
                startPos += xf.GetTranslation();
              }

              if (foundAlign) {
                break;
              }
            } else if (poi->GetValue() == kUE_AlignTargetPos) {
              alignTime = poi->GetTime();
              foundAlign = true;

              const CAdvancementResults adv =
                  node->GetAdvancementResults(alignTime, CCharAnimTime::ZeroFlat());
              alignPos = adv.GetAdvancementDeltas().GetOffsetDelta();

              if (parms.GetIsUseLocator()) {
                const CTransform4f xf = GetLocatorTransform(locator, &alignTime);
                alignPos += xf.GetTranslation();
              }

              if (foundStart) {
                break;
              }
            }
          }
        }

        if (foundStart && foundAlign) {
          const CVector3f* const objectScale = parms.GetObjectScale();

          const CVector3f scaleStart = CVector3f::ByElementMultiply(*objectScale, startPos);
          const CVector3f scaleAlign = CVector3f::ByElementMultiply(*objectScale, alignPos);
          const CVector3f delta =
              objectXf->GetInverse() * (*targetPos) - scaleStart - (scaleAlign - scaleStart);
          CVector3f normalized = delta;
          normalized[kDX] /= (*objectScale)[kDX];
          normalized[kDY] /= (*objectScale)[kDY];
          normalized[kDZ] /= (*objectScale)[kDZ];

          const float timeScale = 1.f / (alignTime.GetSeconds() - startTime.GetSeconds());
          normalized *= timeScale;
          x1dc_alignPos = normalized;
          x220_28_ = true;
          x220_26_aligningPos = false;
        } else {
          x1dc_alignPos = CVector3f::Zero();
          x220_28_ = false;
          x220_26_aligningPos = false;
        }
      }
    } else {
      x1dc_alignPos = CVector3f::Zero();
      x220_28_ = false;
      x220_26_aligningPos = false;
    }
  } else {
    const CVector3f* targetPos = parms.GetTargetPos();
    bool foundStart = false;
    bool foundAlign = false;
    CVector3f startPos = CVector3f::Zero();
    CCharAnimTime startTime = CCharAnimTime::ZeroPlus();
    CCharAnimTime alignTime = CCharAnimTime::ZeroPlus();

    if (targetPos != nullptr && objectXf != nullptr) {
      ResetPOILists();
      x210_passedIntCount += node->GetInt32POIList(CCharAnimTime::Infinity(), mInt32POINodes.data(),
                                                   16, x210_passedIntCount, 64);

      const int count = x210_passedIntCount;
      if (count > 0) {
        for (int i = 0; i < count; ++i) {
          const CInt32POINode* poi = &mInt32POINodes[i];
          if (poi->GetPoiType() == kPT_UserEvent) {
            if (poi->GetValue() == kUE_AlignTargetPosStart) {
              startTime = poi->GetTime();
              foundStart = true;
              if (foundAlign) {
                break;
              }
            } else if (poi->GetValue() == kUE_AlignTargetPos) {
              alignTime = poi->GetTime();
              foundAlign = true;
              if (foundStart) {
                break;
              }
            }
          }
        }

        if (foundStart && foundAlign) {
          alignRot = CQuaternion::NoRotation();
          x1e8_alignRot = alignRot;
          x220_27_ = true;

          foundStart = false;
          CCharAnimTime time = CCharAnimTime::ZeroFlat();
          CVector3f alignPos = CVector3f::Zero();
          const CCharAnimTime frameDt(1.f / 60.f);
          CQuaternion curRot = CQuaternion::NoRotation();

          while (time < alignTime) {
            const CAdvancementResults adv = node->GetAdvancementResults(frameDt, time);
            alignPos += curRot.BuildTransform() * adv.GetAdvancementDeltas().GetOffsetDelta();
            curRot *= adv.GetAdvancementDeltas().GetOrientationDelta() * alignRot;

            if (!foundStart && time >= startTime) {
              foundStart = true;
              startPos = alignPos;
            }

            time += frameDt;
          }

          const CVector3f* const objectScale = parms.GetObjectScale();

          const CVector3f scaleStart = CVector3f::ByElementMultiply(*objectScale, startPos);
          const CVector3f scaleAlign = CVector3f::ByElementMultiply(*objectScale, alignPos);
          const CVector3f delta =
              objectXf->GetInverse() * (*targetPos) - scaleStart - (scaleAlign - scaleStart);
          CVector3f normalized = delta;
          normalized[kDX] /= (*objectScale)[kDX];
          normalized[kDY] /= (*objectScale)[kDY];
          normalized[kDZ] /= (*objectScale)[kDZ];

          const float timeScale = 1.f / (alignTime.GetSeconds() - startTime.GetSeconds());
          normalized *= timeScale;
          x1dc_alignPos = normalized;
          x220_28_ = true;
          x220_26_aligningPos = false;
        } else {
          x1dc_alignPos = CVector3f::Zero();
          x220_28_ = false;
          x220_26_aligningPos = false;
        }
      }
    } else {
      x1dc_alignPos = CVector3f::Zero();
      x220_28_ = false;
      x220_26_aligningPos = false;
    }
  }
}

void CAnimData::SetRandomPlaybackRate(CRandom16& random) {
  for (int i = 0; i < x210_passedIntCount; ++i) {
    const CInt32POINode& poi = mInt32POINodes[i];
    if (poi.GetPoiType() == kPT_RandRate) {
      const float scale = static_cast< float >(random.Next() % poi.GetValue()) / 100.f;
      if ((random.Next() % 100) < 50) {
        x200_speedScale = 1.f + scale;
      } else {
        x200_speedScale = 1.f - scale;
      }
      break;
    }
  }
}

void CAnimData::SetPlaybackRate(float set) { x200_speedScale = set; }

void CAnimData::MultiplyPlaybackRate(float scale) { x200_speedScale *= scale; }

CCharAnimTime CAnimData::GetTimeOfUserEvent(EUserEventType type, const CCharAnimTime& time) const {
  const int count = x1f8_animRoot->GetInt32POIList(time, sInt32TransientCacheData, 16, 0, 64);
  for (int i = 0; i < count; ++i) {
    CInt32POINode& poi = sInt32TransientCacheData[i];
    if (poi.GetPoiType() == kPT_UserEvent) {
      const int value = poi.GetValue();
      if (value == static_cast< int >(type)) {
        CCharAnimTime ret = poi.GetTime();
        for (int j = i; j < count; ++j) {
          sInt32TransientCacheData[j] =
              CInt32POINode(rstl::string_l(""), kPT_EmptyInt32, CCharAnimTime(0.f), -1, false, 1.f,
                            -1, 0, 0, rstl::string_l("root"));
        }
        return ret;
      }
    }
    sInt32TransientCacheData[i] =
        CInt32POINode(rstl::string_l(""), kPT_EmptyInt32, CCharAnimTime(0.f), -1, false, 1.f, -1, 0,
                      0, rstl::string_l("root"));
  }
  return CCharAnimTime::Infinity();
}

void CAnimData::Touch(const CSkinnedModel& model, int shaderIdx) const {
  model.GetModel()->Touch(shaderIdx);
}

void CAnimData::InitializeEffects(CStateManager& mgr, TAreaId areaId, const CVector3f& scale) {
  const TEffectList& effects = xc_charInfo.GetEffectList();
  const uint effectCount = effects.size();
  for (uint i = 0; i < effectCount; ++i) {
    TEffectList::value_type effect = effects[i];
    const uint componentCount = effect.second.size();
    for (uint j = 0; j < componentCount; ++j) {
      const CEffectComponent& component = effect.second[j];
      x120_particleDB.CacheParticleDesc(component.GetParticleTag());
      {
        const CParticleData data(0, component.GetParticleTag(), component.GetSegmentName(),
                                 component.GetScale(), component.GetParentedMode());
        x120_particleDB.AddParticleEffect(component.GetComponentName(), component.GetFlags(), data,
                                          scale, mgr, areaId, true, x21c_particleLightIdx);
      }
      x120_particleDB.SetParticleEffectState(component.GetComponentName(), false, mgr);
    }
  }
}

void CAnimData::SetParticleEffectState(const rstl::string& name, bool active, CStateManager& mgr) {
  rstl::vector< rstl::pair< rstl::string, rstl::vector< CEffectComponent > > > effects =
      xc_charInfo.GetEffectList();

  rstl::vector< rstl::pair< rstl::string, rstl::vector< CEffectComponent > > >::const_iterator it =
      rstl::find_by_key(effects, name);
  if (it != effects.end()) {
    const rstl::vector< CEffectComponent >& components = it->second;
    rstl::vector< CEffectComponent >::const_iterator compIt = components.begin();
    rstl::vector< CEffectComponent >::const_iterator compEnd = components.end();
    for (; compIt != compEnd; ++compIt) {
      x120_particleDB.SetParticleEffectState(compIt->GetComponentName(), active, mgr);
    }
  }
}

void CAnimData::SetParticleCEXTValue(const rstl::string& name, int index, float value) {
  TEffectList effects = xc_charInfo.GetEffectList();
  AUTO(it, rstl::find_by_key(effects, name));
  if (it != effects.end()) {
    const rstl::vector< CEffectComponent >& components = it->second;
    if (components.begin() != components.end()) {
      x120_particleDB.SetExternalVarValue(components.front().GetComponentName(), index, value);
    }
  }
}

void CAnimData::SetPhase(float phase) { x1f8_animRoot->VSetPhase(phase); }

rstl::rc_ptr< CAnimationManager > CAnimData::GetAnimationManager() { return x100_animMgr; }

void CAnimData::AddAdditiveAnimation(uint idx, float weight, bool active, bool fadeOut) {
  const uint animIdx = xc_charInfo.GetAnimationIndexList()[idx];
  rstl::pair< uint, CAdditiveAnimPlayback >* end = x434_additiveAnims.end();
  rstl::pair< uint, CAdditiveAnimPlayback >* search = x434_additiveAnims.begin();

  while (search != end) {
    if (animIdx == search->first) {
      break;
    }
    ++search;
  }

  if (search != end) {
    search->second.SetLoop(active);
    CAdditiveAnimPlayback& playback = search->second;
    playback.SetWeight(weight);
    playback.SetFadeOutWhenAnimOver(!playback.IsLoop() && fadeOut);
  } else {
    rstl::ncrc_ptr< CAnimTreeNode > animTree(GetAnimationManager()->GetAnimationTree(
        animIdx, CMetaAnimTreeBuildOrders::NoSpecialOrders()));

    typedef rstl::vector< rstl::pair< uint, CAdditiveAnimationInfo > > TAdditiveInfoList;
    const TAdditiveInfoList& infoList = x0_charFactory->GetAdditiveAnimInfoList();

    AUTO(finder, rstl::default_pair_sorter_finder< TAdditiveInfoList >());
    TAdditiveInfoList::const_iterator infoSearch =
        rstl::binary_find(infoList.begin(), infoList.end(), animIdx, finder);

    const CAdditiveAnimationInfo& infoRef = infoSearch != infoList.end()
                                                ? infoSearch->second
                                                : x0_charFactory->GetDefaultAdditiveAnimInfo();
    const CAdditiveAnimationInfo info(infoRef);

    x434_additiveAnims.push_back(rstl::pair< uint, CAdditiveAnimPlayback >(
        animIdx, CAdditiveAnimPlayback(animTree, weight, active, info, fadeOut)));
  }
}

void CAnimData::DelAdditiveAnimation(uint idx) {
  const uint animIdx = xc_charInfo.GetAnimationIndexList()[idx];
  rstl::pair< uint, CAdditiveAnimPlayback >* end = x434_additiveAnims.end();
  rstl::pair< uint, CAdditiveAnimPlayback >* search = x434_additiveAnims.begin();

  while (search != end) {
    if (animIdx == search->first) {
      break;
    }
    ++search;
  }

  if (search != end) {
    CAdditiveAnimPlayback& playback = search->second;
    const CAdditiveAnimPlayback::EPlaybackPhase phase = playback.GetFadingMode();
    if (phase != CAdditiveAnimPlayback::kPP_FadingOut &&
        phase != CAdditiveAnimPlayback::kPP_FadedOut) {
      playback.FadeOut();
    }
  }
}

float CAnimData::GetAdditiveAnimationWeight(uint idx) {
  const uint animIdx = xc_charInfo.GetAnimationIndexList()[idx];
  rstl::pair< uint, CAdditiveAnimPlayback >* end = x434_additiveAnims.end();
  rstl::pair< uint, CAdditiveAnimPlayback >* search = x434_additiveAnims.begin();

  while (search != end) {
    if (animIdx == search->first) {
      return search->second.GetWeight();
    }
    ++search;
  }

  return 0.f;
}

bool CAnimData::IsAdditiveAnimationAdded(uint idx) const {
  const uint animIdx = xc_charInfo.GetAnimationIndexList()[idx];
  const rstl::pair< uint, CAdditiveAnimPlayback >* end = x434_additiveAnims.end();
  const rstl::pair< uint, CAdditiveAnimPlayback >* search = x434_additiveAnims.begin();

  while (search != end) {
    if (animIdx == search->first) {
      return true;
    }
    ++search;
  }

  return false;
}

const rstl::rc_ptr< CAnimTreeNode >& CAnimData::GetAdditiveAnimationTree(uint idx) const {
  const uint animIdx = xc_charInfo.GetAnimationIndexList()[idx];
  const rstl::pair< uint, CAdditiveAnimPlayback >* end = x434_additiveAnims.end();
  const rstl::pair< uint, CAdditiveAnimPlayback >* search = x434_additiveAnims.begin();

  while (search != end) {
    if (animIdx == search->first) {
      break;
    }
    ++search;
  }

  return search->second.GetAnimationTree();
}

const rstl::ncrc_ptr< CAnimTreeNode >& CAnimData::GetRootAnimationTree() const {
  return x1f8_animRoot;
}

bool CAnimData::IsAdditiveAnimation(uint idx) const {
  const uint animIdx = xc_charInfo.GetAnimationIndexList()[idx];

  typedef rstl::vector< rstl::pair< uint, CAdditiveAnimationInfo > > TAdditiveInfoList;
  const TAdditiveInfoList& infoList = x0_charFactory->GetAdditiveAnimInfoList();

  AUTO(finder, rstl::default_pair_sorter_finder< TAdditiveInfoList >());
  TAdditiveInfoList::const_iterator found =
      rstl::binary_find(infoList.begin(), infoList.end(), animIdx, finder);
  return found != infoList.end();
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > IAnimReader::Simplified() {
  return VSimplified();
}

CAdvancementResults CAnimData::AdvanceAdditiveAnim(rstl::rc_ptr< CAnimTreeNode >& anim,
                                                   CCharAnimTime time) {
  CAdvancementResults ret = anim->VAdvanceView(time);

  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified = anim->Simplified();
  if (simplified.valid()) {
    anim = Cast(simplified.data());
  }

  return ret;
}

CAdvancementDeltas CAnimData::UpdateAdditiveAnims(float dt) {
  rstl::pair< uint, CAdditiveAnimPlayback >* it = x434_additiveAnims.begin();
  rstl::pair< uint, CAdditiveAnimPlayback >* const begin = x434_additiveAnims.begin();

  while (it != begin + x434_additiveAnims.size()) {
    CAdditiveAnimPlayback& playback = it->second;
    playback.Update(dt);

    const CCharAnimTime remTime = playback.AnimationTree()->VGetTimeRemaining();
    if (close_enough(remTime.GetSeconds(), 0.f) && playback.IsFadeOutWhenAnimOver() != 0) {
      playback.FadeOut();
    }

    if (playback.GetFadingMode() == CAdditiveAnimPlayback::kPP_FadedOut) {
      it = x434_additiveAnims.erase(it);
    } else {
      ++it;
    }
  }

  return AdvanceAdditiveAnims(dt);
}

CAdvancementDeltas CAnimData::AdvanceAdditiveAnims(float dt) {
  CQuaternion rotDelta(CQuaternion::NoRotation());
  float posDeltaX = 0.f;
  float posDeltaY = 0.f;
  float posDeltaZ = 0.f;

  const uint count = x434_additiveAnims.size();
  for (uint i = 0; i < count; ++i) {
    CAdditiveAnimPlayback& playback = x434_additiveAnims[i].second;
    rstl::rc_ptr< CAnimTreeNode >& anim = playback.AnimationTree();

    CCharAnimTime time(dt);

    if (playback.IsLoop()) {
      while (time.GreaterThanZero() && !close_enough(time.GetSeconds(), 0.f)) {
        x210_passedIntCount +=
            anim->GetInt32POIList(time, mInt32POINodes.data(), 16, x210_passedIntCount, 0);
        x20c_passedBoolCount +=
            anim->GetBoolPOIList(time, mBoolPOINodes.data(), 8, x20c_passedBoolCount, 0);
        x214_passedParticleCount += anim->GetParticlePOIList(time, mParticlePOINodes.data(), 20,
                                                             x214_passedParticleCount, 0);
        x218_passedSoundCount +=
            anim->GetSoundPOIList(time, mSoundPOINodes.data(), 20, x218_passedSoundCount, 0);

        const CAdvancementResults advResult = AdvanceAdditiveAnim(anim, time);
        const CAdvancementDeltas deltas = advResult.GetAdvancementDeltas();
        const CQuaternion thisRot = deltas.GetOrientationDelta();

        posDeltaX += deltas.GetOffsetDelta().GetX();
        posDeltaY += deltas.GetOffsetDelta().GetY();
        posDeltaZ += deltas.GetOffsetDelta().GetZ();
        rotDelta = rotDelta * thisRot;
        time = advResult.GetRemainder();
      }
    } else {
      CCharAnimTime remTime = anim->VGetTimeRemaining();

      while (!close_enough(remTime.GetSeconds(), 0.f) && !close_enough(time.GetSeconds(), 0.f)) {
        x210_passedIntCount +=
            anim->GetInt32POIList(time, mInt32POINodes.data(), 16, x210_passedIntCount, 0);
        x20c_passedBoolCount +=
            anim->GetBoolPOIList(time, mBoolPOINodes.data(), 8, x20c_passedBoolCount, 0);
        x214_passedParticleCount += anim->GetParticlePOIList(time, mParticlePOINodes.data(), 20,
                                                             x214_passedParticleCount, 0);
        x218_passedSoundCount +=
            anim->GetSoundPOIList(time, mSoundPOINodes.data(), 20, x218_passedSoundCount, 0);

        const CAdvancementResults advResult = AdvanceAdditiveAnim(anim, time);
        const CAdvancementDeltas deltas = advResult.GetAdvancementDeltas();
        const CQuaternion thisRot = deltas.GetOrientationDelta();

        posDeltaX += deltas.GetOffsetDelta().GetX();
        posDeltaY += deltas.GetOffsetDelta().GetY();
        posDeltaZ += deltas.GetOffsetDelta().GetZ();
        rotDelta = rotDelta * thisRot;
        time = advResult.GetRemainder();

        remTime = anim->VGetTimeRemaining();
        time = CCharAnimTime(rstl::min_val(time.GetSeconds(), remTime.GetSeconds()));
      }
    }
  }

  return CAdvancementDeltas(CVector3f(posDeltaX, posDeltaY, posDeltaZ), rotDelta);
}

void CAnimData::AddAdditiveSegData(const CSegIdList& list, CSegStatementSet& setOut) const {
  const uint count = x434_additiveAnims.size();
  uint i = 0;
  while (i < count) {
    const CAdditiveAnimPlayback& playback = x434_additiveAnims[i].second;
    if (!close_enough(playback.GetWeight(), 0.f)) {
      playback.AddToSegStatementSet(list, **xcc_layoutData, setOut);
    }
    ++i;
  }
}

int CAnimData::GetEventResourceIdForAnimResourceId(int id) const {
  return x0_charFactory->GetEventResourceIdForAnimResourceId(id);
}
