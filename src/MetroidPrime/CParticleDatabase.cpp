#include "MetroidPrime/CParticleDatabase.hpp"

#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CPoseAsTransforms.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "MetroidPrime/CParticleGenInfoGeneric.hpp"

CParticleDatabase::CParticleDatabase()
: mUpdatesEnabled(true), mAnySystemsDrawnWithModel(false) {}

CParticleDatabase::~CParticleDatabase() {}

void CParticleDatabase::CacheParticleDesc(const CCharacterInfo::CParticleResData& data) {
  for (rstl::vector< CAssetId >::const_iterator it = data.mPart.begin(), end = data.mPart.end();
       it != end; ++it) {
    const CAssetId id = *it;
    AUTO(cached, mParticleDescs.find(id));
    if (cached == mParticleDescs.end()) {
      rstl::rc_ptr< TLockedToken< CGenDescription > > desc(
          rs_new TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', id))));
      mParticleDescs.insert(
          rstl::pair< CAssetId, rstl::rc_ptr< TLockedToken< CGenDescription > > >(id, desc));
    }
  }
  for (rstl::vector< CAssetId >::const_iterator it = data.mSwhc.begin(),
                                                end = data.mSwhc.end();
       it != end; ++it) {
    const CAssetId id = *it;
    AUTO(cached, mSwooshDescs.find(id));
    if (cached == mSwooshDescs.end()) {
      rstl::rc_ptr< TLockedToken< CSwooshDescription > > desc(
          rs_new TLockedToken< CSwooshDescription >(gpSimplePool->GetObj(SObjectTag('SWHC', id))));
      mSwooshDescs.insert(
          rstl::pair< CAssetId, rstl::rc_ptr< TLockedToken< CSwooshDescription > > >(id, desc));
    }
  }
  for (rstl::vector< CAssetId >::const_iterator it = data.mElscA.begin(),
                                                end = data.mElscA.end();
       it != end; ++it) {
    const CAssetId id = *it;
    AUTO(cached, mElectricDescs.find(id));
    if (cached == mElectricDescs.end()) {
      rstl::rc_ptr< TLockedToken< CElectricDescription > > desc(
          rs_new TLockedToken< CElectricDescription >(
              gpSimplePool->GetObj(SObjectTag('ELSC', id))));
      mElectricDescs.insert(
          rstl::pair< CAssetId, rstl::rc_ptr< TLockedToken< CElectricDescription > > >(id, desc));
    }
  }
}

void CParticleDatabase::CacheParticleDesc(const SObjectTag& tag) {
  const CAssetId id = tag.GetId();
  switch (tag.GetType()) {
  case 'PART': {
    AUTO(cached, mParticleDescs.find(id));
    if (cached == mParticleDescs.end()) {
      rstl::rc_ptr< TLockedToken< CGenDescription > > desc(
          rs_new TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', id))));
      mParticleDescs.insert(
          rstl::pair< CAssetId, rstl::rc_ptr< TLockedToken< CGenDescription > > >(id, desc));
    }
    break;
  }
  case 'SWHC': {
    AUTO(cached, mSwooshDescs.find(id));
    if (cached == mSwooshDescs.end()) {
      rstl::rc_ptr< TLockedToken< CSwooshDescription > > desc(
          rs_new TLockedToken< CSwooshDescription >(gpSimplePool->GetObj(SObjectTag('SWHC', id))));
      mSwooshDescs.insert(
          rstl::pair< CAssetId, rstl::rc_ptr< TLockedToken< CSwooshDescription > > >(id, desc));
    }
    break;
  }
  case 'ELSC': {
    AUTO(cached, mElectricDescs.find(id));
    if (cached == mElectricDescs.end()) {
      rstl::rc_ptr< TLockedToken< CElectricDescription > > desc(
          rs_new TLockedToken< CElectricDescription >(
              gpSimplePool->GetObj(SObjectTag('ELSC', id))));
      mElectricDescs.insert(
          rstl::pair< CAssetId, rstl::rc_ptr< TLockedToken< CElectricDescription > > >(id, desc));
    }
    break;
  }
  }
}

void CParticleDatabase::InsertParticleGen(const bool oneShot, int flags, const rstl::string& name,
                                          const rstl::auto_ptr< CParticleGenInfo >& gen) {
  DrawMap* map;
  if (oneShot) {
    switch (flags & 0x60) {
    case 0x20:
      map = &mFirstDraw;
      break;
    case 0x40:
      map = &mLastDraw;
      break;
    default:
      map = &mRendererDraw;
      break;
    }
  } else {
    switch (flags & 0x60) {
    case 0x20:
      map = &mFirstDrawLoop;
      break;
    case 0x40:
      map = &mLastDrawLoop;
      break;
    default:
      map = &mRendererDrawLoop;
      break;
    }
  }
  map->insert(DrawMap::value_type(name, gen));
  if ((flags & 0x60) != 0) {
    mAnySystemsDrawnWithModel = true;
  }
}

template < class T, class U >
static int _getGraphicLightId(const T& system, const U& desc) {
  if (system->SystemHasLight()) {
    return desc.GetTag().GetId();
  }
  return -1;
}

void CParticleDatabase::AddParticleEffect(const rstl::string& name, int flags,
                                          const CParticleData& data, const CVector3f& scale,
                                          CStateManager& mgr, TAreaId areaId, bool oneShot,
                                          int particleLightIdx) {
  const SObjectTag tag = data.GetParticleAssetInfo();
  const int duration = data.GetDuration();
  const rstl::string& locator = data.GetLocatorName();
  const float scaleFactor = data.GetScale();
  const CParticleData::EParentedMode mode = data.GetParentedMode();
  CParticleGenInfo* effect = GetParticleEffect(name);
  if (effect == nullptr) {
    const FourCC type = tag.GetType();
    CVector3f particleScale(1.f, 1.f, 1.f);
    if (flags & 2) {
      particleScale *= scaleFactor;
    } else {
      particleScale = CVector3f(scaleFactor * scale.GetX(), scaleFactor * scale.GetY(),
                                scaleFactor * scale.GetZ());
    }
    rstl::auto_ptr< CParticleGenInfo > gen;
    if (type == 'PART') {
      AUTO(it, mParticleDescs.find(tag.GetId()));
      if (it != mParticleDescs.end()) {
        rstl::ncrc_ptr< CParticleGen > system = rs_new CElementGen(*it->second);
        const int lightId =
            particleLightIdx +
            _getGraphicLightId< rstl::ncrc_ptr< CParticleGen >, TToken< CGenDescription > >(
                system, *it->second);
        gen = rs_new CParticleGenInfoGeneric(tag, system, duration, locator, particleScale, mode,
                                             flags, mgr, areaId, lightId, kPGT_Normal);
      }
    } else if (type == 'SWHC') {
      AUTO(it, mSwooshDescs.find(tag.GetId()));
      if (it != mSwooshDescs.end()) {
        rstl::ncrc_ptr< CParticleGen > system = rs_new CParticleSwoosh(*it->second, 0);
        gen = rs_new CParticleGenInfoGeneric(tag, system, duration, locator, particleScale, mode,
                                             flags, mgr, areaId, -1, kPGT_Normal);
      }
    } else if (type == 'ELSC') {
      AUTO(it, mElectricDescs.find(tag.GetId()));
      if (it != mElectricDescs.end()) {
        rstl::ncrc_ptr< CParticleGen > system = rs_new CParticleElectric(*it->second);
        const int lightId =
            particleLightIdx +
            _getGraphicLightId< rstl::ncrc_ptr< CParticleGen >, TToken< CElectricDescription > >(
                system, *it->second);
        gen = rs_new CParticleGenInfoGeneric(tag, system, duration, locator, particleScale, mode,
                                             flags, mgr, areaId, lightId, kPGT_Normal);
      }
    }
    if (!gen.null()) {
      gen->SetIsActive(true);
      gen->SetParticleEmission(true, mgr);
      gen->SetIsGrabInitialData(true);
      InsertParticleGen(oneShot, flags, name, gen);
    }
  } else if (!effect->GetIsActive()) {
    effect->SetParticleEmission(true, mgr);
    effect->SetIsActive(true);
    effect->SetIsGrabInitialData(true);
    effect->SetFlags(flags);
  }
}

void CParticleDatabase::AddAuxiliaryParticleEffect(const rstl::string& name, int flags,
                                                   const CAuxiliaryParticleData& data,
                                                   const CVector3f& scale, CStateManager& mgr,
                                                   TAreaId areaId, int particleLightIdx) {
  const SObjectTag tag = data.GetParticleAssetInfo();
  const int duration = data.GetDuration();
  const float scaleFactor = data.GetScale();
  CParticleGenInfo* effect = GetParticleEffect(name);
  if (effect == nullptr) {
    const FourCC type = tag.GetType();
    CVector3f particleScale(1.f, 1.f, 1.f);
    if (flags & 2) {
      particleScale *= scaleFactor;
    } else {
      particleScale = CVector3f(scaleFactor * scale.GetX(), scaleFactor * scale.GetY(),
                                scaleFactor * scale.GetZ());
    }
    rstl::auto_ptr< CParticleGenInfo > gen;
    if (type == 'PART') {
      AUTO(it, mParticleDescs.find(tag.GetId()));
      if (it != mParticleDescs.end()) {
        rstl::ncrc_ptr< CParticleGen > system = rs_new CElementGen(*it->second);
        const int lightId =
            particleLightIdx +
            _getGraphicLightId< rstl::ncrc_ptr< CParticleGen >, TToken< CGenDescription > >(
                system, *it->second);
        gen = rs_new CParticleGenInfoGeneric(tag, system, duration, "NOT_A_VALID_LOCATOR",
                                             particleScale, CParticleData::kPM_Initial, flags, mgr,
                                             areaId, lightId, kPGT_Auxiliary);
        gen->SetGlobalTranslation(data.GetTranslation(), mgr);
        gen->SetIsGrabInitialData(false);
        InsertParticleGen(false, flags, name, gen);
      } else {
        SObjectTag::Type2Text(type);
      }
    }
  } else if (!effect->GetIsActive()) {
    effect->SetParticleEmission(true, mgr);
    effect->SetIsActive(true);
    effect->SetIsGrabInitialData(true);
    effect->SetFlags(flags);
  }
}

CParticleGenInfo* CParticleDatabase::GetParticleEffect(const rstl::string& name) {
  {
    AUTO(it, mRendererDrawLoop.find(name));
    if (it != mRendererDrawLoop.end()) {
      return it->second.get();
    }
  }
  {
    DrawMap::const_iterator it = mFirstDrawLoop.find(name);
    if (it != mFirstDrawLoop.end()) {
      return it->second.get();
    }
  }
  {
    DrawMap::const_iterator it = mLastDrawLoop.find(name);
    if (it != mLastDrawLoop.end()) {
      return it->second.get();
    }
  }
  {
    DrawMap::const_iterator it = mRendererDraw.find(name);
    if (it != mRendererDraw.end()) {
      return it->second.get();
    }
  }
  {
    DrawMap::const_iterator it = mFirstDraw.find(name);
    if (it != mFirstDraw.end()) {
      return it->second.get();
    }
  }
  {
    DrawMap::const_iterator it = mLastDraw.find(name);
    if (it != mLastDraw.end()) {
      return it->second.get();
    }
  }
  return nullptr;
}

void CParticleDatabase::SetParticleEffectState(const rstl::string& name, bool active,
                                               CStateManager& mgr) {
  CParticleGenInfo* effect = GetParticleEffect(name);
  if (effect != nullptr) {
    effect->SetParticleEmission(active, mgr);
    effect->SetIsActive(active);
    if (!active && (effect->GetFlags() & 1)) {
      effect->DestroyParticles();
    }
    effect->SetIsGrabInitialData(true);
  }
}

void CParticleDatabase::SetExternalVarValue(const rstl::string& name, int index, float value) {
  CParticleGenInfo* effect = GetParticleEffect(name);
  if (effect != nullptr) {
    CElementGen* system = static_cast< CElementGen* >(
        static_cast< CParticleGenInfoGeneric* >(effect)->GetParticleSystem().GetPtr());
    system->SetExternalVar(index, value);
  }
}

void CParticleDatabase::Update(float dt, const CPoseAsTransforms& pose,
                               const CCharLayoutInfo& layout, const CTransform4f& xf,
                               const CVector3f& scale, CStateManager& mgr) {
  if (!mUpdatesEnabled) {
    return;
  }
  UpdateParticleGenDB(dt, pose, layout, xf, scale, mgr, mRendererDrawLoop, true);
  UpdateParticleGenDB(dt, pose, layout, xf, scale, mgr, mFirstDrawLoop, true);
  UpdateParticleGenDB(dt, pose, layout, xf, scale, mgr, mLastDrawLoop, true);
  UpdateParticleGenDB(dt, pose, layout, xf, scale, mgr, mRendererDraw, false);
  UpdateParticleGenDB(dt, pose, layout, xf, scale, mgr, mFirstDraw, false);
  UpdateParticleGenDB(dt, pose, layout, xf, scale, mgr, mLastDraw, false);
  mAnySystemsDrawnWithModel = mFirstDrawLoop.size() || mLastDrawLoop.size() ||
                                    mFirstDraw.size() || mLastDraw.size();
}

void CParticleDatabase::UpdateParticleGenDB(float dt, const CPoseAsTransforms& pose,
                                            const CCharLayoutInfo& layout, const CTransform4f& xf,
                                            const CVector3f& scale, CStateManager& mgr,
                                            DrawMap& map, bool deleteIfDone) {
  AUTO(it, map.begin());
  while (it != map.end()) {
    CParticleGenInfo& info = *it->second;
    if (info.GetIsActive()) {
      switch (info.GetType()) {
      case kPGT_Normal: {
        const CParticleData::EParentedMode mode = info.GetParentedMode();
        const CSegId seg = layout.GetSegIdFromString(info.GetLocatorName());
        if (seg == CSegId::Invalid()) {
          ++it;
          continue;
        }
        if (!pose.ContainsDataFor(seg)) {
          ++it;
          continue;
        }
        const CVector3f offset = pose.GetOffset(seg);
        switch (mode) {
        case CParticleData::kPM_Initial:
          if (info.GetIsGrabInitialData()) {
            const uint initialFlags = info.GetFlags();
            CTransform4f segXf((initialFlags & 0x10) ? CMatrix3f::Identity()
                                                     : pose.GetTransformMinusOffset(seg),
                               CVector3f(offset.GetX() * scale.GetX(), offset.GetY() * scale.GetY(),
                                         offset.GetZ() * scale.GetZ()));
            CTransform4f composed(xf * segXf);
            info.SetCurTransform(composed.GetRotation());
            info.SetCurOffset(composed.GetTranslation());
            info.ResetTime();
            info.SetIsGrabInitialData(false);
          }
          info.SetOrientation(info.GetCurTransform(), mgr);
          info.SetTranslation(info.GetCurOffset(), mgr);
          if (info.GetFlags() & 0x2000) {
            const CVector3f& particleScale = info.GetScale();
            info.SetGlobalScale(CVector3f(particleScale.GetX() * scale.GetX(),
                                          particleScale.GetY() * scale.GetY(),
                                          particleScale.GetZ() * scale.GetZ()));
          } else {
            info.SetGlobalScale(info.GetScale());
          }
          break;
        case CParticleData::kPM_ContinuousEmitter:
        case CParticleData::kPM_ContinuousSystem: {
          if (info.GetIsGrabInitialData()) {
            info.ResetTime();
            info.SetIsGrabInitialData(false);
          }
          CTransform4f segXf(pose.GetTransformMinusOffset(seg),
                             CVector3f(offset.GetX() * scale.GetX(), offset.GetY() * scale.GetY(),
                                       offset.GetZ() * scale.GetZ()));
          CTransform4f composed(xf * segXf);
          if (mode == CParticleData::kPM_ContinuousEmitter) {
            info.SetTranslation(composed.GetTranslation(), mgr);
            if (info.GetFlags() & 0x10) {
              info.SetOrientation(xf.GetRotation(), mgr);
            } else {
              info.SetOrientation(composed.GetRotation(), mgr);
            }
          } else {
            info.SetGlobalTranslation(composed.GetTranslation(), mgr);
            if (info.GetFlags() & 0x10) {
              info.SetGlobalOrientation(xf.GetRotation(), mgr);
            } else {
              info.SetGlobalOrientation(composed.GetRotation(), mgr);
            }
          }
          if (info.GetFlags() & 0x2000) {
            const CVector3f& particleScale = info.GetScale();
            info.SetGlobalScale(CVector3f(particleScale.GetX() * scale.GetX(),
                                          particleScale.GetY() * scale.GetY(),
                                          particleScale.GetZ() * scale.GetZ()));
          } else {
            info.SetGlobalScale(info.GetScale());
          }
          break;
        }
        }
        break;
      }
      case kPGT_Auxiliary:
        break;
      }
      const float duration =
          info.GetInactiveStartTime() == 0.f ? 10000000.f : info.GetInactiveStartTime();
      if (info.GetCurrentTime() > duration) {
        info.SetIsActive(false);
        info.SetParticleEmission(false, mgr);
        info.MarkFinishTime();
        if (info.GetFlags() & 1) {
          info.DestroyParticles();
        }
      }
    }
    info.Update(dt, mgr);
    if (!info.GetIsActive()) {
      if (!info.HasActiveParticles() && info.GetCurrentTime() - info.GetFinishTime() > 5.f &&
          deleteIfDone) {
        info.DeleteLight(mgr);
        it = map.erase(it);
        continue;
      }
    } else if (info.IsSystemDeletable()) {
      info.DeleteLight(mgr);
      it = map.erase(it);
      continue;
    }
    info.OffsetTime(dt);
    ++it;
  }
}

void CParticleDatabase::AddToRendererClipped(const CFrustumPlanes& frustum) const {
  AddToRendererClippedParticleGenMap(mRendererDraw, frustum);
  AddToRendererClippedParticleGenMap(mRendererDrawLoop, frustum);
}

void CParticleDatabase::AddToRendererClippedMasked(const CFrustumPlanes& frustum, int mask,
                                                   int target) const {
  AddToRendererClippedParticleGenMapMasked(mRendererDraw, frustum, mask, target);
  AddToRendererClippedParticleGenMapMasked(mRendererDrawLoop, frustum, mask, target);
}

void CParticleDatabase::AddToRendererClippedParticleGenMap(const DrawMap& map,
                                                           const CFrustumPlanes& frustum) const {
  for (AUTO(it, map.begin()); it != map.end(); ++it) {
    CParticleGenInfo* const gen = it->second.get();
    if ((frustum.BoxInFrustumPlanes(gen->GetBounds()) == true)) {
      gen->AddToRenderer();
    }
  }
}

void CParticleDatabase::AddToRendererClippedParticleGenMapMasked(const DrawMap& map,
                                                                 const CFrustumPlanes& frustum,
                                                                 int mask, int target) const {
  for (AUTO(it, map.begin()); it != map.end(); ++it) {
    CParticleGenInfo* const gen = it->second.get();
    if ((gen->GetFlags() & mask) == target &&
        (frustum.BoxInFrustumPlanes(gen->GetBounds()) == true)) {
      gen->AddToRenderer();
    }
  }
}

void CParticleDatabase::RenderSystemsToBeDrawnFirst() const {
  RenderParticleGenMap(mFirstDraw);
  RenderParticleGenMap(mFirstDrawLoop);
}

void CParticleDatabase::RenderSystemsToBeDrawnFirstMasked(int mask, int target) const {
  RenderParticleGenMapMasked(mFirstDraw, mask, target);
  RenderParticleGenMapMasked(mFirstDrawLoop, mask, target);
}

void CParticleDatabase::RenderSystemsToBeDrawnLast() const {
  RenderParticleGenMap(mLastDraw);
  RenderParticleGenMap(mLastDrawLoop);
}

void CParticleDatabase::RenderSystemsToBeDrawnLastMasked(int mask, int target) const {
  RenderParticleGenMapMasked(mLastDraw, mask, target);
  RenderParticleGenMapMasked(mLastDrawLoop, mask, target);
}

void CParticleDatabase::RenderParticleGenMap(const DrawMap& map) {
  for (AUTO(it, map.begin()); it != map.end(); ++it) {
    it->second->Render();
  }
}

void CParticleDatabase::RenderParticleGenMapMasked(const DrawMap& map, int mask, int target) {
  for (AUTO(it, map.begin()); it != map.end(); ++it) {
    if ((it->second->GetFlags() & mask) == target) {
      it->second->Render();
    }
  }
}

void CParticleDatabase::DeleteAllLights(CStateManager& mgr) {
  DeleteAllLightsForParticleDB(mgr, mRendererDrawLoop);
  DeleteAllLightsForParticleDB(mgr, mFirstDrawLoop);
  DeleteAllLightsForParticleDB(mgr, mLastDrawLoop);
  DeleteAllLightsForParticleDB(mgr, mRendererDraw);
  DeleteAllLightsForParticleDB(mgr, mFirstDraw);
  DeleteAllLightsForParticleDB(mgr, mLastDraw);
}

void CParticleDatabase::DeleteAllLightsForParticleDB(CStateManager& mgr, DrawMap& map) {
  for (AUTO(it, map.begin()); it != map.end(); ++it) {
    it->second->DeleteLight(mgr);
  }
}

void CParticleDatabase::SuspendAllActiveEffects(CStateManager& mgr) {
  SuspendAllActiveEffectsForParticleDB(mgr, mRendererDrawLoop);
  SuspendAllActiveEffectsForParticleDB(mgr, mFirstDrawLoop);
  SuspendAllActiveEffectsForParticleDB(mgr, mLastDrawLoop);
}

void CParticleDatabase::SuspendAllActiveEffectsForParticleDB(CStateManager& mgr, DrawMap& map) {
  for (AUTO(it, map.begin()); it != map.end(); ++it) {
    it->second->SetParticleEmission(false, mgr);
  }
}

void CParticleDatabase::SetModulationColorAllActiveEffects(const CColor& color) {
  SetModulationColorAllActiveEffectsForParticleDB(color, mRendererDrawLoop);
  SetModulationColorAllActiveEffectsForParticleDB(color, mFirstDrawLoop);
  SetModulationColorAllActiveEffectsForParticleDB(color, mLastDrawLoop);
  SetModulationColorAllActiveEffectsForParticleDB(color, mRendererDraw);
  SetModulationColorAllActiveEffectsForParticleDB(color, mFirstDraw);
  SetModulationColorAllActiveEffectsForParticleDB(color, mLastDraw);
}

void CParticleDatabase::SetModulationColorAllActiveEffectsForParticleDB(const CColor& color,
                                                                        DrawMap& map) {
  for (AUTO(it, map.begin()); it != map.end(); ++it) {
    if (!it->second.null()) {
      it->second->SetModulationColor(color);
    }
  }
}
