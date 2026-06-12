#include "Runtime/World/CFluidPlane.hpp"

#include "Graphics/CGX.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CRipple.hpp"
#include "Runtime/World/CScriptWater.hpp"

namespace metaforce {

CFluidPlane::CFluidPlane(CAssetId texPattern1, CAssetId texPattern2, CAssetId texColor, float alpha,
                         EFluidType fluidType, float rippleIntensity, const CFluidUVMotion& motion)
: x4_texPattern1Id(texPattern1)
, x8_texPattern2Id(texPattern2)
, xc_texColorId(texColor)
, x40_alpha(alpha)
, x44_fluidType(fluidType)
, x48_rippleIntensity(rippleIntensity)
, x4c_uvMotion(motion) {
  if (g_ResFactory->GetResourceTypeById(texPattern1) == FOURCC('TXTR'))
    x10_texPattern1 = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), texPattern1});
  if (g_ResFactory->GetResourceTypeById(texPattern2) == FOURCC('TXTR'))
    x20_texPattern2 = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), texPattern2});
  if (g_ResFactory->GetResourceTypeById(texColor) == FOURCC('TXTR'))
    x30_texColor = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), texColor});
}

float CFluidPlane::ProjectRippleVelocity(float baseI, float velDot) const {
  float tmp = 0.5f * baseI * velDot * velDot;
  if (tmp != 0.f)
    tmp = std::sqrt(tmp);
  if (tmp >= 160.f)
    return 1.f;
  return tmp / 160.f;
}

float CFluidPlane::CalculateRippleIntensity(float baseI) const {
  float mul = 0.0f;
  switch (x44_fluidType) {
  case EFluidType::NormalWater:
    mul = g_tweakGame->GetRippleIntensityNormal();
    break;
  case EFluidType::PoisonWater:
    mul = g_tweakGame->GetRippleIntensityPoison();
    break;
  case EFluidType::Lava:
    mul = g_tweakGame->GetRippleIntensityLava();
    break;
  case EFluidType::PhazonFluid:
  case EFluidType::Four:
    mul = 0.8f;
    break;
  case EFluidType::ThickLava:
    mul = 1.f;
    break;
  }

  return zeus::clamp(0.f, baseI * mul * (1.f - x48_rippleIntensity + 0.5f), 1.f);
}

void CFluidPlane::AddRipple(float mag, TUniqueId rippler, const zeus::CVector3f& center, CScriptWater& water,
                            CStateManager& mgr) {
  if (!water.CanRippleAtPoint(center))
    return;

  mag = CalculateRippleIntensity(mag);
  mgr.GetFluidPlaneManager()->RippleManager().AddRipple(CRipple(rippler, center, mag));
}

void CFluidPlane::AddRipple(float intensity, TUniqueId rippler, const zeus::CVector3f& center,
                            const zeus::CVector3f& velocity, const CScriptWater& water, CStateManager& mgr,
                            const zeus::CVector3f& upVec) {
  if (!water.CanRippleAtPoint(center))
    return;

  intensity = CalculateRippleIntensity(ProjectRippleVelocity(intensity, upVec.dot(velocity)));
  mgr.GetFluidPlaneManager()->RippleManager().AddRipple(CRipple(rippler, center, intensity));
}

void CFluidPlane::AddRipple(const CRipple& ripple, const CScriptWater& water, CStateManager& mgr) {
  if (!water.CanRippleAtPoint(ripple.GetCenter()))
    return;
  mgr.GetFluidPlaneManager()->RippleManager().AddRipple(ripple);
}

void RenderTileWithRipplesNoNormals(float curY, const CFluidPlaneRender::SHFieldSample (&heights)[45][45], float startX,
                                    int startYDiv, const CFluidPlaneRender::SPatchInfo& info) {
  const CFluidPlaneRender::SHFieldSample* samples = heights[0];
  for (int numSubdivisions = CFluidPlaneRender::numSubdivisionsInTile; numSubdivisions > 0; --numSubdivisions) {
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, startYDiv << 1);
    float rippleRes = info.x18_rippleResolution;
    int count = startYDiv;
    float curX = curY;
    const CFluidPlaneRender::SHFieldSample* s = samples;
    for (; count != 0; --count, ++s, curX += rippleRes) {
      GXPosition3f32(curX, startX, s->height);
      GXColor4u8(static_cast<u8>(s->wavecapIntensity >> info.x34_redShift),
                 static_cast<u8>(s->wavecapIntensity >> info.x35_greenShift),
                 static_cast<u8>(s->wavecapIntensity >> info.x36_blueShift), 0xff);
      GXPosition3f32(curX, startX + rippleRes, s[45].height);
      GXColor4u8(static_cast<u8>(s[45].wavecapIntensity >> info.x34_redShift),
                 static_cast<u8>(s[45].wavecapIntensity >> info.x35_greenShift),
                 static_cast<u8>(s[45].wavecapIntensity >> info.x36_blueShift), 0xff);
    }
    CGX::End();
    samples += 45;
    startX += info.x18_rippleResolution;
  }
}

static void RenderTileStripNormals(const CFluidPlaneRender::SHFieldSample* samples, float curX, float curY,
                                   float rippleRes, int count, const CFluidPlaneRender::SPatchInfo& info) {
  for (; count != 0; --count, ++samples, curX += rippleRes) {
    GXPosition3f32(curX, curY, samples->height);
    GXNormal3s8(samples->nx, samples->ny, samples->nz);
    GXColor4u8(static_cast<u8>(samples->wavecapIntensity >> info.x34_redShift),
               static_cast<u8>(samples->wavecapIntensity >> info.x35_greenShift),
               static_cast<u8>(samples->wavecapIntensity >> info.x36_blueShift), 0xff);
    GXPosition3f32(curX, curY + rippleRes, samples[45].height);
    GXNormal3s8(samples[45].nx, samples[45].ny, samples[45].nz);
    GXColor4u8(static_cast<u8>(samples[45].wavecapIntensity >> info.x34_redShift),
               static_cast<u8>(samples[45].wavecapIntensity >> info.x35_greenShift),
               static_cast<u8>(samples[45].wavecapIntensity >> info.x36_blueShift), 0xff);
  }
}

void RenderTileWithRipplesNormals(float curY, const CFluidPlaneRender::SHFieldSample (&heights)[45][45], float startX,
                                  int startYDiv, const CFluidPlaneRender::SPatchInfo& info) {
  const CFluidPlaneRender::SHFieldSample* samples = heights[0];
  for (int numSubdivisions = CFluidPlaneRender::numSubdivisionsInTile; numSubdivisions > 0; --numSubdivisions) {
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, startYDiv << 1);
    RenderTileStripNormals(samples, curY, startX, info.x18_rippleResolution, startYDiv, info);
    CGX::End();
    samples += 45;
    startX += info.x18_rippleResolution;
  }
}

static void RenderTileStripNBT(const CFluidPlaneRender::SHFieldSample* samples, float curX, float curY, float rippleRes,
                               int count, const CFluidPlaneRender::SPatchInfo& info) {
  for (; count != 0; --count, ++samples, curX += rippleRes) {
    GXPosition3f32(curX, curY, samples->height);
    GXNormal3s8(samples->nx, samples->ny, samples->nz);
    GXNormal3s8(samples->nx, samples->nz, -samples->ny);
    GXNormal3s8(samples->nz, samples->ny, -samples->nx);
    GXColor4u8(static_cast<u8>(samples->wavecapIntensity >> info.x34_redShift),
               static_cast<u8>(samples->wavecapIntensity >> info.x35_greenShift),
               static_cast<u8>(samples->wavecapIntensity >> info.x36_blueShift), 0xff);
    GXPosition3f32(curX, curY + rippleRes, samples[45].height);
    GXNormal3s8(samples[45].nx, samples[45].ny, samples[45].nz);
    GXNormal3s8(samples[45].nx, samples[45].nz, -samples[45].ny);
    GXNormal3s8(samples[45].nz, samples[45].ny, -samples[45].nx);
    GXColor4u8(static_cast<u8>(samples[45].wavecapIntensity >> info.x34_redShift),
               static_cast<u8>(samples[45].wavecapIntensity >> info.x35_greenShift),
               static_cast<u8>(samples[45].wavecapIntensity >> info.x36_blueShift), 0xff);
  }
}

void RenderTileWithRipplesNBT(float curY, const CFluidPlaneRender::SHFieldSample (&heights)[45][45], float startX,
                              int startYDiv, const CFluidPlaneRender::SPatchInfo& info) {
  const CFluidPlaneRender::SHFieldSample* samples = heights[0];
  for (int numSubdivisions = CFluidPlaneRender::numSubdivisionsInTile; numSubdivisions > 0; --numSubdivisions) {
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, startYDiv << 1);
    RenderTileStripNBT(samples, curY, startX, info.x18_rippleResolution, startYDiv, info);
    CGX::End();
    samples += 45;
    startX += info.x18_rippleResolution;
  }
}

static void RenderStripEndNoNormals(const CFluidPlaneRender::SHFieldSample* samples,
                                    const CFluidPlaneRender::SPatchInfo& info, bool last, float curX, float curY) {
  if (last) {
    for (int i = 0; i <= CFluidPlaneRender::numSubdivisionsInTile; ++i) {
      GXPosition3f32(curX, curY, samples->height);
      GXColor4u8(static_cast<u8>(samples->wavecapIntensity >> info.x34_redShift),
                 static_cast<u8>(samples->wavecapIntensity >> info.x35_greenShift),
                 static_cast<u8>(samples->wavecapIntensity >> info.x36_blueShift), 0xff);
      samples += 45;
      curY += info.x18_rippleResolution;
    }
    return;
  }
  GXPosition3f32(curX, curY, samples->height);
  GXColor4u8(static_cast<u8>(samples->wavecapIntensity >> info.x34_redShift),
             static_cast<u8>(samples->wavecapIntensity >> info.x35_greenShift),
             static_cast<u8>(samples->wavecapIntensity >> info.x36_blueShift), 0xff);
  samples += CFluidPlaneRender::numSubdivisionsInTile * 45;
  GXPosition3f32(curX, curY + info.x14_tileSize, samples->height);
  GXColor4u8(static_cast<u8>(samples->wavecapIntensity >> info.x34_redShift),
             static_cast<u8>(samples->wavecapIntensity >> info.x35_greenShift),
             static_cast<u8>(samples->wavecapIntensity >> info.x36_blueShift), 0xff);
}

static void RenderStripEndNormals(const CFluidPlaneRender::SHFieldSample* samples,
                                  const CFluidPlaneRender::SPatchInfo& info, bool last, float curX, float curY) {
  if (last) {
    for (int i = 0; i <= CFluidPlaneRender::numSubdivisionsInTile; ++i) {
      GXPosition3f32(curX, curY, samples->height);
      GXNormal3s8(samples->nx, samples->ny, samples->nz);
      GXColor4u8(static_cast<u8>(samples->wavecapIntensity >> info.x34_redShift),
                 static_cast<u8>(samples->wavecapIntensity >> info.x35_greenShift),
                 static_cast<u8>(samples->wavecapIntensity >> info.x36_blueShift), 0xff);
      samples += 45;
      curY += info.x18_rippleResolution;
    }
    return;
  }
  GXPosition3f32(curX, curY, samples->height);
  GXNormal3s8(samples->nx, samples->ny, samples->nz);
  GXColor4u8(static_cast<u8>(samples->wavecapIntensity >> info.x34_redShift),
             static_cast<u8>(samples->wavecapIntensity >> info.x35_greenShift),
             static_cast<u8>(samples->wavecapIntensity >> info.x36_blueShift), 0xff);
  samples += CFluidPlaneRender::numSubdivisionsInTile * 45;
  GXPosition3f32(curX, curY + info.x14_tileSize, samples->height);
  GXNormal3s8(samples->nx, samples->ny, samples->nz);
  GXColor4u8(static_cast<u8>(samples->wavecapIntensity >> info.x34_redShift),
             static_cast<u8>(samples->wavecapIntensity >> info.x35_greenShift),
             static_cast<u8>(samples->wavecapIntensity >> info.x36_blueShift), 0xff);
}

static void RenderStripEndNBT(const CFluidPlaneRender::SHFieldSample* samples,
                              const CFluidPlaneRender::SPatchInfo& info, bool last, float curX, float curY) {
  if (last) {
    for (int i = 0; i <= CFluidPlaneRender::numSubdivisionsInTile; ++i) {
      GXPosition3f32(curX, curY, samples->height);
      GXNormal3s8(samples->nx, samples->ny, samples->nz);
      GXNormal3s8(samples->nx, samples->nz, -samples->ny);
      GXNormal3s8(samples->nz, samples->ny, -samples->nx);
      GXColor4u8(static_cast<u8>(samples->wavecapIntensity >> info.x34_redShift),
                 static_cast<u8>(samples->wavecapIntensity >> info.x35_greenShift),
                 static_cast<u8>(samples->wavecapIntensity >> info.x36_blueShift), 0xff);
      samples += 45;
      curY += info.x18_rippleResolution;
    }
    return;
  }
  GXPosition3f32(curX, curY, samples->height);
  GXNormal3s8(samples->nx, samples->ny, samples->nz);
  GXNormal3s8(samples->nx, samples->nz, -samples->ny);
  GXNormal3s8(samples->nz, samples->ny, -samples->nx);
  GXColor4u8(static_cast<u8>(samples->wavecapIntensity >> info.x34_redShift),
             static_cast<u8>(samples->wavecapIntensity >> info.x35_greenShift),
             static_cast<u8>(samples->wavecapIntensity >> info.x36_blueShift), 0xff);
  samples += CFluidPlaneRender::numSubdivisionsInTile * 45;
  GXPosition3f32(curX, curY + info.x14_tileSize, samples->height);
  GXNormal3s8(samples->nx, samples->ny, samples->nz);
  GXNormal3s8(samples->nx, samples->nz, -samples->ny);
  GXNormal3s8(samples->nz, samples->ny, -samples->nx);
  GXColor4u8(static_cast<u8>(samples->wavecapIntensity >> info.x34_redShift),
             static_cast<u8>(samples->wavecapIntensity >> info.x35_greenShift),
             static_cast<u8>(samples->wavecapIntensity >> info.x36_blueShift), 0xff);
}

void CFluidPlane::RenderStripWithRipples(const Heights& heights, const Flags& flags, int startYDiv, float curY,
                                         const CFluidPlaneRender::SPatchInfo& info) {
  int iDiv = (startYDiv + CFluidPlaneRender::numSubdivisionsInTile - 1) / CFluidPlaneRender::numSubdivisionsInTile;
  float halfRes = info.x18_rippleResolution * static_cast<float>(CFluidPlaneRender::numSubdivisionsInTile / 2);
  float centerY = halfRes + curY;
  float curX = info.x4_localMin.x();
  int numSubTimesStride = CFluidPlaneRender::numSubdivisionsInTile * 45;
  int centerOffset = CFluidPlaneRender::numSubdivisionsInTile / 2 + numSubTimesStride / 2;
  int numTilesX =
      (info.x0_xSubdivs + CFluidPlaneRender::numSubdivisionsInTile - 4) / CFluidPlaneRender::numSubdivisionsInTile;
  int gridOffset = info.x28_tileX + info.x2a_gridDimX * (info.x2e_tileY + iDiv - 1);
  const u8* flagsPtr = &flags[0][0] + iDiv * 9;
  int bottomOffset = numSubTimesStride;
  int topRightOffset = CFluidPlaneRender::numSubdivisionsInTile + numSubTimesStride;
  int topOffset = CFluidPlaneRender::numSubdivisionsInTile;
  const CFluidPlaneRender::SHFieldSample* base = &heights[startYDiv][1];
  int tileIdx = 1;
  int xPos = 1;

  while (xPos < info.x0_xSubdivs - 2) {
    int numCombined = 1;
    if (info.x30_gridFlags != nullptr && !info.x30_gridFlags[gridOffset]) {
      goto nextTile;
    }

    {
      const u8* flagByte = flagsPtr + tileIdx;
      if ((*flagByte & 0x1f) == 0x1f) {
        // Fully rippled tile - check for consecutive fully-rippled tiles
        const u8* nextFlag = flagByte + 1;
        while (tileIdx + numCombined <= numTilesX) {
          if ((*nextFlag & 0x1f) != 0x1f) {
            break;
          }
          if (info.x30_gridFlags != nullptr && !info.x30_gridFlags[gridOffset + numCombined]) {
            break;
          }
          ++nextFlag;
          ++numCombined;
        }

        int numVerts = numCombined * CFluidPlaneRender::numSubdivisionsInTile + 1;
        CFluidPlaneRender::NormalMode normalMode =
            static_cast<CFluidPlaneRender::NormalMode>(static_cast<signed char>(info.x37_normalMode));

        switch (normalMode) {
        case CFluidPlaneRender::NormalMode::None: {
          const CFluidPlaneRender::SHFieldSample* samples = base;
          float stripY = curY;
          for (int subdivsLeft = CFluidPlaneRender::numSubdivisionsInTile; subdivsLeft > 0; --subdivsLeft) {
            CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, static_cast<ushort>(numVerts * 2));
            float rippleRes = info.x18_rippleResolution;
            float endY = stripY + rippleRes;
            float startX = curX;
            int count = numVerts;
            const CFluidPlaneRender::SHFieldSample* s = samples;
            for (; count != 0; --count) {
              GXPosition3f32(startX, stripY, s->height);
              GXPosition3f32(startX, endY, s[45].height);
              startX += rippleRes;
              ++s;
            }
            CGX::End();
            samples += 45;
            stripY += info.x18_rippleResolution;
          }
          break;
        }
        case CFluidPlaneRender::NormalMode::NoNormals:
          RenderTileWithRipplesNoNormals(
              curX, reinterpret_cast<const CFluidPlaneRender::SHFieldSample(&)[45][45]>(*base), curY, numVerts, info);
          break;
        case CFluidPlaneRender::NormalMode::Normals:
          RenderTileWithRipplesNormals(curX, reinterpret_cast<const CFluidPlaneRender::SHFieldSample(&)[45][45]>(*base),
                                       curY, numVerts, info);
          break;
        case CFluidPlaneRender::NormalMode::NBT:
          RenderTileWithRipplesNBT(curX, reinterpret_cast<const CFluidPlaneRender::SHFieldSample(&)[45][45]>(*base),
                                   curY, numVerts, info);
          break;
        default:
          break;
        }
      } else {
        // Partial ripple - extract edge flags
        const u8* nextFlag = flagByte + 1;
        u8 below = flagByte[9];
        u8 left = flagByte[-1];
        u8 right = flagByte[1];
        u8 above = flagByte[-9];

        bool hasBelow = (below >> 1) & 1;
        bool isLeftEdge = (left >> 3) & 1;
        bool hasRight = (right >> 2) & 1;
        bool hasAbove = above & 1;

        int totalVerts = (hasBelow ? CFluidPlaneRender::numSubdivisionsInTile : 1) + 2;
        totalVerts += hasRight ? CFluidPlaneRender::numSubdivisionsInTile : 1;
        totalVerts += hasAbove ? CFluidPlaneRender::numSubdivisionsInTile : 1;
        totalVerts += isLeftEdge ? CFluidPlaneRender::numSubdivisionsInTile : 1;

        if (totalVerts == 6 && (info.x37_normalMode == CFluidPlaneRender::NormalMode::Normals ||
                                info.x37_normalMode == CFluidPlaneRender::NormalMode::NBT)) {
          // Special strip mode - combine consecutive non-rippled non-edge tiles
          while (tileIdx + numCombined <= numTilesX) {
            if ((*nextFlag & 0x1f) == 0x1f) {
              break;
            }
            if (info.x30_gridFlags != nullptr && !info.x30_gridFlags[gridOffset + numCombined]) {
              break;
            }
            if ((nextFlag[9] & 2) != 0) {
              break;
            }
            if ((nextFlag[1] & 4) != 0) {
              break;
            }
            if ((nextFlag[-9] & 1) != 0) {
              break;
            }
            ++nextFlag;
            ++numCombined;
          }

          int stripCount = numCombined + 1;
          CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, static_cast<ushort>(stripCount * 2));

          CFluidPlaneRender::NormalMode normalMode =
              static_cast<CFluidPlaneRender::NormalMode>(static_cast<signed char>(info.x37_normalMode));
          switch (normalMode) {
          case CFluidPlaneRender::NormalMode::Normals: {
            const CFluidPlaneRender::SHFieldSample* topSamples = base;
            const CFluidPlaneRender::SHFieldSample* bottomSamples =
                base + CFluidPlaneRender::numSubdivisionsInTile * 45;
            float stripX = curX;
            for (int n = stripCount; n > 0; --n, topSamples += CFluidPlaneRender::numSubdivisionsInTile,
                     bottomSamples += CFluidPlaneRender::numSubdivisionsInTile, stripX += info.x14_tileSize) {
              GXPosition3f32(stripX, curY, topSamples->height);
              GXNormal3s8(topSamples->nx, topSamples->ny, topSamples->nz);
              GXColor4u8(static_cast<u8>(topSamples->wavecapIntensity >> info.x34_redShift),
                         static_cast<u8>(topSamples->wavecapIntensity >> info.x35_greenShift),
                         static_cast<u8>(topSamples->wavecapIntensity >> info.x36_blueShift), 0xff);
              GXPosition3f32(stripX, info.x14_tileSize + curY, bottomSamples->height);
              GXNormal3s8(bottomSamples->nx, bottomSamples->ny, bottomSamples->nz);
              GXColor4u8(static_cast<u8>(bottomSamples->wavecapIntensity >> info.x34_redShift),
                         static_cast<u8>(bottomSamples->wavecapIntensity >> info.x35_greenShift),
                         static_cast<u8>(bottomSamples->wavecapIntensity >> info.x36_blueShift), 0xff);
            }
            break;
          }
          case CFluidPlaneRender::NormalMode::NBT: {
            const CFluidPlaneRender::SHFieldSample* topSamples = base;
            const CFluidPlaneRender::SHFieldSample* bottomSamples =
                base + CFluidPlaneRender::numSubdivisionsInTile * 45;
            float stripX = curX;
            for (int n = stripCount; n > 0; --n, topSamples += CFluidPlaneRender::numSubdivisionsInTile,
                     bottomSamples += CFluidPlaneRender::numSubdivisionsInTile, stripX += info.x14_tileSize) {
              GXPosition3f32(stripX, curY, topSamples->height);
              GXNormal3s8(topSamples->nx, topSamples->ny, topSamples->nz);
              GXNormal3s8(topSamples->nx, topSamples->nz, -topSamples->ny);
              GXNormal3s8(topSamples->nz, topSamples->ny, -topSamples->nx);
              GXColor4u8(static_cast<u8>(topSamples->wavecapIntensity >> info.x34_redShift),
                         static_cast<u8>(topSamples->wavecapIntensity >> info.x35_greenShift),
                         static_cast<u8>(topSamples->wavecapIntensity >> info.x36_blueShift), 0xff);
              GXPosition3f32(stripX, info.x14_tileSize + curY, bottomSamples->height);
              GXNormal3s8(bottomSamples->nx, bottomSamples->ny, bottomSamples->nz);
              GXNormal3s8(bottomSamples->nx, bottomSamples->nz, -bottomSamples->ny);
              GXNormal3s8(bottomSamples->nz, bottomSamples->ny, -bottomSamples->nx);
              GXColor4u8(static_cast<u8>(bottomSamples->wavecapIntensity >> info.x34_redShift),
                         static_cast<u8>(bottomSamples->wavecapIntensity >> info.x35_greenShift),
                         static_cast<u8>(bottomSamples->wavecapIntensity >> info.x36_blueShift), 0xff);
            }
            break;
          }
          default:
            break;
          }
          CGX::End();
        } else {
          // Triangle fan rendering
          CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, static_cast<ushort>(totalVerts));

          CFluidPlaneRender::NormalMode normalMode =
              static_cast<CFluidPlaneRender::NormalMode>(static_cast<signed char>(info.x37_normalMode));

          switch (normalMode) {
          case CFluidPlaneRender::NormalMode::None: {
            // Center vertex
            float centerX = halfRes + curX;
            GXPosition3f32(centerX, centerY, base[centerOffset].height);

            // South edge (going right along bottom)
            {
              float endY = info.x14_tileSize + curY;
              const CFluidPlaneRender::SHFieldSample* s = base + bottomOffset;
              float stripX = curX;
              int count = hasBelow ? CFluidPlaneRender::numSubdivisionsInTile : 1;
              for (; count > 0; --count) {
                GXPosition3f32(stripX, endY, s->height);
                stripX += info.x18_rippleResolution;
                ++s;
              }
            }

            // East edge (going up along right side)
            {
              float endY = info.x14_tileSize + curY;
              float endX = info.x14_tileSize + curX;
              const CFluidPlaneRender::SHFieldSample* s = base + topRightOffset;
              int count = hasRight ? CFluidPlaneRender::numSubdivisionsInTile : 1;
              for (; count > 0; --count) {
                GXPosition3f32(endX, endY, s->height);
                endY -= info.x18_rippleResolution;
                s -= 45;
              }
            }

            // North edge (going left along top)
            {
              float endX = info.x14_tileSize + curX;
              const CFluidPlaneRender::SHFieldSample* s = base + topOffset;
              int count = hasAbove ? CFluidPlaneRender::numSubdivisionsInTile : 1;
              for (; count > 0; --count) {
                GXPosition3f32(endX, curY, s->height);
                endX -= info.x18_rippleResolution;
                --s;
              }
            }

            // West edge
            {
              const CFluidPlaneRender::SHFieldSample* s = base;
              float y = curY;
              if (isLeftEdge) {
                int count = CFluidPlaneRender::numSubdivisionsInTile + 1;
                for (; count > 0; --count) {
                  GXPosition3f32(curX, y, s->height);
                  s += 45;
                  y += info.x18_rippleResolution;
                }
              } else {
                GXPosition3f32(curX, curY, base->height);
                GXPosition3f32(curX, curY + info.x14_tileSize,
                               base[CFluidPlaneRender::numSubdivisionsInTile * 45].height);
              }
            }
            break;
          }
          case CFluidPlaneRender::NormalMode::NoNormals: {
            // Center vertex with color
            const CFluidPlaneRender::SHFieldSample* centerSample = base + centerOffset;
            float centerX = halfRes + curX;
            GXPosition3f32(centerX, centerY, centerSample->height);
            GXColor4u8(static_cast<u8>(centerSample->wavecapIntensity >> info.x34_redShift),
                       static_cast<u8>(centerSample->wavecapIntensity >> info.x35_greenShift),
                       static_cast<u8>(centerSample->wavecapIntensity >> info.x36_blueShift), 0xff);

            // South edge
            {
              float endY = info.x14_tileSize + curY;
              const CFluidPlaneRender::SHFieldSample* s = base + bottomOffset;
              float stripX = curX;
              int count = hasBelow ? CFluidPlaneRender::numSubdivisionsInTile : 1;
              for (; count > 0; --count) {
                GXPosition3f32(stripX, endY, s->height);
                GXColor4u8(static_cast<u8>(s->wavecapIntensity >> info.x34_redShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x35_greenShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x36_blueShift), 0xff);
                stripX += info.x18_rippleResolution;
                ++s;
              }
            }

            // East edge
            {
              float endY = info.x14_tileSize + curY;
              float endX = info.x14_tileSize + curX;
              const CFluidPlaneRender::SHFieldSample* s = base + topRightOffset;
              int count = hasRight ? CFluidPlaneRender::numSubdivisionsInTile : 1;
              for (; count > 0; --count) {
                GXPosition3f32(endX, endY, s->height);
                GXColor4u8(static_cast<u8>(s->wavecapIntensity >> info.x34_redShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x35_greenShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x36_blueShift), 0xff);
                endY -= info.x18_rippleResolution;
                s -= 45;
              }
            }

            // North edge
            {
              float endX = info.x14_tileSize + curX;
              const CFluidPlaneRender::SHFieldSample* s = base + topOffset;
              int count = hasAbove ? CFluidPlaneRender::numSubdivisionsInTile : 1;
              for (; count > 0; --count) {
                GXPosition3f32(endX, curY, s->height);
                GXColor4u8(static_cast<u8>(s->wavecapIntensity >> info.x34_redShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x35_greenShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x36_blueShift), 0xff);
                endX -= info.x18_rippleResolution;
                --s;
              }
            }

            // West edge
            RenderStripEndNoNormals(base, info, isLeftEdge, curX, curY);
            break;
          }
          case CFluidPlaneRender::NormalMode::Normals: {
            // Center vertex with normals and color
            const CFluidPlaneRender::SHFieldSample* centerSample = base + centerOffset;
            float centerX = halfRes + curX;
            GXPosition3f32(centerX, centerY, centerSample->height);
            GXNormal3s8(centerSample->nx, centerSample->ny, centerSample->nz);
            GXColor4u8(static_cast<u8>(centerSample->wavecapIntensity >> info.x34_redShift),
                       static_cast<u8>(centerSample->wavecapIntensity >> info.x35_greenShift),
                       static_cast<u8>(centerSample->wavecapIntensity >> info.x36_blueShift), 0xff);

            // South edge
            {
              float endY = info.x14_tileSize + curY;
              const CFluidPlaneRender::SHFieldSample* s = base + bottomOffset;
              float stripX = curX;
              int count = hasBelow ? CFluidPlaneRender::numSubdivisionsInTile : 1;
              for (; count > 0; --count) {
                GXPosition3f32(stripX, endY, s->height);
                GXNormal3s8(s->nx, s->ny, s->nz);
                GXColor4u8(static_cast<u8>(s->wavecapIntensity >> info.x34_redShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x35_greenShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x36_blueShift), 0xff);
                stripX += info.x18_rippleResolution;
                ++s;
              }
            }

            // East edge
            {
              float endY = info.x14_tileSize + curY;
              float endX = info.x14_tileSize + curX;
              const CFluidPlaneRender::SHFieldSample* s = base + topRightOffset;
              int count = hasRight ? CFluidPlaneRender::numSubdivisionsInTile : 1;
              for (; count > 0; --count) {
                GXPosition3f32(endX, endY, s->height);
                GXNormal3s8(s->nx, s->ny, s->nz);
                GXColor4u8(static_cast<u8>(s->wavecapIntensity >> info.x34_redShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x35_greenShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x36_blueShift), 0xff);
                endY -= info.x18_rippleResolution;
                s -= 45;
              }
            }

            // North edge
            {
              float endX = info.x14_tileSize + curX;
              const CFluidPlaneRender::SHFieldSample* s = base + topOffset;
              int count = hasAbove ? CFluidPlaneRender::numSubdivisionsInTile : 1;
              for (; count > 0; --count) {
                GXPosition3f32(endX, curY, s->height);
                GXNormal3s8(s->nx, s->ny, s->nz);
                GXColor4u8(static_cast<u8>(s->wavecapIntensity >> info.x34_redShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x35_greenShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x36_blueShift), 0xff);
                endX -= info.x18_rippleResolution;
                --s;
              }
            }

            // West edge
            RenderStripEndNormals(base, info, isLeftEdge, curX, curY);
            break;
          }
          case CFluidPlaneRender::NormalMode::NBT: {
            // Center vertex with NBT and color
            const CFluidPlaneRender::SHFieldSample* centerSample = base + centerOffset;
            float centerX = halfRes + curX;
            GXPosition3f32(centerX, centerY, centerSample->height);
            GXNormal3s8(centerSample->nx, centerSample->ny, centerSample->nz);
            GXNormal3s8(centerSample->nx, centerSample->nz, -centerSample->ny);
            GXNormal3s8(centerSample->nz, centerSample->ny, -centerSample->nx);
            GXColor4u8(static_cast<u8>(centerSample->wavecapIntensity >> info.x34_redShift),
                       static_cast<u8>(centerSample->wavecapIntensity >> info.x35_greenShift),
                       static_cast<u8>(centerSample->wavecapIntensity >> info.x36_blueShift), 0xff);

            // South edge
            {
              float endY = info.x14_tileSize + curY;
              const CFluidPlaneRender::SHFieldSample* s = base + bottomOffset;
              float stripX = curX;
              int count = 1;
              if (hasBelow) {
                count = CFluidPlaneRender::numSubdivisionsInTile;
              }
              for (; count > 0; --count) {
                GXPosition3f32(stripX, endY, s->height);
                GXNormal3s8(s->nx, s->ny, s->nz);
                GXNormal3s8(s->nx, s->nz, -s->ny);
                GXNormal3s8(s->nz, s->ny, -s->nx);
                GXColor4u8(static_cast<u8>(s->wavecapIntensity >> info.x34_redShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x35_greenShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x36_blueShift), 0xff);
                stripX += info.x18_rippleResolution;
                ++s;
              }
            }

            // East edge
            {
              float endY = info.x14_tileSize + curY;
              float endX = info.x14_tileSize + curX;
              const CFluidPlaneRender::SHFieldSample* s = base + topRightOffset;
              int count = 1;
              if (hasRight) {
                count = CFluidPlaneRender::numSubdivisionsInTile;
              }
              for (; count > 0; --count) {
                GXPosition3f32(endX, endY, s->height);
                GXNormal3s8(s->nx, s->ny, s->nz);
                GXNormal3s8(s->nx, s->nz, -s->ny);
                GXNormal3s8(s->nz, s->ny, -s->nx);
                GXColor4u8(static_cast<u8>(s->wavecapIntensity >> info.x34_redShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x35_greenShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x36_blueShift), 0xff);
                endY -= info.x18_rippleResolution;
                s -= 45;
              }
            }

            // North edge
            {
              float endX = info.x14_tileSize + curX;
              const CFluidPlaneRender::SHFieldSample* s = base + topOffset;
              int count = 1;
              if (hasAbove) {
                count = CFluidPlaneRender::numSubdivisionsInTile;
              }
              for (; count > 0; --count) {
                GXPosition3f32(endX, curY, s->height);
                GXNormal3s8(s->nx, s->ny, s->nz);
                GXNormal3s8(s->nx, s->nz, -s->ny);
                GXNormal3s8(s->nz, s->ny, -s->nx);
                GXColor4u8(static_cast<u8>(s->wavecapIntensity >> info.x34_redShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x35_greenShift),
                           static_cast<u8>(s->wavecapIntensity >> info.x36_blueShift), 0xff);
                endX -= info.x18_rippleResolution;
                --s;
              }
            }

            // West edge
            RenderStripEndNBT(base, info, isLeftEdge, curX, curY);
            break;
          }
          default:
            break;
          }
          CGX::End();
        }
      }
    }

  nextTile:
    tileIdx += numCombined;
    gridOffset += numCombined;
    xPos += CFluidPlaneRender::numSubdivisionsInTile * numCombined;
    base += CFluidPlaneRender::numSubdivisionsInTile * numCombined;
    curX += info.x14_tileSize * static_cast<float>(numCombined);
  }
}

void CFluidPlane::RenderPatch(const CFluidPlaneRender::SPatchInfo& info, const Heights& heights, const Flags& flags,
                              bool noRipples, bool flaggedGridGen) {
  if (noRipples) {
    int ySubdivs = static_cast<signed char>(info.x1_ySubdivs);
    int xSubdivs = static_cast<signed char>(info.x0_xSubdivs);
    CFluidPlaneRender::NormalMode normalMode = info.x37_normalMode;
    float localMinX = info.x4_localMin.x();
    float localMinY = info.x4_localMin.y();
    float endY = info.x18_rippleResolution * static_cast<float>(ySubdivs - 2) + localMinY;
    float endX = info.x18_rippleResolution * static_cast<float>(xSubdivs - 2) + localMinX;

    switch (normalMode) {
    case CFluidPlaneRender::NormalMode::None:
      CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
      GXPosition3f32(localMinX, localMinY, 0.f);
      GXPosition3f32(localMinX, endY, 0.f);
      GXPosition3f32(endX, localMinY, 0.f);
      GXPosition3f32(endX, endY, 0.f);
      CGX::End();
      break;
    case CFluidPlaneRender::NormalMode::NoNormals:
      CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
      GXPosition3f32(localMinX, localMinY, 0.f);
      GXColor4u8(0, 0, 0, 0xff);
      GXPosition3f32(localMinX, endY, 0.f);
      GXColor4u8(0, 0, 0, 0xff);
      GXPosition3f32(endX, localMinY, 0.f);
      GXColor4u8(0, 0, 0, 0xff);
      GXPosition3f32(endX, endY, 0.f);
      GXColor4u8(0, 0, 0, 0xff);
      CGX::End();
      break;
    case CFluidPlaneRender::NormalMode::Normals: {
      xSubdivs = (xSubdivs - 3) / CFluidPlaneRender::numSubdivisionsInTile;
      int numTilesXP1 = xSubdivs + 1;
      int gridOffset =
          static_cast<int>(info.x2e_tileY) * static_cast<int>(info.x2a_gridDimX) + static_cast<int>(info.x28_tileX);
      ySubdivs = (ySubdivs - 3) / CFluidPlaneRender::numSubdivisionsInTile + 1;
      endY = localMinY;

      for (int iY = ySubdivs; iY > 0; --iY) {
        bool isFirstRow = (ySubdivs - iY) == 0;
        bool isLastRow = (1 - iY) == 0;
        endX = localMinX;
        int iX = 0;

        while (iX < numTilesXP1) {
          const bool* gridFlags = info.x30_gridFlags;
          if (gridFlags == nullptr || gridFlags[gridOffset + iX]) {
            bool isLeftEdge = iX == 0;
            bool isRightEdge = (numTilesXP1 - 1 - iX) == 0;

            if (isFirstRow || isLastRow || isLeftEdge || isRightEdge) {
              int totalVerts = (isLastRow ? CFluidPlaneRender::numSubdivisionsInTile : 1) + 2;
              totalVerts += (isRightEdge ? CFluidPlaneRender::numSubdivisionsInTile : 1);
              totalVerts += (isFirstRow ? CFluidPlaneRender::numSubdivisionsInTile : 1);
              totalVerts += (isLeftEdge ? CFluidPlaneRender::numSubdivisionsInTile : 1);

              CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, static_cast<ushort>(totalVerts));

              float halfTile = 0.5f * info.x14_tileSize;
              GXPosition3f32(endX + halfTile, endY + halfTile, 0.f);
              GXNormal3s8(0, 0, 63);
              GXColor4u8(0, 0, 0, 0xff);

              {
                int count = isLastRow ? CFluidPlaneRender::numSubdivisionsInTile : 1;
                float edgeX = endX;
                for (int e = count; e > 0; --e) {
                  GXPosition3f32(edgeX, endY + info.x14_tileSize, 0.f);
                  GXNormal3s8(0, 0, 63);
                  GXColor4u8(0, 0, 0, 0xff);
                  edgeX += info.x18_rippleResolution;
                }
              }

              {
                int count = isRightEdge ? CFluidPlaneRender::numSubdivisionsInTile : 1;
                float edgeY = endY + info.x14_tileSize;
                for (int e = count; e > 0; --e) {
                  GXPosition3f32(endX + info.x14_tileSize, edgeY, 0.f);
                  GXNormal3s8(0, 0, 63);
                  GXColor4u8(0, 0, 0, 0xff);
                  edgeY -= info.x18_rippleResolution;
                }
              }

              {
                int count = isFirstRow ? CFluidPlaneRender::numSubdivisionsInTile : 1;
                float edgeX = endX + info.x14_tileSize;
                for (int e = count; e > 0; --e) {
                  GXPosition3f32(edgeX, endY, 0.f);
                  GXNormal3s8(0, 0, 63);
                  GXColor4u8(0, 0, 0, 0xff);
                  edgeX -= info.x18_rippleResolution;
                }
              }

              {
                int count = isLeftEdge ? CFluidPlaneRender::numSubdivisionsInTile : 1;
                float edgeY = endY;
                for (int e = count; e > 0; --e) {
                  GXPosition3f32(endX, edgeY, 0.f);
                  GXNormal3s8(0, 0, 63);
                  GXColor4u8(0, 0, 0, 0xff);
                  edgeY += info.x18_rippleResolution;
                }
              }

              GXPosition3f32(endX, endY + info.x14_tileSize, 0.f);
              GXNormal3s8(0, 0, 63);
              GXColor4u8(0, 0, 0, 0xff);
              CGX::End();
              ++iX;
              endX += info.x14_tileSize;
            } else {
              int endIX = iX;
              do {
                ++endIX;
                if (numTilesXP1 <= endIX)
                  break;
              } while (gridFlags == nullptr || gridFlags[gridOffset + endIX]);
              int runLen = (endIX - iX) + 1;
              CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, static_cast<ushort>(runLen * 2));
              if (runLen > 0) {
                do {
                  GXPosition3f32(endX, endY, 0.f);
                  GXNormal3s8(0, 0, 63);
                  GXColor4u8(0, 0, 0, 255);
                  GXPosition3f32(endX, endY + info.x14_tileSize, 0.f);
                  GXNormal3s8(0, 0, 63);
                  GXColor4u8(0, 0, 0, 255);
                  endX += info.x14_tileSize;
                  --runLen;
                } while (runLen != 0);
              }
              CGX::End();
              iX = endIX + 1;
              if (iX == numTilesXP1) {
                iX = endIX;
                endX -= info.x14_tileSize;
              }
            }
          } else {
            endX += info.x14_tileSize;
            ++iX;
            while (iX < numTilesXP1) {
              if (gridFlags[gridOffset + iX]) {
                break;
              }
              endX += info.x14_tileSize;
              ++iX;
            }
          }
        }
        endY += info.x14_tileSize;
        gridOffset += static_cast<int>(info.x2a_gridDimX);
      }
      break;
    }
    case CFluidPlaneRender::NormalMode::NBT: {
      if (flaggedGridGen || info.x30_gridFlags == nullptr) {
        CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
        GXPosition3f32(localMinX, localMinY, 0.f);
        GXNormal3s8(0, 0, 63); // Normal
        GXNormal3s8(0, 63, 0); // Binormal
        GXNormal3s8(63, 0, 0); // Tangent
        GXColor4u8(0, 0, 0, 255);
        GXPosition3f32(localMinX, endY, 0.f);
        GXNormal3s8(0, 0, 63);
        GXNormal3s8(0, 63, 0);
        GXNormal3s8(63, 0, 0);
        GXColor4u8(0, 0, 0, 255);
        GXPosition3f32(endX, localMinY, 0.f);
        GXNormal3s8(0, 0, 63);
        GXNormal3s8(0, 63, 0);
        GXNormal3s8(63, 0, 0);
        GXColor4u8(0, 0, 0, 255);
        GXPosition3f32(endX, endY, 0.f);
        GXNormal3s8(0, 0, 63);
        GXNormal3s8(0, 63, 0);
        GXNormal3s8(63, 0, 0);
        GXColor4u8(0, 0, 0, 255);
        CGX::End();
      } else {
        int xSub = static_cast<signed char>(info.x0_xSubdivs);
        int ySub = static_cast<signed char>(info.x1_ySubdivs);
        int numTilesX = (xSub - 3) / CFluidPlaneRender::numSubdivisionsInTile + 1;
        int gridOffset =
            static_cast<int>(info.x28_tileX) + static_cast<int>(info.x2e_tileY) * static_cast<int>(info.x2a_gridDimX);
        endY = localMinY;
        for (int iY = (ySub - 3) / CFluidPlaneRender::numSubdivisionsInTile + 1; iY > 0; --iY) {
          endX = localMinX;
          int iX = 0;
          while (iX < numTilesX) {
            const bool* gridFlags = info.x30_gridFlags;
            if (reinterpret_cast<const char*>(gridFlags)[iX + gridOffset] != 0) {
              int endIX = iX + 1;
              const bool* ptr = &gridFlags[endIX + gridOffset];
              while (endIX < numTilesX && *ptr) {
                ++endIX;
                ++ptr;
              }
              int runLen = (endIX - iX) + 1;
              CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, static_cast<ushort>(runLen * 2));
              for (int v = runLen; v > 0; --v) {
                GXPosition3f32(endX, endY, 0.f);
                GXNormal3s8(0, 0, 63);
                GXNormal3s8(0, 63, 0);
                GXNormal3s8(63, 0, 0);
                GXColor4u8(0, 0, 0, 255);
                GXPosition3f32(endX, endY + info.x14_tileSize, 0.f);
                GXNormal3s8(0, 0, 63);
                GXNormal3s8(0, 63, 0);
                GXNormal3s8(63, 0, 0);
                GXColor4u8(0, 0, 0, 255);
                endX += info.x14_tileSize;
              }
              CGX::End();
              iX = endIX + 1;
            } else {
              endX += info.x14_tileSize;
              ++iX;
              const bool* ptr = &gridFlags[iX + gridOffset];
              while (iX < numTilesX && !*ptr) {
                endX += info.x14_tileSize;
                ++iX;
                ++ptr;
              }
            }
          }
          endY += info.x14_tileSize;
          gridOffset += static_cast<int>(info.x2a_gridDimX);
        }
      }
      break;
    }
    default:
      break;
    }
  } else {
    float curY = info.x4_localMin.y();
    int startYDiv = 1;
    for (; startYDiv < static_cast<int>(static_cast<signed char>(info.x1_ySubdivs)) - 2;
         startYDiv += CFluidPlaneRender::numSubdivisionsInTile) {
      RenderStripWithRipples(heights, flags, startYDiv, curY, info);
      curY += info.x14_tileSize;
    }
  }
}

} // namespace metaforce
