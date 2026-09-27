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
, mFluidPlane(NULL)
, mPositionMorphed(pos)
, mExtentMorphed(box.GetWidth(), box.GetHeight(), box.GetDepth())
, mMorphInTime(morphInTime)
, mPositionOrig(pos)
, mExtentOrig(box.GetWidth(), box.GetHeight(), box.GetDepth())
, mDamageOrig(dInfo.GetDamage())
, mDamageMorphed(dInfo.GetDamage())
, mMorphOutTime(morphOutTime)
, mMorphFactor(0.f)
, mFogBias(fogBias)
, mFogMagnitude(fogMagnitude)
, mOrigFogBias(fogBias)
, mOrigFogMagnitude(fogMagnitude)
, mFogSpeed(fogSpeed)
, mFogColor(fogColor)
, mSplashParticle1Id(splashParticle1)
, mSplashParticle2Id(splashParticle2)
, mSplashParticle3Id(splashParticle3)
, mVisorRunoffParticleId(visorRunoffParticle)
, mUnmorphVisorRunoffParticleId(unmorphVisorRunoffParticle)
, mVisorRunoffSfx(CSfxManager::TranslateSFXID(visorRunoffSfx))
, mUnmorphVisorRunoffSfx(CSfxManager::TranslateSFXID(unmorphVisorRunoffSfx))
, mSplashColor(splashColor)
, mInsideFogColor(insideFogColor)
, mAlphaInTime(alphaInTime)
, mAlphaOutTime(alphaOutTime)
, mAlphaInRecip(alphaInTime ? 1.f / alphaInTime : 0.f)
, mAlphaOutRecip(alphaOutTime ? 1.f / alphaOutTime : 0.f)
, mAlpha(alpha)
, mTileSize(tileSize)
, mGridDimX(
      (int)CMath::FloorF((mTileSize + GetTriggerBoundsWR().GetWidth() - 0.01f) / mTileSize))
, mGridDimY((int)CMath::FloorF((mTileSize + GetTriggerBoundsWR().GetHeight() - 0.01f) /
                                   mTileSize))
, mGridCellCount((mGridDimX + 1) * (mGridDimY + 1))
, mPatchDimX(0)
, mPatchDimY(0)
, mTileIntersects(NULL)
, mVertIntersects(NULL)
, mPatchIntersects(NULL)
, mComputedGridCellCount(0)
, mB4(b4)
, mMorphIn(false)
, mMorphing(false)
, mAllowRender(displaySurface)
, mRecomputeClipping(true)
, mAlphaIn(false)
, mAlphaOut(false) {
  mFluidPlane = rs_new CFluidPlaneCPU(
      patternMap1, patternMap2, colorMap, bumpMap, envMap, envBumpMap, unitsPerLightmapTexel,
      lightmap, tileSubdivisions, fluidType, tileSize, bumpLightDir, alpha, uvMotion, bumpScale,
      turbSpeed, turbDistance, turbFreqMax, turbFreqMin, turbPhaseMax, turbPhaseMin,
      turbAmplitudeMax, turbAmplitudeMin, specularMin, specularMax, reflectionBlend, reflectionSize,
      rippleIntensity);

  delete const_cast< uint* >(bitset);

  mSplashEffects.push_back(rstl::optional_object< TLockedToken< CGenDescription > >());
  mSplashEffects.push_back(rstl::optional_object< TLockedToken< CGenDescription > >());
  mSplashEffects.push_back(rstl::optional_object< TLockedToken< CGenDescription > >());

  if (mSplashParticle1Id != kInvalidAssetId) {
    mSplashEffects[0] = TLockedToken< CGenDescription >(TToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', mSplashParticle1Id))));
  }
  if (mSplashParticle2Id != kInvalidAssetId) {
    mSplashEffects[1] = TLockedToken< CGenDescription >(TToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', mSplashParticle2Id))));
  }
  if (mSplashParticle3Id != kInvalidAssetId) {
    mSplashEffects[2] = TLockedToken< CGenDescription >(TToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', mSplashParticle3Id))));
  }

  if (mVisorRunoffParticleId != kInvalidAssetId) {
    mVisorRunoffEffect = TLockedToken< CGenDescription >(TToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', mVisorRunoffParticleId))));
  }
  if (mUnmorphVisorRunoffParticleId != kInvalidAssetId) {
    mUnmorphVisorRunoffEffect = TLockedToken< CGenDescription >(TToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', mUnmorphVisorRunoffParticleId))));
  }

  mSplashSounds.push_back(CSfxManager::TranslateSFXID(splashSfx1));
  mSplashSounds.push_back(CSfxManager::TranslateSFXID(splashSfx2));
  mSplashSounds.push_back(CSfxManager::TranslateSFXID(splashSfx3));

  SetCalculateLighting(true);
  if (lightmap != kInvalidAssetId) {
    ActorLights()->SetMaxAreaLights(0);
    ActorLights()->SetInArea(false);
  }
  ActorLights()->SetMaxDynamicLights(4);
  ActorLights()->SetCastShadows(false);
  ActorLights()->SetAmbienceGenerated(false);
  ActorLights()->SetFindNearestDynamicLights(true);
  mDetectCamera = true;
  CalculateRenderBounds();
  SetThermalFlags(thermalCold ? kTF_Hot : kTF_Cold);
  if (!GetActive()) {
    mAlpha = 0.f;
    mFogBias = 0.f;
    mFogMagnitude = 0.f;
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
  return mSplashEffects[GetSplashIndex(scale)];
}

const ushort CScriptWater::GetSplashSound(float scale) const {
  int idx = GetSplashIndex(scale);
  return mSplashSounds[idx];
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
  float fogZ = mFogMagnitude + (mFogBias + (maxPoint.GetZ() - 1.f));
  if (fogZ > maxPoint.GetZ()) {
    maxPoint[kDZ] = fogZ;
  }
  return CAABox(bounds.GetMinPoint(), maxPoint);
}

void CScriptWater::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (mAllowRender) {
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
      mFrustum = frustum;
    }
  } else {
    SetPreRenderClipped(true);
  }
}

void CScriptWater::AddToRenderer(const CFrustumPlanes&, const CStateManager& mgr) const {
  if (GetPreRenderClipped()) {
    return;
  }
  const float transZ = GetTranslation().GetZ();
  const float boundsMaxZ = mBounds.GetMaxPoint().GetZ();
  mgr.AddDrawableActorPlane(
      *this, CPlane(boundsMaxZ + transZ, CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes)),
      GetSortingBounds(mgr));
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
    mFluidPlane->Render(
        mgr, mAlpha, aabb, xf, mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetTM(), false,
        mFrustum, mgr.GetFluidPlaneManager()->GetRippleManager(), GetUniqueId(),
        mTileIntersects.get(), mGridDimX, mGridDimY, areaCenter);
    if (mFogBias) {
      if (mgr.GetPlayerState()->CanVisorSeeFog(mgr)) {
        if (gkWaterFog) {
          float sinVal = CMath::FastSinR(mFogSpeed * CGraphics::GetSecondsMod900());
          float fogLevel = mgr.IntegrateVisorFog(mFogMagnitude * sinVal + mFogBias);
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
            gpRender->RenderFogVolume(mFogColor, renderAABB, NULL, NULL);
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
  if (m != mMorphing) {
    mMorphing = m;
    SetupGrid(!m);
  }
}

void CScriptWater::SetupGridClipping(CStateManager& mgr, int computeVerts) {
  if (mRecomputeClipping) {
    mComputedGridCellCount = 0;
    mVertIntersects = static_cast< bool* >(nullptr);
    mRecomputeClipping = false;
  }

  if (mComputedGridCellCount < mGridCellCount) {
    static CMaterialFilter kSolidFilter =
        CMaterialFilter::MakeInclude(CMaterialList(SolidMaterial));

    if (mVertIntersects.get() == NULL) {
      mVertIntersects = rs_new bool[(mGridDimX + 1) * (mGridDimY + 1)];
    }

    CVector3f downVec(0.f, 0.f, -1.f);
    CAABox trigBounds = GetTriggerBoundsWR();
    float baseZ = gkFluidMaxCrest + trigBounds.GetMaxPoint().GetZ();
    CAABox trigBounds2 = GetTriggerBoundsWR();

    int gridDimXP1 = mGridDimX + 1;
    int curCell = mComputedGridCellCount;
    int row = curCell / gridDimXP1;
    int col = curCell % gridDimXP1;
    bool* vertPtr = mVertIntersects.get() + curCell;
    float zDiff = mBounds.GetMaxPoint().GetZ() - mBounds.GetMinPoint().GetZ();
    float baseX = trigBounds2.GetMinPoint().GetX();
    float yOffset = mTileSize * (float)row;
    float baseY = trigBounds2.GetMinPoint().GetY();
    float xOffset = mTileSize * (float)col;
    float mag = 2.f * zDiff + gkFluidMaxCrest;
    float useMag = rstl::min_val(mag, kMaxRayLength);

    int i = mComputedGridCellCount;
    for (; i < rstl::min_val(mGridCellCount, mComputedGridCellCount + computeVerts);
         ++i, ++vertPtr) {
      CVector3f pos(xOffset + baseX, yOffset + baseY, baseZ);
      CRayCastResult result = mgr.RayStaticIntersection(pos, downVec, useMag, kSolidFilter);
      col += 1;
      *vertPtr = result.GetValid();
      xOffset += mTileSize;
      if (col > mGridDimX) {
        yOffset += mTileSize;
        xOffset = 0.f;
        col = 0;
      }
    }

    mComputedGridCellCount += computeVerts;
    if (mComputedGridCellCount >= mGridCellCount) {
      mComputedGridCellCount = mGridCellCount;
      mTileIntersects = rs_new char[mGridDimX * mGridDimY];

      for (int i = 0; i < mGridDimY; ++i) {
        char* tileRow = mTileIntersects.get() + i * mGridDimX;
        const char* vertRow = (const char*)(mVertIntersects.get()) + i * (mGridDimX + 1);
        for (int j = 0; j < mGridDimX; ++j, ++tileRow, ++vertRow) {
          int dimX = mGridDimX;
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

      mPatchDimX = (tilesPerPatch + mGridDimX - 1) / tilesPerPatch;
      mPatchDimY = (tilesPerPatch + mGridDimY - 1) / tilesPerPatch;
      mPatchIntersects = rs_new char[mPatchDimX * mPatchDimY];

      int curTileY = 0;
      int patchIdx = 0;
      for (; patchIdx < mPatchDimY; ++patchIdx) {
        int curTileX = 0;
        int patchJ = 0;
        char* const patchRow = mPatchIntersects.get() + patchIdx * mPatchDimX;
        for (; patchJ < mPatchDimX; ++patchJ) {
          bool allClear = true;
          bool allIntersect = true;
          for (int k = curTileY; k < rstl::min_val(mGridDimY, curTileY + tilesPerPatch); ++k) {
            if (!allClear && !allIntersect)
              break;
            for (int l = curTileX; l < rstl::min_val(mGridDimX, curTileX + tilesPerPatch);
                 ++l) {
              if (((const char*)mTileIntersects.get())[l + k * mGridDimX] != 0) {
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

      mVertIntersects = static_cast< bool* >(nullptr);
    }
  }
}

void CScriptWater::SetupGrid(bool recomputeClipping) {
  CAABox trigBoundsX = GetTriggerBoundsWR();
  float tileSize = mTileSize;
  int dimX = (int)CMath::FloorF(
      (tileSize + (trigBoundsX.GetMaxPoint().GetX() - trigBoundsX.GetMinPoint().GetX()) - 0.01f) /
      tileSize);

  CAABox trigBoundsY = GetTriggerBoundsWR();
  tileSize = mTileSize;
  int dimY = (int)CMath::FloorF(
      (tileSize + (trigBoundsY.GetMaxPoint().GetY() - trigBoundsY.GetMinPoint().GetY()) - 0.01f) /
      tileSize);

  mGridCellCount = (dimX + 1) * (dimY + 1);
  mComputedGridCellCount = mGridCellCount;

  mVertIntersects = static_cast< bool* >(nullptr);

  if (mTileIntersects.get() == NULL || dimX != mGridDimX || dimY != mGridDimY) {
    mTileIntersects = rs_new char[dimX * dimY];
  }

  mGridDimX = dimX;
  mGridDimY = dimY;

  for (int i = 0; i < mGridDimY; ++i) {
    char* row = mTileIntersects.get() + i * mGridDimX;
    for (int j = 0; j < mGridDimX; ++j, ++row) {
      *row = true;
    }
  }

  if (mPatchIntersects.get() == NULL || mPatchDimX != 0 || mPatchDimY != 0) {
    mPatchIntersects = rs_new char[32];
  }

  for (int i = 0; i < 32; ++i) {
    mPatchIntersects.get()[i] = 1;
  }

  mPatchDimY = 0;
  mPatchDimX = 0;
  mRecomputeClipping = recomputeClipping;
}

bool CScriptWater::CanRippleAtPoint(const CVector3f& point) const {
  if (mTileIntersects.null()) {
    return true;
  }

  int xTile = (int)((point.GetX() - GetTriggerBoundsWR().GetMinPoint().GetX()) / mTileSize);
  if (xTile < 0 || xTile >= mGridDimX) {
    return false;
  }

  int yTile = (int)((point.GetY() - GetTriggerBoundsWR().GetMinPoint().GetY()) / mTileSize);
  if (yTile < 0 || yTile >= mGridDimY) {
    return false;
  }

  return mTileIntersects.get()[xTile + yTile * mGridDimX] != 0;
}

int CScriptWater::GetPatchRenderFlags(int x, int y) const {
  return mPatchIntersects.get()[CalculateIndex(x, y, mPatchDimX)];
}

CScriptWater::~CScriptWater() {}

ENTITY_ACCEPT_IMPL(CScriptWater)

void CScriptWater::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Next:
    if (GetActive()) {
      mMorphIn = !mMorphIn;
      if (mMorphIn) {
        rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
        for (; conn != GetConnectionList().end(); ++conn) {
          if (conn->mState != kSS_Play || conn->mMsg != kSM_Activate) {
            continue;
          }
          CStateManager::TIdListResult search = mgr.GetIdListForScript(conn->mObjId);
          if (search.first != search.second) {
            if (const CScriptTrigger* trig =
                    TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(search.first->second))) {
              mPositionMorphed = trig->GetTranslation();
              mExtentMorphed =
                  CVector3f(trig->mBounds.GetWidth(), trig->mBounds.GetHeight(),
                            trig->mBounds.GetDepth());
              mDamageMorphed = trig->mDamageInfo.GetDamage();
              mPositionOrig = GetTranslation();
              mExtentOrig = CVector3f(mBounds.GetWidth(), mBounds.GetHeight(),
                                          mBounds.GetDepth());
              mDamageOrig = mDamageInfo.GetDamage();
              break;
            }
          }
        }
      }
      SetMorphing(true);
    }
    break;
  case kSM_Activate:
    mAlphaOut = false;
    if (close_enough(mAlphaInTime, 0.f)) {
      mAlpha = mFluidPlane->GetAlpha();
      mFogBias = mOrigFogBias;
      mFogMagnitude = mOrigFogMagnitude;
    } else {
      mAlphaIn = true;
    }
    break;
  case kSM_Action:
    mAlphaIn = false;
    if (close_enough(mAlphaOutTime, 0.f)) {
      mAlpha = 0.f;
      mFogBias = 0.f;
      mFogMagnitude = 0.f;
    } else {
      mAlphaOut = true;
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
  if (mCamSubmerged && !oldCamSubmerged) {
    mgr.DeliverScriptMsg(curCam, GetUniqueId(), kSM_AddSplashInhabitant);
  } else if (!mCamSubmerged && oldCamSubmerged) {
    mgr.DeliverScriptMsg(curCam, GetUniqueId(), kSM_RemoveSplashInhabitant);
  }

  UpdateSplashInhabitants(mgr);

  if (mAlphaOut) {
    mAlpha -= dt * mFluidPlane->GetAlpha() * mAlphaOutRecip;
    mFogBias -= dt * mOrigFogBias * mAlphaOutRecip;
    mFogMagnitude -= dt * mOrigFogMagnitude * mAlphaOutRecip;
    if (mAlpha <= 0.f) {
      mFogMagnitude = 0.f;
      mFogBias = 0.f;
      mAlpha = 0.f;
      mAlphaOut = false;
    }
  } else if (mAlphaIn) {
    mAlpha += dt * mFluidPlane->GetAlpha() * mAlphaInRecip;
    mFogBias -= dt * mOrigFogBias * mAlphaInRecip;
    mFogMagnitude -= dt * mOrigFogMagnitude * mAlphaInRecip;
    if (mAlpha > mFluidPlane->GetAlpha()) {
      mAlpha = mFluidPlane->GetAlpha();
      mFogBias = mOrigFogBias;
      mFogMagnitude = mOrigFogMagnitude;
      mAlphaIn = false;
    }
  }

  if (IsMorphing()) {
    bool stillMorphing = true;
    if (mMorphIn) {
      mMorphFactor += dt / mMorphInTime;
      if (mMorphFactor > 1.f) {
        mMorphFactor = 1.f;
        stillMorphing = false;
      }
    } else {
      mMorphFactor -= dt / mMorphOutTime;
      if (mMorphFactor < 0.f) {
        mMorphFactor = 0.f;
        stillMorphing = false;
      }
    }

    SetTranslation(FakeLerp(mPositionOrig, mPositionMorphed, mMorphFactor));

    mDamageInfo.SetDamage(mDamageOrig * (1.f - mMorphFactor) +
                              mDamageMorphed * mMorphFactor);

    CVector3f lerpExtent = FakeLerp(mExtentOrig, mExtentMorphed, mMorphFactor);
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
  rstl::list< rstl::pair< TUniqueId, bool > >::iterator it = mWaterInhabitants.begin();
  while (it != mWaterInhabitants.end()) {
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
      mWaterInhabitants.erase(it);
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

  rstl::list< rstl::pair< TUniqueId, bool > >::iterator it = mWaterInhabitants.begin();
  for (; it != mWaterInhabitants.end(); ++it) {
    if (it->first == otherAct.GetUniqueId()) {
      it->second = true;
      return;
    }
  }

  const rstl::optional_object< CAABox >& touchBounds = otherAct.GetTouchBounds();
  if (!touchBounds) {
    return;
  }

  mWaterInhabitants.push_back(rstl::pair< TUniqueId, bool >(otherAct.GetUniqueId(), true));
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
    if (conn->mState != kSS_Play || conn->mMsg != kSM_Activate) {
      continue;
    }
    CStateManager::TIdListResult search = mgr.GetIdListForScript(conn->mObjId);
    if (search.first != search.second) {
      if (const CScriptWater* water =
              TCastToConstPtr< CScriptWater >(mgr.GetObjectById(search.first->second))) {
        return water;
      }
    }
  }
  return NULL;
}
