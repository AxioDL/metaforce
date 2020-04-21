#include "Runtime/Character/CParticleDatabase.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CCharLayoutInfo.hpp"
#include "Runtime/Character/CPoseAsTransforms.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"

namespace urde {

CParticleDatabase::CParticleDatabase() = default;

void CParticleDatabase::CacheParticleDesc(const SObjectTag& tag) {}

void CParticleDatabase::CacheParticleDesc(const CCharacterInfo::CParticleResData& desc) {
  for (CAssetId id : desc.x0_part) {
    auto search = x0_particleDescs.find(id);
    if (search == x0_particleDescs.cend())
      x0_particleDescs[id] =
          std::make_shared<TLockedToken<CGenDescription>>(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), id}));
  }
  for (CAssetId id : desc.x10_swhc) {
    auto search = x14_swooshDescs.find(id);
    if (search == x14_swooshDescs.cend())
      x14_swooshDescs[id] =
          std::make_shared<TLockedToken<CSwooshDescription>>(g_SimplePool->GetObj(SObjectTag{FOURCC('SWHC'), id}));
  }
  for (CAssetId id : desc.x20_elsc) {
    auto search = x28_electricDescs.find(id);
    if (search == x28_electricDescs.cend())
      x28_electricDescs[id] =
          std::make_shared<TLockedToken<CElectricDescription>>(g_SimplePool->GetObj(SObjectTag{FOURCC('ELSC'), id}));
  }
}

void CParticleDatabase::SetModulationColorAllActiveEffectsForParticleDB(
    const zeus::CColor& color, std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map) {
  for (auto& e : map) {
    if (e.second)
      e.second->SetModulationColor(color);
  }
}

void CParticleDatabase::SetModulationColorAllActiveEffects(const zeus::CColor& color) {
  SetModulationColorAllActiveEffectsForParticleDB(color, x3c_rendererDrawLoop);
  SetModulationColorAllActiveEffectsForParticleDB(color, x50_firstDrawLoop);
  SetModulationColorAllActiveEffectsForParticleDB(color, x64_lastDrawLoop);
  SetModulationColorAllActiveEffectsForParticleDB(color, x78_rendererDraw);
  SetModulationColorAllActiveEffectsForParticleDB(color, x8c_firstDraw);
  SetModulationColorAllActiveEffectsForParticleDB(color, xa0_lastDraw);
}

void CParticleDatabase::SuspendAllActiveEffectsForParticleDB(
    CStateManager& mgr, std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map) {
  for (auto& e : map) {
    e.second->SetParticleEmission(false, mgr);
  }
}

void CParticleDatabase::SuspendAllActiveEffects(CStateManager& stateMgr) {
  SuspendAllActiveEffectsForParticleDB(stateMgr, x3c_rendererDrawLoop);
  SuspendAllActiveEffectsForParticleDB(stateMgr, x50_firstDrawLoop);
  SuspendAllActiveEffectsForParticleDB(stateMgr, x64_lastDrawLoop);
}

void CParticleDatabase::DeleteAllLightsForParticleDB(CStateManager& mgr,
                                                     std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map) {
  for (auto& e : map) {
    e.second->DeleteLight(mgr);
  }
}

void CParticleDatabase::DeleteAllLights(CStateManager& mgr) {
  DeleteAllLightsForParticleDB(mgr, x3c_rendererDrawLoop);
  DeleteAllLightsForParticleDB(mgr, x50_firstDrawLoop);
  DeleteAllLightsForParticleDB(mgr, x64_lastDrawLoop);
  DeleteAllLightsForParticleDB(mgr, x78_rendererDraw);
  DeleteAllLightsForParticleDB(mgr, x8c_firstDraw);
  DeleteAllLightsForParticleDB(mgr, xa0_lastDraw);
}

void CParticleDatabase::UpdateParticleGenDB(float dt, const CPoseAsTransforms& pose, const CCharLayoutInfo& charInfo,
                                            const zeus::CTransform& xf, const zeus::CVector3f& scale,
                                            CStateManager& stateMgr,
                                            std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map,
                                            bool deleteIfDone) {
  for (auto it = map.begin(); it != map.end();) {
    CParticleGenInfo& info = *it->second;
    if (info.GetIsActive()) {
      if (info.GetType() == EParticleGenType::Normal) {
        const CSegId segId = charInfo.GetSegIdFromString(info.GetLocatorName());
        if (segId.IsInvalid()) {
          ++it;
          continue;
        }
        if (!pose.ContainsDataFor(segId)) {
          ++it;
          continue;
        }
        const zeus::CVector3f& off = pose.GetOffset(segId);
        switch (info.GetParentedMode()) {
        case CParticleData::EParentedMode::Initial: {
          if (info.GetIsGrabInitialData()) {
            zeus::CTransform segXf((info.GetFlags() & 0x10) ? zeus::CMatrix3f()
                                                            : pose.GetTransformMinusOffset(segId),
                                   off * scale);
            zeus::CTransform compXf = xf * segXf;
            info.SetCurTransform(compXf.getRotation());
            info.SetCurOffset(compXf.origin);
            info.SetCurrentTime(0.f);
            info.SetIsGrabInitialData(false);
          }

          info.SetOrientation(info.GetCurTransform(), stateMgr);
          info.SetTranslation(info.GetCurOffset(), stateMgr);

          if (info.GetFlags() & 0x2000)
            info.SetGlobalScale(info.GetCurScale() * scale);
          else
            info.SetGlobalScale(info.GetCurScale());

          break;
        }
        case CParticleData::EParentedMode::ContinuousEmitter:
        case CParticleData::EParentedMode::ContinuousSystem: {
          if (info.GetIsGrabInitialData()) {
            info.SetCurrentTime(0.f);
            info.SetIsGrabInitialData(false);
          }

          zeus::CTransform segXf(pose.GetTransformMinusOffset(segId), off * scale);
          zeus::CTransform compXf = xf * segXf;

          if (info.GetParentedMode() == CParticleData::EParentedMode::ContinuousEmitter) {
            info.SetTranslation(compXf.origin, stateMgr);
            if (info.GetFlags() & 0x10)
              info.SetOrientation(xf.getRotation(), stateMgr);
            else
              info.SetOrientation(compXf.getRotation(), stateMgr);
          } else {
            info.SetGlobalTranslation(compXf.origin, stateMgr);
            if (info.GetFlags() & 0x10)
              info.SetGlobalOrientation(xf.getRotation(), stateMgr);
            else
              info.SetGlobalOrientation(compXf.getRotation(), stateMgr);
          }

          if (info.GetFlags() & 0x2000)
            info.SetGlobalScale(info.GetCurScale() * scale);
          else
            info.SetGlobalScale(info.GetCurScale());

          break;
        }
        default:
          break;
        }
      }

      float sec = (info.GetInactiveStartTime() == 0.f) ? 10000000.f : info.GetInactiveStartTime();
      if (info.GetCurrentTime() > sec) {
        info.SetIsActive(false);
        info.SetParticleEmission(false, stateMgr);
        info.MarkFinishTime();
        if (info.GetFlags() & 1)
          info.DestroyParticles();
      }
    }

    info.Update(dt, stateMgr);

    if (!info.GetIsActive()) {
      if (!info.HasActiveParticles() && info.GetCurrentTime() - info.GetFinishTime() > 5.f && deleteIfDone) {
        info.DeleteLight(stateMgr);
        it = map.erase(it);
        continue;
      }
    } else if (info.IsSystemDeletable()) {
      info.DeleteLight(stateMgr);
      it = map.erase(it);
      continue;
    }

    info.OffsetTime(dt);
    ++it;
  }
}

void CParticleDatabase::Update(float dt, const CPoseAsTransforms& pose, const CCharLayoutInfo& charInfo,
                               const zeus::CTransform& xf, const zeus::CVector3f& scale, CStateManager& stateMgr) {
  if (!xb4_24_updatesEnabled)
    return;

  UpdateParticleGenDB(dt, pose, charInfo, xf, scale, stateMgr, x3c_rendererDrawLoop, true);
  UpdateParticleGenDB(dt, pose, charInfo, xf, scale, stateMgr, x50_firstDrawLoop, true);
  UpdateParticleGenDB(dt, pose, charInfo, xf, scale, stateMgr, x64_lastDrawLoop, true);
  UpdateParticleGenDB(dt, pose, charInfo, xf, scale, stateMgr, x78_rendererDraw, false);
  UpdateParticleGenDB(dt, pose, charInfo, xf, scale, stateMgr, x8c_firstDraw, false);
  UpdateParticleGenDB(dt, pose, charInfo, xf, scale, stateMgr, xa0_lastDraw, false);

  xb4_25_anySystemsDrawnWithModel =
      (x50_firstDrawLoop.size() || x64_lastDrawLoop.size() || x8c_firstDraw.size() || xa0_lastDraw.size());
}

void CParticleDatabase::RenderParticleGenMap(const std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map) {
  for (auto& e : map) {
    e.second->Render();
  }
}

void CParticleDatabase::RenderParticleGenMapMasked(const std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map,
                                                   int mask, int target) {
  for (auto& e : map) {
    if ((e.second->GetFlags() & mask) == target)
      e.second->Render();
  }
}

void CParticleDatabase::AddToRendererClippedParticleGenMap(
    const std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map, const zeus::CFrustum& frustum) {
  for (auto& e : map) {
    auto bounds = e.second->GetBounds();
    if (bounds && frustum.aabbFrustumTest(*bounds))
      e.second->AddToRenderer();
  }
}

void CParticleDatabase::AddToRendererClippedParticleGenMapMasked(
    const std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map, const zeus::CFrustum& frustum, int mask,
    int target) {
  for (auto& e : map) {
    if ((e.second->GetFlags() & mask) == target) {
      auto bounds = e.second->GetBounds();
      if (bounds && frustum.aabbFrustumTest(*bounds))
        e.second->AddToRenderer();
    }
  }
}

void CParticleDatabase::RenderSystemsToBeDrawnLastMasked(int mask, int target) const {
  RenderParticleGenMapMasked(xa0_lastDraw, mask, target);
  RenderParticleGenMapMasked(x64_lastDrawLoop, mask, target);
}

void CParticleDatabase::RenderSystemsToBeDrawnLast() const {
  RenderParticleGenMap(xa0_lastDraw);
  RenderParticleGenMap(x64_lastDrawLoop);
}

void CParticleDatabase::RenderSystemsToBeDrawnFirstMasked(int mask, int target) const {
  RenderParticleGenMapMasked(x8c_firstDraw, mask, target);
  RenderParticleGenMapMasked(x50_firstDrawLoop, mask, target);
}

void CParticleDatabase::RenderSystemsToBeDrawnFirst() const {
  RenderParticleGenMap(x8c_firstDraw);
  RenderParticleGenMap(x50_firstDrawLoop);
}

void CParticleDatabase::AddToRendererClippedMasked(const zeus::CFrustum& frustum, int mask, int target) const {
  AddToRendererClippedParticleGenMapMasked(x78_rendererDraw, frustum, mask, target);
  AddToRendererClippedParticleGenMapMasked(x3c_rendererDrawLoop, frustum, mask, target);
}

void CParticleDatabase::AddToRendererClipped(const zeus::CFrustum& frustum) const {
  AddToRendererClippedParticleGenMap(x78_rendererDraw, frustum);
  AddToRendererClippedParticleGenMap(x3c_rendererDrawLoop, frustum);
}

CParticleGenInfo* CParticleDatabase::GetParticleEffect(std::string_view name) const {
  // TODO: Heterogeneous lookup when C++20 available
  auto search = x3c_rendererDrawLoop.find(name.data());
  if (search != x3c_rendererDrawLoop.end())
    return search->second.get();
  search = x50_firstDrawLoop.find(name.data());
  if (search != x50_firstDrawLoop.end())
    return search->second.get();
  search = x64_lastDrawLoop.find(name.data());
  if (search != x64_lastDrawLoop.end())
    return search->second.get();
  search = x78_rendererDraw.find(name.data());
  if (search != x78_rendererDraw.end())
    return search->second.get();
  search = x8c_firstDraw.find(name.data());
  if (search != x8c_firstDraw.end())
    return search->second.get();
  search = xa0_lastDraw.find(name.data());
  if (search != xa0_lastDraw.end())
    return search->second.get();
  return nullptr;
}

void CParticleDatabase::SetParticleEffectState(std::string_view name, bool active, CStateManager& mgr) {
  if (CParticleGenInfo* info = GetParticleEffect(name)) {
    info->SetParticleEmission(active, mgr);
    info->SetIsActive(active);
    if (!active && (info->GetFlags() & 1) != 0)
      info->DestroyParticles();
    info->SetIsGrabInitialData(true);
  }
}

void CParticleDatabase::SetCEXTValue(std::string_view name, int idx, float value) {
  if (CParticleGenInfo* info = GetParticleEffect(name)) {
    std::static_pointer_cast<CElementGen>(static_cast<CParticleGenInfoGeneric*>(info)->GetParticleSystem())
        ->SetExternalVar(idx, value);
  }
}

template <class T, class U>
static s32 _getGraphicLightId(const T& system, const U& desc) {
  if (system->SystemHasLight())
    return s32(desc.GetObjectTag()->id.Value());
  return -1;
}

void CParticleDatabase::AddAuxiliaryParticleEffect(std::string_view name, int flags, const CAuxiliaryParticleData& data,
                                                   const zeus::CVector3f& scale, CStateManager& mgr, TAreaId aid,
                                                   int lightId) {
  if (CParticleGenInfo* info = GetParticleEffect(name)) {
    if (!info->GetIsActive()) {
      info->SetParticleEmission(true, mgr);
      info->SetIsActive(true);
      info->SetIsGrabInitialData(true);
      info->SetFlags(flags);
    }
    return;
  }

  zeus::CVector3f scaleVec;
  if (flags & 0x2)
    scaleVec.splat(data.GetScale());
  else
    scaleVec = scale * data.GetScale();

  switch (data.GetTag().type.toUint32()) {
  case SBIG('PART'): {
    const auto search = x0_particleDescs.find(data.GetTag().id);
    if (search != x0_particleDescs.cend()) {
      auto sys = std::make_shared<CElementGen>(*search->second);
      auto newGen = std::make_unique<CParticleGenInfoGeneric>(
          data.GetTag(), sys, data.GetDuration(), "NOT_A_VALID_LOCATOR", scaleVec,
          CParticleData::EParentedMode::Initial, flags, mgr, aid, lightId + _getGraphicLightId(sys, *search->second),
          EParticleGenType::Auxiliary);

      newGen->SetGlobalTranslation(data.GetTranslation(), mgr);
      newGen->SetIsGrabInitialData(false);
      InsertParticleGen(false, flags, name, std::move(newGen));
    }
    break;
  }
  default:
    break;
  }
}

void CParticleDatabase::AddParticleEffect(std::string_view name, int flags, const CParticleData& data,
                                          const zeus::CVector3f& scale, CStateManager& mgr, TAreaId aid, bool oneShot,
                                          int lightId) {
  if (CParticleGenInfo* info = GetParticleEffect(name)) {
    if (!info->GetIsActive()) {
      info->SetParticleEmission(true, mgr);
      info->SetIsActive(true);
      info->SetIsGrabInitialData(true);
      info->SetFlags(flags);
    }
    return;
  }

  zeus::CVector3f scaleVec;
  if (flags & 0x2)
    scaleVec.splat(data.GetScale());
  else
    scaleVec = scale * data.GetScale();

  std::unique_ptr<CParticleGenInfo> newGen;
  switch (data.GetTag().type.toUint32()) {
  case SBIG('PART'): {
    const auto search = x0_particleDescs.find(data.GetTag().id);
    if (search != x0_particleDescs.cend()) {
      auto sys = std::make_shared<CElementGen>(*search->second);
      newGen = std::make_unique<CParticleGenInfoGeneric>(
          data.GetTag(), sys, data.GetDuration(), data.GetSegmentName(), scaleVec, data.GetParentedMode(), flags, mgr,
          aid, lightId + _getGraphicLightId(sys, *search->second), EParticleGenType::Normal);
    }
    break;
  }
  case SBIG('SWHC'): {
    const auto search = x14_swooshDescs.find(data.GetTag().id);
    if (search != x14_swooshDescs.cend()) {
      auto sys = std::make_shared<CParticleSwoosh>(*search->second, 0);
      newGen = std::make_unique<CParticleGenInfoGeneric>(data.GetTag(), sys, data.GetDuration(), data.GetSegmentName(),
                                                         scaleVec, data.GetParentedMode(), flags, mgr, aid, -1,
                                                         EParticleGenType::Normal);
    }
    break;
  }
  case SBIG('ELSC'): {
    const auto search = x28_electricDescs.find(data.GetTag().id);
    if (search != x28_electricDescs.cend()) {
      auto sys = std::make_shared<CParticleElectric>(*search->second);
      newGen = std::make_unique<CParticleGenInfoGeneric>(
          data.GetTag(), sys, data.GetDuration(), data.GetSegmentName(), scaleVec, data.GetParentedMode(), flags, mgr,
          aid, lightId + _getGraphicLightId(sys, *search->second), EParticleGenType::Normal);
    }
    break;
  }
  default:
    break;
  }

  if (newGen) {
    newGen->SetIsActive(true);
    newGen->SetParticleEmission(true, mgr);
    newGen->SetIsGrabInitialData(true);
    InsertParticleGen(oneShot, flags, name, std::move(newGen));
  }
}

void CParticleDatabase::InsertParticleGen(bool oneShot, int flags, std::string_view name,
                                          std::unique_ptr<CParticleGenInfo>&& gen) {
  std::map<std::string, std::unique_ptr<CParticleGenInfo>>* useMap;
  if (oneShot) {
    if (flags & 0x40)
      useMap = &xa0_lastDraw;
    else if (flags & 0x20)
      useMap = &x8c_firstDraw;
    else
      useMap = &x78_rendererDraw;
  } else {
    if (flags & 0x40)
      useMap = &x64_lastDrawLoop;
    else if (flags & 0x20)
      useMap = &x50_firstDrawLoop;
    else
      useMap = &x3c_rendererDrawLoop;
  }

  useMap->insert(std::make_pair(std::string(name), std::move(gen)));

  if (flags & 0x60)
    xb4_25_anySystemsDrawnWithModel = true;
}

} // namespace urde
