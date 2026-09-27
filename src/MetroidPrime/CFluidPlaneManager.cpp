#include "MetroidPrime/CFluidPlaneManager.hpp"

#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CRipple.hpp"
#include "MetroidPrime/CRippleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"
#include "rstl/math.hpp"

const bool gkWaterEnable = true;
const bool gkWaterTurbulence = true;
const bool gkWaterBumpMapping = true;
const int gkWaterEnvMap = 1;
const bool gkWaterFog = true;

CFluidPlaneManager::CFluidProfile CFluidPlaneManager::sProfile;

uchar sRippleValues[64][64];
uchar sRippleMins[64];
uchar sRippleMaxs[64];

const float CFluidPlaneManager::kOldestTime = 9999.f;

void CFluidPlaneManager::CFluidProfile::Clear() {
  x10_ = 0.f;
  xc_ = 0.f;
  x8_ = 0.f;
  x4_ = 0.f;
  x0_ = 0.f;
}

CFluidPlaneManager::CFluidPlaneManager()
: mRippleManager(20, 0.5f), mUvT(0.f), x120_(false), x121_(false) {
  sProfile.Clear();
  for (rstl::reserved_vector< CSplashRecord, 32 >::iterator it = mSplashes.begin();
       it != mSplashes.end(); ++it) {
    it->SetTime(9999.f);
  }
  SetupRippleMap();
}

void CFluidPlaneManager::Update(float dt) {
  mUvT = dt + mUvT;
  mRippleManager.Update(dt);
  for (CSplashRecord* it = SplashRecords().begin(); it != SplashRecords().end(); ++it) {
    it->SetTime(dt + it->GetTime());
    if (it->GetTime() > 9999.f) {
      it->SetTime(9999.f);
    }
  }
}

void CFluidPlaneManager::StartFrame(bool b) const {
  x121_ = b;
  sProfile.Clear();
}

void CFluidPlaneManager::EndFrame() const { x121_ = false; }

float CFluidPlaneManager::GetLastSplashDeltaTime(TUniqueId splasher) const {
  float newestTime = 9999.f;
  for (const CSplashRecord* it = GetSplashRecords().begin(); it != GetSplashRecords().end(); ++it) {
    if (splasher == it->GetUniqueId() && newestTime > it->GetTime()) {
      newestTime = it->GetTime();
    }
  }
  return newestTime;
}

float CFluidPlaneManager::GetLastRippleDeltaTime(TUniqueId rippler) const {
  return mRippleManager.GetLastRippleDeltaTime(rippler);
}

void CFluidPlaneManager::CreateSplash(TUniqueId splasher, CStateManager& mgr,
                                      const CScriptWater& water, const CVector3f& pos, float factor,
                                      bool sfx) {
  if (!water.CanRippleAtPoint(pos)) {
    return;
  }

  float oldestTime = 0.f;
  CSplashRecord* oldestRecord = nullptr;
  for (CSplashRecord* it = SplashRecords().begin(); it != SplashRecords().end(); ++it) {
    if (it->GetTime() > oldestTime) {
      oldestRecord = it;
      oldestTime = it->GetTime();
    }
  }

  CSplashRecord newRecord(splasher);
  if (oldestRecord != nullptr) {
    *oldestRecord = newRecord;
  } else {
    SplashRecords().push_back(newRecord);
  }

  float splashScale = water.GetSplashEffectScale(factor);
  if (water.GetSplashEffect(factor)) {
    CEntity* expl = rs_new CExplosion(
            *water.GetSplashEffect(factor), mgr.AllocateUniqueId(), true,
            CEntityInfo(water.GetCurrentAreaId(), CEntity::NullConnectionList),
            rstl::string_l("Splash"), CTransform4f(CMatrix3f::Identity(), pos), 1,
            CVector3f(splashScale, splashScale, splashScale), water.GetSplashColor());
    if (expl) {
      mgr.AddObject(*expl);
    }
  }

  if (sfx) {
    CSfxManager::AddEmitter(water.GetSplashSound(factor), pos, CVector3f(0.f, 0.f, 1.f), true,
                            false, CSfxManager::kMedPriority, CSfxManager::kAllAreas);
  }
}

static inline float ComputeRippleAmplitude(float curX, float curY) {
  float rVal = 1.f - curY;
  float minX = curY;
  float maxX = 1.25f * (0.25f * rVal + 0.1f) + curY;
  if (curY < 0.f) {
    minX = 0.f;
  } else if (maxX > 1.f) {
    maxX = 1.f;
  }
  float val = 0.f;
  if (curX >= minX && curX <= maxX) {
    float t = (curX - minX) / (maxX - minX);
    if (t < 0.4f) {
      val = 2.5f * t;
    } else if (t > 0.75f) {
      val = 4.f * (1.f - t);
    } else {
      val = 1.f;
    }
  }
  return val;
}

void CFluidPlaneManager::SetupRippleMap() {
  float curX = 0.f;
  for (int i = 0; i < 64; ++i) {
    uchar* rowPtr = sRippleValues[i];
    float curY = 0.f;
    float minY = 1.f;
    float maxY = curY;
    for (int j = 64; j > 0; --j) {
      float val = ComputeRippleAmplitude(curX, curY);
      uchar valA = CCast::ToUint8(255.f * (val * ((1.f - curY) * (1.f - curY))));
      *rowPtr = valA;
      if (valA != 0 && curY < minY) {
        minY = curY;
      }
      if (valA != 0 && curY > maxY) {
        maxY = curY;
      }
      curY += (1.f / 63.f);
      rowPtr++;
    }
    int valB = static_cast< int >(CCast::ToUint8(255.f * minY)) - 1;
    sRippleMins[i] = static_cast< uchar >(rstl::max_val(0, valB));
    int valC = static_cast< int >(CCast::ToUint8(255.f * maxY)) + 1;
    sRippleMaxs[i] = static_cast< uchar >(rstl::min_val(255, valC));
    curX += (1.f / 63.f);
  }
}

uint CFluidPlaneManager::GetFreqTableIndex(float) { return 0; }
