#ifndef _CACTORMODELPARTICLES
#define _CACTORMODELPARTICLES

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CActor;
class CEntity;
class CElementGen;
class CElectricDescription;
class CGenDescription;
class CParticleElectric;
class CRainSplashGenerator;
class CStateManager;
class CTexture;

class CActorModelParticles {
public:
  enum ESystemTypes {
    kST_OnFire,
    kST_Ice,
    kST_Ash,
    kST_FirePop,
    kST_Electric,
    kST_IcePop,
  };

  struct CSystem {
    rstl::vector< CToken > mTokens;
    int mRefCount;
    bool mLoaded;

    explicit CSystem(const char* name);

    void Update();
    void Unlock();
    void Lock();
    void DelRef();
    void AddRef();
  };

  class CItem {
    friend class CActorModelParticles;

  private:
    TUniqueId mId;
    TAreaId mAreaId;
    rstl::reserved_vector< rstl::pair< rstl::auto_ptr< CElementGen >, uint >, 8 > mOnFireGens;
    float mOnFireDelayTimer;
    bool mOnFire;
    CSfxHandle mSfx;
    rstl::auto_ptr< CElementGen > mAshGen;
    int mAshPointIterator;
    int mAshMaxParticles;
    uint mAshSeed;
    rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 4 > mIceGens;
    int mIcePointIterator;
    uint mIceSeed;
    rstl::auto_ptr< CElementGen > mFirePopGen;
    rstl::auto_ptr< CParticleElectric > mElectricGen;
    int mElectricPointIterator;
    uint mElectricSeed;
    CColor mElectricColor;
    rstl::auto_ptr< CRainSplashGenerator > mRainSplashGen;
    CToken mAshy;
    rstl::auto_ptr< CElementGen > mIcePopGen;
    CVector3f mParticleOffsetScale;
    CTransform4f mIceXf;
    CActorModelParticles* mParent;
    mutable bool mThermalCold : 1;
    mutable bool mThermalHot : 1;
    float mRemTime;
    mutable uchar mLockDeps;

    bool UpdateOnFire(float dt, CActor* actor, CStateManager& mgr);
    bool UpdateAshGen(float dt, const CActor* actor, CStateManager& mgr);
    bool UpdateIcePop(float dt, const CActor* actor);
    bool UpdateFirePop(float dt, const CActor* actor);
    bool UpdateElectric(float dt, const CActor* actor, CStateManager& mgr);
    bool UpdateIce(float dt, const CActor* actor, CStateManager& mgr);
    bool UpdateRainSplash(float dt, const CActor* actor, CStateManager& mgr);
    bool UpdateBurn(float dt, const CActor* actor, CStateManager& mgr);
    void UseType(ESystemTypes dep);

  public:
    CItem(const CEntity& ent, CActorModelParticles& parent);
    ~CItem();

    void GeneratePoints(const CVector3f* vertices, const CVector3f* normals, int count);
    bool Update(float dt, CStateManager& mgr);
    void DontUseType(ESystemTypes dep);
  };

  CActorModelParticles();

  CTexture* GetAshyTexture(const CActor& actor) const;
  void StartBurnDeath(CActor& actor);
  void Render(const CStateManager& mgr, const CActor& actor) const;
  void AddStragglersToRenderer(const CStateManager& mgr) const;
  rstl::list< CItem >::iterator FindSystem(TUniqueId uid);
  rstl::list< CItem >::const_iterator FindSystem(TUniqueId uid) const;
  rstl::list< CItem >::iterator FindOrCreateSystem(CActor& actor);
  void SetupHook(TUniqueId uid) const;
  static void PointGenerator(void* context, const CVector3f* vertices, const CVector3f* normals,
                             int count);
  void RemoveRainSplashGenerator(CActor& actor);
  void AddRainSplashGenerator(CActor& actor, CStateManager& mgr, int maxSplashes, int genRate,
                              float minZ);
  void StopFire(CActor& actor);
  void LightDudeOnFire(CActor& actor);
  void StopElectric(CActor& actor);
  void StartElectric(CActor& actor);
  void StartIce(CActor& actor);
  void DoIcePop(CActor& actor);
  void DoFirePop(CActor& actor);
  void StartAsh(CActor& actor);
  void Update(float dt, CStateManager& mgr);

private:
  friend class CItem;

  rstl::list< CItem > mItems;
  TToken< CGenDescription > mOnFire;
  TToken< CGenDescription > mAsh;
  TToken< CGenDescription > mIceBreak;
  TToken< CGenDescription > mFirePop;
  TToken< CGenDescription > mIcePop;
  TToken< CElectricDescription > mElectric;
  CToken mAshy;
  rstl::reserved_vector< CSystem, 6 > mDgrps;
  uchar mLoadingDeps;
  uchar mJustLoadedDeps;
  uchar mLoadedDeps;

  void UpdateSystemTypes();
  void DelTypeRef(ESystemTypes dep);
  void AddTypeRef(ESystemTypes dep);
  void InitializeSystemTypes();

  CElementGen* MakeOnFireGen();
  CParticleElectric* MakeElectricGen();
  CElementGen* MakeIcePopGen();
  CElementGen* MakeFirePopGen();
  CElementGen* MakeAshGen();
  CElementGen* MakeIceGen();
};
NESTED_CHECK_SIZEOF(CActorModelParticles, CSystem, 0x18)
NESTED_CHECK_SIZEOF(CActorModelParticles, CItem, 0x138)
CHECK_SIZEOF(CActorModelParticles, 0xe8);

#endif // _CACTORMODELPARTICLES
