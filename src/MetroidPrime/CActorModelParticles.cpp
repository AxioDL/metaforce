#include "MetroidPrime/CActorModelParticles.hpp"

#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CRainSplashGenerator.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/SFX/Misc.h"

static const char* const skParticleNames[] = {
    "Effect_OnFire",  "Effect_IceBreak", "Effect_Ash",
    "Effect_FirePop", "Effect_Electric", "Effect_IcePop",
};

static bool IsMediumOrLarge(const CActor& actor) {
  if (const CPatterned* patterned = TCastToConstPtr< CPatterned >(&actor)) {
    return patterned->GetKnockBackCtrl().GetCreatureSize() != kCS_Small;
  }
  return false;
}

CActorModelParticles::CSystem::CSystem(const char* name) : mRefCount(0), mLoaded(false) {
  TLockedToken< CDependencyGroup > group = gpSimplePool->GetObj(name);
  const rstl::vector< SObjectTag >& tags = group->GetObjectTagVector();
  mTokens.reserve(tags.size());
  for (AUTO(it, tags.begin()); it != tags.end(); ++it) {
    mTokens.push_back(gpSimplePool->GetObj(*it));
  }
}

void CActorModelParticles::CSystem::AddRef() {
  ++mRefCount;
  if (mRefCount == 1) {
    Lock();
  }
}

void CActorModelParticles::CSystem::DelRef() {
  --mRefCount;
  if (mRefCount <= 0) {
    Unlock();
  }
}

void CActorModelParticles::CSystem::Lock() {
  bool loading = false;
  for (AUTO(it, mTokens.begin()); it != mTokens.end(); ++it) {
    if (!it->IsLocked()) {
      it->Lock();
      loading = true;
    } else if (!it->IsLoaded()) {
      loading = true;
    }
  }
  if (!loading) {
    mLoaded = true;
  }
}

void CActorModelParticles::CSystem::Unlock() {
  for (AUTO(it, mTokens.begin()); it != mTokens.end(); ++it) {
    it->Unlock();
  }
  mLoaded = false;
}

void CActorModelParticles::CSystem::Update() {
  if (mLoaded || mRefCount == 0) {
    return;
  }
  bool loading = false;
  for (AUTO(it, mTokens.begin()); it != mTokens.end(); ++it) {
    if (!it->IsLoaded()) {
      loading = true;
      break;
    }
  }
  if (!loading) {
    mLoaded = true;
  }
}

CActorModelParticles::CItem::CItem(const CEntity& ent, CActorModelParticles& parent)
: mId(ent.GetUniqueId())
, mAreaId(ent.GetCurrentAreaId())
, mOnFireGens(
      rstl::pair< rstl::auto_ptr< CElementGen >, uint >(rstl::auto_ptr< CElementGen >(), 0))
, mOnFireDelayTimer(0.f)
, mOnFire(false)
, mAshPointIterator(0)
, mAshMaxParticles(-1)
, mAshSeed(99)
, mIcePointIterator(-1)
, mIceSeed(99)
, mElectricPointIterator(0)
, mElectricSeed(99)
, mElectricColor(CColor::White())
, mAshy(parent.mAshy)
, mParticleOffsetScale(1.f, 1.f, 1.f)
, mIceXf(CTransform4f::Identity())
, mParent(&parent)
, mThermalCold(false)
, mThermalHot(false)
, mRemTime(10.f)
, mLockDeps(0) {}

CActorModelParticles::CItem::~CItem() {
  if (mSfx) {
    CSfxManager::RemoveEmitter(mSfx);
  }
  if (mLockDeps != 0) {
    for (int i = 0; i < 6; ++i) {
      if (mLockDeps & (1 << i)) {
        mParent->DelTypeRef(static_cast< ESystemTypes >(i));
      }
    }
  }
}

bool CActorModelParticles::CItem::Update(float dt, CStateManager& mgr) {
  bool active = false;
  CActor* actor = static_cast< CActor* >(mgr.ObjectById(mId));
  if (actor != nullptr && actor->HasModelData()) {
    mParticleOffsetScale = actor->GetModelScale();
    mIceXf = actor->GetTransform();
    mAreaId = actor->GetCurrentAreaId();
  } else {
    mId = kInvalidUniqueId;
    mAshMaxParticles = 0;
    mIcePointIterator = -1;
    if (!mElectricGen.null()) {
      mElectricGen->SetParticleEmission(false);
    }
    if (mSfx) {
      CSfxManager::RemoveEmitter(mSfx);
      mSfx.Clear();
    }
    mRemTime -= dt;
    if (mRemTime <= 0.f) {
      return false;
    }
  }
  if (UpdateOnFire(dt, actor, mgr)) {
    active = true;
  }
  if (UpdateAshGen(dt, actor, mgr)) {
    active = true;
  }
  if (UpdateIce(dt, actor, mgr)) {
    active = true;
  }
  if (UpdateFirePop(dt, actor)) {
    active = true;
  }
  if (UpdateElectric(dt, actor, mgr)) {
    active = true;
  }
  if (UpdateRainSplash(dt, actor, mgr)) {
    active = true;
  }
  if (UpdateBurn(dt, actor, mgr)) {
    active = true;
  }
  if (UpdateIcePop(dt, actor)) {
    active = true;
  }
  return active;
}

bool CActorModelParticles::CItem::UpdateElectric(float dt, const CActor* actor,
                                                 CStateManager& mgr) {
  if (!mElectricGen.null()) {
    if (mElectricGen->IsSystemDeletable()) {
      mElectricGen = rstl::auto_ptr< CParticleElectric >();
    } else {
      if (actor != nullptr && actor->GetActive()) {
        mElectricGen->SetGlobalOrientation(actor->GetTransform().GetRotation());
        mElectricGen->SetGlobalTranslation(actor->GetTranslation());
      }
      if (actor == nullptr || actor->GetActive()) {
        mElectricGen->SetModulationColor(mElectricColor);
        mElectricGen->Update(dt);
        return true;
      }
    }
  } else if (mLockDeps & (1 << kST_Electric)) {
    if (mParent->mLoadedDeps & (1 << kST_Electric)) {
      CParticleElectric* gen = mParent->MakeElectricGen();
      gen->SetModulationColor(mElectricColor);
      mElectricGen = gen;
      mElectricPointIterator = 0;
      mElectricSeed = mgr.Random()->Next();
    }
    return true;
  }
  DontUseType(kST_Electric);
  return false;
}

bool CActorModelParticles::CItem::UpdateRainSplash(float dt, const CActor* actor,
                                                   CStateManager& mgr) {
  if (!mRainSplashGen.null()) {
    if (!mRainSplashGen->IsRaining()) {
      mRainSplashGen = rstl::auto_ptr< CRainSplashGenerator >();
    } else {
      mRainSplashGen->Update(dt, mgr);
      return true;
    }
  }
  return false;
}

bool CActorModelParticles::CItem::UpdateIce(float dt, const CActor* actor, CStateManager& mgr) {
  if (mIcePointIterator != -1) {
    return true;
  }
  if (!mIceGens.empty()) {
    bool active = false;
    for (AUTO(it, mIceGens.begin()); it != mIceGens.end(); ++it) {
      CElementGen* gen = it->get();
      if (!gen->IsSystemDeletable()) {
        active = true;
      }
      gen->Update(dt);
    }
    if (!active) {
      mIceGens.clear();
    } else {
      return true;
    }
  } else if ((mLockDeps & (1 << kST_Ice)) && actor != nullptr) {
    if (mParent->mLoadedDeps & (1 << kST_Ice)) {
      mIcePointIterator = 0;
      mIceSeed = mgr.Random()->Next();
    }
    return true;
  }
  DontUseType(kST_Ice);
  return false;
}

bool CActorModelParticles::CItem::UpdateFirePop(float dt, const CActor* actor) {
  if (!mFirePopGen.null()) {
    if (mFirePopGen->IsSystemDeletable()) {
      mFirePopGen = rstl::auto_ptr< CElementGen >();
    } else {
      mFirePopGen->Update(dt);
      return true;
    }
  } else if ((mLockDeps & (1 << kST_FirePop)) && actor != nullptr) {
    if (mParent->mLoadedDeps & (1 << kST_FirePop)) {
      CElementGen* gen = mParent->MakeFirePopGen();
      gen->SetGlobalOrientation(actor->GetTransform());
      gen->SetGlobalTranslation(actor->GetRenderBoundsCached().GetCenterPoint());
      mFirePopGen = gen;
    }
    return true;
  }
  DontUseType(kST_FirePop);
  return false;
}

bool CActorModelParticles::CItem::UpdateIcePop(float dt, const CActor* actor) {
  if (!mIcePopGen.null()) {
    if (mIcePopGen->IsSystemDeletable()) {
      mIcePopGen = rstl::auto_ptr< CElementGen >();
    } else {
      mIcePopGen->Update(dt);
      return true;
    }
  } else if ((mLockDeps & (1 << kST_IcePop)) && actor != nullptr) {
    if (mParent->mLoadedDeps & (1 << kST_IcePop)) {
      CElementGen* gen = mParent->MakeIcePopGen();
      gen->SetGlobalOrientation(actor->GetTransform());
      gen->SetGlobalTranslation(actor->GetRenderBoundsCached().GetCenterPoint());
      mIcePopGen = gen;
    }
    return true;
  }
  DontUseType(kST_IcePop);
  return false;
}

bool CActorModelParticles::CItem::UpdateAshGen(float dt, const CActor* actor, CStateManager& mgr) {
  if (!mAshGen.null()) {
    if (mAshMaxParticles == 0 && mAshGen->IsSystemDeletable()) {
      mAshGen = rstl::auto_ptr< CElementGen >();
    } else {
      if (actor != nullptr) {
        mAshGen->SetGlobalOrientAndTrans(actor->GetTransform());
      }
      mAshGen->Update(dt);
      return true;
    }
  } else if ((mLockDeps & (1 << kST_Ash)) && actor != nullptr) {
    if (mParent->mLoadedDeps & (1 << kST_Ash)) {
      CElementGen* gen = mParent->MakeAshGen();
      mAshGen = gen;
      mAshPointIterator = 0;
      gen->SetGlobalOrientAndTrans(actor->GetTransform());
      float scale = IsMediumOrLarge(*actor) ? 1.f : 0.3f;
      mAshMaxParticles = static_cast< uint >(scale * gen->GetMaxParticles());
      mAshSeed = mgr.Random()->Next();
    }
    return true;
  }
  DontUseType(kST_Ash);
  return false;
}

bool CActorModelParticles::CItem::UpdateOnFire(float dt, CActor* actor, CStateManager& mgr) {
  bool sfxActive = false;
  bool effectActive = false;
  mOnFireDelayTimer -= dt;
  if (mOnFireDelayTimer < 0.f) {
    mOnFireDelayTimer = 0.f;
  }
  if (mLockDeps & (1 << kST_OnFire)) {
    if (mParent->mLoadedDeps & (1 << kST_OnFire)) {
      if (mOnFire && actor != nullptr) {
        bool create = true;
        if (!mAshGen.null() || mAshy.IsLocked()) {
          create = false;
        } else if (!IsMediumOrLarge(*actor)) {
          int count = 0;
          for (int i = 0; i < 8; ++i) {
            if (!mOnFireGens[i].first.null()) {
              ++count;
            }
          }
          if (count >= 4) {
            create = false;
          }
        }
        if (create) {
          for (int i = 0; i < 8; ++i) {
            rstl::pair< rstl::auto_ptr< CElementGen >, uint >& pair = mOnFireGens[i];
            if (pair.first.null()) {
              pair.second = mgr.Random()->Next();
              pair.first = mParent->MakeOnFireGen();
              mOnFireDelayTimer = 0.3f;
              break;
            }
          }
        }
        if (!mSfx) {
          const int sfx = IsMediumOrLarge(*actor) ? SFXeff_x_fire_lp_00 : SFXeff_x_fire_lp_01;
          mSfx =
              CSfxManager::AddEmitter(sfx, actor->GetTranslation(), CVector3f::Zero(), true, true);
        }
        mOnFire = false;
      }
      for (int i = 0; i < 8; ++i) {
        if (!mOnFireGens[i].first.null()) {
          CElementGen* const gen = mOnFireGens[i].first.get();
          if (gen->IsSystemDeletable()) {
            mOnFireGens[i].first = rstl::auto_ptr< CElementGen >();
          } else {
            if (actor != nullptr) {
              gen->SetGlobalOrientAndTrans(actor->GetTransform());
            }
            gen->Update(dt);
            effectActive = true;
            sfxActive = true;
          }
        }
      }
    } else {
      effectActive = true;
    }
  }
  if (mSfx) {
    if (sfxActive) {
      CSfxManager::UpdateEmitter(mSfx, mIceXf.GetTranslation(), CVector3f::Zero(), 0x7f);
    } else {
      CSfxManager::RemoveEmitter(mSfx);
      mSfx.Clear();
    }
  }
  if (!effectActive) {
    DontUseType(kST_OnFire);
  }
  return effectActive;
}

bool CActorModelParticles::CItem::UpdateBurn(float dt, const CActor* actor, CStateManager& mgr) {
  if (actor == nullptr) {
    mAshy.Unlock();
  }
  return mAshy.IsLocked();
}

void CActorModelParticles::CItem::UseType(ESystemTypes dep) {
  const uchar mask = 1 << dep;
  if (!(mLockDeps & mask)) {
    mParent->AddTypeRef(dep);
    mLockDeps |= mask;
  }
}

void CActorModelParticles::CItem::DontUseType(ESystemTypes dep) {
  const uchar mask = 1 << dep;
  if (mLockDeps & mask) {
    mParent->DelTypeRef(dep);
    mLockDeps &= ~mask;
  }
}

void CActorModelParticles::PointGenerator(void* context, const CVector3f* vertices,
                                          const CVector3f* normals, int count) {
  reinterpret_cast< CItem* >(context)->GeneratePoints(vertices, normals, count);
}

void CActorModelParticles::SetupHook(TUniqueId uid) const {
  AUTO(it, FindSystem(uid));
  if (it != mItems.end()) {
    CSkinnedModel::SetPointGeneratorFunc(const_cast< CItem* >(&*it), PointGenerator);
  }
}

rstl::list< CActorModelParticles::CItem >::iterator
CActorModelParticles::FindOrCreateSystem(CActor& actor) {
  const TUniqueId uid = actor.GetUniqueId();
  if (actor.GetPointGeneratorParticles()) {
    for (AUTO(it, mItems.begin()); it != mItems.end(); ++it) {
      if (it->mId == uid) {
        return it;
      }
    }
  }
  actor.SetPointGeneratorParticles(true);
  return mItems.insert(mItems.begin(), CItem(actor, *this));
}

rstl::list< CActorModelParticles::CItem >::const_iterator
CActorModelParticles::FindSystem(TUniqueId uid) const {
  for (AUTO(it, mItems.begin()); it != mItems.end(); ++it) {
    if (it->mId == uid) {
      return it;
    }
  }
  return mItems.end();
}

rstl::list< CActorModelParticles::CItem >::iterator
CActorModelParticles::FindSystem(TUniqueId uid) {
  for (AUTO(it, mItems.begin()); it != mItems.end(); ++it) {
    if (it->mId == uid) {
      return it;
    }
  }
  return mItems.end();
}

CActorModelParticles::CActorModelParticles()
: mOnFire(gpSimplePool->GetObj(*gpResourceFactory->GetResourceIdByName(skParticleNames[0])))
, mAsh(gpSimplePool->GetObj(*gpResourceFactory->GetResourceIdByName(skParticleNames[2])))
, mIceBreak(gpSimplePool->GetObj(*gpResourceFactory->GetResourceIdByName(skParticleNames[1])))
, mFirePop(gpSimplePool->GetObj(*gpResourceFactory->GetResourceIdByName(skParticleNames[3])))
, mIcePop(gpSimplePool->GetObj(*gpResourceFactory->GetResourceIdByName(skParticleNames[5])))
, mElectric(gpSimplePool->GetObj(*gpResourceFactory->GetResourceIdByName(skParticleNames[4])))
, mAshy(gpSimplePool->GetObj(*gpResourceFactory->GetResourceIdByName("TXTR_Ashy"))) {
  InitializeSystemTypes();
}

CElementGen* CActorModelParticles::MakeAshGen() { return rs_new CElementGen(mAsh); }

CElementGen* CActorModelParticles::MakeFirePopGen() { return rs_new CElementGen(mFirePop); }

CElementGen* CActorModelParticles::MakeIcePopGen() { return rs_new CElementGen(mIcePop); }

CParticleElectric* CActorModelParticles::MakeElectricGen() {
  return rs_new CParticleElectric(mElectric);
}

CElementGen* CActorModelParticles::MakeOnFireGen() { return rs_new CElementGen(mOnFire); }

void CActorModelParticles::StartAsh(CActor& actor) {
  AUTO(it, FindOrCreateSystem(actor));
  it->UseType(kST_Ash);
}

void CActorModelParticles::DoFirePop(CActor& actor) {
  AUTO(it, FindOrCreateSystem(actor));
  it->UseType(kST_FirePop);
}

void CActorModelParticles::DoIcePop(CActor& actor) {
  AUTO(it, FindOrCreateSystem(actor));
  it->UseType(kST_IcePop);
}

void CActorModelParticles::StartIce(CActor& actor) {
  AUTO(it, FindOrCreateSystem(actor));
  it->UseType(kST_Ice);
}

void CActorModelParticles::StartElectric(CActor& actor) {
  AUTO(it, FindOrCreateSystem(actor));
  if (it->mElectricGen.get() == nullptr) {
    it->UseType(kST_Electric);
  } else {
    CParticleElectric* gen = it->mElectricGen.get();
    if (!gen->GetParticleEmission()) {
      gen->SetParticleEmission(true);
    }
  }
}

void CActorModelParticles::StopElectric(CActor& actor) {
  if (actor.GetPointGeneratorParticles()) {
    AUTO(it, FindSystem(actor.GetUniqueId()));
    if (it != mItems.end() && !it->mElectricGen.null()) {
      it->mElectricGen->SetParticleEmission(false);
    }
  }
}

void CActorModelParticles::LightDudeOnFire(CActor& actor) {
  AUTO(it, FindOrCreateSystem(actor));
  it->UseType(kST_OnFire);
  if (it->mOnFireDelayTimer <= 0.f) {
    it->mOnFire = true;
  }
}

void CActorModelParticles::StopFire(CActor& actor) {
  if (actor.GetPointGeneratorParticles()) {
    AUTO(it, FindSystem(actor.GetUniqueId()));
    if (it != mItems.end()) {
      for (int i = 0; i < 8; ++i) {
        CElementGen* gen = it->mOnFireGens[i].first.get();
        if (gen != nullptr) {
          gen->SetParticleEmission(false);
        }
      }
    }
  }
}

void CActorModelParticles::AddRainSplashGenerator(CActor& actor, CStateManager& mgr,
                                                  int maxSplashes, int genRate, float minZ) {
  AUTO(it, FindOrCreateSystem(actor));
  if (it->mRainSplashGen.null() && actor.HasModelData()) {
    it->mRainSplashGen =
        rs_new CRainSplashGenerator(actor.GetModelScale(), maxSplashes, genRate, minZ, 0.1875f);
  }
}

void CActorModelParticles::RemoveRainSplashGenerator(CActor& actor) {
  AUTO(it, FindOrCreateSystem(actor));
  if (!it->mRainSplashGen.null()) {
    it->mRainSplashGen = rstl::auto_ptr< CRainSplashGenerator >();
  }
}

CElementGen* CActorModelParticles::MakeIceGen() { return rs_new CElementGen(mIceBreak); }

void CActorModelParticles::InitializeSystemTypes() {
  for (int i = 0; i < 6; ++i) {
    const rstl::string name = rstl::string_l(skParticleNames[i]) + rstl::string_l("_DGRP");
    mDgrps.push_back(CSystem(name.data()));
  }
}

void CActorModelParticles::AddTypeRef(ESystemTypes dep) {
  const uchar mask = 1 << dep;
  mDgrps[dep].AddRef();
  if (!(mLoadedDeps & mask)) {
    mLoadingDeps |= mask;
  }
}

void CActorModelParticles::DelTypeRef(ESystemTypes dep) {
  CSystem& system = mDgrps[dep];
  system.DelRef();
  if (system.mRefCount == 0) {
    const uchar mask = ~(1 << dep);
    mLoadingDeps &= mask;
    mLoadedDeps &= mask;
    mJustLoadedDeps &= mask;
  }
}

void CActorModelParticles::UpdateSystemTypes() {
  if (mLoadingDeps != 0) {
    mJustLoadedDeps = 0;
    for (int i = 0; i < 6; ++i) {
      const uchar mask = 1 << i;
      CSystem& system = mDgrps[i];
      if (mLoadingDeps & mask) {
        system.Update();
        if (system.mLoaded) {
          mJustLoadedDeps |= mask;
          mLoadingDeps &= ~mask;
        }
      }
    }
    mLoadedDeps |= mJustLoadedDeps;
  }
}

void CActorModelParticles::StartBurnDeath(CActor& actor) {
  AUTO(it, FindOrCreateSystem(actor));
  const short sfx = IsMediumOrLarge(actor) ? SFXeff_x_ash_00 : SFXeff_x_ash_01;
  CSfxManager::AddEmitter(sfx, actor.GetTranslation(), CVector3f::Zero(), true, false);
  it->mAshy.Lock();
}

CTexture* CActorModelParticles::GetAshyTexture(const CActor& actor) const {
  AUTO(it, FindSystem(actor.GetUniqueId()));
  if (it != mItems.end() && it->mAshy.IsLocked() && it->mAshy.IsLoaded()) {
    return *TToken< CTexture >(it->mAshy);
  }
  return nullptr;
}

void CActorModelParticles::Update(float dt, CStateManager& mgr) {
  UpdateSystemTypes();
  AUTO(it, mItems.begin());
  while (it != mItems.end()) {
    if (!it->Update(dt, mgr)) {
      if (CActor* actor = static_cast< CActor* >(mgr.ObjectById(it->mId))) {
        actor->SetPointGeneratorParticles(false);
      }
      it = mItems.erase(it);
    } else {
      ++it;
    }
  }
}

static int GetNextBestPt(int start, const CVector3f* vertices, int count, CRandom16& random) {
  int best = start;
  const CVector3f& startVec = vertices[start];
  float maxDistance = 0.f;
  for (int i = 0; i < 10; ++i) {
    const int index = random.Range(0, count - 1);
    const CVector3f& point = vertices[index];
    const CVector3f& delta = startVec - point;
    const float distance = delta.MagSquared();
    if (distance > maxDistance) {
      best = index;
      maxDistance = distance;
    }
  }
  return best;
}

void CActorModelParticles::CItem::GeneratePoints(const CVector3f* vertices,
                                                 const CVector3f* normals, int count) {
  for (int i = 0; i < 8; ++i) {
    CElementGen* const gen = mOnFireGens[i].first.get();
    if (gen != nullptr) {
      CRandom16 random(mOnFireGens[i].second);
      const float randomValue = random.Float();
      const int index = randomValue * (count - 1);
      gen->SetTranslation(CVector3f::ByElementMultiply(mParticleOffsetScale, vertices[index]));
    }
  }
  if (mAshMaxParticles > 0) {
    CRandom16 random(mAshSeed);
    const int numParticles = rstl::min_val(16, mAshMaxParticles);
    int previousIndex = mAshPointIterator;
    for (int i = 0; i < numParticles; ++i) {
      const int index = GetNextBestPt(previousIndex, vertices, count, random);
      mAshGen->SetTranslation(
          CVector3f::ByElementMultiply(mParticleOffsetScale, vertices[index]));
      CVector3f normal = normals[index];
      normal.SetZ(0.f);
      if (normal.CanBeNormalized()) {
        normal.Normalize();
        const CVector3f& right = CVector3f::Cross(normal, CVector3f::Up());
        CElementGen* gen = mAshGen.get();
        gen->SetOrientation(CTransform4f::FromColumns(right, normal, CVector3f::Up(), CVector3f::Zero()));
      }
      mAshGen->ForceParticleCreation(1);
      previousIndex = index;
    }
    mAshMaxParticles -= numParticles;
    mAshSeed = random.GetSeed();
    mAshPointIterator = previousIndex;
  }
  if (mIcePointIterator != -1) {
    CRandom16 random(mIceSeed);
    CElementGen* gen = mParent->MakeIceGen();
    gen->SetGlobalOrientAndTrans(mIceXf);
    const int index = GetNextBestPt(mIcePointIterator, vertices, count, random);
    gen->SetTranslation(CVector3f::ByElementMultiply(mParticleOffsetScale, vertices[index]));
    const CUnitVector3f normal(normals[index]);
    gen->SetOrientation(CTransform4f::MakeRotationsBasedOnY(normal));
    mIceGens.push_back(gen);
    if (mIceGens.size() == 4) {
      mIcePointIterator = -1;
    } else {
      mIcePointIterator = index;
    }
  }
  if (!mElectricGen.null() && mElectricGen->GetParticleEmission()) {
    CRandom16 random(mElectricSeed);
    const int numParticles = rstl::min_val(1, 4);
    int previousIndex = mElectricPointIterator;
    for (int i = 0; i < numParticles; ++i) {
      const int initialIndex = random.Range(0, count - 1);
      mElectricGen->SetOverrideIPos(
          CVector3f::ByElementMultiply(mParticleOffsetScale, vertices[initialIndex]));
      const int index = random.Range(0, count - 1);
      mElectricGen->SetOverrideFPos(
          CVector3f::ByElementMultiply(mParticleOffsetScale, vertices[index]));
      mElectricGen->ForceParticleCreation(1);
      previousIndex = index;
    }
    mElectricSeed = random.GetSeed();
    mElectricPointIterator = previousIndex;
  }
  if (!mRainSplashGen.null()) {
    mRainSplashGen->GeneratePoints(vertices, normals, count);
  }
}

void CActorModelParticles::AddStragglersToRenderer(const CStateManager& mgr) const {
  const bool notCold = mgr.GetThermalDrawFlag() != kTD_Cold;
  const bool notHot = mgr.GetThermalDrawFlag() != kTD_Hot;
  for (AUTO(it, mItems.begin()); it != mItems.end(); ++it) {
    const CItem& item = *it;
    if (item.mAreaId != kInvalidAreaId) {
      const CGameArea& area = mgr.GetWorld()->GetAreaAlways(item.mAreaId);
      if (!area.IsPostConstructed() || area.GetOcclusionState() == CGameArea::kOS_Occluded) {
        continue;
      }
    }
    if (mgr.GetObjectById(item.mId) &&
        ((notCold && item.mThermalCold) || (notHot && item.mThermalHot))) {
      item.mThermalHot = item.mThermalCold = false;
      continue;
    }
    if (notCold) {
      for (int i = 0; i < 8; ++i) {
        if (!item.mOnFireGens[i].first.null()) {
          gpRender->AddParticleGen(*item.mOnFireGens[i].first);
        }
      }
      if (mgr.GetThermalDrawFlag() != kTD_Hot && !item.mAshGen.null()) {
        gpRender->AddParticleGen(*item.mAshGen);
      }
      if (!item.mFirePopGen.null()) {
        gpRender->AddParticleGen(*item.mFirePopGen);
      }
      if (!item.mElectricGen.null()) {
        gpRender->AddParticleGen(*item.mElectricGen);
      }
    }
    if (notHot) {
      for (AUTO(gen, item.mIceGens.begin()); gen != item.mIceGens.end(); ++gen) {
        gpRender->AddParticleGen(**gen);
      }
      if (!item.mIcePopGen.null()) {
        gpRender->AddParticleGen(*item.mIcePopGen);
      }
    }
    if (notCold) {
      item.mThermalHot = item.mThermalCold = false;
    }
  }
}

void CActorModelParticles::Render(const CStateManager& mgr, const CActor& actor) const {
  const CTransform4f modelMatrix = CGraphics::GetModelMatrix();
  const TUniqueId uid = actor.GetUniqueId();
  AUTO(it, FindSystem(uid));
  if (it == mItems.end()) {
    return;
  }
  const CItem& item = *it;
  if (item.mAreaId != kInvalidAreaId) {
    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(item.mAreaId);
    if (!area.IsPostConstructed() || area.GetOcclusionState() == CGameArea::kOS_Occluded) {
      return;
    }
  }
  const bool notCold = mgr.GetThermalDrawFlag() != kTD_Cold;
  const bool notHot = mgr.GetThermalDrawFlag() != kTD_Hot;
  if (notCold) {
    for (int i = 0; i < 8; ++i) {
      if (!item.mOnFireGens[i].first.null()) {
        item.mOnFireGens[i].first->Render();
      }
    }
    if (mgr.GetThermalDrawFlag() != kTD_Hot && !item.mAshGen.null()) {
      item.mAshGen->Render();
    }
    if (!item.mFirePopGen.null()) {
      item.mFirePopGen->Render();
    }
    if (!item.mElectricGen.null()) {
      item.mElectricGen->Render();
    }
    item.mThermalCold = true;
  }
  if (notHot) {
    for (AUTO(gen, item.mIceGens.begin()); gen != item.mIceGens.end(); ++gen) {
      (*gen)->Render();
    }
    if (!item.mRainSplashGen.null() && actor.HasModelData()) {
      item.mRainSplashGen->Draw(actor.GetTransform());
    }
    if (!item.mIcePopGen.null()) {
      item.mIcePopGen->Render();
    }
    item.mThermalHot = true;
  }
  CGraphics::SetModelMatrix(modelMatrix);
}
