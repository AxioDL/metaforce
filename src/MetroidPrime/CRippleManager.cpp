#include "MetroidPrime/CRippleManager.hpp"

#include "MetroidPrime/CFluidPlaneManager.hpp"
#include "MetroidPrime/CRipple.hpp"

#include "rstl/math.hpp"

CRippleManager::CRippleManager(int maxRipples, float alpha)
: mMaxTimeFalloff(0.f)
, mAlpha(alpha) {
  Init(maxRipples);
}

void CRippleManager::Init(int maxRipples) {
  mRipples.resize(maxRipples);
  for (AUTO(it, mRipples.begin()); it != mRipples.end(); ++it) {
    it->SetTime(CFluidPlaneManager::kOldestTime);
  }
}

void CRippleManager::Update(float dt) {
  for (AUTO(it, Ripples().begin()); it != Ripples().end(); ++it) {
    it->SetTime(it->GetTime() + dt);
    if (it->GetTime() > CFluidPlaneManager::kOldestTime) {
      it->SetTime(CFluidPlaneManager::kOldestTime);
    }
  }
}

float CRippleManager::GetLastRippleDeltaTime(TUniqueId rippler) const {
  float res = CFluidPlaneManager::kOldestTime;

  for (AUTO(it, GetRipples().begin()); it != GetRipples().end(); ++it) {
    if (rippler == it->GetUniqueId()) {
      if (res > it->GetTime()) {
        res = it->GetTime();
      }
    }
  }

  return res;
}

void CRippleManager::AddRipple(const CRipple& ripple) {
  float maxTime = 0.f;
  rstl::vector< CRipple >::iterator oldestRipple = Ripples().end();

  for (AUTO(it, Ripples().begin()); it != Ripples().end(); ++it) {
    if (it->GetTime() == CFluidPlaneManager::kOldestTime) {
      oldestRipple = it;
      break;
    }

    if (it->GetTime() > maxTime) {
      oldestRipple = it;
      maxTime = it->GetTime();
    }
  }

  if (oldestRipple != Ripples().end()) {
    *oldestRipple = ripple;
    oldestRipple->SetTime(0.f);
    SetMaxTimeFalloff(rstl::max_val(GetMaxTimeFalloff(), ripple.GetTimeFalloff()));
  }
}
