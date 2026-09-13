#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"

#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CFluidUVMotion.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"

#include "MetroidPrime/CRipple.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/CCubeRenderer.hpp"

#include "rstl/math.hpp"

#include "MetroidPrime/TCastTo.hpp"
#include "rstl/optional_object.hpp"

static float kMaxRayLength = 120.f;
static int kMaxTilesPerPatch = 7;

const float CScriptWater::kSplashScales[6] = {
    1.0f, 3.0f, 0.709f, 1.19f, 0.709f, 1.0f,
};

CScriptWater::CScriptWater(
    CStateManager& mgr, TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
    const CVector3f& pos, const CAABox& box, const CDamageInfo& dInfo,
    const CVector3f& orientedForce, uint triggerFlags, bool thermalCold, bool displaySurface,
    uint patternMap1, uint patternMap2, uint colorMap, uint bumpMap, uint envMap, uint envBumpMap,
    uint unusedMap, const CVector3f& bumpLightDir, float bumpScale, float morphInTime,
    float morphOutTime, bool active, CFluidPlane::EFluidType fluidType, bool b4, float alpha,
    const CFluidUVMotion& uvMotion, float turbSpeed, float turbDistance, float turbFreqMax,
    float turbFreqMin, float turbPhaseMax, float turbPhaseMin, float turbAmplitudeMax,
    float turbAmplitudeMin, const CColor& splashColor, const CColor& insideFogColor,
    uint splashParticle1, uint splashParticle2, uint splashParticle3, uint visorRunoffParticle,
    uint unmorphVisorRunoffParticle, int visorRunoffSfx, int unmorphVisorRunoffSfx, int splashSfx1,
    int splashSfx2, int splashSfx3, float tileSize, uint tileSubdivisions, float specularMin,
    float specularMax, float reflectionSize, float rippleIntensity, float reflectionBlend,
    float fogBias, float fogMagnitude, float fogSpeed, const CColor& fogColor, uint lightmap,
    float unitsPerLightmapTexel, float alphaInTime, float alphaOutTime, uint w21, uint w22, bool b5,
    int bitVal0, int bitVal1, const uint* bitset)
: CScriptTrigger(uid, name, info, pos, box, dInfo, orientedForce, triggerFlags, active, false,
                 false)
, x1b4_fluidPlane(NULL)
, x1b8_positionMorphed(pos)
, x1c4_extentMorphed(box.GetWidth(), box.GetHeight(), box.GetDepth())
, x1d0_morphInTime(morphInTime)
, x1d4_positionOrig(pos)
, x1e0_extentOrig(box.GetWidth(), box.GetHeight(), box.GetDepth())
, x1ec_damageOrig(dInfo.GetDamage())
, x1f0_damageMorphed(dInfo.GetDamage())
, x1f4_morphOutTime(morphOutTime)
, x1f8_morphFactor(0.f)
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
, x24c_unmorphVisorRunoffParticleId(unmorphVisorRunoffParticle)
, x260_visorRunoffSfx(CSfxManager::TranslateSFXID(visorRunoffSfx))
, x262_unmorphVisorRunoffSfx(CSfxManager::TranslateSFXID(unmorphVisorRunoffSfx))
, x2a4_splashColor(splashColor)
, x2a8_insideFogColor(insideFogColor)
, x2ac_alphaInTime(alphaInTime)
, x2b0_alphaOutTime(alphaOutTime)
, x2b4_alphaInRecip(alphaInTime ? 1.f / alphaInTime : 0.f)
, x2b8_alphaOutRecip(alphaOutTime ? 1.f / alphaOutTime : 0.f)
, x2bc_alpha(alpha)
, x2c0_tileSize(tileSize)
, x2c4_gridDimX(
      (int)CMath::FloorF((x2c0_tileSize + GetTriggerBoundsWR().GetWidth() - 0.01f) / x2c0_tileSize))
, x2c8_gridDimY((int)CMath::FloorF((x2c0_tileSize + GetTriggerBoundsWR().GetHeight() - 0.01f) /
                                   x2c0_tileSize))
, x2cc_gridCellCount((x2c4_gridDimX + 1) * (x2c8_gridDimY + 1))
, x2d0_patchDimX(0)
, x2d4_patchDimY(0)
, x2d8_tileIntersects(NULL)
, x2dc_vertIntersects(NULL)
, x2e0_patchIntersects(NULL)
, x2e4_computedGridCellCount(0)
, x2e8_24_b4(b4)
, x2e8_25_morphIn(false)
, x2e8_26_morphing(false)
, x2e8_27_allowRender(displaySurface)
, x2e8_28_recomputeClipping(true)
, x2e8_29_alphaIn(false)
, x2e8_30_alphaOut(false) {
  x1b4_fluidPlane = rs_new CFluidPlaneCPU(
      patternMap1, patternMap2, colorMap, bumpMap, envMap, envBumpMap, unitsPerLightmapTexel,
      lightmap, tileSubdivisions, fluidType, tileSize, bumpLightDir, alpha, uvMotion, bumpScale,
      turbSpeed, turbDistance, turbFreqMax, turbFreqMin, turbPhaseMax, turbPhaseMin,
      turbAmplitudeMax, turbAmplitudeMin, specularMin, specularMax, reflectionBlend, reflectionSize,
      rippleIntensity);

  delete const_cast< uint* >(bitset);

  x264_splashEffects.push_back(rstl::optional_object< TLockedToken< CGenDescription > >());
  x264_splashEffects.push_back(rstl::optional_object< TLockedToken< CGenDescription > >());
  x264_splashEffects.push_back(rstl::optional_object< TLockedToken< CGenDescription > >());

  if (x22c_splashParticle1Id != kInvalidAssetId) {
    x264_splashEffects[0] = TLockedToken< CGenDescription >(TToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', x22c_splashParticle1Id))));
  }
  if (x230_splashParticle2Id != kInvalidAssetId) {
    x264_splashEffects[1] = TLockedToken< CGenDescription >(TToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', x230_splashParticle2Id))));
  }
  if (x234_splashParticle3Id != kInvalidAssetId) {
    x264_splashEffects[2] = TLockedToken< CGenDescription >(TToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', x234_splashParticle3Id))));
  }

  if (x238_visorRunoffParticleId != kInvalidAssetId) {
    x23c_visorRunoffEffect = TLockedToken< CGenDescription >(TToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', x238_visorRunoffParticleId))));
  }
  if (x24c_unmorphVisorRunoffParticleId != kInvalidAssetId) {
    x250_unmorphVisorRunoffEffect = TLockedToken< CGenDescription >(TToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', x24c_unmorphVisorRunoffParticleId))));
  }

  x298_splashSounds.push_back(CSfxManager::TranslateSFXID(splashSfx1));
  x298_splashSounds.push_back(CSfxManager::TranslateSFXID(splashSfx2));
  x298_splashSounds.push_back(CSfxManager::TranslateSFXID(splashSfx3));

  SetCalculateLighting(true);
  if (lightmap != kInvalidAssetId) {
    ActorLights()->SetMaxAreaLights(0);
    ActorLights()->SetInArea(false);
  }
  ActorLights()->SetMaxDynamicLights(4);
  ActorLights()->SetCastShadows(false);
  ActorLights()->SetAmbienceGenerated(false);
  ActorLights()->SetFindNearestDynamicLights(true);
  x148_24_detectCamera = true;
  CalculateRenderBounds();
  SetThermalFlags(thermalCold ? kTF_Hot : kTF_Cold);
  if (!GetActive()) {
    x2bc_alpha = 0.f;
    x214_fogBias = 0.f;
    x218_fogMagnitude = 0.f;
  }
  SetupGrid(true);
}

int CScriptWater::GetSplashIndex(float scale) const {
  scale *= 3.f;
  int idx = static_cast< int >(scale);
  if (idx >= 3) {
    idx -= 1;
  }
  return idx;
}

const rstl::optional_object< TLockedToken< CGenDescription > >&
CScriptWater::GetSplashEffect(float scale) const {
  return x264_splashEffects[GetSplashIndex(scale)];
}

const ushort CScriptWater::GetSplashSound(float scale) const {
  int idx = GetSplashIndex(scale);
  return x298_splashSounds[idx];
}

float CScriptWater::GetSplashEffectScale(float scale) const {
  if (close_enough(scale, 1.f)) {
    return kSplashScales[5];
  }
  int idx = GetSplashIndex(scale);
  scale *= 3.f;
  scale = scale - CMath::FloorF(scale);
  return (1.f - scale) * kSplashScales[idx * 2] + scale * kSplashScales[idx * 2 + 1];
}

void CScriptWater::CalculateRenderBounds() {
  const CAABox& bounds = GetTriggerBounds();
  CVector3f translation = GetTranslation();
  float maxZ = bounds.GetMaxPoint().GetZ();
  CVector3f worldMin;
  CVector3f localMin;
  CVector3f worldMax;
  CVector3f localMax(bounds.GetMaxPoint().GetX(), bounds.GetMaxPoint().GetY(), maxZ + 1.f);
  const CVector3f& boundsMax = localMax;
  worldMax = boundsMax + translation;
  localMin = CVector3f(bounds.GetMinPoint().GetX(), bounds.GetMinPoint().GetY(), maxZ - 1.f);
  const CVector3f& boundsMin = localMin;
  worldMin = boundsMin + translation;
  SetRenderBounds(CAABox(worldMin, worldMax));
}

CAABox CScriptWater::GetSortingBounds(const CStateManager&) const {
  const CAABox& bounds = GetRenderBoundsCached();
  CVector3f maxPoint = bounds.GetMaxPoint();
  float fogZ = x218_fogMagnitude + (x214_fogBias + (maxPoint.GetZ() - 1.f));
  if (fogZ > maxPoint.GetZ()) {
    maxPoint[kDZ] = fogZ;
  }
  return CAABox(bounds.GetMinPoint(), maxPoint);
}

void CScriptWater::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (x2e8_27_allowRender) {
    SetPreRenderClipped(!frustum.BoxInFrustumPlanes(GetSortingBounds(mgr)));
    if (!GetPreRenderClipped()) {
      if (GetCurrentAreaId() != kInvalidAreaId) {
        if ((uint)ActorLights()->GetMaxAreaLights() != 0) {
          if (GetPreRenderHasMoved() || ActorLights()->GetIsDirty()) {
            const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
            if (area.IsPostConstructed()) {
              ActorLights()->BuildAreaLightList(
                  mgr, mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()), GetTriggerBoundsWR());
              SetPreRenderHasMoved(false);
            }
          }
        }
        ActorLights()->BuildDynamicLightList(mgr, GetTriggerBoundsWR());
      }
      x150_frustum = frustum;
    }
  } else {
    SetPreRenderClipped(true);
  }
}

void CScriptWater::AddToRenderer(const CFrustumPlanes&, const CStateManager& mgr) const {
  if (GetPreRenderClipped()) {
    return;
  }
  float transZ = GetTranslation().GetZ();
  float boundsMaxZ = x130_bounds.GetMaxPoint().GetZ();
  CUnitVector3f upVec(0.f, 0.f, 1.f);
  upVec.Normalize();
  CPlane plane(boundsMaxZ + transZ, upVec);
  mgr.AddDrawableActorPlane(*this, plane, GetSortingBounds(mgr));
}

void CScriptWater::Render(const CStateManager& mgr) const {
  if (GetActive() && !GetPreRenderClipped()) {
    GetActorLights()->ActivateLights();
    float zOffset = 0.5f * (GetRenderBoundsCached().GetMaxPoint().GetZ() +
                            GetRenderBoundsCached().GetMinPoint().GetZ()) -
                    GetTransform().Get23();
    CAABox aabb = GetRenderBoundsCached().GetTransformedAABox(CTransform4f::Translate(
        -GetTransform().Get03(), -GetTransform().Get13(), -GetTransform().Get23() - zOffset));
    CTransform4f xf(GetTransform());
    xf.AddTranslationZ(zOffset);
    const CVector3f& areaCenter =
        mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId()).GetAABB().GetCenterPoint();
    x1b4_fluidPlane->Render(
        mgr, x2bc_alpha, aabb, xf, mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetTM(), false,
        x150_frustum, mgr.GetFluidPlaneManager()->GetRippleManager(), GetUniqueId(),
        x2d8_tileIntersects.get(), x2c4_gridDimX, x2c8_gridDimY, areaCenter);
    if (x214_fogBias) {
      if (mgr.GetPlayerState()->CanVisorSeeFog(mgr)) {
        if (gkWaterFog) {
          float sinVal = CMath::FastSinR(x224_fogSpeed * CGraphics::GetSecondsMod900());
          float fogLevel = mgr.IntegrateVisorFog(x218_fogMagnitude * sinVal + x214_fogBias);
          if (fogLevel > 0.f) {
            const CAABox fogBox = GetTriggerBoundsWR();
            const CVector3f& fogMin = fogBox.GetMinPoint();
            const CVector3f& fogMax = fogBox.GetMaxPoint();
            CAABox renderBounds =
                CAABox(CVector3f(fogMin.GetX(), fogMin.GetY(), fogMax.GetZ()),
                       CVector3f(fogMax.GetX(), fogMax.GetY(), fogLevel + fogMax.GetZ()));
            CTransform4f modelXf(
                CTransform4f::Translate(renderBounds.GetCenterPoint()) *
                CTransform4f::Scale((renderBounds.GetMaxPoint() - renderBounds.GetMinPoint()) *
                                    0.5f));
            CAABox renderAABB(CVector3f(-1.f, -1.f, -1.f), CVector3f(1.f, 1.f, 1.f));
            gpRender->SetModelMatrix(modelXf);
            gpRender->SetAmbientColor(CColor::White());
            gpRender->RenderFogVolume(x228_fogColor, renderAABB, NULL, NULL);
          }
        }
      }
    }
    CGraphics::DisableAllLights();
  }
  CActor::Render(mgr);
}

EWeaponCollisionResponseTypes CScriptWater::GetCollisionResponseType(const CVector3f&,
                                                                     const CVector3f&,
                                                                     const CWeaponMode&,
                                                                     int) const {
  return kWCR_Water;
}

void CScriptWater::SetMorphing(const bool m) {
  if (m != x2e8_26_morphing) {
    x2e8_26_morphing = m;
    SetupGrid(!m);
  }
}

void CScriptWater::SetupGridClipping(CStateManager& mgr, int computeVerts) {
  if (x2e8_28_recomputeClipping) {
    x2e4_computedGridCellCount = 0;
    x2dc_vertIntersects = static_cast< bool* >(nullptr);
    x2e8_28_recomputeClipping = false;
  }

  if (x2e4_computedGridCellCount < x2cc_gridCellCount) {
    static CMaterialFilter kSolidFilter =
        CMaterialFilter::MakeInclude(CMaterialList(SolidMaterial));

    if (x2dc_vertIntersects.get() == NULL) {
      x2dc_vertIntersects = rs_new bool[(x2c4_gridDimX + 1) * (x2c8_gridDimY + 1)];
    }

    CVector3f downVec(0.f, 0.f, -1.f);
    CAABox trigBounds = GetTriggerBoundsWR();
    float baseZ = gkFluidMaxCrest + trigBounds.GetMaxPoint().GetZ();
    CAABox trigBounds2 = GetTriggerBoundsWR();

    int gridDimXP1 = x2c4_gridDimX + 1;
    int curCell = x2e4_computedGridCellCount;
    int row = curCell / gridDimXP1;
    int col = curCell % gridDimXP1;
    bool* vertPtr = x2dc_vertIntersects.get() + curCell;
    float zDiff = x130_bounds.GetMaxPoint().GetZ() - x130_bounds.GetMinPoint().GetZ();
    float baseX = trigBounds2.GetMinPoint().GetX();
    float yOffset = x2c0_tileSize * (float)row;
    float baseY = trigBounds2.GetMinPoint().GetY();
    float xOffset = x2c0_tileSize * (float)col;
    float mag = 2.f * zDiff + gkFluidMaxCrest;
    float useMag = rstl::min_val(mag, kMaxRayLength);

    int i = x2e4_computedGridCellCount;
    for (; i < rstl::min_val(x2cc_gridCellCount, x2e4_computedGridCellCount + computeVerts);
         ++i, ++vertPtr) {
      CVector3f pos(xOffset + baseX, yOffset + baseY, baseZ);
      CRayCastResult result = mgr.RayStaticIntersection(pos, downVec, useMag, kSolidFilter);
      col += 1;
      *vertPtr = result.GetValid();
      xOffset += x2c0_tileSize;
      if (col > x2c4_gridDimX) {
        yOffset += x2c0_tileSize;
        xOffset = 0.f;
        col = 0;
      }
    }

    x2e4_computedGridCellCount += computeVerts;
    if (x2e4_computedGridCellCount >= x2cc_gridCellCount) {
      x2e4_computedGridCellCount = x2cc_gridCellCount;
      x2d8_tileIntersects = rs_new char[x2c4_gridDimX * x2c8_gridDimY];

      for (int i = 0; i < x2c8_gridDimY; ++i) {
        char* tileRow = x2d8_tileIntersects.get() + i * x2c4_gridDimX;
        const char* vertRow = (const char*)(x2dc_vertIntersects.get()) + i * (x2c4_gridDimX + 1);
        for (int j = 0; j < x2c4_gridDimX; ++j, ++tileRow, ++vertRow) {
          int dimX = x2c4_gridDimX;
          if (vertRow[0] != 0 || vertRow[1] != 0 || vertRow[dimX + 1] != 0 ||
              vertRow[dimX + 2] != 0) {
            *tileRow = true;
          } else {
            *tileRow = false;
          }
        }
      }

      const int tilesPerPatch = rstl::min_val(
          kMaxTilesPerPatch, static_cast< int >(42u / GetFluidPlane().GetTileSubdivisions()));

      x2d0_patchDimX = (tilesPerPatch + x2c4_gridDimX - 1) / tilesPerPatch;
      x2d4_patchDimY = (tilesPerPatch + x2c8_gridDimY - 1) / tilesPerPatch;
      x2e0_patchIntersects = rs_new char[x2d0_patchDimX * x2d4_patchDimY];

      int curTileY = 0;
      int patchIdx = 0;
      for (; patchIdx < x2d4_patchDimY; ++patchIdx) {
        int curTileX = 0;
        int patchJ = 0;
        char* const patchRow = x2e0_patchIntersects.get() + patchIdx * x2d0_patchDimX;
        for (; patchJ < x2d0_patchDimX; ++patchJ) {
          bool allClear = true;
          bool allIntersect = true;
          for (int k = curTileY; k < rstl::min_val(x2c8_gridDimY, curTileY + tilesPerPatch); ++k) {
            if (!allClear && !allIntersect)
              break;
            for (int l = curTileX; l < rstl::min_val(x2c4_gridDimX, curTileX + tilesPerPatch);
                 ++l) {
              if (((const char*)x2d8_tileIntersects.get())[l + k * x2c4_gridDimX] != 0) {
                allClear = false;
                if (!allIntersect)
                  break;
              } else {
                allIntersect = false;
                if (!allClear)
                  break;
              }
            }
          }

          char flag;
          if (allIntersect) {
            flag = 1;
          } else {
            flag = 2;
            if (allClear) {
              flag = 0;
            }
          }
          patchRow[patchJ] = flag;
          curTileX += tilesPerPatch;
        }
        curTileY += tilesPerPatch;
      }

      x2dc_vertIntersects = static_cast< bool* >(nullptr);
    }
  }
}

void CScriptWater::SetupGrid(bool recomputeClipping) {
  CAABox trigBoundsX = GetTriggerBoundsWR();
  float tileSize = x2c0_tileSize;
  int dimX = (int)CMath::FloorF(
      (tileSize + (trigBoundsX.GetMaxPoint().GetX() - trigBoundsX.GetMinPoint().GetX()) - 0.01f) /
      tileSize);

  CAABox trigBoundsY = GetTriggerBoundsWR();
  tileSize = x2c0_tileSize;
  int dimY = (int)CMath::FloorF(
      (tileSize + (trigBoundsY.GetMaxPoint().GetY() - trigBoundsY.GetMinPoint().GetY()) - 0.01f) /
      tileSize);

  x2cc_gridCellCount = (dimX + 1) * (dimY + 1);
  x2e4_computedGridCellCount = x2cc_gridCellCount;

  x2dc_vertIntersects = static_cast< bool* >(nullptr);

  if (x2d8_tileIntersects.get() == NULL || dimX != x2c4_gridDimX || dimY != x2c8_gridDimY) {
    x2d8_tileIntersects = rs_new char[dimX * dimY];
  }

  x2c4_gridDimX = dimX;
  x2c8_gridDimY = dimY;

  for (int i = 0; i < x2c8_gridDimY; ++i) {
    char* row = x2d8_tileIntersects.get() + i * x2c4_gridDimX;
    for (int j = 0; j < x2c4_gridDimX; ++j, ++row) {
      *row = true;
    }
  }

  if (x2e0_patchIntersects.get() == NULL || x2d0_patchDimX != 0 || x2d4_patchDimY != 0) {
    x2e0_patchIntersects = rs_new char[32];
  }

  for (int i = 0; i < 32; ++i) {
    x2e0_patchIntersects.get()[i] = 1;
  }

  x2d4_patchDimY = 0;
  x2d0_patchDimX = 0;
  x2e8_28_recomputeClipping = recomputeClipping;
}

bool CScriptWater::CanRippleAtPoint(const CVector3f& point) const {
  if (x2d8_tileIntersects.null()) {
    return true;
  }

  int xTile = (int)((point.GetX() - GetTriggerBoundsWR().GetMinPoint().GetX()) / x2c0_tileSize);
  if (xTile < 0 || xTile >= x2c4_gridDimX) {
    return false;
  }

  int yTile = (int)((point.GetY() - GetTriggerBoundsWR().GetMinPoint().GetY()) / x2c0_tileSize);
  if (yTile < 0 || yTile >= x2c8_gridDimY) {
    return false;
  }

  return x2d8_tileIntersects.get()[xTile + yTile * x2c4_gridDimX] != 0;
}

int CScriptWater::GetPatchRenderFlags(int x, int y) const {
  return x2e0_patchIntersects.get()[CalculateIndex(x, y, x2d0_patchDimX)];
}

CScriptWater::~CScriptWater() {}

ENTITY_ACCEPT_IMPL(CScriptWater)

void CScriptWater::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Next:
    if (GetActive()) {
      x2e8_25_morphIn = !x2e8_25_morphIn;
      if (x2e8_25_morphIn) {
        rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
        for (; conn != GetConnectionList().end(); ++conn) {
          if (conn->x0_state != kSS_Play || conn->x4_msg != kSM_Activate) {
            continue;
          }
          CStateManager::TIdListResult search = mgr.GetIdListForScript(conn->x8_objId);
          if (search.first != search.second) {
            if (const CScriptTrigger* trig =
                    TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(search.first->second))) {
              x1b8_positionMorphed = trig->GetTranslation();
              x1c4_extentMorphed =
                  CVector3f(trig->x130_bounds.GetWidth(), trig->x130_bounds.GetHeight(),
                            trig->x130_bounds.GetDepth());
              x1f0_damageMorphed = trig->x100_damageInfo.GetDamage();
              x1d4_positionOrig = GetTranslation();
              x1e0_extentOrig = CVector3f(x130_bounds.GetWidth(), x130_bounds.GetHeight(),
                                          x130_bounds.GetDepth());
              x1ec_damageOrig = x100_damageInfo.GetDamage();
              break;
            }
          }
        }
      }
      SetMorphing(true);
    }
    break;
  case kSM_Activate:
    x2e8_30_alphaOut = false;
    if (close_enough(x2ac_alphaInTime, 0.f)) {
      x2bc_alpha = x1b4_fluidPlane->GetAlpha();
      x214_fogBias = x21c_origFogBias;
      x218_fogMagnitude = x220_origFogMagnitude;
    } else {
      x2e8_29_alphaIn = true;
    }
    break;
  case kSM_Action:
    x2e8_29_alphaIn = false;
    if (close_enough(x2b0_alphaOutTime, 0.f)) {
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
  CScriptTrigger::AcceptScriptMsg(msg, uid, mgr);
}

// Real CVector3f::Lerp doesn't work, and demo map doesn't show anything relevant.
// But this hacked up version works for now
static inline CVector3f FakeLerp(const CVector3f& a, const CVector3f& b, float v) {
  float inv = 1.f - v;
  float by = v * b.GetY();
  float bz = v * b.GetZ();
  float ay = inv * a.GetY();
  float az = inv * a.GetZ();
  float ax = inv * a.GetX();
  float bx = v * b.GetX();
  float x = ax + bx;
  float y = ay + by;
  float z = az + bz;
  return CVector3f(x, y, z);
}

void CScriptWater::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  bool oldCamSubmerged = CameraInside();
  CScriptTrigger::Think(dt, mgr);

  CEntity* curCam = &mgr.CameraManager()->CurrentCamera(mgr);
  if (x148_25_camSubmerged && !oldCamSubmerged) {
    mgr.DeliverScriptMsg(curCam, GetUniqueId(), kSM_AddSplashInhabitant);
  } else if (!x148_25_camSubmerged && oldCamSubmerged) {
    mgr.DeliverScriptMsg(curCam, GetUniqueId(), kSM_RemoveSplashInhabitant);
  }

  UpdateSplashInhabitants(mgr);

  if (x2e8_30_alphaOut) {
    x2bc_alpha -= dt * x1b4_fluidPlane->GetAlpha() * x2b8_alphaOutRecip;
    x214_fogBias -= dt * x21c_origFogBias * x2b8_alphaOutRecip;
    x218_fogMagnitude -= dt * x220_origFogMagnitude * x2b8_alphaOutRecip;
    if (x2bc_alpha <= 0.f) {
      x218_fogMagnitude = 0.f;
      x214_fogBias = 0.f;
      x2bc_alpha = 0.f;
      x2e8_30_alphaOut = false;
    }
  } else if (x2e8_29_alphaIn) {
    x2bc_alpha += dt * x1b4_fluidPlane->GetAlpha() * x2b4_alphaInRecip;
    x214_fogBias -= dt * x21c_origFogBias * x2b4_alphaInRecip;
    x218_fogMagnitude -= dt * x220_origFogMagnitude * x2b4_alphaInRecip;
    if (x2bc_alpha > x1b4_fluidPlane->GetAlpha()) {
      x2bc_alpha = x1b4_fluidPlane->GetAlpha();
      x214_fogBias = x21c_origFogBias;
      x218_fogMagnitude = x220_origFogMagnitude;
      x2e8_29_alphaIn = false;
    }
  }

  if (IsMorphing()) {
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

    SetTranslation(FakeLerp(x1d4_positionOrig, x1b8_positionMorphed, x1f8_morphFactor));

    x100_damageInfo.SetDamage(x1ec_damageOrig * (1.f - x1f8_morphFactor) +
                              x1f0_damageMorphed * x1f8_morphFactor);

    CVector3f lerpExtent = FakeLerp(x1e0_extentOrig, x1c4_extentMorphed, x1f8_morphFactor);
    CAABox bounds = CAABox(lerpExtent * -0.5f, lerpExtent * 0.5f);
    SetTriggerBounds(bounds);
    CalculateRenderBounds();

    if (!stillMorphing) {
      SetMorphing(false);
    } else {
      SetupGrid(false);
    }
  }

  SetupGridClipping(mgr, 4);
}

void CScriptWater::UpdateSplashInhabitants(CStateManager& mgr) {
  rstl::list< rstl::pair< TUniqueId, bool > >::iterator it = x1fc_waterInhabitants.begin();
  while (it != x1fc_waterInhabitants.end()) {
    rstl::list< rstl::pair< TUniqueId, bool > >::iterator next = it;
    ++next;
    CActor* const act = TCastToPtr< CActor >(mgr.ObjectById(it->first));
    bool intersects = false;
    if (act != NULL) {
      rstl::optional_object< CAABox > touchBounds = act->GetTouchBounds();
      if (touchBounds) {
        CAABox trigBounds = GetTriggerBoundsWR();
        float trigMaxZ = trigBounds.GetMaxPoint().GetZ();
        if (touchBounds.data().GetMinPoint().GetZ() <= trigMaxZ &&
            touchBounds.data().GetMaxPoint().GetZ() >= trigMaxZ) {
          intersects = true;
        }
      }
    }

    if (act != NULL && it->second) {
      if (intersects) {
        act->FluidFXThink(kFS_InFluid, *this, mgr);
      }
      mgr.DeliverScriptMsg(act, GetUniqueId(), kSM_UpdateSplashInhabitant);
      it->second = false;
    } else {
      x1fc_waterInhabitants.erase(it);
      if (act != NULL) {
        if (intersects) {
          act->FluidFXThink(kFS_LeftFluid, *this, mgr);
        }
        mgr.DeliverScriptMsg(act, GetUniqueId(), kSM_RemoveSplashInhabitant);
      }
    }
    it = next;
  }
}

void CScriptWater::Touch(CActor& otherAct, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CScriptTrigger::Touch(otherAct, mgr);
  if (otherAct.GetMaterialList().HasMaterial(kMT_Trigger)) {
    return;
  }

  rstl::list< rstl::pair< TUniqueId, bool > >::iterator it = x1fc_waterInhabitants.begin();
  for (; it != x1fc_waterInhabitants.end(); ++it) {
    if (it->first == otherAct.GetUniqueId()) {
      it->second = true;
      return;
    }
  }

  const rstl::optional_object< CAABox >& touchBounds = otherAct.GetTouchBounds();
  if (!touchBounds) {
    return;
  }

  x1fc_waterInhabitants.push_back(rstl::pair< TUniqueId, bool >(otherAct.GetUniqueId(), true));
  CAABox trigBounds = GetTriggerBoundsWR();
  float trigMaxZ = trigBounds.GetMaxPoint().GetZ();
  if (touchBounds.data().GetMinPoint().GetZ() <= trigMaxZ &&
      touchBounds.data().GetMaxPoint().GetZ() >= trigMaxZ) {
    otherAct.FluidFXThink(kFS_EnteredFluid, *this, mgr);
  }
  mgr.DeliverScriptMsg(&otherAct, GetUniqueId(), kSM_AddSplashInhabitant);
}

const CScriptWater* CScriptWater::GetNextConnectedWater(const CStateManager& mgr) const {
  rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
  for (; conn != GetConnectionList().end(); ++conn) {
    if (conn->x0_state != kSS_Play || conn->x4_msg != kSM_Activate) {
      continue;
    }
    CStateManager::TIdListResult search = mgr.GetIdListForScript(conn->x8_objId);
    if (search.first != search.second) {
      if (const CScriptWater* water =
              TCastToConstPtr< CScriptWater >(mgr.GetObjectById(search.first->second))) {
        return water;
      }
    }
  }
  return NULL;
}
