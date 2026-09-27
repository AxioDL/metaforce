#ifndef _CFISHCLOUD
#define _CFISHCLOUD

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CFishCloud : public CActor {
public:
  ~CFishCloud();
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void Render(const CStateManager& mgr) const override;
  void CalculateRenderBounds() override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& other, CStateManager& mgr) override;

  CFishCloud(TUniqueId uid, bool active, const rstl::string& name, const CEntityInfo& info,
             const CVector3f& scale, const CTransform4f& xf, const CModelData& mData,
             const CAnimRes& aRes, int numBoids, float speed, float separationRadius,
             float cohesionMagnitude, float alignmentWeight, float separationMagnitude,
             float weaponRepelMagnitude, float playerRepelMagnitude, float containmentMagnitude,
             float scatterVel, float maxScatterAngle, float weaponRepelDampingSpeed,
             float playerRepelDampingSpeed, float containmentRadius, int updateShift,
             const CColor& color, bool killable, float weaponKillRadius, CAssetId part1,
             int partCount1, CAssetId part2, int partCount2, CAssetId part3, int partCount3,
             CAssetId part4, int partCount4, int deathSfx, bool repelFromThreats,
             bool hotInThermal);

  bool AddRepulsor(TUniqueId source, const bool swirl, float radius, float priority);
  bool AddAttractor(TUniqueId source, const bool swirl, float radius, float priority);
  void RemoveRepulsor(TUniqueId source);
  void RemoveAttractor(TUniqueId source);

private:
  class CModifierSource {
    TUniqueId mSource;
    float mRadius;
    float mPriority;
    bool mIsRepulsor;
    bool mIsSwirl;

  public:
    CModifierSource(const TUniqueId& source, bool repulsor, bool swirl, float radius,
                    float priority);
    void SetAffectPriority(float priority) { mPriority = priority; }
    void SetAffectRadius(float radius) { mRadius = radius; }
    float GetAffectPriority() const { return mPriority; }
    float GetAffectRadius() const { return mRadius; }
    bool IsRepulsor() const { return mIsRepulsor; }
    bool IsSwirl() const { return mIsSwirl; }
    const TUniqueId& GetSource() const { return mSource; }
    bool operator<(const CModifierSource& other) const;
  };
  typedef rstl::vector<CModifierSource> TModifierSourceVector;
  class CBoid {
    friend class CFishCloud;
    CVector3f mPos;
    CVector3f mVel;
    float mScale;
    CBoid* mNext;
    bool mActive;

  public:
    CBoid(const CVector3f& pos, const CVector3f& vel, float scale);
    CVector3f& Translation() { return mPos; }
    const CVector3f& GetTranslation() const { return mPos; }
  };
  typedef rstl::vector<CBoid*> TBoidPtrVector;
  typedef rstl::vector<CBoid> TBoidVector;



  void InitAnimBoids(CStateManager& mgr, CModelData::EWhichModel which);
  CAABox GetBoundingBox() const;
  CAABox GetUntransformedBoundingBox() const;
  bool PointInBox(const CAABox& aabb, const CVector3f& point) const;
  CPlane FindClosestPlane(const CAABox& aabb, const CVector3f& point) const;
  void PlaceBoid(CStateManager& mgr, CBoid& boid, const CAABox& aabb);
  void CreatePartitionList();
  void RenderBoid(int idx, const CBoid& boid, uint& drawMask, const bool thermalHot,
                  const CModelFlags& flags) const;
  void KillBoid(CBoid& boid);
  void UpdatePartitionList();
  CBoid* GetListAt(const CVector3f& pos);
  void BuildBoidNearList(const CVector3f& pos, float radius,
                         rstl::reserved_vector< CBoid*, 25 >& nearList);
  void OldBuildBoidNearList(const CVector3f& pos, float radius,
                            rstl::reserved_vector< CBoid*, 25 >& nearList);
  void ApplyRotation(CBoid& boid, float magnitude, const CVector3f& point, float radius,
                     bool clockwise);
  void ApplyAlignment(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList);
  void ApplyWander(CStateManager& mgr, CBoid& boid);
  void ApplyCohesion(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList);
  void ApplyCohesion(CBoid& boid, const CVector3f& point, float radius, float magnitude);
  void ApplySeparation(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList);
  void ApplySeparation(CBoid& boid, const CVector3f& point, float radius, float magnitude);
  void ApplyAttraction(CBoid& boid, const CVector3f& point, float radius, float magnitude);
  void ApplyRepulsion(CBoid& boid, const CVector3f& point, float radius, float magnitude);
  void ApplyContainment(CBoid& boid, const CAABox& aabb);
  void AddParticles(const CVector3f& pos);
  void UpdateParticles(float dt);
  void RenderParticles() const;

  TBoidVector mBoids;
  TBoidPtrVector mBoidPartitionLists;
  TModifierSourceVector mModifierSources;
  int mThinkCounter;
  int mUpdateMask;
  CVector3f mScale;
  float mRandomMovementTimer;
  float mSpeed;
  int mNumBoids;
  float mSeparationRadius;
  float mCohesionMagnitude;
  float mAlignmentWeight;
  float mSeparationMagnitude;
  float mWeaponRepelMagnitude;
  float mPlayerRepelMagnitude;
  float mScatterVel;
  float mMaxScatterAngle;
  float mContainmentMagnitude;
  float mPlayerRepelDampingSpeed;
  float mWeaponRepelDampingSpeed;
  float mPlayerRepelDamping;
  float mWeaponRepelDamping;
  CColor mColor;
  float mWeaponKillRadius;
  float mContainmentRadius;
  rstl::reserved_vector< rstl::auto_ptr< float >, 4 > mPosWorkspaces;
  rstl::reserved_vector< float*, 4 > mNrmWorkspaces;
  rstl::reserved_vector< rstl::ncrc_ptr< CModelData >, 4 > mModels;
  rstl::reserved_vector< TLockedToken< CGenDescription >, 4 > mParticleDescs;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 4 > mParticleGens;
  rstl::reserved_vector< int, 4 > mDeathParticleCounts;
  CModelData::EWhichModel mWhichModel;
  ushort mDeathSfx;
  CVector3f mPartitionPitch;
  CVector3f mOoPartitionPitch;
  bool mRandomMovement : 1;
  bool mWorldSpace : 1;
  bool mEnableWeaponRepelDamping : 1;
  bool mValidModel : 1;
  bool mKillable : 1;
  bool mRepelFromThreats : 1;
  bool mEnablePlayerRepelDamping : 1;
  bool mUpdateWithoutPartitions : 1;
};
CHECK_SIZEOF(CFishCloud, (VERSION >= VERSION_GM8E_02 ? 0x268 : 0x258))

#endif // _CFISHCLOUD
