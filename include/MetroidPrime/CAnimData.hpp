#ifndef _CANIMDATA
#define _CANIMDATA

#include "types.h"

#include "Kyoto/Animation/CAdditiveAnimPlayback.hpp"
#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CCharacterInfo.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "Kyoto/Animation/CPoseAsTransforms.hpp"
#include "Kyoto/Animation/CHierarchyPoseBuilder.hpp"
#include "MetroidPrime/CParticleDatabase.hpp"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CAnimationManager.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/set.hpp"

class CAnimSysContext;
class CAnimTreeNode;
class CCharacterFactory;
class CCharLayoutInfo;
class CRandom16;
class CSegIdList;
class CSegStatementSet;
class CSkinnedModel;
class CSkinnedModelWithAvgNormals;
class CTransitionManager;
class CVertexMorphEffect;
class CModelFlags;
class CPrimitive;
class CFrustumPlanes;

struct CAdvancementDeltas;
struct CAdvancementResults;

class CAnimData {
public:
  enum EAnimDir {
    kAD_Forward,
    kAD_Backward,
  };

  CAnimData(uint, const CCharacterInfo&, int, int, bool, const TLockedToken< CCharLayoutInfo >&,
            const TToken< CSkinnedModel >&,
            const rstl::optional_object< TLockedToken< CSkinnedModelWithAvgNormals > >&,
            const rstl::ncrc_ptr< CAnimSysContext >&, const rstl::rc_ptr< CAnimationManager >&,
            const rstl::rc_ptr< CTransitionManager >&, const TLockedToken< CCharacterFactory >&);
  ~CAnimData();

  void PreRender();
  bool GetIsLoop() const { return mLoop; }
  void EnableLooping(const bool v) {
    mLoop = v;
    mAnimating = true;
  }

  const TLockedToken< CSkinnedModel >& GetModelData() const { return mModelData; }
  CSkinnedModel* GetXRayModel() const { return mXrayModel.GetPtr(); }
  CSkinnedModel* GetInfraModel() const { return mInfraModel.GetPtr(); }

  void SetIsAnimating(bool v) { mAnimating = v; }
  bool IsAnimating() const { return mAnimating; }
  void SetParticleCEXTValue(const rstl::string& name, int index, float value);
  void SetParticleEffectState(const rstl::string& name, const bool active, CStateManager& mgr);

  CAssetId GetSelfId() const { return mSelfId; }
  int GetCharacterIndex() const { return mCharIdx; }
  float GetAverageVelocity(int idx) const;

  const CBoolPOINode* GetBoolPOIList(int& count) const {
    count = mPassedBoolCount;
    return mBoolPOINodes.data();
  }
  const CInt32POINode* GetInt32POIList(int& count) const {
    count = mPassedIntCount;
    return mInt32POINodes.data();
  }
  const CParticlePOINode* GetParticlePOIList(int& count) const {
    count = mPassedParticleCount;
    return mParticlePOINodes.data();
  }
  const CSoundPOINode* GetSoundPOIList(int& count) const {
    count = mPassedSoundCount;
    return mSoundPOINodes.data();
  }
  CParticleDatabase& GetParticleDB() { return mParticleDB; }
  const CParticleDatabase& GetParticleDB() const { return mParticleDB; }
  // SetIsAnimating__9CAnimDataFb
  void SetAnimDir(EAnimDir dir) { mAnimDir = dir; }
  CAABox GetBoundingBox() const;
  CAABox GetBoundingBox(const CTransform4f& xf) const;
  CSegId GetLocatorSegId(const rstl::string& name) const;
  void ResetPOILists();
  // GetAverageVelocity__9CAnimDataCFi
  void AdvanceParticles(const CTransform4f&, float, const CVector3f&, CStateManager&);
  void PoseSkinnedModel(const CSkinnedModel&, const CPoseAsTransforms&,
                        const rstl::optional_object< CVertexMorphEffect >&, const float*) const;
  void DrawSkinnedModel(const CSkinnedModel&, const CModelFlags&) const;
  // InitializeCache__9CAnimDataFv
  // FreeCache__9CAnimDataFv
  void SetInfraModel(const TLockedToken< CModel >&, const TLockedToken< CSkinRules >&);
  void SetXRayModel(const TLockedToken< CModel >&, const TLockedToken< CSkinRules >&);
  void SubstituteModelData(const TLockedToken< CSkinnedModel >&);
  void AdvanceAnim(CCharAnimTime&, CVector3f&, CQuaternion&);
  CAdvancementDeltas Advance(float, const CVector3f&, CStateManager&, TAreaId, bool);
  CAdvancementDeltas AdvanceIgnoreParticles(float, CRandom16&, bool);
  CAdvancementDeltas DoAdvance(float, bool&, CRandom16&, bool);
  void SetAnimation(const CAnimPlaybackParms& parms, const bool noTrans);
  void GetAnimationPrimitives(const CAnimPlaybackParms& parms,
                              rstl::set< CPrimitive >& primsOut) const;
  static void PrimitiveSetToTokenVector(const rstl::set< CPrimitive >&, rstl::vector< CToken >&,
                                        bool);
  void BuildPose();
  // PreRender__9CAnimDataFv
  void SetupRender(const CSkinnedModel&, const rstl::optional_object< CVertexMorphEffect >&,
                   const float*) const;
  // Render__9CAnimDataCFRC13CSkinnedModelRC11CModelFlagsRCQ24rstl37optional_object<18CVertexMorphEffect>PCf
  void Render(const CSkinnedModel&, const CModelFlags&,
              const rstl::optional_object< CVertexMorphEffect >&, const float*) const;
  void RenderAuxiliary(const CFrustumPlanes&) const;
  void RecalcPoseBuilder(const CCharAnimTime*) const;
  float GetAnimationDuration(int animIn) const;
  float GetAnimTimeRemaining(const rstl::string& name) const;
  // IsAnimTimeRemaining__9CAnimDataCFfRCQ24rstl66basic_string<c,Q24rstl14char_traits<c>,Q24rstl17rmemory_allocator>
  bool IsAnimTimeRemaining(float, const rstl::string&) const;
  CTransform4f GetLocatorTransform(const rstl::string&, const CCharAnimTime*) const;
  CTransform4f GetLocatorTransform(CSegId, const CCharAnimTime*) const;
  void CalcPlaybackAlignmentParms(const CAnimPlaybackParms&,
                                  const rstl::ncrc_ptr< CAnimTreeNode >&);
  void SetRandomPlaybackRate(CRandom16& random);
  void SetPlaybackRate(float set);
  void MultiplyPlaybackRate(float scale);
  CCharAnimTime GetTimeOfUserEvent(EUserEventType type, const CCharAnimTime& time = CCharAnimTime::Infinity()) const;
  // GetAdvancementDeltas__9CAnimDataCFRC13CCharAnimTimeRC13CCharAnimTime
  void Touch(const CSkinnedModel&, int) const;
  void InitializeEffects(CStateManager&, TAreaId, const CVector3f&);
  // SetPhase__9CAnimDataFf -> SetPhase__11IAnimReaderFf
  void SetPhase(float ph);
  void AddAdditiveAnimation(uint idx, float weight, bool active, bool fadeOut);
  void DelAdditiveAnimation(uint idx);
  bool IsAdditiveAnimation(uint idx) const;
  const rstl::rc_ptr< CAnimTreeNode >& GetAdditiveAnimationTree(uint idx) const;
  const rstl::ncrc_ptr< CAnimTreeNode >& GetRootAnimationTree() const;
  // GetAnimationTree__9CAnimDataCFv
  // AnimationTree__9CAnimDataFv
  // IsAdditiveAnimation__9CAnimDataCFUi
  bool IsAdditiveAnimationAdded(uint idx) const;
  CAdvancementDeltas UpdateAdditiveAnims(float);
  CAdvancementDeltas AdvanceAdditiveAnims(float);
  static CAdvancementResults AdvanceAdditiveAnim(rstl::rc_ptr< CAnimTreeNode >&, CCharAnimTime);
  void AddAdditiveSegData(const CSegIdList&, CSegStatementSet&) const;
  int GetEventResourceIdForAnimResourceId(int id) const;
  rstl::rc_ptr< CAnimationManager > GetAnimationManager();
  // SetPoseValid__9CAnimDataFb

  float GetAdditiveAnimationWeight(uint idx);

  short GetCurrentAnimation() const { return mCurrentAnim; }
  const CCharacterInfo& GetCharacterInfo() const { return mCharInfo; }
  const CCharLayoutInfo* GetCharLayoutInfo() const { return *mLayoutData; }
  // GetDeltaRotation__9CAnimDataCFv
  // GetDeltaOffset__9CAnimDataCFv
  // IsDeltaOffsetInUse__9CAnimDataCFv
  // GetAdvancementDeltas__19CAdvancementResultsCFv
  // SetDeltaRotation__9CAnimDataFRC11CQuaternionb
  // SetDeltaOffset__9CAnimDataFRC9CVector3fb
  // SetDeltaOffsetInUse__9CAnimDataFv
  // IsDeltaRotationInUse__9CAnimDataCFv
  // IsDeltaOffsetPrimed__9CAnimDataCFv
  // GetAnimDir__9CAnimDataCFv
  // GetIsLoop__9CAnimDataCFv
  // IsAnimating__9CAnimDataCFv
  void SetPoseBuilderValid(bool valid) { mPoseBuilt = valid; }
  rstl::rc_ptr< CAnimationManager > GetAnimationManager() const;
  // GetPoseValid__9CAnimDataCFv
  // GetPoseBuilderValid__9CAnimDataCFv
  rstl::ncrc_ptr< CAnimSysContext > GetAnimSysContext() const;
  // CacheInt32PoiList__9CAnimDataFRC13CCharAnimTimeiRCQ24rstl25ncrc_ptr<13CAnimTreeNode>

  const rstl::optional_object< TLockedToken< CSkinnedModelWithAvgNormals > >& GetIceModel() const {
    return mIceModelData;
  }
  const CPASDatabase& GetPASDatabase() const { return mCharInfo.GetPASDatabase(); }
  // EnableLooping__9CAnimDataFb
  // GetSkinnedModel__9CAnimDataCFv
  // GetXRayModel__9CAnimDataCFv
  // GetInfraModel__9CAnimDataCFv
  const CPoseAsTransforms& GetPose() const { return mPose; }
  CHierarchyPoseBuilder& PoseBuilder() const { return mPoseBuilder; }
  float GetPlaybackRate() const { return mSpeedScale; }
  // Pose__9CAnimDataFv
  const CHierarchyPoseBuilder& GetPoseBuilder() const { return mPoseBuilder; }

  // CacheSoundPoiList__9CAnimDataFRCQ24rstl25ncrc_ptr<13CAnimTreeNode>RC13CCharAnimTimei
  // CacheParticlePoiList__9CAnimDataFRCQ24rstl25ncrc_ptr<13CAnimTreeNode>RC13CCharAnimTimei
  // CacheBoolPoiList__9CAnimDataFRCQ24rstl25ncrc_ptr<13CAnimTreeNode>RC13CCharAnimTimei
  // CacheInt32PoiList__9CAnimDataFRCQ24rstl25ncrc_ptr<13CAnimTreeNode>RC13CCharAnimTimei

  void SetParticleLightIdx(int lightIdx) { mParticleLightIdx = lightIdx; }

  static void InitializeCache();
  static void FreeCache();

private:
  TLockedToken< CCharacterFactory > mCharFactory;
  CCharacterInfo mCharInfo;
  TLockedToken< CCharLayoutInfo > mLayoutData;
  TLockedToken< CSkinnedModel > mModelData;
  rstl::optional_object< TLockedToken< CSkinnedModelWithAvgNormals > > mIceModelData;
  rstl::rc_ptr< CSkinnedModel > mXrayModel;
  rstl::rc_ptr< CSkinnedModel > mInfraModel;
  rstl::ncrc_ptr< CAnimSysContext > mAnimCtx;
  rstl::rc_ptr< CAnimationManager > mAnimMgr;
  EAnimDir mAnimDir;
  CAABox mAabb;
  CParticleDatabase mParticleDB;
  CAssetId mSelfId;
  CVector3f mAlignPos;
  CQuaternion mAlignRot;
  rstl::ncrc_ptr< CAnimTreeNode > mAnimRoot;
  rstl::rc_ptr< CTransitionManager > mTransMgr;
  float mSpeedScale;
  int mCharIdx;
  short mCurrentAnim;
  short mPadding;
  int mPassedBoolCount;
  int mPassedIntCount;
  int mPassedParticleCount;
  int mPassedSoundCount;
  int mParticleLightIdx;
  bool mAnimating : 1;
  bool mLoop : 1;
  bool mAligningPos : 1;
  bool x220_27_ : 1;
  bool x220_28_ : 1;
  bool mAnimationJustStarted : 1;
  bool mPoseBuilt : 1;
  bool mPoseCached : 1;
  CPoseAsTransforms mPose;
  mutable CHierarchyPoseBuilder mPoseBuilder;
  CAnimPlaybackParms mPlaybackParms;
  rstl::reserved_vector< rstl::pair< uint, CAdditiveAnimPlayback >, 8 > mAdditiveAnims;

  static rstl::reserved_vector< CBoolPOINode, 8 > mBoolPOINodes;
  static rstl::reserved_vector< CInt32POINode, 16 > mInt32POINodes;
  static rstl::reserved_vector< CParticlePOINode, 20 > mParticlePOINodes;
  static rstl::reserved_vector< CSoundPOINode, 20 > mSoundPOINodes;
  // in cpp -> rstl::reserved_vector< CInt32POINode, 16 > sInt32TransientCache;
};
CHECK_SIZEOF(CAnimData, 0x434 + 0x144)

#endif // _CANIMDATA
