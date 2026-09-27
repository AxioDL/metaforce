#ifndef _CSNAKEWEEDSWARM
#define _CSNAKEWEEDSWARM

#include "Kyoto/Math/CVector2i.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "rstl/rc_ptr.hpp"

class CAnimRes;
class CElementGen;
class CGenDescription;

class CSnakeWeedSwarm : public CActor {
public:
  ~CSnakeWeedSwarm() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  enum ESnakeWeedBoidState { kSWBS_Raised, kSWBS_Raising, kSWBS_Lowered, kSWBS_Lowering };
  enum EBoidPlacement { kBP_None, kBP_Ready, kBP_Invalid, kBP_Placed };

  class CBoid {
    CVector3f mPos;
    ESnakeWeedBoidState mState;
    float mTimeOut;
    float mDepth;
    float mSpeed;
    float x1c_;
    float mScale;

  public:
    CBoid(const CVector3f& pos, float depth, float speed, float scale);
    void SetBoidState(ESnakeWeedBoidState state);
    ESnakeWeedBoidState GetBoidState() const;
    const CVector3f& GetLocation() const;
    float GetTimeOut() const;
    void SetTimeOut(float time);
    float GetDepth() const;
    void SetDepth(float depth);
    float GetSpeed() const;
    void SetSpeed(float speed);
    float GetSize() const;
  };

  CSnakeWeedSwarm(TUniqueId uid, bool active, const rstl::string& name, const CEntityInfo& info,
                  const CVector3f& pos, const CVector3f& scale, const CAnimRes& animRes,
                  const CActorParameters& actParms, float spacing, float height, float variance,
                  float weaponDamageRadius, float maxPlayerDistance, float loweredTime,
                  float loweredTimeVariation, float maxDepth, float speed, float speedVariation,
                  float slopeAngle, float scaleMin, float scaleMax, float distanceBelowGround,
                  const CDamageInfo& damageInfo, float f15, uint sfxId1, uint sfxId2, uint sfxId3,
                  uint particle1, uint particleCount, uint particle2, float f16);
  void ApplyRadiusDamage(CVector3f pos, const CDamageInfo& info, CStateManager& mgr);
  void ScareSnakeWeeds(CStateManager& mgr, const CVector3f& pos, float radius);
  float GetWeaponDamageRadius() const { return mWeaponDamageRadius; }

private:
  void InitAnimBoids(CStateManager& mgr, CModelData::EWhichModel which);
  CAABox GetBoundingBox() const;
  void UpdateTouchBounds();
  void RenderBoid(uint index, const CBoid& boid, uint& posesToBuild) const;
  void CreateSwarm(CStateManager& mgr);
  CVector2i GetGridPosition(CVector3f pos);
  int GetGridWidth();
  int GetGridDepth();
  void PushBackPoint(CVector3f pos);
  void FloodFill(CStateManager& mgr, int count);
  float GetXVariance(const CVector3f& pos);
  float GetYVariance(const CVector3f& pos);
  bool PlaceSnakeWeedAtPoint(const CVector3f& pos, CStateManager& mgr);
  void AddContinuousParticles(const CVector3f& pos);
  void AddRetreatParticles(const CVector3f& pos);

  CVector3f mScale;
  float mBoidSpacing;
  float mHeight;
  float mVariance;
  float mWeaponDamageRadius;
  float mMaxPlayerDistance;
  float mLoweredTime;
  float mLoweredTimeVariation;
  float mMaxDepth;
  float mSpeed;
  float mSpeedVariation;
  float mCosSlopeAngle;
  float mScaleMin;
  float mScaleMax;
  float mDistanceBelowGround;
  uint x12c_;
  rstl::vector< CBoid > mBoids;
  bool mHasGround : 1;
  bool mModelAssetDirty : 1;
  bool mPlayerTouching : 1;
  CAABox mTouchBounds;
  CDamageInfo mDamageInfo;
  rstl::reserved_vector< rstl::auto_ptr< float >, 4 > mPosWorkspaces;
  rstl::reserved_vector< float*, 4 > mNrmWorkspaces;
  rstl::reserved_vector< rstl::ncrc_ptr< CModelData >, 4 > mModelData;
  CModelData::EWhichModel mWhich;
  rstl::single_ptr< rstl::vector< CVector3f > > mBoidPositions;
  rstl::single_ptr< rstl::vector< EBoidPlacement > > mBoidPlacement;
  ushort mSfx1;
  ushort mSfx2;
  ushort mSfx3;
  CSfxHandle mSfxHandle;
  rstl::auto_ptr< TLockedToken< CGenDescription > > mParticleGenDescA;
  rstl::auto_ptr< TLockedToken< CGenDescription > > mParticleGenDescB;
  rstl::auto_ptr< CElementGen > mParticleGen1;
  rstl::auto_ptr< CElementGen > mParticleGen2;
  uint mParticleCount;
  float x200_;
  float mParticleTimer;
};
CHECK_SIZEOF(CSnakeWeedSwarm, (VERSION >= VERSION_GM8E_02 ? 0x218 : 0x208))
NESTED_CHECK_SIZEOF(CSnakeWeedSwarm, CBoid, 0x24)

#endif // _CSNAKEWEEDSWARM
