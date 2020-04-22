#pragma once

#include <memory>
#include <set>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Character/CAdditiveAnimPlayback.hpp"
#include "Runtime/Character/CAnimPlaybackParms.hpp"
#include "Runtime/Character/CCharLayoutInfo.hpp"
#include "Runtime/Character/CCharacterFactory.hpp"
#include "Runtime/Character/CCharacterInfo.hpp"
#include "Runtime/Character/CHierarchyPoseBuilder.hpp"
#include "Runtime/Character/CParticleDatabase.hpp"
#include "Runtime/Character/CPoseAsTransforms.hpp"
#include "Runtime/Character/IAnimReader.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CQuaternion.hpp>
#include <zeus/CVector3f.hpp>

enum class EUserEventType {
  Projectile = 0,
  EggLay = 1,
  LoopedSoundStop = 2,
  AlignTargetPos = 3,
  AlignTargetRot = 4,
  ChangeMaterial = 5,
  Delete = 6,
  GenerateEnd = 7,
  DamageOn = 8,
  DamageOff = 9,
  AlignTargetPosStart = 10,
  DeGenerate = 11,
  Landing = 12,
  TakeOff = 13,
  FadeIn = 14,
  FadeOut = 15,
  ScreenShake = 16,
  BeginAction = 17,
  EndAction = 18,
  BecomeRagDoll = 19,
  IkLock = 20,
  IkRelease = 21,
  BreakLockOn = 22,
  BecomeShootThrough = 23,
  RemoveCollision = 24,
  ObjectPickUp = 25,
  ObjectDrop = 26,
  EventStart = 27,
  EventStop = 28,
  Activate = 29,
  Deactivate = 30,
  SoundPlay = 31,
  SoundStop = 32,
  EffectOn = 33,
  EffectOff = 34
};

namespace urde {
class CAnimTreeNode;
class CAnimationManager;
class CBoolPOINode;
class CCharAnimTime;
class CCharLayoutInfo;
class CInt32POINode;
class CModel;
class CMorphableSkinnedModel;
class CParticlePOINode;
class CPrimitive;
class CRandom16;
class CSegIdList;
class CSegStatementSet;
class CSkinRules;
class CSoundPOINode;
class CStateManager;
class CTransitionManager;
class CVertexMorphEffect;
class IAnimReader;
class IMetaAnim;

struct CAnimSysContext;
struct CModelFlags;
struct SAdvancementDeltas;
struct SAdvancementResults;

class CAnimData {
  friend class CModelData;
  friend class CActor;
  friend class CPlayerGun;
  friend class CGrappleArm;
  friend class CWallCrawlerSwarm;

public:
  enum class EAnimDir { Forward, Backward };

private:
  TLockedToken<CCharacterFactory> x0_charFactory;
  CCharacterInfo xc_charInfo;
  TLockedToken<CCharLayoutInfo> xcc_layoutData;
  TLockedToken<CSkinnedModel> xd8_modelData;
  TLockedToken<CMorphableSkinnedModel> xe4_iceModelData;
  std::shared_ptr<CSkinnedModel> xf4_xrayModel;
  std::shared_ptr<CSkinnedModel> xf8_infraModel;
  std::shared_ptr<CAnimSysContext> xfc_animCtx;
  std::shared_ptr<CAnimationManager> x100_animMgr;
  EAnimDir x104_animDir = EAnimDir::Forward;
  zeus::CAABox x108_aabb;
  CParticleDatabase x120_particleDB;
  CAssetId x1d8_selfId;
  zeus::CVector3f x1dc_alignPos;
  zeus::CQuaternion x1e8_alignRot;
  std::shared_ptr<CAnimTreeNode> x1f8_animRoot;
  std::shared_ptr<CTransitionManager> x1fc_transMgr;

  float x200_speedScale = 1.f;
  s32 x204_charIdx;
  s32 x208_defaultAnim;
  u32 x20c_passedBoolCount = 0;
  u32 x210_passedIntCount = 0;
  u32 x214_passedParticleCount = 0;
  u32 x218_passedSoundCount = 0;
  s32 x21c_particleLightIdx = 0;
  bool x220_24_animating : 1 = false;
  bool x220_25_loop : 1 = false;
  bool x220_26_aligningPos : 1 = false;
  bool x220_27_ : 1 = false;
  bool x220_28_ : 1 = false;
  bool x220_29_animationJustStarted : 1 = false;
  bool x220_30_poseBuilt : 1 = false;
  bool x220_31_poseCached : 1 = false;
  CPoseAsTransforms x224_pose;
  CHierarchyPoseBuilder x2fc_poseBuilder;

  CAnimPlaybackParms x40c_playbackParms;
  rstl::reserved_vector<std::pair<s32, CAdditiveAnimPlayback>, 8> x434_additiveAnims;

  static rstl::reserved_vector<CBoolPOINode, 8> g_BoolPOINodes;
  static rstl::reserved_vector<CInt32POINode, 16> g_Int32POINodes;
  static rstl::reserved_vector<CParticlePOINode, 20> g_ParticlePOINodes;
  static rstl::reserved_vector<CSoundPOINode, 20> g_SoundPOINodes;
  static rstl::reserved_vector<CInt32POINode, 16> g_TransientInt32POINodes;

  int m_drawInstCount;

public:
  CAnimData(CAssetId, const CCharacterInfo& character, int defaultAnim, int charIdx, bool loop,
            TLockedToken<CCharLayoutInfo> layout, TToken<CSkinnedModel> model,
            const std::optional<TToken<CMorphableSkinnedModel>>& iceModel, const std::weak_ptr<CAnimSysContext>& ctx,
            std::shared_ptr<CAnimationManager> animMgr, std::shared_ptr<CTransitionManager> transMgr,
            TLockedToken<CCharacterFactory> charFactory, int drawInstCount);

  void SetParticleEffectState(std::string_view effectName, bool active, CStateManager& mgr);
  void InitializeEffects(CStateManager& mgr, TAreaId aId, const zeus::CVector3f& scale);
  CAssetId GetEventResourceIdForAnimResourceId(CAssetId id) const;
  void AddAdditiveSegData(const CSegIdList& list, CSegStatementSet& stSet);
  static SAdvancementResults AdvanceAdditiveAnim(std::shared_ptr<CAnimTreeNode>& anim, const CCharAnimTime& time);
  SAdvancementDeltas AdvanceAdditiveAnims(float dt);
  SAdvancementDeltas UpdateAdditiveAnims(float dt);
  bool IsAdditiveAnimation(s32 idx) const;
  bool IsAdditiveAnimationAdded(s32 idx) const;
  const std::shared_ptr<CAnimTreeNode>& GetRootAnimationTree() const { return x1f8_animRoot; }
  const std::shared_ptr<CAnimTreeNode>& GetAdditiveAnimationTree(s32 idx) const;
  bool IsAdditiveAnimationActive(s32 idx) const;
  void DelAdditiveAnimation(s32 idx);
  void AddAdditiveAnimation(s32 idx, float weight, bool active, bool fadeOut);
  float GetAdditiveAnimationWeight(s32 idx) const;
  std::shared_ptr<CAnimationManager> GetAnimationManager();
  const CCharacterInfo& GetCharacterInfo() const { return xc_charInfo; }
  const CCharLayoutInfo& GetCharLayoutInfo() const { return *xcc_layoutData.GetObj(); }
  void SetPhase(float ph);
  void Touch(const CSkinnedModel& model, int shaderIdx) const;
  SAdvancementDeltas GetAdvancementDeltas(const CCharAnimTime& a, const CCharAnimTime& b) const;
  CCharAnimTime GetTimeOfUserEvent(EUserEventType type, const CCharAnimTime& time) const;
  void MultiplyPlaybackRate(float mul);
  void SetPlaybackRate(float set);
  void SetRandomPlaybackRate(CRandom16& r);
  void CalcPlaybackAlignmentParms(const CAnimPlaybackParms& parms, const std::shared_ptr<CAnimTreeNode>& node);
  zeus::CTransform GetLocatorTransform(CSegId id, const CCharAnimTime* time) const;
  zeus::CTransform GetLocatorTransform(std::string_view name, const CCharAnimTime* time) const;
  bool IsAnimTimeRemaining(float rem, std::string_view name) const;
  float GetAnimTimeRemaining(std::string_view name) const;
  float GetAnimationDuration(int animIn) const;
  bool GetIsLoop() const { return x220_25_loop; }
  void EnableLooping(bool val) {
    x220_25_loop = val;
    x220_24_animating = true;
  }
  void EnableAnimation(bool val) { x220_24_animating = val; }
  bool IsAnimating() const { return x220_24_animating; }
  void SetAnimDir(EAnimDir dir) { x104_animDir = dir; }
  std::shared_ptr<CAnimSysContext> GetAnimSysContext() const;
  std::shared_ptr<CAnimationManager> GetAnimationManager() const;
  void RecalcPoseBuilder(const CCharAnimTime* time);
  void RenderAuxiliary(const zeus::CFrustum& frustum) const;
  void Render(CSkinnedModel& model, const CModelFlags& drawFlags,
              const std::optional<CVertexMorphEffect>& morphEffect, const float* morphMagnitudes);
  void SetupRender(CSkinnedModel& model, const CModelFlags& drawFlags,
                   const std::optional<CVertexMorphEffect>& morphEffect, const float* morphMagnitudes);
  static void DrawSkinnedModel(CSkinnedModel& model, const CModelFlags& flags);
  void PreRender();
  void BuildPose();
  const CPoseAsTransforms& GetPose() const { return x224_pose; }
  static void PrimitiveSetToTokenVector(const std::set<CPrimitive>& primSet, std::vector<CToken>& tokensOut,
                                        bool preLock);
  void GetAnimationPrimitives(const CAnimPlaybackParms& parms, std::set<CPrimitive>& primsOut) const;
  void SetAnimation(const CAnimPlaybackParms& parms, bool noTrans);
  SAdvancementDeltas DoAdvance(float dt, bool& suspendParticles, CRandom16& random, bool advTree);
  SAdvancementDeltas Advance(float dt, const zeus::CVector3f& scale, CStateManager& stateMgr, TAreaId aid, bool advTree);
  SAdvancementDeltas AdvanceIgnoreParticles(float dt, CRandom16& random, bool advTree);
  void AdvanceAnim(CCharAnimTime& time, zeus::CVector3f& offset, zeus::CQuaternion& quat);
  void SetXRayModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules);
  std::shared_ptr<CSkinnedModel> GetXRayModel() const { return xf4_xrayModel; }
  void SetInfraModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules);
  std::shared_ptr<CSkinnedModel> GetInfraModel() const { return xf8_infraModel; }
  TLockedToken<CSkinnedModel>& GetModelData() { return xd8_modelData; }
  const TLockedToken<CSkinnedModel>& GetModelData() const { return xd8_modelData; }

  static void PoseSkinnedModel(CSkinnedModel& model, const CPoseAsTransforms& pose, const CModelFlags& drawFlags,
                               const std::optional<CVertexMorphEffect>& morphEffect,
                               const float* morphMagnitudes);
  void AdvanceParticles(const zeus::CTransform& xf, float dt, const zeus::CVector3f&, CStateManager& stateMgr);
  float GetAverageVelocity(int animIn) const;
  void ResetPOILists();
  CSegId GetLocatorSegId(std::string_view name) const;
  zeus::CAABox GetBoundingBox(const zeus::CTransform& xf) const;
  zeus::CAABox GetBoundingBox() const;
  void SubstituteModelData(const TCachedToken<CSkinnedModel>& model);
  static void FreeCache();
  static void InitializeCache();
  CHierarchyPoseBuilder& PoseBuilder() { return x2fc_poseBuilder; }
  const CHierarchyPoseBuilder& GetPoseBuilder() const { return x2fc_poseBuilder; }
  const CParticleDatabase& GetParticleDB() const { return x120_particleDB; }
  CParticleDatabase& GetParticleDB() { return x120_particleDB; }
  void SetParticleCEXTValue(std::string_view name, int idx, float value);

  float GetSpeedScale() const { return x200_speedScale; }
  u32 GetPassedBoolPOICount() const { return x20c_passedBoolCount; }
  u32 GetPassedIntPOICount() const { return x210_passedIntCount; }
  u32 GetPassedParticlePOICount() const { return x214_passedParticleCount; }
  u32 GetPassedSoundPOICount() const { return x218_passedSoundCount; }

  s32 GetCharacterIndex() const { return x204_charIdx; }
  u16 GetDefaultAnimation() const { return x208_defaultAnim; }
  TLockedToken<CMorphableSkinnedModel>& GetIceModel() { return xe4_iceModelData; }
  const TLockedToken<CMorphableSkinnedModel>& GetIceModel() const { return xe4_iceModelData; }
  void SetParticleLightIdx(s32 idx) { x21c_particleLightIdx = idx; }

  void MarkPoseDirty() { x220_30_poseBuilt = false; }
};

} // namespace urde
