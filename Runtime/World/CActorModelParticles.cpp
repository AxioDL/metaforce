#include "Runtime/World/CActorModelParticles.hpp"

#include <algorithm>
#include <array>

#include "Runtime/CDependencyGroup.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CScriptPlayerActor.hpp"
#include "Runtime/World/CWorld.hpp"

namespace urde {

static bool IsMediumOrLarge(const CActor& act) {
  if (const TCastToConstPtr<CPatterned> pat = act) {
    return pat->GetKnockBackController().GetVariant() != EKnockBackVariant::Small;
  }
  return false;
}

CActorModelParticles::CItem::CItem(const CEntity& ent, CActorModelParticles& parent)
: x0_id(ent.GetUniqueId())
, x4_areaId(ent.GetAreaIdAlways())
, xdc_ashy(parent.x48_ashy)
, x128_parent(parent) {
  x8_onFireGens.resize(8);
}

static s32 GetNextBestPt(s32 start, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn,
                         CRandom16& rnd) {
  const zeus::CVector3f& startVec = vn[start].first;
  s32 ret = start;
  float maxMag = 0.f;
  for (s32 i = 0; i < 10; ++i) {
    s32 idx = rnd.Range(0, s32(vn.size()) - 1);
    const zeus::CVector3f& rndVec = vn[idx].first;
    float mag = (startVec - rndVec).magSquared();
    if (mag > maxMag) {
      ret = idx;
      maxMag = mag;
    }
  }
  return ret;
}
void CActorModelParticles::CItem::GeneratePoints(const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn) {
  for (std::pair<std::unique_ptr<CElementGen>, u32>& pair : x8_onFireGens) {
    if (pair.first) {
      CRandom16 rnd(pair.second);
      const zeus::CVector3f& vec = vn[rnd.Float() * (vn.size() - 1)].first;
      pair.first->SetTranslation(xec_particleOffsetScale * vec);
    }
  }

  if (x84_ashMaxParticles > 0) {
    CRandom16 rnd(x88_ashSeed);
    s32 count = (x84_ashMaxParticles >= 16 ? 16 : x84_ashMaxParticles);
    s32 idx = x80_ashPointIterator;
    for (u32 i = 0; i < count; ++i) {
      idx = GetNextBestPt(idx, vn, rnd);
      x78_ashGen->SetTranslation(xec_particleOffsetScale * vn[idx].first);
      zeus::CVector3f v = vn[idx].second;
      if (v.canBeNormalized()) {
        v.normalize();
        x78_ashGen->SetOrientation(
            zeus::CTransform{v.cross(zeus::skUp), v, zeus::skUp, zeus::skZero3f});
      }
      x78_ashGen->ForceParticleCreation(1);
    }
    x84_ashMaxParticles -= count;
    x88_ashSeed = rnd.GetSeed();
    x80_ashPointIterator = idx;
  }

  if (xb0_icePointIterator != -1) {
    CRandom16 rnd(xb4_iceSeed);

    std::unique_ptr<CElementGen> iceGen = x128_parent.MakeIceGen();
    iceGen->SetGlobalOrientAndTrans(xf8_iceXf);

    s32 idx = GetNextBestPt(xb0_icePointIterator, vn, rnd);
    iceGen->SetTranslation(xec_particleOffsetScale * vn[idx].first);

    iceGen->SetOrientation(zeus::CTransform::MakeRotationsBasedOnY(zeus::CUnitVector3f(vn[idx].second)));

    x8c_iceGens.push_back(std::move(iceGen));
    xb0_icePointIterator = (x8c_iceGens.size() == 4 ? -1 : idx);
  }

  if (xc0_electricGen && xc0_electricGen->GetParticleEmission()) {
    CRandom16 rnd(xcc_electricSeed);
    u32 end = 1;
#if 0
    if (4 < 1)
      end = 4;
#endif
    s32 idx = xc8_electricPointIterator;
    for (u32 i = 0; i < end; ++i) {
      xc0_electricGen->SetOverrideIPos(vn[rnd.Range(0, s32(vn.size()) - 1)].first * xec_particleOffsetScale);
      idx = rnd.Range(0, s32(vn.size()) - 1);
      xc0_electricGen->SetOverrideFPos(vn[idx].first * xec_particleOffsetScale);
      xc0_electricGen->ForceParticleCreation(1);
    }

    xcc_electricSeed = rnd.GetSeed();
    xc8_electricPointIterator = idx;
  }

  if (xd4_rainSplashGen)
    xd4_rainSplashGen->GeneratePoints(vn);
}

bool CActorModelParticles::CItem::UpdateOnFire(float dt, CActor* actor, CStateManager& mgr) {
  bool effectActive = false;
  bool sfxActive = false;
  x6c_onFireDelayTimer -= dt;
  if (x6c_onFireDelayTimer < 0.f)
    x6c_onFireDelayTimer = 0.f;
  if (x134_lockDeps & 0x1) {
    if (x128_parent.xe6_loadedDeps & 0x1) {
      if (x70_onFire && actor) {
        bool doCreate = true;
        if (x78_ashGen || xdc_ashy) {
          doCreate = false;
        } else if (!IsMediumOrLarge(*actor)) {
          int activeParts = 0;
          for (const auto& p : x8_onFireGens)
            if (p.first)
              ++activeParts;
          if (activeParts >= 4)
            doCreate = false;
        }
        if (doCreate) {
          for (auto& p : x8_onFireGens) {
            if (!p.first) {
              p.second = mgr.GetActiveRandom()->Next();
              p.first = x128_parent.MakeOnFireGen();
              x6c_onFireDelayTimer = 0.3f;
              break;
            }
          }
        }
        if (!x74_sfx) {
          x74_sfx = CSfxManager::AddEmitter(SFXsfx0480 + (IsMediumOrLarge(*actor) ? 1 : 0), actor->GetTranslation(),
                                            zeus::skZero3f, true, true, 0x7f, kInvalidAreaId);
        }
        x70_onFire = false;
      }
      for (auto& p : x8_onFireGens) {
        if (p.first) {
          if (p.first->IsSystemDeletable())
            p.first.reset();
          else if (actor)
            p.first->SetGlobalOrientAndTrans(actor->GetTransform());
          p.first->Update(dt);
          effectActive = true;
          sfxActive = true;
        }
      }
    } else {
      effectActive = true;
    }
  }
  if (x74_sfx) {
    if (sfxActive) {
      CSfxManager::UpdateEmitter(x74_sfx, xf8_iceXf.origin, zeus::skZero3f, 1.f);
    } else {
      CSfxManager::RemoveEmitter(x74_sfx);
      x74_sfx.reset();
    }
  }
  if (!effectActive)
    Unlock(EDependency::OnFire);
  return effectActive;
}

bool CActorModelParticles::CItem::UpdateAshGen(float dt, CActor* actor, CStateManager& mgr) {
  if (x78_ashGen) {
    if (x84_ashMaxParticles == 0 && x78_ashGen->IsSystemDeletable()) {
      x78_ashGen.reset();
    } else {
      if (actor)
        x78_ashGen->SetGlobalOrientAndTrans(actor->GetTransform());
      x78_ashGen->Update(dt);
      return true;
    }
  } else if (x134_lockDeps & 0x4 && actor) {
    if (x128_parent.xe6_loadedDeps & 0x4) {
      x78_ashGen = x128_parent.MakeAshGen();
      x80_ashPointIterator = 0;
      x78_ashGen->SetGlobalOrientAndTrans(actor->GetTransform());
      x84_ashMaxParticles = s32((IsMediumOrLarge(*actor) ? 1.f : 0.3f) * x78_ashGen->GetMaxParticles());
      x88_ashSeed = mgr.GetActiveRandom()->Next();
    }
    return true;
  }
  Unlock(EDependency::Ash);
  return false;
}

bool CActorModelParticles::CItem::UpdateIceGen(float dt, CActor* actor, CStateManager& mgr) {
  if (xb0_icePointIterator != -1)
    return true;
  if (!x8c_iceGens.empty()) {
    bool active = false;
    for (auto& p : x8c_iceGens) {
      if (!p->IsSystemDeletable())
        active = true;
      p->Update(dt);
    }
    if (!active)
      x8c_iceGens.clear();
    else
      return true;
  } else if (x134_lockDeps & 0x2 && actor) {
    if (x128_parent.xe6_loadedDeps & 0x2) {
      xb0_icePointIterator = 0;
      xb4_iceSeed = mgr.GetActiveRandom()->Next();
    }
    return true;
  }
  Unlock(EDependency::Ice);
  return false;
}

bool CActorModelParticles::CItem::UpdateFirePop(float dt, CActor* actor, CStateManager& mgr) {
  if (xb8_firePopGen) {
    if (xb8_firePopGen->IsSystemDeletable()) {
      xb8_firePopGen.reset();
    } else {
      xb8_firePopGen->Update(dt);
      return true;
    }
  } else if (x134_lockDeps & 0x8 && actor) {
    if (x128_parent.xe6_loadedDeps & 0x8) {
      xb8_firePopGen = x128_parent.MakeFirePopGen();
      xb8_firePopGen->SetGlobalOrientation(actor->GetTransform());
      xb8_firePopGen->SetGlobalTranslation(actor->GetRenderBounds().center());
    }
    return true;
  }
  Unlock(EDependency::FirePop);
  return false;
}

bool CActorModelParticles::CItem::UpdateElectric(float dt, CActor* actor, CStateManager& mgr) {
  if (xc0_electricGen) {
    if (xc0_electricGen->IsSystemDeletable()) {
      xc0_electricGen.reset();
    } else {
      if (actor && actor->GetActive()) {
        xc0_electricGen->SetGlobalOrientation(actor->GetTransform().getRotation());
        xc0_electricGen->SetGlobalTranslation(actor->GetTranslation());
      }
      if (!actor || actor->GetActive()) {
        xc0_electricGen->SetModulationColor(xd0_electricColor);
        xc0_electricGen->Update(dt);
        return true;
      }
    }
  } else if (x134_lockDeps & 0x10) {
    if (x128_parent.xe6_loadedDeps & 0x10) {
      xc0_electricGen = x128_parent.MakeElectricGen();
      xc0_electricGen->SetModulationColor(xd0_electricColor);
      xc8_electricPointIterator = 0;
      xcc_electricSeed = mgr.GetActiveRandom()->Next();
    }
    return true;
  }
  Unlock(EDependency::Electric);
  return false;
}

bool CActorModelParticles::CItem::UpdateRainSplash(float dt, CActor* actor, CStateManager& mgr) {
  if (xd4_rainSplashGen) {
    if (!xd4_rainSplashGen->IsRaining()) {
      xd4_rainSplashGen.reset();
    } else {
      xd4_rainSplashGen->Update(dt, mgr);
      return true;
    }
  }
  return false;
}

bool CActorModelParticles::CItem::UpdateBurn(float dt, CActor* actor, CStateManager& mgr) {
  if (!actor)
    xdc_ashy.Unlock();
  return xdc_ashy.IsLocked();
}

bool CActorModelParticles::CItem::UpdateIcePop(float dt, CActor* actor, CStateManager& mgr) {
  if (xe4_icePopGen) {
    if (xe4_icePopGen->IsSystemDeletable()) {
      xe4_icePopGen.reset();
    } else {
      xe4_icePopGen->Update(dt);
      return true;
    }
  } else if (x134_lockDeps & 0x20 && actor) {
    if (x128_parent.xe6_loadedDeps & 0x20) {
      xe4_icePopGen = x128_parent.MakeIcePopGen();
      xe4_icePopGen->SetGlobalOrientation(actor->GetTransform());
      xe4_icePopGen->SetGlobalTranslation(actor->GetRenderBounds().center());
    }
    return true;
  }
  Unlock(EDependency::IcePop);
  return false;
}

bool CActorModelParticles::CItem::Update(float dt, CStateManager& mgr) {
  CActor* act = static_cast<CActor*>(mgr.ObjectById(x0_id));
  if (act && act->HasModelData() && !act->GetModelData()->IsNull()) {
    xec_particleOffsetScale = act->GetModelData()->GetScale();
    xf8_iceXf = act->GetTransform();
    x4_areaId = act->GetAreaIdAlways();
  } else {
    x0_id = kInvalidUniqueId;
    x84_ashMaxParticles = 0;
    xb0_icePointIterator = -1;
    if (xc0_electricGen)
      xc0_electricGen->SetParticleEmission(false);
    if (x74_sfx) {
      CSfxManager::RemoveEmitter(x74_sfx);
      x74_sfx.reset();
    }
    x130_remTime -= dt;
    if (x130_remTime <= 0.f)
      return false;
  }
  bool ret = false;
  if (UpdateOnFire(dt, act, mgr))
    ret = true;
  if (UpdateAshGen(dt, act, mgr))
    ret = true;
  if (UpdateIceGen(dt, act, mgr))
    ret = true;
  if (UpdateFirePop(dt, act, mgr))
    ret = true;
  if (UpdateElectric(dt, act, mgr))
    ret = true;
  if (UpdateRainSplash(dt, act, mgr))
    ret = true;
  if (UpdateBurn(dt, act, mgr))
    ret = true;
  if (UpdateIcePop(dt, act, mgr))
    ret = true;
  return ret;
}

void CActorModelParticles::CItem::Lock(EDependency d) {
  if (!(x134_lockDeps & (1 << int(d)))) {
    x128_parent.IncrementDependency(d);
    x134_lockDeps |= (1 << int(d));
  }
}

void CActorModelParticles::CItem::Unlock(EDependency d) {
  if (x134_lockDeps & (1 << int(d))) {
    x128_parent.DecrementDependency(d);
    x134_lockDeps &= ~(1 << int(d));
  }
}

void CActorModelParticles::DecrementDependency(EDependency d) {
  Dependency& dep = x50_dgrps[int(d)];
  dep.Decrement();
  if (dep.x10_refCount == 0) {
    xe4_loadingDeps &= ~(1 << int(d));
    xe6_loadedDeps &= ~(1 << int(d));
    xe5_justLoadedDeps &= ~(1 << int(d));
  }
}

void CActorModelParticles::IncrementDependency(EDependency d) {
  x50_dgrps[int(d)].Increment();
  if (!(xe6_loadedDeps & (1 << int(d))))
    xe4_loadingDeps |= (1 << int(d));
}

CActorModelParticles::Dependency CActorModelParticles::GetParticleDGRPTokens(std::string_view name) const {
  Dependency ret = {};
  TToken<CDependencyGroup> dgrp = g_SimplePool->GetObj(name);
  const auto& vector = dgrp->GetObjectTagVector();
  ret.x0_tokens.reserve(vector.size());
  for (const SObjectTag& tag : vector) {
    ret.x0_tokens.push_back(g_SimplePool->GetObj(tag));
  }
  return ret;
}

void CActorModelParticles::LoadParticleDGRPs() {
  static constexpr std::array particleDGRPs{
      "Effect_OnFire_DGRP"sv,  "Effect_IceBreak_DGRP"sv, "Effect_Ash_DGRP"sv,
      "Effect_FirePop_DGRP"sv, "Effect_Electric_DGRP"sv, "Effect_IcePop_DGRP"sv,
  };

  for (const auto& dgrp : particleDGRPs) {
    x50_dgrps.push_back(GetParticleDGRPTokens(dgrp));
  }
}

std::unique_ptr<CElementGen> CActorModelParticles::MakeOnFireGen() const {
  return std::make_unique<CElementGen>(x18_onFire);
}

std::unique_ptr<CElementGen> CActorModelParticles::MakeAshGen() const {
  return std::make_unique<CElementGen>(x20_ash);
}

std::unique_ptr<CElementGen> CActorModelParticles::MakeIceGen() const {
  return std::make_unique<CElementGen>(x28_iceBreak);
}

std::unique_ptr<CElementGen> CActorModelParticles::MakeFirePopGen() const {
  return std::make_unique<CElementGen>(x30_firePop);
}

std::unique_ptr<CElementGen> CActorModelParticles::MakeIcePopGen() const {
  return std::make_unique<CElementGen>(x38_icePop);
}

std::unique_ptr<CParticleElectric> CActorModelParticles::MakeElectricGen() const {
  return std::make_unique<CParticleElectric>(x40_electric);
}

CActorModelParticles::CActorModelParticles() {
  x18_onFire = g_SimplePool->GetObj("Effect_OnFire");
  x20_ash = g_SimplePool->GetObj("Effect_Ash");
  x28_iceBreak = g_SimplePool->GetObj("Effect_IceBreak");
  x30_firePop = g_SimplePool->GetObj("Effect_FirePop");
  x38_icePop = g_SimplePool->GetObj("Effect_IcePop");
  x40_electric = g_SimplePool->GetObj("Effect_Electric");
  x48_ashy = g_SimplePool->GetObj("TXTR_Ashy");
  LoadParticleDGRPs();
}

void CActorModelParticles::AddStragglersToRenderer(const CStateManager& mgr) {
  bool isNotCold = mgr.GetThermalDrawFlag() != EThermalDrawFlag::Cold;
  bool isNotHot = mgr.GetThermalDrawFlag() != EThermalDrawFlag::Hot;

  for (CItem& item : x0_items) {
    if (item.x4_areaId != kInvalidAreaId) {
      const CGameArea* area = mgr.GetWorld()->GetAreaAlways(item.x4_areaId);
      if (!area->IsPostConstructed())
        continue;
      CGameArea::EOcclusionState occState = area->GetPostConstructed()->x10dc_occlusionState;
      if (occState == CGameArea::EOcclusionState::Occluded)
        continue;
    }
    if (mgr.GetObjectById(item.x0_id) &&
        ((isNotCold && item.x12c_24_thermalCold) || (isNotHot && item.x12c_25_thermalHot))) {
      item.x12c_24_thermalCold = false;
      item.x12c_25_thermalHot = false;
      continue;
    }
    if (isNotCold) {
      // Hot Draw
      for (auto& entry : item.x8_onFireGens) {
        std::unique_ptr<CElementGen>& gen = entry.first;
        if (gen) {
          g_Renderer->AddParticleGen(*gen);
        }
      }
      if (mgr.GetThermalDrawFlag() != EThermalDrawFlag::Hot && item.x78_ashGen)
        g_Renderer->AddParticleGen(*item.x78_ashGen);
      if (item.xb8_firePopGen)
        g_Renderer->AddParticleGen(*item.xb8_firePopGen);
      if (item.xc0_electricGen)
        g_Renderer->AddParticleGen(*item.xc0_electricGen);
    }
    if (isNotHot) {
      /* Cold Draw */
      for (std::unique_ptr<CElementGen>& gen : item.x8c_iceGens)
        g_Renderer->AddParticleGen(*gen);
      if (item.xe4_icePopGen)
        g_Renderer->AddParticleGen(*item.xe4_icePopGen);
    }
    if (isNotCold) {
      /* Thermal Reset */
      item.x12c_24_thermalCold = false;
      item.x12c_25_thermalHot = false;
    }
  }
}

void CActorModelParticles::UpdateLoad() {
  if (!xe4_loadingDeps) {
    return;
  }

  xe5_justLoadedDeps = 0;
  for (size_t i = 0; i < x50_dgrps.size(); ++i) {
    if ((xe4_loadingDeps & (1U << i)) != 0) {
      x50_dgrps[i].UpdateLoad();
      if (x50_dgrps[i].x14_loaded) {
        xe5_justLoadedDeps |= (1U << i);
        xe4_loadingDeps &= ~(1U << i);
      }
    }
  }
  xe6_loadedDeps |= xe5_justLoadedDeps;
}

void CActorModelParticles::Update(float dt, CStateManager& mgr) {
  UpdateLoad();
  for (auto it = x0_items.begin(); it != x0_items.end();) {
    if (!it->Update(dt, mgr)) {
      if (CActor* act = static_cast<CActor*>(mgr.ObjectById(it->x0_id)))
        act->SetPointGeneratorParticles(false);
      it = x0_items.erase(it);
      continue;
    }
    ++it;
  }
}

void CActorModelParticles::PointGenerator(void* ctx,
                                          const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn) {
  static_cast<CItem*>(ctx)->GeneratePoints(vn);
}

void CActorModelParticles::SetupHook(TUniqueId uid) {
  const auto search = FindSystem(uid);

  if (search == x0_items.cend()) {
    return;
  }

  CSkinnedModel::SetPointGeneratorFunc(&*search, PointGenerator);
}

std::list<CActorModelParticles::CItem>::iterator CActorModelParticles::FindSystem(TUniqueId uid) {
  return std::find_if(x0_items.begin(), x0_items.end(), [uid](const auto& entry) { return entry.x0_id == uid; });
}

std::list<CActorModelParticles::CItem>::const_iterator CActorModelParticles::FindSystem(TUniqueId uid) const {
  return std::find_if(x0_items.begin(), x0_items.end(), [uid](const auto& entry) { return entry.x0_id == uid; });
}

std::list<CActorModelParticles::CItem>::iterator CActorModelParticles::FindOrCreateSystem(CActor& act) {
  if (act.GetPointGeneratorParticles()) {
    for (auto it = x0_items.begin(); it != x0_items.end(); ++it)
      if (it->x0_id == act.GetUniqueId())
        return it;
  }

  act.SetPointGeneratorParticles(true);
  return x0_items.emplace(x0_items.end(), act, *this);
}

void CActorModelParticles::StartIce(CActor& act) {
  auto iter = FindOrCreateSystem(act);
  iter->Lock(EDependency::Ice);
}

void CActorModelParticles::StartElectric(CActor& act) {
  auto iter = FindOrCreateSystem(act);
  if (iter->xc0_electricGen && !iter->xc0_electricGen->GetParticleEmission())
    iter->xc0_electricGen->SetParticleEmission(true);
}

void CActorModelParticles::StopElectric(CActor& act) {
  if (act.GetPointGeneratorParticles()) {
    auto iter = FindSystem(act.GetUniqueId());
    if (iter != x0_items.cend() && iter->xc0_electricGen)
      iter->xc0_electricGen->SetParticleEmission(false);
  }
}

void CActorModelParticles::LoadAndStartElectric(CActor& act) {
  auto iter = FindOrCreateSystem(act);
  if (!iter->xc0_electricGen)
    iter->Lock(EDependency::Electric);
  else {
    if (!iter->xc0_electricGen->GetParticleEmission())
      iter->xc0_electricGen->SetParticleEmission(true);
  }
}

void CActorModelParticles::StopThermalHotParticles(CActor& act) {
  if (act.GetPointGeneratorParticles()) {
    auto iter = FindSystem(act.GetUniqueId());
    if (iter != x0_items.cend()) {
      for (auto& part : iter->x8_onFireGens)
        if (part.first)
          part.first->SetParticleEmission(false);
    }
  }
}

void CActorModelParticles::StartBurnDeath(CActor& act) {
  auto iter = FindOrCreateSystem(act);
  u16 sfx = SFXeff_x_smallburndeath_lp_00 - s16(IsMediumOrLarge(act));
  CSfxManager::AddEmitter(sfx, act.GetTranslation(), zeus::skZero3f, true, false, 0x7f, kInvalidAreaId);
  iter->xdc_ashy.Lock();
}

void CActorModelParticles::EnsureElectricLoaded(CActor& act) {
  auto iter = FindOrCreateSystem(act);
  iter->Lock(EDependency::IcePop);
}

void CActorModelParticles::EnsureFirePopLoaded(CActor& act) {
  auto iter = FindOrCreateSystem(act);
  iter->Lock(EDependency::FirePop);
}

void CActorModelParticles::EnsureIceBreakLoaded(CActor& act) {
  auto iter = FindOrCreateSystem(act);
  iter->Lock(EDependency::Ash);
}

void CActorModelParticles::LightDudeOnFire(CActor& act) {
  auto iter = FindOrCreateSystem(act);
  iter->Lock(EDependency::OnFire);
  if (iter->x6c_onFireDelayTimer <= 0.f)
    iter->x70_onFire = true;
}

const CTexture* CActorModelParticles::GetAshyTexture(const CActor& act) {
  auto iter = FindSystem(act.GetUniqueId());
  if (iter != x0_items.cend() && iter->xdc_ashy && iter->xdc_ashy.IsLoaded()) {
    iter->xdc_ashy->GetBooTexture()->setClampMode(boo::TextureClampMode::ClampToEdge);
    return iter->xdc_ashy.GetObj();
  }
  return nullptr;
}

void CActorModelParticles::AddRainSplashGenerator(CActor& act, CStateManager& mgr, u32 maxSplashes, u32 genRate,
                                                  float minZ) {
  auto it = FindOrCreateSystem(act);
  if (it->xd4_rainSplashGen)
    return;

  if (act.GetModelData() && !act.GetModelData()->IsNull())
    it->xd4_rainSplashGen =
        std::make_unique<CRainSplashGenerator>(act.GetModelData()->GetScale(), maxSplashes, genRate, minZ, 0.1875f);
}

void CActorModelParticles::RemoveRainSplashGenerator(CActor& act) {
  auto it = FindOrCreateSystem(act);
  it->xd4_rainSplashGen.reset();
}

void CActorModelParticles::Render(const CStateManager& mgr, const CActor& actor) const {
  zeus::CTransform backupModel = CGraphics::g_GXModelMatrix;
  auto search = FindSystem(actor.GetUniqueId());
  if (search == x0_items.end())
    return;
  if (search->x4_areaId != kInvalidAreaId) {
    const CGameArea* area = mgr.GetWorld()->GetAreaAlways(search->x4_areaId);
    if (!area->IsPostConstructed())
      return;
    if (area->GetOcclusionState() == CGameArea::EOcclusionState::Occluded)
      return;
  }
  if (mgr.GetThermalDrawFlag() != EThermalDrawFlag::Cold) {
    for (const auto& gen : search->x8_onFireGens)
      if (gen.first)
        gen.first->Render();
    if (mgr.GetThermalDrawFlag() != EThermalDrawFlag::Hot && search->x78_ashGen)
      search->x78_ashGen->Render();
    if (search->xb8_firePopGen)
      search->xb8_firePopGen->Render();
    if (search->xc0_electricGen)
      search->xc0_electricGen->Render();
    search->x134_lockDeps |= 0x80;
  }
  if (mgr.GetThermalDrawFlag() != EThermalDrawFlag::Hot) {
    for (const auto& gen : search->x8c_iceGens)
      gen->Render();
    if (search->xd4_rainSplashGen && actor.GetModelData() && !actor.GetModelData()->IsNull())
      search->xd4_rainSplashGen->Draw(actor.GetTransform());
    if (search->xe4_icePopGen)
      search->xe4_icePopGen->Render();
    search->x134_lockDeps |= 0x40;
  }
  CGraphics::SetModelMatrix(backupModel);
}

} // namespace urde
