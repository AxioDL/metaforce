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
: mCharFactory(charFactory)
, mCharInfo(charInfo)
, mLayoutData(layoutData)
, mModelData(modelData)
, mIceModelData(iceModelData)
, mXrayModel(nullptr)
, mInfraModel(nullptr)
, mAnimCtx(animCtx)
, mAnimMgr(animMgr)
, mAnimDir(kAD_Forward)
, mAabb(CAABox::MakeMaxInvertedBox())
, mParticleDB()
, mSelfId(selfId)
, mAlignPos(CVector3f::Zero())
, mAlignRot(CQuaternion::NoRotation())
, mAnimRoot()
, mTransMgr(transMgr)
, mSpeedScale(1.f)
, mCharIdx(charIdx)
, mCurrentAnim(defaultAnim)
, mPassedBoolCount(0)
, mPassedIntCount(0)
, mPassedParticleCount(0)
, mPassedSoundCount(0)
, mParticleLightIdx(0)
, mAnimating(false)
, mLoop(loop)
, mAligningPos(false)
, x220_27_(false)
, x220_28_(false)
, mAnimationJustStarted(false)
, mPoseBuilt(false)
, mPoseCached(false)
, mPose(static_cast< uchar >(layoutData->GetBodyPartSegIds().size()))
, mPoseBuilder(CLayoutDescription(layoutData))
, mPlaybackParms(-1, -1, 1.f, true)
, mAdditiveAnims() {
  if (skPOICacheReferenceCount == 0) {
    mBoolPOINodes.resize(8);
    mInt32POINodes.resize(16);
    mParticlePOINodes.resize(20);
    mSoundPOINodes.resize(20);
  }
  ++skPOICacheReferenceCount;

  mModelData->CalculateDefault();
  const CVector3f* pointItr =
      reinterpret_cast< const CVector3f* >(mModelData->GetModel()->GetPositions());
  for (int i = 0; i < mModelData->GetNumPoints(); ++i) {
    mAabb.AccumulateBounds(pointItr[i]);
  }

  mParticleDB.CacheParticleDesc(charInfo.GetParticleResData());

  CLayoutDescription layoutDesc(mLayoutData);
  CHierarchyPoseBuilder pb(layoutDesc);
  pb.BuildNoScale(mPose);
  mPoseBuilt = true;

  int initialAnim = defaultAnim;
  if (initialAnim == -1) {
    initialAnim = 0;
    rstl::string warning =
        rstl::string_l("Character ") + charInfo.GetCharacterName() +
        rstl::string_l(" has invalid initial animation, so defaulting to first.\n");
  }

  const uint animRes = charInfo.GetAnimationIndexList()[initialAnim];
  mAnimRoot =
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
      mCharInfo.GetAnimBBoxList();

  if (aabbList.size() > 0) {
    CAnimTreeEffectiveContribution contrib = mAnimRoot->GetContributionOfHighestInfluence();
    rstl::string name = contrib.GetPrimitiveName();

    rstl::vector< rstl::pair< rstl::string, CAABox > >::const_iterator search =
        rstl::find_by_key(aabbList, rstl::string(name));
    if (search != aabbList.end()) {
      return search->second;
    }
  }
  return mAabb;
}

CAABox CAnimData::GetBoundingBox(const CTransform4f& xf) const {
  return GetBoundingBox().GetTransformedAABox(xf);
}

CSegId CAnimData::GetLocatorSegId(const rstl::string& name) const {
  return mLayoutData->GetSegIdFromString(name);
}

void CAnimData::ResetPOILists() {
  mPassedBoolCount = 0;
  mPassedIntCount = 0;
  mPassedParticleCount = 0;
  mPassedSoundCount = 0;
}

float CAnimData::GetAverageVelocity(int animIn) const {
  const uint animRes = mCharInfo.GetAnimationIndexList()[animIn];
  rstl::rc_ptr< IMetaAnim > anim = mAnimMgr->GetMetaAnimation(animRes);

  rstl::set< CPrimitive > primitiveSet;
  anim->GetUniquePrimitives(primitiveSet);

  float ret;
  float weightedVel = 0.f;
  float totalDur = 0.f;
  rstl::set< CPrimitive >::const_iterator it = primitiveSet.begin();
  rstl::set< CPrimitive >::const_iterator end = primitiveSet.end();
  while (it != end) {
    const SObjectTag animTag('ANIM', it->GetAnimResId());

    TLockedToken< CAllFormatsAnimSource > animData = mAnimCtx->GetSimplePool().GetObj(animTag);

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
  mParticleDB.Update(dt, mPose, **mLayoutData, xf, scale, mgr);
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
  mModelData = model;

  mModelData->CalculateDefault();
  mAabb = CAABox::MakeMaxInvertedBox();

  const CVector3f* pointItr =
      reinterpret_cast< const CVector3f* >(mModelData->GetModel()->GetPositions());
  for (int i = 0; i < mModelData->GetNumPoints(); ++i) {
    mAabb.AccumulateBounds(pointItr[i]);
  }
}

void CAnimData::SetInfraModel(const TLockedToken< CModel >& model,
                              const TLockedToken< CSkinRules >& skinRules) {
  CSkinnedModel* skinnedModel = rs_new CSkinnedModel(
      model, skinRules, mModelData->GetLayoutInfo(), CSkinnedModel::kDO_Owned);
  skinnedModel->CalculateDefault();
  mInfraModel = rstl::rc_ptr< CSkinnedModel >(skinnedModel);
}

void CAnimData::SetXRayModel(const TLockedToken< CModel >& model,
                             const TLockedToken< CSkinRules >& skinRules) {
  CSkinnedModel* skinnedModel = rs_new CSkinnedModel(
      model, skinRules, mModelData->GetLayoutInfo(), CSkinnedModel::kDO_Owned);
  skinnedModel->CalculateDefault();
  mXrayModel = rstl::rc_ptr< CSkinnedModel >(skinnedModel);
}

void CAnimData::AdvanceAnim(CCharAnimTime& time, CVector3f& offset, CQuaternion& rotation) {
  const float dt = time.GetSeconds();
  CAdvancementResults results(CCharAnimTime(0.f), CAdvancementDeltas());
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified;

  if (mAnimDir == kAD_Forward) {
    results = mAnimRoot->VAdvanceView(time);
    simplified = mAnimRoot->Simplified();
  }

  if (simplified.valid()) {
    mAnimRoot = Cast(simplified.data());
  }

  if (x220_28_ || x220_27_) {
    const int count = mPassedIntCount;
    const CInt32POINode* node = mInt32POINodes.data();
    if (count > 0) {
      for (int i = 0; i < count; ++i, ++node) {
        if (node->GetPoiType() == kPT_UserEvent) {
          switch (node->GetValue()) {
          case kUE_AlignTargetPosStart:
            mAligningPos = true;
            break;
          case kUE_AlignTargetPos:
            mAlignPos = CVector3f::Zero();
            x220_28_ = false;
            mAligningPos = false;
            break;
          case kUE_AlignTargetRot:
            mAlignRot = CQuaternion::NoRotation();
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
  if (mAligningPos) {
    offset += mAlignPos * dt;
  }

  CQuaternion alignRot = deltaRot * mAlignRot;
  rotation *= alignRot;
  mAlignPos = alignRot.BuildInverted().Transform(mAlignPos);

  time = results.mRemTime;
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
    mParticleDB.SuspendAllActiveEffects(mgr);
  }

  const int passedParticleCount = mPassedParticleCount;
  for (int i = 0; i < passedParticleCount; ++i) {
    const CParticlePOINode& node = mParticlePOINodes[i];
    const int charIdx = node.GetCharacterIndex();
    if (charIdx == -1 || charIdx == mCharIdx) {
      mParticleDB.AddParticleEffect(node.GetString(), node.GetFlags(), node.GetParticleData(),
                                        scale, mgr, aid, false, mParticleLightIdx);
    }
  }

  return deltas;
}

CAdvancementDeltas CAnimData::DoAdvance(float dt, bool& suspendParticles, CRandom16& random,
                                        bool advTree) {
  suspendParticles = false;

  CVector3f offset(0.f, 0.f, 0.f);
  CQuaternion rotation(CQuaternion::NoRotation());

  const float scaledDt = dt * mSpeedScale;
  CVector3f additiveOffset(0.f, 0.f, 0.f);
  CQuaternion additiveRotation(CQuaternion::NoRotation());

  ResetPOILists();

  if (mAdditiveAnims.size() > 0) {
    const CAdvancementDeltas additiveDeltas = UpdateAdditiveAnims(scaledDt);
    additiveOffset = additiveDeltas.GetOffsetDelta();
    additiveRotation = additiveDeltas.GetOrientationDelta();
    mPoseCached = false;
    mPoseBuilt = false;
  }

  const bool animating = IsAnimating() == true;
  if (!animating) {
    suspendParticles = true;
    return CAdvancementDeltas(offset, rotation);
  }

  if (mAnimationJustStarted) {
    mAnimationJustStarted = false;
    suspendParticles = true;
  }

  if (advTree) {
    SetRandomPlaybackRate(random);

    CCharAnimTime time(scaledDt);

    if (mLoop) {
      while (time.GreaterThanZero() && !close_enough(time.GetSeconds(), 0.f)) {
        mPassedIntCount +=
            mAnimRoot->GetInt32POIList(time, mInt32POINodes.data(), 16, mPassedIntCount, 0);
        mPassedBoolCount +=
            mAnimRoot->GetBoolPOIList(time, mBoolPOINodes.data(), 8, mPassedBoolCount, 0);
        mPassedParticleCount += mAnimRoot->GetParticlePOIList(
            time, mParticlePOINodes.data(), 20, mPassedParticleCount, 0);
        mPassedSoundCount += mAnimRoot->GetSoundPOIList(time, mSoundPOINodes.data(), 20,
                                                                mPassedSoundCount, 0);

        AdvanceAnim(time, offset, rotation);
      }
    } else {
      CCharAnimTime remTime = mAnimRoot->VGetTimeRemaining();

      while (!close_enough(remTime.GetSeconds(), 0.f) && !close_enough(time.GetSeconds(), 0.f)) {
        mPassedIntCount +=
            mAnimRoot->GetInt32POIList(time, mInt32POINodes.data(), 16, mPassedIntCount, 0);
        mPassedBoolCount +=
            mAnimRoot->GetBoolPOIList(time, mBoolPOINodes.data(), 8, mPassedBoolCount, 0);
        mPassedParticleCount += mAnimRoot->GetParticlePOIList(
            time, mParticlePOINodes.data(), 20, mPassedParticleCount, 0);
        mPassedSoundCount += mAnimRoot->GetSoundPOIList(time, mSoundPOINodes.data(), 20,
                                                                mPassedSoundCount, 0);

        AdvanceAnim(time, offset, rotation);

        remTime = mAnimRoot->VGetTimeRemaining();
        time = CCharAnimTime(
            rstl::max_val(0.f, rstl::min_val(time.GetSeconds(), remTime.GetSeconds())));

        if (close_enough(remTime.GetSeconds(), 0.f)) {
          mAnimating = false;
          mAlignPos = CVector3f::Zero();
          x220_28_ = false;
          mAligningPos = false;
        }
      }
    }

    mPoseCached = false;
    mPoseBuilt = false;
  }

  return CAdvancementDeltas(offset + additiveOffset, rotation * additiveRotation);
}

void CAnimData::SetAnimation(const CAnimPlaybackParms& parms, bool noTrans) {
  if (parms.GetAnimationId() == mPlaybackParms.GetAnimationId() ||
      (parms.GetSecondAnimationId() == mPlaybackParms.GetSecondAnimationId() &&
       parms.GetSecondAnimationId() != -1) ||
      (parms.GetBlendFactor() == mPlaybackParms.GetBlendFactor() &&
       parms.GetBlendFactor() != 1.f)) {
    if (mAnimationJustStarted)
      return;
  }

  mPlaybackParms.SetAnimationId(parms.GetAnimationId());
  mPlaybackParms.SetSecondAnimationId(parms.GetSecondAnimationId());
  mPlaybackParms.SetBlendFactor(parms.GetBlendFactor());

  const int animA = parms.GetAnimationId();
  const bool animating = parms.GetIsPlayAnimation();
  mSpeedScale = 1.f;
  mCurrentAnim = animA;

  const int animB = parms.GetSecondAnimationId();
  const float blendFactor = parms.GetBlendFactor();
  const uint animResA = mCharInfo.GetAnimationIndexList()[animA];
  ResetPOILists();

  rstl::ncrc_ptr< CAnimTreeNode > newAnimTree;

  if (animB != -1) {
    const uint animResB = mCharInfo.GetAnimationIndexList()[animB];

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
    mAnimRoot = mTransMgr->GetTransitionTree(mAnimRoot, newAnimTree);
  } else {
    mAnimRoot = newAnimTree;
  }

  mAnimating = animating;
  CalcPlaybackAlignmentParms(parms, newAnimTree);
  ResetPOILists();
  mAnimationJustStarted = true;
}

void CAnimData::GetAnimationPrimitives(const CAnimPlaybackParms& parms,
                                       rstl::set< CPrimitive >& primsOut) const {
  const int animB = parms.GetSecondAnimationId();

  const uint animResA = mCharInfo.GetAnimationIndexList()[parms.GetAnimationId()];
  GetAnimationManager()->GetMetaAnimation(animResA)->GetUniquePrimitives(primsOut);

  if (animB != -1) {
    const uint animResB = mCharInfo.GetAnimationIndexList()[animB];
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
  if (!mPoseCached) {
    RecalcPoseBuilder(nullptr);
    mPoseCached = true;
    mPoseBuilt = false;
  }

  if (!mPoseBuilt) {
    mPoseBuilder.BuildNoScale(mPose);
    mPoseBuilt = true;
  }
}

void CAnimData::PreRender() {
  if (!mPoseCached) {
    RecalcPoseBuilder(nullptr);
    mPoseCached = true;
    mPoseBuilt = false;
  }
}

void CAnimData::SetupRender(const CSkinnedModel& model,
                            const rstl::optional_object< CVertexMorphEffect >& morphEffect,
                            const float* avgNormals) const {
  if (!mPoseBuilt) {
    CAnimData* self = const_cast< CAnimData* >(this);
    self->mPoseBuilder.BuildNoScale(self->mPose);
    self->mPoseBuilt = true;
  }

  PoseSkinnedModel(model, mPose, morphEffect, avgNormals);
}

void CAnimData::Render(const CSkinnedModel& model, const CModelFlags& flags,
                       const rstl::optional_object< CVertexMorphEffect >& morphEffect,
                       const float* avgNormals) const {
  SetupRender(model, morphEffect, avgNormals);
  DrawSkinnedModel(model, flags);
}

void CAnimData::RenderAuxiliary(const CFrustumPlanes& frustum) const {
  mParticleDB.AddToRendererClipped(frustum);
}

#if VERSION >= VERSION_GM8P_00
void CHierarchyPoseBuilder::Insert(const CSegId& id, const CQuaternion& rot) {
  mTreeMap[id].SetRotation(rot);
}

void CHierarchyPoseBuilder::Insert(const CSegId& id, const CVector3f& off) {
  mTreeMap[id].SetOffset(off);
}
#endif

void CAnimData::RecalcPoseBuilder(const CCharAnimTime* time) const {
  const CSegIdList* segIdList = &mLayoutData->GetBodyPartSegIds();

  CStackSegStatementSet statementSet;
  CHierarchyPoseBuilder& poseBuilder = mPoseBuilder;

  if (time == nullptr) {
    mAnimRoot->VGetSegStatementSet(*segIdList, statementSet);
  } else {
    mAnimRoot->VGetSegStatementSet(*segIdList, statementSet, *time);
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

rstl::rc_ptr< CAnimationManager > CAnimData::GetAnimationManager() const { return mAnimMgr; }

float CAnimData::GetAnimationDuration(int animIn) const {
  const uint animRes = mCharInfo.GetAnimationIndexList()[animIn];
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

inline rstl::ncrc_ptr< CAnimSysContext > CAnimData::GetAnimSysContext() const { return mAnimCtx; }

float CAnimData::GetAnimTimeRemaining(const rstl::string&) const {
  float remTime = mAnimRoot->VGetTimeRemaining().GetSeconds();
  if (mSpeedScale > 0.f) {
    remTime /= mSpeedScale;
  }
  return remTime;
}

bool CAnimData::IsAnimTimeRemaining(float rem, const rstl::string&) const {
  if (mAnimRoot.GetPtr() != 0) {
    const float remTime = mAnimRoot->VGetTimeRemaining().GetSeconds();
    return !close_enough(remTime, 0.f, rem);
  }

  return false;
}

CTransform4f CAnimData::GetLocatorTransform(const rstl::string& name,
                                            const CCharAnimTime* time) const {
  CSegId seg = mLayoutData->GetSegIdFromString(name);
  CSegId segCopy = seg;
  return GetLocatorTransform(segCopy, time);
}

CTransform4f CAnimData::GetLocatorTransform(CSegId seg, const CCharAnimTime* time) const {
  CTransform4f xf = CTransform4f::Identity();

  if (seg.val() != 0xFF) {
    if (time != nullptr || !mPoseCached) {
      CAnimData* self = const_cast< CAnimData* >(this);
      self->RecalcPoseBuilder(time);
      self->mPoseCached = !time;
    }

    if (!mPoseBuilt) {
      mPoseBuilder.BuildTransform(seg, xf);
    } else {
      const CMatrix3f& rot = mPose.GetTransformMinusOffset(seg);
      const CVector3f& offset = mPose.GetOffset(seg);
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
  mAlignRot = alignRot;
  x220_27_ = false;

  if (deltaOrient != nullptr && objectXf != nullptr) {
    ResetPOILists();
    mPassedIntCount += node->GetInt32POIList(CCharAnimTime::Infinity(), mInt32POINodes.data(),
                                                 16, mPassedIntCount, 64);

    const int count = mPassedIntCount;
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
          mAlignRot = alignRot;
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
      mPassedIntCount += node->GetInt32POIList(CCharAnimTime::Infinity(), mInt32POINodes.data(),
                                                   16, mPassedIntCount, 64);

      const int count = mPassedIntCount;
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
          mAlignPos = normalized;
          x220_28_ = true;
          mAligningPos = false;
        } else {
          mAlignPos = CVector3f::Zero();
          x220_28_ = false;
          mAligningPos = false;
        }
      }
    } else {
      mAlignPos = CVector3f::Zero();
      x220_28_ = false;
      mAligningPos = false;
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
      mPassedIntCount += node->GetInt32POIList(CCharAnimTime::Infinity(), mInt32POINodes.data(),
                                                   16, mPassedIntCount, 64);

      const int count = mPassedIntCount;
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
          mAlignRot = alignRot;
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
          mAlignPos = normalized;
          x220_28_ = true;
          mAligningPos = false;
        } else {
          mAlignPos = CVector3f::Zero();
          x220_28_ = false;
          mAligningPos = false;
        }
      }
    } else {
      mAlignPos = CVector3f::Zero();
      x220_28_ = false;
      mAligningPos = false;
    }
  }
}

void CAnimData::SetRandomPlaybackRate(CRandom16& random) {
  for (int i = 0; i < mPassedIntCount; ++i) {
    const CInt32POINode& poi = mInt32POINodes[i];
    if (poi.GetPoiType() == kPT_RandRate) {
      const float scale = static_cast< float >(random.Next() % poi.GetValue()) / 100.f;
      if ((random.Next() % 100) < 50) {
        mSpeedScale = 1.f + scale;
      } else {
        mSpeedScale = 1.f - scale;
      }
      break;
    }
  }
}

void CAnimData::SetPlaybackRate(float set) { mSpeedScale = set; }

void CAnimData::MultiplyPlaybackRate(float scale) { mSpeedScale *= scale; }

CCharAnimTime CAnimData::GetTimeOfUserEvent(EUserEventType type, const CCharAnimTime& time) const {
  const int count = mAnimRoot->GetInt32POIList(time, sInt32TransientCacheData, 16, 0, 64);
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
  const TEffectList& effects = mCharInfo.GetEffectList();
  const uint effectCount = effects.size();
  for (uint i = 0; i < effectCount; ++i) {
    TEffectList::value_type effect = effects[i];
    const uint componentCount = effect.second.size();
    for (uint j = 0; j < componentCount; ++j) {
      const CEffectComponent& component = effect.second[j];
      mParticleDB.CacheParticleDesc(component.GetParticleTag());
      {
        const CParticleData data(0, component.GetParticleTag(), component.GetSegmentName(),
                                 component.GetScale(), component.GetParentedMode());
        mParticleDB.AddParticleEffect(component.GetComponentName(), component.GetFlags(), data,
                                          scale, mgr, areaId, true, mParticleLightIdx);
      }
      mParticleDB.SetParticleEffectState(component.GetComponentName(), false, mgr);
    }
  }
}

void CAnimData::SetParticleEffectState(const rstl::string& name, bool active, CStateManager& mgr) {
  rstl::vector< rstl::pair< rstl::string, rstl::vector< CEffectComponent > > > effects =
      mCharInfo.GetEffectList();

  rstl::vector< rstl::pair< rstl::string, rstl::vector< CEffectComponent > > >::const_iterator it =
      rstl::find_by_key(effects, name);
  if (it != effects.end()) {
    const rstl::vector< CEffectComponent >& components = it->second;
    rstl::vector< CEffectComponent >::const_iterator compIt = components.begin();
    rstl::vector< CEffectComponent >::const_iterator compEnd = components.end();
    for (; compIt != compEnd; ++compIt) {
      mParticleDB.SetParticleEffectState(compIt->GetComponentName(), active, mgr);
    }
  }
}

void CAnimData::SetParticleCEXTValue(const rstl::string& name, int index, float value) {
  TEffectList effects = mCharInfo.GetEffectList();
  AUTO(it, rstl::find_by_key(effects, name));
  if (it != effects.end()) {
    const rstl::vector< CEffectComponent >& components = it->second;
    if (components.begin() != components.end()) {
      mParticleDB.SetExternalVarValue(components.front().GetComponentName(), index, value);
    }
  }
}

void CAnimData::SetPhase(float phase) { mAnimRoot->VSetPhase(phase); }

rstl::rc_ptr< CAnimationManager > CAnimData::GetAnimationManager() { return mAnimMgr; }

void CAnimData::AddAdditiveAnimation(uint idx, float weight, bool active, bool fadeOut) {
  const uint animIdx = mCharInfo.GetAnimationIndexList()[idx];
  rstl::pair< uint, CAdditiveAnimPlayback >* end = mAdditiveAnims.end();
  rstl::pair< uint, CAdditiveAnimPlayback >* search = mAdditiveAnims.begin();

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
    const TAdditiveInfoList& infoList = mCharFactory->GetAdditiveAnimInfoList();

    AUTO(finder, rstl::default_pair_sorter_finder< TAdditiveInfoList >());
    TAdditiveInfoList::const_iterator infoSearch =
        rstl::binary_find(infoList.begin(), infoList.end(), animIdx, finder);

    const CAdditiveAnimationInfo& infoRef = infoSearch != infoList.end()
                                                ? infoSearch->second
                                                : mCharFactory->GetDefaultAdditiveAnimInfo();
    const CAdditiveAnimationInfo info(infoRef);

    mAdditiveAnims.push_back(rstl::pair< uint, CAdditiveAnimPlayback >(
        animIdx, CAdditiveAnimPlayback(animTree, weight, active, info, fadeOut)));
  }
}

void CAnimData::DelAdditiveAnimation(uint idx) {
  const uint animIdx = mCharInfo.GetAnimationIndexList()[idx];
  rstl::pair< uint, CAdditiveAnimPlayback >* end = mAdditiveAnims.end();
  rstl::pair< uint, CAdditiveAnimPlayback >* search = mAdditiveAnims.begin();

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
  const uint animIdx = mCharInfo.GetAnimationIndexList()[idx];
  rstl::pair< uint, CAdditiveAnimPlayback >* end = mAdditiveAnims.end();
  rstl::pair< uint, CAdditiveAnimPlayback >* search = mAdditiveAnims.begin();

  while (search != end) {
    if (animIdx == search->first) {
      return search->second.GetWeight();
    }
    ++search;
  }

  return 0.f;
}

bool CAnimData::IsAdditiveAnimationAdded(uint idx) const {
  const uint animIdx = mCharInfo.GetAnimationIndexList()[idx];
  const rstl::pair< uint, CAdditiveAnimPlayback >* end = mAdditiveAnims.end();
  const rstl::pair< uint, CAdditiveAnimPlayback >* search = mAdditiveAnims.begin();

  while (search != end) {
    if (animIdx == search->first) {
      return true;
    }
    ++search;
  }

  return false;
}

const rstl::rc_ptr< CAnimTreeNode >& CAnimData::GetAdditiveAnimationTree(uint idx) const {
  const uint animIdx = mCharInfo.GetAnimationIndexList()[idx];
  const rstl::pair< uint, CAdditiveAnimPlayback >* end = mAdditiveAnims.end();
  const rstl::pair< uint, CAdditiveAnimPlayback >* search = mAdditiveAnims.begin();

  while (search != end) {
    if (animIdx == search->first) {
      break;
    }
    ++search;
  }

  return search->second.GetAnimationTree();
}

const rstl::ncrc_ptr< CAnimTreeNode >& CAnimData::GetRootAnimationTree() const {
  return mAnimRoot;
}

bool CAnimData::IsAdditiveAnimation(uint idx) const {
  const uint animIdx = mCharInfo.GetAnimationIndexList()[idx];

  typedef rstl::vector< rstl::pair< uint, CAdditiveAnimationInfo > > TAdditiveInfoList;
  const TAdditiveInfoList& infoList = mCharFactory->GetAdditiveAnimInfoList();

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
  rstl::pair< uint, CAdditiveAnimPlayback >* it = mAdditiveAnims.begin();
  rstl::pair< uint, CAdditiveAnimPlayback >* const begin = mAdditiveAnims.begin();

  while (it != begin + mAdditiveAnims.size()) {
    CAdditiveAnimPlayback& playback = it->second;
    playback.Update(dt);

    const CCharAnimTime remTime = playback.AnimationTree()->VGetTimeRemaining();
    if (close_enough(remTime.GetSeconds(), 0.f) && playback.IsFadeOutWhenAnimOver() != 0) {
      playback.FadeOut();
    }

    if (playback.GetFadingMode() == CAdditiveAnimPlayback::kPP_FadedOut) {
      it = mAdditiveAnims.erase(it);
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

  const uint count = mAdditiveAnims.size();
  for (uint i = 0; i < count; ++i) {
    CAdditiveAnimPlayback& playback = mAdditiveAnims[i].second;
    rstl::rc_ptr< CAnimTreeNode >& anim = playback.AnimationTree();

    CCharAnimTime time(dt);

    if (playback.IsLoop()) {
      while (time.GreaterThanZero() && !close_enough(time.GetSeconds(), 0.f)) {
        mPassedIntCount +=
            anim->GetInt32POIList(time, mInt32POINodes.data(), 16, mPassedIntCount, 0);
        mPassedBoolCount +=
            anim->GetBoolPOIList(time, mBoolPOINodes.data(), 8, mPassedBoolCount, 0);
        mPassedParticleCount += anim->GetParticlePOIList(time, mParticlePOINodes.data(), 20,
                                                             mPassedParticleCount, 0);
        mPassedSoundCount +=
            anim->GetSoundPOIList(time, mSoundPOINodes.data(), 20, mPassedSoundCount, 0);

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
        mPassedIntCount +=
            anim->GetInt32POIList(time, mInt32POINodes.data(), 16, mPassedIntCount, 0);
        mPassedBoolCount +=
            anim->GetBoolPOIList(time, mBoolPOINodes.data(), 8, mPassedBoolCount, 0);
        mPassedParticleCount += anim->GetParticlePOIList(time, mParticlePOINodes.data(), 20,
                                                             mPassedParticleCount, 0);
        mPassedSoundCount +=
            anim->GetSoundPOIList(time, mSoundPOINodes.data(), 20, mPassedSoundCount, 0);

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
  const uint count = mAdditiveAnims.size();
  uint i = 0;
  while (i < count) {
    const CAdditiveAnimPlayback& playback = mAdditiveAnims[i].second;
    if (!close_enough(playback.GetWeight(), 0.f)) {
      playback.AddToSegStatementSet(list, **mLayoutData, setOut);
    }
    ++i;
  }
}

int CAnimData::GetEventResourceIdForAnimResourceId(int id) const {
  return mCharFactory->GetEventResourceIdForAnimResourceId(id);
}
