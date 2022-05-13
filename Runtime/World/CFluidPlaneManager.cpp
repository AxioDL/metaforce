#include "Runtime/World/CFluidPlaneManager.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CExplosion.hpp"
#include "Runtime/World/CScriptWater.hpp"

namespace metaforce {

CFluidPlaneManager::CFluidProfile CFluidPlaneManager::sProfile = {};

CFluidPlaneManager::CFluidPlaneManager() : x0_rippleManager(20, 0.5f) {
  sProfile.Clear();
  SetupRippleMap();
}

void CFluidPlaneManager::CFluidProfile::Clear() {
  x0_ = 0.f;
  x4_ = 0.f;
  x8_ = 0.f;
  xc_ = 0.f;
  x10_ = 0.f;
}

void CFluidPlaneManager::StartFrame(bool b) {
  x121_ = b;
  sProfile.Clear();
}

void CFluidPlaneManager::Update(float dt) {
  x11c_uvT += dt;
  x0_rippleManager.Update(dt);
  for (CSplashRecord& record : x18_splashes) {
    record.SetTime(record.GetTime() + dt);
    if (record.GetTime() > 9999.f)
      record.SetTime(9999.f);
  }
}

float CFluidPlaneManager::GetLastRippleDeltaTime(TUniqueId rippler) const {
  return x0_rippleManager.GetLastRippleDeltaTime(rippler);
}

float CFluidPlaneManager::GetLastSplashDeltaTime(TUniqueId splasher) const {
  float newestTime = 9999.f;
  for (const CSplashRecord& record : x18_splashes)
    if (record.GetUniqueId() == splasher && newestTime > record.GetTime())
      newestTime = record.GetTime();
  return newestTime;
}

void CFluidPlaneManager::CreateSplash(TUniqueId splasher, CStateManager& mgr, const CScriptWater& water,
                                      const zeus::CVector3f& pos, float factor, bool sfx) {
  if (water.CanRippleAtPoint(pos)) {
    float oldestTime = 0.f;
    CSplashRecord* oldestRecord = nullptr;
    for (CSplashRecord& record : x18_splashes) {
      if (record.GetTime() > oldestTime) {
        oldestRecord = &record;
        oldestTime = record.GetTime();
      }
    }
    if (oldestRecord)
      *oldestRecord = CSplashRecord(0.f, splasher);
    else
      x18_splashes.emplace_back(0.f, splasher);
    float splashScale = water.GetSplashEffectScale(factor);
    if (water.GetSplashEffect(factor)) {
      CExplosion* expl = new CExplosion(*water.GetSplashEffect(factor), mgr.AllocateUniqueId(), true,
                                        CEntityInfo(water.GetAreaIdAlways(), CEntity::NullConnectionList), "Splash",
                                        zeus::CTransform(zeus::CMatrix3f(), pos), 1, zeus::CVector3f{splashScale},
                                        water.GetSplashColor());
      mgr.AddObject(expl);
    }
    if (sfx) {
      CSfxManager::AddEmitter(water.GetSplashSound(factor), pos, zeus::skUp, true, false, 0x7f, kInvalidAreaId);
    }
  }
}

static bool g_RippleMapSetup = false;
std::array<std::array<u8, 64>, 64> CFluidPlaneManager::RippleValues{};
std::array<u8, 64> CFluidPlaneManager::RippleMins{};
std::array<u8, 64> CFluidPlaneManager::RippleMaxs{};
aurora::gfx::TextureHandle CFluidPlaneManager::RippleMapTex;

void CFluidPlaneManager::SetupRippleMap() {
  if (g_RippleMapSetup) {
    return;
  }
  g_RippleMapSetup = true;

  float curX = 0.f;
  for (size_t i = 0; i < 64; ++i) {
    float curY = 0.f;
    float minY = 1.f;
    float maxY = 0.f;
    for (size_t j = 0; j < 64; ++j) {
      const float rVal = 1.f - curY;
      float minX = curY;
      float maxX = 1.25f * (0.25f * rVal + 0.1f) + curY;
      if (curY < 0.f) {
        minX = 0.f;
      } else if (maxX > 1.f) {
        maxX = 1.f;
      }

      float val = 0.f;
      if (curX >= minX && curX <= maxX) {
        const float t = (curX - minX) / (maxX - minX);
        if (t < 0.4f) {
          val = 2.5f * t;
        } else if (t > 0.75f) {
          val = 4.f * (1.f - t);
        } else {
          val = 1.f;
        }
      }

      const auto valA = u8(std::max(int(255.f * val * rVal * rVal) - 1, 0));
      RippleValues[i][j] = valA;
      if (valA != 0 && curY < minY) {
        minY = curY;
      }
      if (valA != 0 && curY > maxY) {
        maxY = curY;
      }

      curY += (1.f / 63.f);
    }

    const auto valB = u8(std::max(int(255.f * minY) - 1, 0));
    const auto valC = u8(std::min(int(255.f * maxY) + 1, 255));
    RippleMins[i] = valB;
    RippleMaxs[i] = valC;
    curX += (1.f / 63.f);
  }

  RippleMapTex = aurora::gfx::new_static_texture_2d(64, 64, 1, GX::TF_I8,
                                                    {reinterpret_cast<const uint8_t*>(RippleValues.data()), 64 * 64},
                                                    "Ripple Map");
}

void CFluidPlaneManager::Shutdown() {
  RippleMapTex.reset();
}

} // namespace metaforce
