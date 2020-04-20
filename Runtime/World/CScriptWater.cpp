#include "Runtime/World/CScriptWater.hpp"

#include <array>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/World/CFluidPlaneGPU.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
constexpr std::array kSplashScales{
    1.0f, 3.0f, 0.709f, 1.19f, 0.709f, 1.f,
};

CScriptWater::CScriptWater(
    CStateManager& mgr, TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CVector3f& pos,
    const zeus::CAABox& box, const urde::CDamageInfo& dInfo, zeus::CVector3f& orientedForce, ETriggerFlags triggerFlags,
    bool thermalCold, bool allowRender, CAssetId patternMap1, CAssetId patternMap2, CAssetId colorMap, CAssetId bumpMap,
    CAssetId envMap, CAssetId envBumpMap, CAssetId unusedMap, const zeus::CVector3f& bumpLightDir, float bumpScale,
    float morphInTime, float morphOutTime, bool active, EFluidType fluidType, bool b4, float alpha,
    const CFluidUVMotion& uvMot, float turbSpeed, float turbDistance, float turbFreqMax, float turbFreqMin,
    float turbPhaseMax, float turbPhaseMin, float turbAmplitudeMax, float turbAmplitudeMin,
    const zeus::CColor& splashColor, const zeus::CColor& insideFogColor, CAssetId splashParticle1,
    CAssetId splashParticle2, CAssetId splashParticle3, CAssetId visorRunoffParticle,
    CAssetId unmorphVisorRunoffparticle, s32 visorRunoffSfx, s32 unmorphVisorRunoffSfx, s32 splashSfx1, s32 splashSfx2,
    s32 splashSfx3, float tileSize, u32 tileSubdivisions, float specularMin, float specularMax, float reflectionSize,
    float rippleIntensity, float reflectionBlend, float fogBias, float fogMagnitude, float fogSpeed,
    const zeus::CColor& fogColor, CAssetId lightmapId, float unitsPerLightmapTexel, float alphaInTime,
    float alphaOutTime, u32, u32, bool, s32, s32, std::unique_ptr<u32[]>&& u32Arr)
: CScriptTrigger(uid, name, info, pos, box, dInfo, orientedForce, triggerFlags, active, false, false)
, x1b8_positionMorphed(pos)
, x1c4_extentMorphed(box.max - box.min)
, x1d0_morphInTime(morphInTime)
, x1d4_positionOrig(pos)
, x1e0_extentOrig(box.max - box.min)
, x1ec_damageOrig(dInfo.GetDamage())
, x1f0_damageMorphed(dInfo.GetDamage())
, x1f4_morphOutTime(morphOutTime)
, x214_fogBias(fogBias)
, x218_fogMagnitude(fogMagnitude)
, x21c_origFogBias(fogBias)
, x220_origFogMagnitude(fogMagnitude)
, x224_fogSpeed(fogSpeed)
, x228_fogColor(fogColor)
, x22c_splashParticle1Id(splashParticle1)
, x230_splashParticle2Id(splashParticle2)
, x234_splashParticle3Id(splashParticle3)
, x238_visorRunoffParticleId(visorRunoffParticle)
, x24c_unmorphVisorRunoffParticleId(unmorphVisorRunoffparticle)
, x260_visorRunoffSfx(CSfxManager::TranslateSFXID(visorRunoffSfx))
, x262_unmorphVisorRunoffSfx(CSfxManager::TranslateSFXID(unmorphVisorRunoffSfx))
, x2a4_splashColor(splashColor)
, x2a8_insideFogColor(insideFogColor)
, x2ac_alphaInTime(alphaInTime)
, x2b0_alphaOutTime(alphaOutTime)
, x2b4_alphaInRecip((alphaInTime != 0.f) ? 1.f / alphaInTime : 0.f)
, x2b8_alphaOutRecip((alphaOutTime != 0.f) ? 1.f / alphaOutTime : 0.f)
, x2bc_alpha(alpha)
, x2c0_tileSize(tileSize)
, x2e8_24_b4(b4)
, x2e8_27_allowRender(allowRender) {
  zeus::CAABox triggerAABB = GetTriggerBoundsWR();
  x2c4_gridDimX = u32((x2c0_tileSize + triggerAABB.max.x() - triggerAABB.min.x() - 0.01f) / x2c0_tileSize);
  x2c8_gridDimY = u32((x2c0_tileSize + triggerAABB.max.y() - triggerAABB.min.y() - 0.01f) / x2c0_tileSize);
  x2cc_gridCellCount = (x2c4_gridDimX + 1) * (x2c8_gridDimY + 1);

  uint32_t maxPatchSize;
  if (CGraphics::g_BooFactory->isTessellationSupported(maxPatchSize))
    x1b4_fluidPlane = std::make_unique<CFluidPlaneGPU>(
        patternMap1, patternMap2, colorMap, bumpMap, envMap, envBumpMap, lightmapId, unitsPerLightmapTexel, tileSize,
        tileSubdivisions * 2, fluidType, x2bc_alpha, bumpLightDir, bumpScale, uvMot, turbSpeed, turbDistance,
        turbFreqMax, turbFreqMin, turbPhaseMax, turbPhaseMin, turbAmplitudeMax, turbAmplitudeMin, specularMin,
        specularMax, reflectionBlend, reflectionSize, rippleIntensity,
        x2cc_gridCellCount * ((std::max(u32(2), tileSubdivisions * 2) * 4 + 2) * 4));
  else
    x1b4_fluidPlane = std::make_unique<CFluidPlaneCPU>(
        patternMap1, patternMap2, colorMap, bumpMap, envMap, envBumpMap, lightmapId, unitsPerLightmapTexel, tileSize,
        tileSubdivisions, fluidType, x2bc_alpha, bumpLightDir, bumpScale, uvMot, turbSpeed, turbDistance, turbFreqMax,
        turbFreqMin, turbPhaseMax, turbPhaseMin, turbAmplitudeMax, turbAmplitudeMin, specularMin, specularMax,
        reflectionBlend, reflectionSize, rippleIntensity,
        x2cc_gridCellCount * ((std::max(u32(2), tileSubdivisions) * 4 + 2) * 4));
  u32Arr.reset();
  x264_splashEffects.resize(3);
  if (x22c_splashParticle1Id.IsValid())
    x264_splashEffects[0].emplace(g_SimplePool->GetObj({FOURCC('PART'), x22c_splashParticle1Id}));
  if (x230_splashParticle2Id.IsValid())
    x264_splashEffects[1].emplace(g_SimplePool->GetObj({FOURCC('PART'), x230_splashParticle2Id}));
  if (x234_splashParticle3Id.IsValid())
    x264_splashEffects[2].emplace(g_SimplePool->GetObj({FOURCC('PART'), x234_splashParticle3Id}));
  if (x238_visorRunoffParticleId.IsValid())
    x23c_visorRunoffEffect.emplace(g_SimplePool->GetObj({FOURCC('PART'), x238_visorRunoffParticleId}));
  if (x24c_unmorphVisorRunoffParticleId.IsValid())
    x250_unmorphVisorRunoffEffect.emplace(g_SimplePool->GetObj({FOURCC('PART'), x24c_unmorphVisorRunoffParticleId}));
  x298_splashSounds.push_back(CSfxManager::TranslateSFXID(splashSfx1));
  x298_splashSounds.push_back(CSfxManager::TranslateSFXID(splashSfx2));
  x298_splashSounds.push_back(CSfxManager::TranslateSFXID(splashSfx3));
  SetCalculateLighting(true);
  if (lightmapId.IsValid())
    x90_actorLights->DisableAreaLights();
  x90_actorLights->SetMaxDynamicLights(4);
  x90_actorLights->SetCastShadows(false);
  x90_actorLights->SetAmbienceGenerated(false);
  x90_actorLights->SetFindNearestDynamicLights(true);
  x148_24_detectCamera = true;
  CalculateRenderBounds();
  xe6_27_thermalVisorFlags = u8(thermalCold ? 2 : 1);
  if (!x30_24_active) {
    x2bc_alpha = 0.f;
    x214_fogBias = 0.f;
    x218_fogMagnitude = 0.f;
  }
  SetupGrid(true);
}

void CScriptWater::SetupGrid(bool recomputeClipping) {
  zeus::CAABox triggerAABB = GetTriggerBoundsWR();
  auto dimX = u32((triggerAABB.max.x() - triggerAABB.min.x() + x2c0_tileSize) / x2c0_tileSize);
  auto dimY = u32((triggerAABB.max.y() - triggerAABB.min.y() + x2c0_tileSize) / x2c0_tileSize);
  x2e4_computedGridCellCount = x2cc_gridCellCount = (dimX + 1) * (dimY + 1);
  x2dc_vertIntersects.reset();
  if (!x2d8_tileIntersects || dimX != x2c4_gridDimX || dimY != x2c8_gridDimY)
    x2d8_tileIntersects.reset(new bool[x2cc_gridCellCount]);
  x2c4_gridDimX = dimX;
  x2c8_gridDimY = dimY;
  for (int i = 0; i < x2c8_gridDimY; ++i)
    for (int j = 0; j < x2c4_gridDimX; ++j)
      x2d8_tileIntersects[i * x2c4_gridDimX + j] = true;
  if (!x2e0_patchIntersects || x2d0_patchDimX != 0 || x2d4_patchDimY != 0)
    x2e0_patchIntersects.reset(new u8[32]);
  for (int i = 0; i < 32; ++i)
    x2e0_patchIntersects[i] = 1;
  x2d4_patchDimY = 0;
  x2d0_patchDimX = 0;
  x2e8_28_recomputeClipping = recomputeClipping;
}

constexpr CMaterialFilter SolidFilter = CMaterialFilter::MakeInclude({EMaterialTypes::Solid});

void CScriptWater::SetupGridClipping(CStateManager& mgr, int computeVerts) {
  if (x2e8_28_recomputeClipping) {
    x2e4_computedGridCellCount = 0;
    x2dc_vertIntersects.reset();
    x2e8_28_recomputeClipping = false;
  }

  if (x2e4_computedGridCellCount >= x2cc_gridCellCount)
    return;

  if (!x2dc_vertIntersects)
    x2dc_vertIntersects.reset(new bool[(x2c4_gridDimX + 1) * (x2c8_gridDimY + 1)]);
  zeus::CAABox triggerBounds = GetTriggerBoundsWR();
  zeus::CVector3f basePos = triggerBounds.min;
  basePos.z() = triggerBounds.max.z() + 0.8f;
  auto gridDiv = std::div(x2e4_computedGridCellCount, x2c4_gridDimX + 1);
  float yOffset = x2c0_tileSize * gridDiv.quot;
  float xOffset = x2c0_tileSize * gridDiv.rem;
  float mag = std::min(120.f, 2.f * (x130_bounds.max.z() - x130_bounds.min.z()) + 0.8f);
  for (int i = x2e4_computedGridCellCount; i < std::min(x2e4_computedGridCellCount + computeVerts, x2cc_gridCellCount);
       ++i) {
    zeus::CVector3f pos = basePos;
    pos.x() += xOffset;
    pos.y() += yOffset;
    x2dc_vertIntersects[i] = mgr.RayStaticIntersection(pos, zeus::skDown, mag, SolidFilter).IsValid();
    gridDiv.rem += 1;
    xOffset += x2c0_tileSize;
    if (gridDiv.rem > x2c4_gridDimX) {
      yOffset += x2c0_tileSize;
      xOffset = 0.f;
      gridDiv.rem = 0;
    }
  }
  x2e4_computedGridCellCount += computeVerts;
  if (x2e4_computedGridCellCount < x2cc_gridCellCount)
    return;

  x2e4_computedGridCellCount = x2cc_gridCellCount;
  x2d8_tileIntersects.reset(new bool[x2cc_gridCellCount]);

  for (int i = 0; i < x2c8_gridDimY; ++i) {
    int rowBase = x2c4_gridDimX * i;
    int nextRowBase = (x2c4_gridDimX + 1) * i;
    for (int j = 0; j < x2c4_gridDimX; ++j) {
      x2d8_tileIntersects[rowBase + j] = x2dc_vertIntersects[nextRowBase + j] ||
                                         x2dc_vertIntersects[nextRowBase + j + 1] ||
                                         x2dc_vertIntersects[nextRowBase + j + x2c4_gridDimX + 1] ||
                                         x2dc_vertIntersects[nextRowBase + j + x2c4_gridDimX + 2];
    }
  }

  int tilesPerPatch = std::min(42 / x1b4_fluidPlane->GetTileSubdivisions(), 7);
  x2d0_patchDimX = (tilesPerPatch + x2c4_gridDimX - 1) / tilesPerPatch;
  x2d4_patchDimY = (tilesPerPatch + x2c8_gridDimY - 1) / tilesPerPatch;
  x2e0_patchIntersects.reset(new u8[x2d0_patchDimX * x2d4_patchDimY]);
  int curTileY = 0;
  int nextTileY;
  for (int i = 0; i < x2d4_patchDimY; ++i, curTileY = nextTileY) {
    nextTileY = curTileY + tilesPerPatch;
    int curTileX = 0;
    int rowBase = x2d0_patchDimX * i;
    for (int j = 0; j < x2d0_patchDimX; ++j) {
      int nextTileX = curTileX + tilesPerPatch;
      bool allClear = true;
      bool allIntersections = true;
      for (int k = curTileY; k < std::min(nextTileY, x2c8_gridDimY); ++k) {
        if (!allClear && !allIntersections)
          break;
        for (int l = curTileX; l < std::min(nextTileX, x2c4_gridDimX); ++l) {
          if (x2d8_tileIntersects[k * x2c4_gridDimX + l]) {
            allClear = false;
            if (!allIntersections)
              break;
          } else {
            allIntersections = false;
            if (!allClear)
              break;
          }
        }
      }

      u8 flag;
      if (allIntersections)
        flag = 1;
      else if (allClear)
        flag = 0;
      else
        flag = 2;
      x2e0_patchIntersects[rowBase + j] = flag;
      curTileX += tilesPerPatch;
    }
  }

  x2dc_vertIntersects.reset();
}

void CScriptWater::UpdateSplashInhabitants(CStateManager& mgr) {
  for (auto it = x1fc_waterInhabitants.begin(); it != x1fc_waterInhabitants.end();) {
    auto& inhab = *it;
    TCastToPtr<CActor> act = mgr.ObjectById(inhab.first);
    bool intersects = false;
    if (act) {
      if (auto tb = act->GetTouchBounds()) {
        zeus::CAABox thisTb = GetTriggerBoundsWR();
        if (tb->min.z() <= thisTb.max.z() && tb->max.z() >= thisTb.max.z())
          intersects = true;
      }
    }

    if (act && inhab.second) {
      if (intersects)
        act->FluidFXThink(EFluidState::InFluid, *this, mgr);
      mgr.SendScriptMsg(act.GetPtr(), GetUniqueId(), EScriptObjectMessage::UpdateSplashInhabitant);
      inhab.second = false;
    } else {
      it = x1fc_waterInhabitants.erase(it);
      if (act) {
        if (intersects)
          act->FluidFXThink(EFluidState::LeftFluid, *this, mgr);
        mgr.SendScriptMsg(act.GetPtr(), GetUniqueId(), EScriptObjectMessage::RemoveSplashInhabitant);
      }
      continue;
    }
    ++it;
  }
}

void CScriptWater::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptWater::Think(float dt, CStateManager& mgr) {
  if (!x30_24_active)
    return;

  bool oldCamSubmerged = x148_25_camSubmerged;
  CScriptTrigger::Think(dt, mgr);

  CGameCamera* curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  if (x148_25_camSubmerged && !oldCamSubmerged)
    mgr.SendScriptMsg(curCam, x8_uid, EScriptObjectMessage::AddSplashInhabitant);
  else if (!x148_25_camSubmerged && oldCamSubmerged)
    mgr.SendScriptMsg(curCam, x8_uid, EScriptObjectMessage::RemoveSplashInhabitant);

  UpdateSplashInhabitants(mgr);

  if (x2e8_30_alphaOut) {
    x2bc_alpha -= x2b8_alphaOutRecip * dt * x1b4_fluidPlane->GetAlpha();
    x214_fogBias -= x2b8_alphaOutRecip * dt * x21c_origFogBias;
    x218_fogMagnitude -= x2b8_alphaOutRecip * dt * x220_origFogMagnitude;
    if (x2bc_alpha <= 0.f) {
      x218_fogMagnitude = 0.f;
      x214_fogBias = 0.f;
      x2bc_alpha = 0.f;
      x2e8_30_alphaOut = false;
    }
  } else if (x2e8_29_alphaIn) {
    x2bc_alpha += x2b4_alphaInRecip * dt * x1b4_fluidPlane->GetAlpha();
    x214_fogBias -= x2b4_alphaInRecip * dt * x21c_origFogBias;
    x218_fogMagnitude -= x2b4_alphaInRecip * dt * x220_origFogMagnitude;
    if (x2bc_alpha > x1b4_fluidPlane->GetAlpha()) {
      x2bc_alpha = x1b4_fluidPlane->GetAlpha();
      x214_fogBias = x21c_origFogBias;
      x218_fogMagnitude = x220_origFogMagnitude;
      x2e8_29_alphaIn = false;
    }
  }

  if (x2e8_26_morphing) {
    bool stillMorphing = true;
    if (x2e8_25_morphIn) {
      x1f8_morphFactor += dt / x1d0_morphInTime;
      if (x1f8_morphFactor > 1.f) {
        x1f8_morphFactor = 1.f;
        stillMorphing = false;
      }
    } else {
      x1f8_morphFactor -= dt / x1f4_morphOutTime;
      if (x1f8_morphFactor < 0.f) {
        x1f8_morphFactor = 0.f;
        stillMorphing = false;
      }
    }

    SetTranslation(zeus::CVector3f::lerp(x1d4_positionOrig, x1b8_positionMorphed, x1f8_morphFactor));
    zeus::CVector3f lerpExtent = zeus::CVector3f::lerp(x1e0_extentOrig, x1c4_extentMorphed, x1f8_morphFactor);
    x130_bounds = zeus::CAABox(lerpExtent * -0.5f, lerpExtent * 0.5f);
    CalculateRenderBounds();

    if (!stillMorphing)
      SetMorphing(false);
    else
      SetupGrid(false);
  }

  SetupGridClipping(mgr, 4);
}

void CScriptWater::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Next:
    if (!x30_24_active)
      break;
    x2e8_25_morphIn = !x2e8_25_morphIn;
    if (x2e8_25_morphIn) {
      for (const SConnection& conn : x20_conns) {
        if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Activate)
          continue;
        auto list = mgr.GetIdListForScript(conn.x8_objId);
        if (list.first == mgr.GetIdListEnd())
          continue;
        if (TCastToConstPtr<CScriptTrigger> trig = mgr.GetObjectById(list.first->second)) {
          x1b8_positionMorphed = trig->GetTranslation();
          x1c4_extentMorphed = trig->GetTriggerBoundsOR().max - trig->GetTriggerBoundsOR().min;
          x1f0_damageMorphed = trig->GetDamageInfo().GetDamage();
          x1d4_positionOrig = GetTranslation();
          x1e0_extentOrig = x130_bounds.max - x130_bounds.min;
          x1ec_damageOrig = x100_damageInfo.GetDamage();
          break;
        }
      }
    }
    SetMorphing(true);
    break;
  case EScriptObjectMessage::Activate:
    x2e8_30_alphaOut = false;
    if (std::fabs(x2ac_alphaInTime) < 0.00001f) {
      x2bc_alpha = x1b4_fluidPlane->GetAlpha();
      x214_fogBias = x21c_origFogBias;
      x218_fogMagnitude = x220_origFogMagnitude;
    } else {
      x2e8_29_alphaIn = true;
    }
    break;
  case EScriptObjectMessage::Action:
    x2e8_29_alphaIn = false;
    if (std::fabs(x2b0_alphaOutTime) < 0.00001f) {
      x2bc_alpha = 0.f;
      x214_fogBias = 0.f;
      x218_fogMagnitude = 0.f;
    } else {
      x2e8_30_alphaOut = true;
    }
    break;
  default:
    break;
  }

  CScriptTrigger::AcceptScriptMsg(msg, other, mgr);
}

void CScriptWater::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  if (x2e8_27_allowRender) {
    zeus::CAABox aabb = GetSortingBounds(mgr);
    xe4_30_outOfFrustum = !frustum.aabbFrustumTest(aabb);
    if (!xe4_30_outOfFrustum) {
      if (x4_areaId != kInvalidAreaId) {
        if (x90_actorLights->GetMaxAreaLights() && (xe4_29_actorLightsDirty || x90_actorLights->GetIsDirty())) {
          const CGameArea* area = mgr.GetWorld()->GetAreaAlways(x4_areaId);
          if (area->IsPostConstructed()) {
            x90_actorLights->BuildAreaLightList(mgr, *area, GetTriggerBoundsWR());
            xe4_29_actorLightsDirty = false;
          }
        }
        x90_actorLights->BuildDynamicLightList(mgr, GetTriggerBoundsWR());
      }
      x150_frustum = frustum;
    }
  } else {
    xe4_30_outOfFrustum = true;
  }
}

void CScriptWater::AddToRenderer(const zeus::CFrustum& /*frustum*/, CStateManager& mgr) {
  if (xe4_30_outOfFrustum) {
    return;
  }

  const zeus::CPlane plane(zeus::skUp, x34_transform.origin.z() + x130_bounds.max.z());
  const zeus::CAABox renderBounds = GetSortingBounds(mgr);
  mgr.AddDrawableActorPlane(*this, plane, renderBounds);
}

void CScriptWater::Render(CStateManager& mgr) {
  if (x30_24_active && !xe4_30_outOfFrustum) {
    float zOffset = 0.5f * (x9c_renderBounds.max.z() + x9c_renderBounds.min.z()) - x34_transform.origin.z();
    zeus::CAABox aabb = x9c_renderBounds.getTransformedAABox(zeus::CTransform::Translate(
        -x34_transform.origin.x(), -x34_transform.origin.y(), -x34_transform.origin.z() - zOffset));
    zeus::CTransform xf = x34_transform;
    xf.origin.z() += zOffset;
    zeus::CVector3f areaCenter = mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId())->GetAABB().center();
    std::optional<CRippleManager> rippleMan(mgr.GetFluidPlaneManager()->GetRippleManager());
    x1b4_fluidPlane->Render(mgr, x2bc_alpha, aabb, xf, mgr.GetWorld()->GetAreaAlways(x4_areaId)->GetTransform(), false,
                            x150_frustum, rippleMan, x8_uid, x2d8_tileIntersects.get(), x2c4_gridDimX, x2c8_gridDimY,
                            areaCenter);
    if (x214_fogBias != 0.f) {
      if (mgr.GetPlayerState()->CanVisorSeeFog(mgr)) {
        float fogLevel = mgr.IntegrateVisorFog(
            x218_fogMagnitude * std::sin(x224_fogSpeed * CGraphics::GetSecondsMod900()) + x214_fogBias);
        if (fogLevel > 0.f) {
          zeus::CAABox fogBox = GetTriggerBoundsWR();
          fogBox.min.z() = float(fogBox.max.z());
          fogBox.max.z() += fogLevel;
          zeus::CTransform modelXf =
              zeus::CTransform::Translate(fogBox.center()) * zeus::CTransform::Scale((fogBox.max - fogBox.min) * 0.5f);
          zeus::CAABox renderAABB(zeus::skNegOne3f, zeus::skOne3f);
          CGraphics::SetModelMatrix(modelXf);
          g_Renderer->SetAmbientColor(zeus::skWhite);
          g_Renderer->RenderFogVolume(x228_fogColor, renderAABB, nullptr, nullptr);
        }
      }
    }
    CGraphics::DisableAllLights();
  }
  CActor::Render(mgr);
}

void CScriptWater::Touch(CActor& otherAct, CStateManager& mgr) {
  if (!x30_24_active)
    return;

  CScriptTrigger::Touch(otherAct, mgr);
  if (otherAct.GetMaterialList().HasMaterial(EMaterialTypes::Trigger))
    return;

  for (auto& inhab : x1fc_waterInhabitants)
    if (inhab.first == otherAct.GetUniqueId()) {
      inhab.second = true;
      return;
    }

  auto touchBounds = otherAct.GetTouchBounds();
  if (!touchBounds)
    return;

  x1fc_waterInhabitants.emplace_back(otherAct.GetUniqueId(), true);
  float triggerMaxZ = GetTriggerBoundsWR().max.z();
  if (touchBounds->min.z() <= triggerMaxZ && touchBounds->max.z() >= triggerMaxZ)
    otherAct.FluidFXThink(EFluidState::EnteredFluid, *this, mgr);

  mgr.SendScriptMsg(&otherAct, x8_uid, EScriptObjectMessage::AddSplashInhabitant);
}

void CScriptWater::CalculateRenderBounds() {
  zeus::CVector3f aabbMin = x130_bounds.min;
  aabbMin.z() = x130_bounds.max.z() - 1.f;
  zeus::CVector3f aabbMax = x130_bounds.max;
  aabbMax.z() += 1.f;
  zeus::CVector3f transAABBMin = aabbMin + GetTranslation();
  zeus::CVector3f transAABBMax = aabbMax + GetTranslation();
  x9c_renderBounds = zeus::CAABox(transAABBMin, transAABBMax);
}

zeus::CAABox CScriptWater::GetSortingBounds(const CStateManager& mgr) const {
  zeus::CVector3f max = x9c_renderBounds.max;
  max.z() = std::max(float(max.z()), x9c_renderBounds.max.z() - 1.f + x214_fogBias + x218_fogMagnitude);
  return {x9c_renderBounds.min, max};
}

EWeaponCollisionResponseTypes CScriptWater::GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                                     const CWeaponMode&, EProjectileAttrib) const {
  return EWeaponCollisionResponseTypes::Water;
}

u16 CScriptWater::GetSplashSound(float mag) const { return x298_splashSounds[GetSplashIndex(mag)]; }

const std::optional<TLockedToken<CGenDescription>>& CScriptWater::GetSplashEffect(float mag) const {
  return x264_splashEffects[GetSplashIndex(mag)];
}

float CScriptWater::GetSplashEffectScale(float dt) const {
  if (std::fabs(dt - 1.f) < 0.00001f)
    return kSplashScales[5];

  u32 idx = GetSplashIndex(dt);
  float s = dt - std::floor(dt * 3.f);
  return ((1.f - s) * (s * kSplashScales[idx * 2])) + kSplashScales[idx];
}

u32 CScriptWater::GetSplashIndex(float mag) const {
  auto idx = u32(mag * 3.f);
  return (idx < 3 ? idx : idx - 1);
}

void CScriptWater::SetMorphing(bool m) {
  if (m == x2e8_26_morphing)
    return;
  x2e8_26_morphing = m;
  SetupGrid(!m);
}

const CScriptWater* CScriptWater::GetNextConnectedWater(const CStateManager& mgr) const {
  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Activate)
      continue;
    auto its = mgr.GetIdListForScript(conn.x8_objId);
    if (its.first != mgr.GetIdListEnd())
      if (TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(its.first->second))
        return water.GetPtr();
  }
  return nullptr;
}

bool CScriptWater::CanRippleAtPoint(const zeus::CVector3f& point) const {
  if (!x2d8_tileIntersects)
    return true;

  auto xTile = int((point.x() - GetTriggerBoundsWR().min.x()) / x2c0_tileSize);
  if (xTile < 0 || xTile >= x2c4_gridDimX)
    return false;

  auto yTile = int((point.y() - GetTriggerBoundsWR().min.y()) / x2c0_tileSize);
  if (yTile < 0 || yTile >= x2c8_gridDimY)
    return false;

  return x2d8_tileIntersects[yTile * x2c4_gridDimX + xTile];
}
} // namespace urde
