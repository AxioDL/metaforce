#pragma once

#include <list>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/Graphics/CRainSplashGenerator.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CStateManager;
class CEntity;
class CElementGen;
class CTexture;
class CGenDescription;
class CActor;
class CScriptPlayerActor;

class CActorModelParticles {
public:
  enum class EDependency { OnFire, Ice, Ash, FirePop, Electric, IcePop };

  class CItem {
    friend class CActorModelParticles;
    TUniqueId x0_id;
    TAreaId x4_areaId;
    rstl::reserved_vector<std::pair<std::unique_ptr<CElementGen>, u32>, 8> x8_onFireGens;
    float x6c_onFireDelayTimer = 0.f;
    bool x70_onFire = false;
    CSfxHandle x74_sfx;
    std::unique_ptr<CElementGen> x78_ashGen;
    s32 x80_ashPointIterator = 0;
    s32 x84_ashMaxParticles = -1;
    u32 x88_ashSeed = 99;
    rstl::reserved_vector<std::unique_ptr<CElementGen>, 4> x8c_iceGens;
    s32 xb0_icePointIterator = -1;
    u32 xb4_iceSeed = 99;
    std::unique_ptr<CElementGen> xb8_firePopGen;
    std::unique_ptr<CParticleElectric> xc0_electricGen;
    s32 xc8_electricPointIterator = 0;
    u32 xcc_electricSeed = 99;
    zeus::CColor xd0_electricColor;
    std::unique_ptr<CRainSplashGenerator> xd4_rainSplashGen;
    TToken<CTexture> xdc_ashy;
    std::unique_ptr<CElementGen> xe4_icePopGen;
    zeus::CVector3f xec_particleOffsetScale = zeus::skOne3f;
    zeus::CTransform xf8_iceXf;
    CActorModelParticles& x128_parent;
    bool x12c_24_thermalCold : 1 = false;
    bool x12c_25_thermalHot : 1 = false;
    float x130_remTime = 10.f;
    mutable u8 x134_lockDeps = 0;
    bool UpdateOnFire(float dt, CActor* actor, CStateManager& mgr);
    bool UpdateAshGen(float dt, CActor* actor, CStateManager& mgr);
    bool UpdateIceGen(float dt, CActor* actor, CStateManager& mgr);
    bool UpdateFirePop(float dt, CActor* actor, CStateManager& mgr);
    bool UpdateElectric(float dt, CActor* actor, CStateManager& mgr);
    bool UpdateRainSplash(float dt, CActor* actor, CStateManager& mgr);
    bool UpdateBurn(float dt, CActor* actor, CStateManager& mgr);
    bool UpdateIcePop(float dt, CActor* actor, CStateManager& mgr);

  public:
    CItem(const CEntity& ent, CActorModelParticles& parent);
    void GeneratePoints(const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn);
    bool Update(float dt, CStateManager& mgr);
    void Lock(EDependency i);
    void Unlock(EDependency i);
  };

private:
  friend class CItem;
  std::list<CItem> x0_items;
  TToken<CGenDescription> x18_onFire;
  TToken<CGenDescription> x20_ash;
  TToken<CGenDescription> x28_iceBreak;
  TToken<CGenDescription> x30_firePop;
  TToken<CGenDescription> x38_icePop;
  TToken<CElectricDescription> x40_electric;
  TToken<CTexture> x48_ashy;
  struct Dependency {
    std::vector<CToken> x0_tokens;
    int x10_refCount = 0;
    bool x14_loaded = false;
    void Increment() {
      ++x10_refCount;
      if (x10_refCount == 1)
        Load();
    }
    void Decrement() {
      --x10_refCount;
      if (x10_refCount <= 0)
        Unload();
    }
    void Load() {
      bool loading = false;
      for (CToken& tok : x0_tokens) {
        tok.Lock();
        if (!tok.IsLoaded())
          loading = true;
      }
      if (!loading)
        x14_loaded = true;
    }
    void Unload() {
      for (CToken& tok : x0_tokens)
        tok.Unlock();
      x14_loaded = false;
    }
    void UpdateLoad() {
      if (x14_loaded || x10_refCount == 0)
        return;
      bool loading = false;
      for (CToken& tok : x0_tokens) {
        if (!tok.IsLoaded())
          loading = true;
      }
      if (!loading)
        x14_loaded = true;
    }
  };
  rstl::reserved_vector<Dependency, 6> x50_dgrps;
  u8 xe4_loadingDeps = 0;
  u8 xe5_justLoadedDeps = 0;
  u8 xe6_loadedDeps = 0;

  Dependency GetParticleDGRPTokens(std::string_view name) const;
  void LoadParticleDGRPs();

  std::unique_ptr<CElementGen> MakeOnFireGen() const;
  std::unique_ptr<CElementGen> MakeAshGen() const;
  std::unique_ptr<CElementGen> MakeIceGen() const;
  std::unique_ptr<CElementGen> MakeFirePopGen() const;
  std::unique_ptr<CElementGen> MakeIcePopGen() const;
  std::unique_ptr<CParticleElectric> MakeElectricGen() const;

  void DecrementDependency(EDependency d);
  void IncrementDependency(EDependency d);

  void UpdateLoad();

public:
  CActorModelParticles();
  static void PointGenerator(void* item, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn);
  void AddStragglersToRenderer(const CStateManager& mgr);
  void Update(float dt, CStateManager& mgr);
  void SetupHook(TUniqueId uid);
  std::list<CItem>::iterator FindSystem(TUniqueId uid);
  std::list<CItem>::const_iterator FindSystem(TUniqueId uid) const;
  std::list<CItem>::iterator FindOrCreateSystem(CActor& act);
  void StartIce(CActor& actor);
  void AddRainSplashGenerator(CActor& act, CStateManager& mgr, u32 maxSplashes, u32 genRate, float minZ);
  void RemoveRainSplashGenerator(CActor& act);
  void Render(const CStateManager& mgr, const CActor& actor) const;
  void StartElectric(CActor& act);
  void StopElectric(CActor& act);
  void LoadAndStartElectric(CActor& act);
  void StopThermalHotParticles(CActor& act);
  void StartBurnDeath(CActor& act);
  void EnsureElectricLoaded(CActor& act);
  void EnsureFirePopLoaded(CActor& act);
  void EnsureIceBreakLoaded(CActor& act);
  void LightDudeOnFire(CActor& act);
  const CTexture* GetAshyTexture(const CActor& act);
};
} // namespace urde
