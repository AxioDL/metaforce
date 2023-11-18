#pragma once

#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/World/CRippleManager.hpp"

namespace metaforce {

class CStateManager;
class CScriptWater;

class CFluidPlaneManager {
  class CSplashRecord {
    float x0_time = 0.f;
    TUniqueId x4_id;

  public:
    CSplashRecord(float time, TUniqueId id) : x0_time(time), x4_id(id) {}
    void SetTime(float t) { x0_time = t; }
    float GetTime() const { return x0_time; }
    void SetUniqueId(TUniqueId id) { x4_id = id; }
    TUniqueId GetUniqueId() const { return x4_id; }
  };

  CRippleManager x0_rippleManager;
  rstl::reserved_vector<CSplashRecord, 32> x18_splashes;
  float x11c_uvT = 0.f;
  bool x120_ = false;
  bool x121_ = false;

  class CFluidProfile {
    float x0_ = 0.f;
    float x4_ = 0.f;
    float x8_ = 0.f;
    float xc_ = 0.f;
    float x10_ = 0.f;

  public:
    void Clear();
  };
  static CFluidProfile sProfile;
  static void SetupRippleMap();

public:
  static std::array<std::array<u8, 64>, 64> RippleValues;
  static std::array<u8, 64> RippleMins;
  static std::array<u8, 64> RippleMaxs;
  static GXTexObj RippleMapTex;

  CFluidPlaneManager();
  void StartFrame(bool);
  void EndFrame() { x121_ = false; }
  void Update(float dt);
  float GetUVT() const { return x11c_uvT; }
  float GetLastRippleDeltaTime(TUniqueId rippler) const;
  float GetLastSplashDeltaTime(TUniqueId splasher) const;
  void CreateSplash(TUniqueId splasher, CStateManager& mgr, const CScriptWater& water, const zeus::CVector3f& pos,
                    float factor, bool sfx);
  rstl::reserved_vector<CSplashRecord, 32>& SplashRecords() { return x18_splashes; }
  const CRippleManager& GetRippleManager() const { return x0_rippleManager; }
  CRippleManager& RippleManager() { return x0_rippleManager; }

  static void Shutdown();
};

} // namespace metaforce
