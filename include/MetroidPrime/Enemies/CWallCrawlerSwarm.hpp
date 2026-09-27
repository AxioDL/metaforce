#ifndef _CWALLCRAWLERSWARM
#define _CWALLCRAWLERSWARM

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "WorldFormat/CCollisionSurface.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"

class CAnimRes;
class CMarkerGrid;
class CAreaCollisionCache;
class CElementGen;
class CGenDescription;

class CWallCrawlerSwarm : public CActor {
public:
  ~CWallCrawlerSwarm() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  void CalculateRenderBounds() override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  enum EFlavor { kF_Parasite, kF_Scarab, kF_Crab };
  class CBoid {
    friend class CWallCrawlerSwarm;

  public:
    CBoid(const CTransform4f& xf, uint index);
    bool GetActive() const { return mActive; }
    CVector3f GetTranslation() const { return mTransform.GetTranslation(); }
    const CTransform4f& GetTransform() const { return mTransform; }

  private:
    CTransform4f mTransform;
    CVector3f mVelocity;
    TUniqueId mTargetWaypoint;
    CColor mAmbientLighting;
    CBoid* mNext;
    float mTimeToDie;
    float mTimeToExplode;
    CCollisionSurface mSurface;
    float mHealth;
    uint mFramesNotOnSurface : 8;
    uint mIndex : 10;
    uint mRemainingLaunchNotOnSurfaceFrames : 8;
    bool mActive : 1;
    bool mInFrustum : 1;
    bool mLaunched : 1;
    bool mScarabExplodeTimerEnabled : 1;
    bool mNearPlayer : 1;
  };

  class CRepulsor {
    friend class CWallCrawlerSwarm;
    CVector3f mCenter;
    float mMagnitude;

  public:
    CRepulsor(CVector3f center, float magnitude) : mCenter(center), mMagnitude(magnitude) {}
  };

  CWallCrawlerSwarm(TUniqueId uid, bool active, const rstl::string& name, const CEntityInfo& info,
                    const CVector3f& boundingBoxExtent, const CTransform4f& xf, uint flavor,
                    const CAnimRes& animRes, uint launchAnim, uint attractAnim, uint particle1,
                    uint particle2, uint particle3, uint particle4, const CDamageInfo& crabDamage,
                    const CDamageInfo& scarabExplodeDamage, float crabDamageCooldown,
                    float boidRadius, float touchRadius, float playerTouchRadius, int numBoids,
                    int maxCreatedBoids, float animPlaybackSpeed, float separationRadius,
                    float cohesionMagnitude, float alignmentWeight, float separationMagnitude,
                    float moveToWaypointWeight, float attractionMagnitude, float attractionRadius,
                    float boidGenRate, int maxLaunches, float scarabBoxMargin,
                    float scarabScatterXYVelocity, float scarabTimeToExplode,
                    const CHealthInfo& healthInfo, const CDamageVulnerability& damageVulnerability,
                    int launchSfx, int scatterSfx, CActorParameters actParams);

  void FreezeCollision(const CMarkerGrid& grid, float duration);
  void ApplyRadiusDamage(CVector3f pos, const CDamageInfo& info, CStateManager& mgr);
  CVector3f GetLastKilledOffset() const { return mLastKilledOffset; }
  int GetCurrentLockOnId() const { return mLockOnIdx; }
  int GetBoidCount() const { return mBoids.size(); }
  bool GetLockOnLocationValid(int id) const {
    return id > -1 && id < mBoids.size() && mBoids[id].GetActive();
  }
  CVector3f GetLockOnLocation(int id) const { return mBoids[id].GetTranslation(); }

private:
  void AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which);
  void AddDoorRepulsors(CStateManager& mgr);
  void UpdateParticles(float dt);
  int GetLockOnIndex(const CStateManager& mgr) const;
  CAABox GetBoundingBox() const;
  TUniqueId GetWaypointForState(EScriptObjectState state, CStateManager& mgr);
  CVector3f ProjectVectorToPlane(const CVector3f& point, const CVector3f& normal);
  CVector3f ProjectPointToPlane(const CVector3f& point, const CVector3f& planePoint,
                                const CVector3f& normal);
  bool PointOnSurface(const CCollisionSurface& surface, const CVector3f& pos, const CPlane& plane);
  bool FindBestSurface(const CAreaCollisionCache& cache, CVector3f pos, float radius,
                       CCollisionSurface& out);
  CCollisionSurface FindBestCollisionInBox(CStateManager& mgr, const CVector3f& pos);
  void CreateBoid(CStateManager& mgr, int index);
  void ExplodeBoid(CBoid& boid, CStateManager& mgr);
  void SetExplodeTimers(const CVector3f& pos, float radius, float minTime, float maxTime);
  CBoid* GetListAt(const CVector3f& pos);
  void BuildBoidNearList(const CBoid& boid, float radius,
                         rstl::reserved_vector< CBoid*, 50 >& nearList);
  void ApplySeparation(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
                       CVector3f& ahead);
  void ApplySeparation(CBoid& boid, const CVector3f& pos, float radius, float magnitude,
                       CVector3f& ahead);
  void ScatterScarabBoid(CBoid& boid, CStateManager& mgr);
  void MoveToWayPoint(CBoid& boid, CStateManager& mgr, CVector3f& ahead);
  void ApplyCohesion(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
                     CVector3f& ahead);
  void ApplyCohesion(CBoid& boid, const CVector3f& pos, float radius, float magnitude,
                     CVector3f& ahead);
  void ApplyAlignment(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
                      CVector3f& ahead);
  void ApplyAttraction(CBoid& boid, const CVector3f& pos, float radius, float magnitude,
                       CVector3f& ahead);
  void UpdateBoid(CAreaCollisionCache& cache, CStateManager& mgr, float dt, CBoid& boid);
  void LaunchBoid(CBoid& boid, const CVector3f& dir);
  void AddParticle(const CTransform4f& xf);
  void KillBoid(CBoid& boid, CStateManager& mgr, float deathRattleChance, float deadChance);
  void UpdatePartition();
  CVector3f FindClosestCell(const CVector3f& pos) const;
  void UpdateEffects(CStateManager& mgr, CAnimData& animData, int volume);
  CAABox BoxForPosition(int x, int y, int z, float margin) const;
  void RenderParticles() const;
  CColor SoftwareLight(const CStateManager& mgr, const CAABox& bounds) const;
  void HardwareLight(const CStateManager& mgr, const CAABox& bounds) const;
  void RenderBoid(CBoid* boid, uint& drawMask, bool thermalHot, const CModelFlags& flags) const;

  CAABox mAabox;
  int mThinkCounter;
  float mOccludedTimer;
  rstl::vector< CBoid > mBoids;
  CVector3f mBoundingBoxExtent;
  mutable CVector3f mLastOrbitPosition;
  CVector3f mLastKilledOffset;
  float mSeparationRadius;
  float mCohesionMagnitude;
  float mAlignmentWeight;
  float mSeparationMagnitude;
  float mMoveToWaypointWeight;
  float mAttractionMagnitude;
  float mAttractionRadius;
  float mScarabScatterXYVelocity;
  float mScarabTimeToExplode;
  float mAnimPlaybackSpeed;
  float mWaypointGoalRadius;
  rstl::reserved_vector< CBoid*, 125 > mPartitionedBoidLists;
  CBoid* mOutlierBoidList;
  float mBoidGenRate;
  float mBoidGenCooldownTimer;
  float mCrabDamageCooldownTimer;
  float mCrabDamageCooldown;
  float mBoidRadius;
  float mTouchRadius;
  float mScarabBoxMargin;
  float mPlayerTouchRadius;
  CDamageInfo mCrabDamage;
  CDamageInfo mScarabExplodeDamage;
  CHealthInfo mHealthInfo;
  CDamageVulnerability mDamageVulnerability;
  int mLockOnIdx;
  rstl::reserved_vector< rstl::auto_ptr< float >, 10 > mPosWorkspaces;
  rstl::reserved_vector< float*, 10 > mNrmWorkspaces;
  rstl::reserved_vector< rstl::ncrc_ptr< CModelData >, 10 > mModelDatas;
  CModelData::EWhichModel mWhichModel;
  rstl::vector< CRepulsor > mDoorRepulsors;
  rstl::reserved_vector< TLockedToken< CGenDescription >, 4 > mParticleDescs;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 4 > mParticleGens;
  int mNumBoids;
  int mMaxCreatedBoids;
  int mCreatedBoids;
  int mMaxLaunches;
  EFlavor mFlavor;
  ushort mLaunchSfx;
  ushort mScatterSfx;
  bool mEnableLighting : 1;
  bool mUseSoftwareLight : 1;
  bool mModelAssetDirty : 1;
};
NESTED_CHECK_SIZEOF(CWallCrawlerSwarm, CBoid, 0x84)
CHECK_SIZEOF(CWallCrawlerSwarm, (VERSION >= VERSION_GM8E_02 ? 0x578 : 0x568))

#endif // _CWALLCRAWLERSWARM
