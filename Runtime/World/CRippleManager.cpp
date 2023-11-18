#include "Runtime/World/CRippleManager.hpp"

namespace metaforce {

CRippleManager::CRippleManager(int maxRipples, float alpha) : x14_alpha(alpha) { Init(maxRipples); }

void CRippleManager::Init(int maxRipples) {
  x4_ripples.resize(maxRipples, CRipple(kInvalidUniqueId, zeus::skZero3f, 0.f));
  for (CRipple& r : x4_ripples)
    r.SetTime(9999.f);
}

void CRippleManager::Update(float dt) {
  for (CRipple& ripple : x4_ripples) {
    ripple.SetTime(ripple.GetTime() + dt);
    if (ripple.GetTime() > 9999.f)
      ripple.SetTime(9999.f);
  }
}

float CRippleManager::GetLastRippleDeltaTime(TUniqueId rippler) const {
  float res = 9999.f;
  for (const CRipple& r : x4_ripples)
    if (r.GetUniqueId() == rippler)
      if (r.GetTime() < res)
        res = r.GetTime();
  return res;
}

void CRippleManager::AddRipple(const CRipple& ripple) {
  float maxTime = 0.f;
  auto oldestRipple = x4_ripples.end();
  for (auto it = x4_ripples.begin(); it != x4_ripples.end(); ++it)
    if (it->GetTime() > maxTime) {
      oldestRipple = it;
      maxTime = it->GetTime();
    }

  if (oldestRipple != x4_ripples.end()) {
    *oldestRipple = ripple;
    oldestRipple->SetTime(0.f);
    SetMaxTimeFalloff(std::max(GetMaxTimeFalloff(), ripple.GetTimeFalloff()));
  }
}

} // namespace metaforce
