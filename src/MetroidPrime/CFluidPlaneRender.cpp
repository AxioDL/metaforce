#include "MetroidPrime/CFluidPlaneManager.hpp"

#include "MetroidPrime/CRipple.hpp"
#include "MetroidPrime/CRippleManager.hpp"

#include "Kyoto/Graphics/CGX.hpp"

#include "rstl/math.hpp"

#include <math.h>

#include "dolphin/gx/GXEnum.h"
#include "dolphin/gx/GXVert.h"
#include "dolphin/os/OSCache.h"

extern unsigned char sRippleValues[64][64];
extern unsigned char sRippleMins[64];
extern unsigned char sRippleMaxs[64];

int CFluidPlaneCPURender::numTilesInHField;
int CFluidPlaneCPURender::numSubdivisionsInTile;
int CFluidPlaneCPURender::numSubdivisionsInHField;

static float sGlobalSineWave[256];
static bool sSineWaveInitialized;

#ifdef __MWERKS__
static inline float fast_sqrt(register float x) {
  if (x == 0.f) {
    return x;
  }
  register float rsq;
  asm {
    ps_rsqrte rsq, x
    ps_mul rsq, rsq, x
  }
  return rsq;
}
#else
static inline float fast_sqrt(float x) { return sqrtf(x); }
#endif

float* InitializeSineWave() {
  if (!sSineWaveInitialized) {
    for (int i = 0; i < 256; ++i) {
      sGlobalSineWave[i] = static_cast< float >(sin(M_2PIF * (static_cast< float >(i) / 256)));
    }
    sSineWaveInitialized = true;
  }
  return sGlobalSineWave;
}

const float (&GetGlobalSineWave())[256] { return sGlobalSineWave; }

bool PrepareRipple(const CRipple& ripple, const CFluidPlaneCPURender::SPatchInfo& info,
                   CFluidPlaneCPURender::SRippleInfo& rippleInfo) {
  int lifeIdx = static_cast< int >(
      64.f * (1.f - (ripple.GetTimeFalloff() - ripple.GetTime()) / ripple.GetTimeFalloff()));
  float scaledDist = (1.f / 255.f) * ripple.GetDistFalloff();
  float centerX = info.mOoRippleResolution * (ripple.GetCenter().GetX() - info.mGlobalMinX);
  float centerY = info.mOoRippleResolution * (ripple.GetCenter().GetY() - info.mGlobalMinY);
  float dist = scaledDist * static_cast< float >(static_cast< int >(sRippleMaxs[lifeIdx]));
  float radius = info.mOoRippleResolution * fast_sqrt(dist * dist) + 1.f;

  int fromX = static_cast< int >(centerX - radius) - 1;
  int fromY = static_cast< int >(centerY - radius) - 1;
  int toX = static_cast< int >(centerX + radius) + 1;
  int toY = static_cast< int >(centerY + radius) + 1;

  rippleInfo.mFromX = rstl::max_val(fromX, 0);
  rippleInfo.mToX =
      rstl::min_val(toX, static_cast< int >(static_cast< signed char >(info.mXSubdivs)));
  rippleInfo.mFromY = rstl::max_val(fromY, 0);
  rippleInfo.mToY =
      rstl::min_val(toY, static_cast< int >(static_cast< signed char >(info.mYSubdivs)));

  rippleInfo.mGfromX = rstl::max_val(fromX, rippleInfo.mGfromX);
  rippleInfo.mGtoX = rstl::min_val(toX, rippleInfo.mGtoX);
  rippleInfo.mGfromY = rstl::max_val(fromY, rippleInfo.mGfromY);
  rippleInfo.mGtoY = rstl::min_val(toY, rippleInfo.mGtoY);

  if (rippleInfo.mGfromX > rippleInfo.mGtoX || rippleInfo.mGfromY > rippleInfo.mGtoY) {
    return false;
  }
  return true;
}

static inline void RenderTileStrip(const CFluidPlaneCPURender::SHFieldSample* samples, float curX,
                                   float curY, float rippleRes, int count,
                                   const CFluidPlaneCPURender::SPatchInfo& info) {
  for (; count != 0; --count, ++samples, curX += rippleRes) {
    GXPosition3f32(curX, curY, samples->height);
    GXPosition3f32(curX, curY + rippleRes, samples[45].height);
  }
}

static inline void RenderTileWithRipples(float startX,
                                         const CFluidPlaneCPURender::SHFieldSample* samples,
                                         float startY, const int vertexCount,
                                         const CFluidPlaneCPURender::SPatchInfo& info) {
  for (int numSubdivisions = CFluidPlaneCPURender::numSubdivisionsInTile; numSubdivisions > 0;
       --numSubdivisions) {
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, vertexCount << 1);
    RenderTileStrip(samples, startX, startY, info.mRippleResolution, vertexCount, info);
    CGX::End();
    samples += 45;
    startY += info.mRippleResolution;
  }
}

static inline void RenderTileStripNoNormals(const CFluidPlaneCPURender::SHFieldSample* samples,
                                            float curX, float curY, float rippleRes, int count,
                                            const CFluidPlaneCPURender::SPatchInfo& info) {
  for (; count != 0; --count, ++samples, curX += rippleRes) {
    GXPosition3f32(curX, curY, samples->height);
    GXColor4u8(static_cast< u8 >(samples->wavecapIntensity >> info.mRedShift),
               static_cast< u8 >(samples->wavecapIntensity >> info.mGreenShift),
               static_cast< u8 >(samples->wavecapIntensity >> info.mBlueShift), 0xff);
    GXPosition3f32(curX, curY + rippleRes, samples[45].height);
    GXColor4u8(static_cast< u8 >(samples[45].wavecapIntensity >> info.mRedShift),
               static_cast< u8 >(samples[45].wavecapIntensity >> info.mGreenShift),
               static_cast< u8 >(samples[45].wavecapIntensity >> info.mBlueShift), 0xff);
  }
}

void RenderTileWithRipplesNoNormals(float startX,
                                    const CFluidPlaneCPURender::SHFieldSample* samples,
                                    float startY, const int vertexCount,
                                    const CFluidPlaneCPURender::SPatchInfo& info) {
  for (int numSubdivisions = CFluidPlaneCPURender::numSubdivisionsInTile; numSubdivisions > 0;
       --numSubdivisions) {
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, vertexCount << 1);
    RenderTileStripNoNormals(samples, startX, startY, info.mRippleResolution, vertexCount, info);
    CGX::End();
    samples += 45;
    startY += info.mRippleResolution;
  }
}

static void RenderTileStripNormals(CFluidPlaneCPURender::SHFieldSample* samples, float curX,
                                   float curY, float rippleRes, int count,
                                   const CFluidPlaneCPURender::SPatchInfo& info) {
  for (; count != 0; --count, ++samples, curX += rippleRes) {
    GXPosition3f32(curX, curY, samples->height);
    GXNormal3s8(samples->nx, samples->ny, samples->nz);
    GXColor4u8(static_cast< u8 >(samples->wavecapIntensity >> info.mRedShift),
               static_cast< u8 >(samples->wavecapIntensity >> info.mGreenShift),
               static_cast< u8 >(samples->wavecapIntensity >> info.mBlueShift), 0xff);
    GXPosition3f32(curX, curY + rippleRes, samples[45].height);
    GXNormal3s8(samples[45].nx, samples[45].ny, samples[45].nz);
    GXColor4u8(static_cast< u8 >(samples[45].wavecapIntensity >> info.mRedShift),
               static_cast< u8 >(samples[45].wavecapIntensity >> info.mGreenShift),
               static_cast< u8 >(samples[45].wavecapIntensity >> info.mBlueShift), 0xff);
  }
}

void RenderTileWithRipplesNormals(float startX, CFluidPlaneCPURender::SHFieldSample* samples,
                                  float startY, const int vertexCount,
                                  const CFluidPlaneCPURender::SPatchInfo& info) {
  for (int numSubdivisions = CFluidPlaneCPURender::numSubdivisionsInTile; numSubdivisions > 0;
       --numSubdivisions) {
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, vertexCount << 1);
    RenderTileStripNormals(samples, startX, startY, info.mRippleResolution, vertexCount, info);
    CGX::End();
    samples += 45;
    startY += info.mRippleResolution;
  }
}

static void RenderTileStripNBT(CFluidPlaneCPURender::SHFieldSample* samples, float curX, float curY,
                               float rippleRes, int count,
                               const CFluidPlaneCPURender::SPatchInfo& info) {
  for (; count != 0; --count, ++samples, curX += rippleRes) {
    GXPosition3f32(curX, curY, samples->height);
    GXNormal3s8(samples->nx, samples->ny, samples->nz);
    GXNormal3s8(samples->nx, samples->nz, -samples->ny);
    GXNormal3s8(samples->nz, samples->ny, -samples->nx);
    GXColor4u8(static_cast< u8 >(samples->wavecapIntensity >> info.mRedShift),
               static_cast< u8 >(samples->wavecapIntensity >> info.mGreenShift),
               static_cast< u8 >(samples->wavecapIntensity >> info.mBlueShift), 0xff);
    GXPosition3f32(curX, curY + rippleRes, samples[45].height);
    GXNormal3s8(samples[45].nx, samples[45].ny, samples[45].nz);
    GXNormal3s8(samples[45].nx, samples[45].nz, -samples[45].ny);
    GXNormal3s8(samples[45].nz, samples[45].ny, -samples[45].nx);
    GXColor4u8(static_cast< u8 >(samples[45].wavecapIntensity >> info.mRedShift),
               static_cast< u8 >(samples[45].wavecapIntensity >> info.mGreenShift),
               static_cast< u8 >(samples[45].wavecapIntensity >> info.mBlueShift), 0xff);
  }
}

void RenderTileWithRipplesNBT(float startX, CFluidPlaneCPURender::SHFieldSample* samples,
                              float startY, const int vertexCount,
                              const CFluidPlaneCPURender::SPatchInfo& info) {
  for (int numSubdivisions = CFluidPlaneCPURender::numSubdivisionsInTile; numSubdivisions > 0;
       --numSubdivisions) {
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, vertexCount << 1);
    RenderTileStripNBT(samples, startX, startY, info.mRippleResolution, vertexCount, info);
    CGX::End();
    samples += 45;
    startY += info.mRippleResolution;
  }
}

static inline void RenderStripEnd(const CFluidPlaneCPURender::SHFieldSample* samples,
                                  const CFluidPlaneCPURender::SPatchInfo& info, bool last,
                                  float curX, float curY) {
  if (last) {
    for (int i = CFluidPlaneCPURender::numSubdivisionsInTile; i >= 0; --i) {
      GXPosition3f32(curX, curY, samples->height);
      samples += 45;
      curY += info.mRippleResolution;
    }
    return;
  }
  GXPosition3f32(curX, curY, samples->height);
  samples += CFluidPlaneCPURender::numSubdivisionsInTile * 45;
  GXPosition3f32(curX, curY + info.mTileSize, samples->height);
}

static void RenderStripEndNoNormals(const CFluidPlaneCPURender::SHFieldSample* samples,
                                    const CFluidPlaneCPURender::SPatchInfo& info, bool last,
                                    float curX, float curY) {
  if (last) {
    for (int i = 0; i <= CFluidPlaneCPURender::numSubdivisionsInTile; ++i) {
      GXPosition3f32(curX, curY, samples->height);
      GXColor4u8(static_cast< u8 >(samples->wavecapIntensity >> info.mRedShift),
                 static_cast< u8 >(samples->wavecapIntensity >> info.mGreenShift),
                 static_cast< u8 >(samples->wavecapIntensity >> info.mBlueShift), 0xff);
      samples += 45;
      curY += info.mRippleResolution;
    }
    return;
  }
  GXPosition3f32(curX, curY, samples->height);
  GXColor4u8(static_cast< u8 >(samples->wavecapIntensity >> info.mRedShift),
             static_cast< u8 >(samples->wavecapIntensity >> info.mGreenShift),
             static_cast< u8 >(samples->wavecapIntensity >> info.mBlueShift), 0xff);
  samples += CFluidPlaneCPURender::numSubdivisionsInTile * 45;
  GXPosition3f32(curX, curY + info.mTileSize, samples->height);
  GXColor4u8(static_cast< u8 >(samples->wavecapIntensity >> info.mRedShift),
             static_cast< u8 >(samples->wavecapIntensity >> info.mGreenShift),
             static_cast< u8 >(samples->wavecapIntensity >> info.mBlueShift), 0xff);
}

static void RenderStripEndNormals(CFluidPlaneCPURender::SHFieldSample* samples,
                                  const CFluidPlaneCPURender::SPatchInfo& info, bool last,
                                  float curX, float curY) {
  if (last) {
    for (int i = 0; i <= CFluidPlaneCPURender::numSubdivisionsInTile; ++i) {
      GXPosition3f32(curX, curY, samples->height);
      GXNormal3s8(samples->nx, samples->ny, samples->nz);
      GXColor4u8(static_cast< u8 >(samples->wavecapIntensity >> info.mRedShift),
                 static_cast< u8 >(samples->wavecapIntensity >> info.mGreenShift),
                 static_cast< u8 >(samples->wavecapIntensity >> info.mBlueShift), 0xff);
      samples += 45;
      curY += info.mRippleResolution;
    }
    return;
  }
  GXPosition3f32(curX, curY, samples->height);
  GXNormal3s8(samples->nx, samples->ny, samples->nz);
  GXColor4u8(static_cast< u8 >(samples->wavecapIntensity >> info.mRedShift),
             static_cast< u8 >(samples->wavecapIntensity >> info.mGreenShift),
             static_cast< u8 >(samples->wavecapIntensity >> info.mBlueShift), 0xff);
  samples += CFluidPlaneCPURender::numSubdivisionsInTile * 45;
  GXPosition3f32(curX, curY + info.mTileSize, samples->height);
  GXNormal3s8(samples->nx, samples->ny, samples->nz);
  GXColor4u8(static_cast< u8 >(samples->wavecapIntensity >> info.mRedShift),
             static_cast< u8 >(samples->wavecapIntensity >> info.mGreenShift),
             static_cast< u8 >(samples->wavecapIntensity >> info.mBlueShift), 0xff);
}

static void RenderStripEndNBT(CFluidPlaneCPURender::SHFieldSample* samples,
                              const CFluidPlaneCPURender::SPatchInfo& info, bool last, float curX,
                              float curY) {
  if (last) {
    for (int i = 0; i <= CFluidPlaneCPURender::numSubdivisionsInTile; ++i) {
      GXPosition3f32(curX, curY, samples->height);
      GXNormal3s8(samples->nx, samples->ny, samples->nz);
      GXNormal3s8(samples->nx, samples->nz, -samples->ny);
      GXNormal3s8(samples->nz, samples->ny, -samples->nx);
      GXColor4u8(static_cast< u8 >(samples->wavecapIntensity >> info.mRedShift),
                 static_cast< u8 >(samples->wavecapIntensity >> info.mGreenShift),
                 static_cast< u8 >(samples->wavecapIntensity >> info.mBlueShift), 0xff);
      samples += 45;
      curY += info.mRippleResolution;
    }
    return;
  }
  GXPosition3f32(curX, curY, samples->height);
  GXNormal3s8(samples->nx, samples->ny, samples->nz);
  GXNormal3s8(samples->nx, samples->nz, -samples->ny);
  GXNormal3s8(samples->nz, samples->ny, -samples->nx);
  GXColor4u8(static_cast< u8 >(samples->wavecapIntensity >> info.mRedShift),
             static_cast< u8 >(samples->wavecapIntensity >> info.mGreenShift),
             static_cast< u8 >(samples->wavecapIntensity >> info.mBlueShift), 0xff);
  samples += CFluidPlaneCPURender::numSubdivisionsInTile * 45;
  GXPosition3f32(curX, curY + info.mTileSize, samples->height);
  GXNormal3s8(samples->nx, samples->ny, samples->nz);
  GXNormal3s8(samples->nx, samples->nz, -samples->ny);
  GXNormal3s8(samples->nz, samples->ny, -samples->nx);
  GXColor4u8(static_cast< u8 >(samples->wavecapIntensity >> info.mRedShift),
             static_cast< u8 >(samples->wavecapIntensity >> info.mGreenShift),
             static_cast< u8 >(samples->wavecapIntensity >> info.mBlueShift), 0xff);
}

void RenderStripWithRipples(CFluidPlaneCPURender::SHFieldSample (&heights)[45][45],
                            const unsigned char (&flags)[9][9], int startYDiv, float curY,
                            const CFluidPlaneCPURender::SPatchInfo& info) {
  int numSubdivisions = CFluidPlaneCPURender::numSubdivisionsInTile;
  CVector3f curPos(info.mLocalMinX, curY, 0.f);
  int iDiv = (startYDiv + numSubdivisions - 1) / numSubdivisions;
  CFluidPlaneCPURender::SHFieldSample* curSamples = &heights[startYDiv][1];
  float halfRes = info.mRippleResolution * static_cast< float >(numSubdivisions / 2);
  float centerY = halfRes + curPos.GetY();
  int numSubTimesStride = numSubdivisions * 45;
  int centerOffset = numSubdivisions / 2 + numSubTimesStride / 2;
  int xSubdivs = info.mXSubdivs;
  int numTilesX = (xSubdivs + numSubdivisions - 4) / numSubdivisions;
  int gridOffset =
      static_cast< int >(info.mTileX) +
      static_cast< int >(info.mGridDimX) * (static_cast< int >(info.mTileY) + iDiv - 1);
  const unsigned char* flagsPtr = &flags[0][0] + iDiv * 9;

  int tileIdx = 1;
  int xPos = 1;

  for (; xPos < static_cast< signed char >(info.mXSubdivs) - 2;) {

    int numCombined = 1;

    if (info.mGridFlags == NULL ||
        (info.mGridFlags != NULL && info.mGridFlags[gridOffset] != 0)) {
      const unsigned char* flagByte = flagsPtr + tileIdx;
      if ((*flagByte & 0x1f) == 0x1f) {
        // Fully rippled tile - check for consecutive fully-rippled tiles
        const unsigned char* nextFlag = flagByte + 1;
        while (tileIdx + numCombined <= numTilesX) {
          if ((*nextFlag & 0x1f) != 0x1f) {
            break;
          }
          if (info.mGridFlags != NULL) {
            if (info.mGridFlags[gridOffset + numCombined] == 0) {
              break;
            }
          }
          ++nextFlag;
          ++numCombined;
        }

        int numVerts = numCombined * CFluidPlaneCPURender::numSubdivisionsInTile + 1;
        int normalMode = static_cast< int >(static_cast< signed char >(info.mNormalMode));

        switch (normalMode) {
        case CFluidPlaneCPURender::kNM_None:
          RenderTileWithRipples(curPos.GetX(), curSamples, curPos.GetY(), numVerts, info);
          break;
        case CFluidPlaneCPURender::kNM_NoNormals:
          RenderTileWithRipplesNoNormals(curPos.GetX(), curSamples, curPos.GetY(), numVerts, info);
          break;
        case CFluidPlaneCPURender::kNM_Normals:
          RenderTileWithRipplesNormals(curPos.GetX(), curSamples, curPos.GetY(), numVerts, info);
          break;
        case CFluidPlaneCPURender::kNM_NBT:
          RenderTileWithRipplesNBT(curPos.GetX(), curSamples, curPos.GetY(), numVerts, info);
          break;
        default:
          break;
        }
      } else {
        // Partial ripple - extract edge flags
        const unsigned char* nextFlag = flagByte + 1;
        unsigned char below = flagByte[9];
        unsigned char left = flagByte[-1];
        unsigned char right = flagByte[1];
        unsigned char above = flagByte[-9];

        bool hasBelow = (below >> 1) & 1;
        bool isLeftEdge = (left >> 3) & 1;
        bool hasRight = (right >> 2) & 1;
        bool hasAbove = above & 1;

        int totalVerts = (hasBelow ? CFluidPlaneCPURender::numSubdivisionsInTile : 1) + 2;
        totalVerts += (hasRight ? CFluidPlaneCPURender::numSubdivisionsInTile : 1);
        totalVerts += (hasAbove ? CFluidPlaneCPURender::numSubdivisionsInTile : 1);
        totalVerts += (isLeftEdge ? CFluidPlaneCPURender::numSubdivisionsInTile : 1);

        if (totalVerts == 6 &&
            (static_cast< signed char >(info.mNormalMode) == CFluidPlaneCPURender::kNM_Normals ||
             static_cast< signed char >(info.mNormalMode) == CFluidPlaneCPURender::kNM_NBT)) {
          // Special strip mode - combine consecutive non-rippled non-edge tiles
          while (tileIdx + numCombined <= numTilesX) {
            if ((*nextFlag & 0x1f) == 0x1f) {
              break;
            }
            if (info.mGridFlags != NULL) {
              if (info.mGridFlags[gridOffset + numCombined] == 0) {
                break;
              }
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
          CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, static_cast< ushort >(stripCount * 2));

          int normalMode = static_cast< int >(static_cast< signed char >(info.mNormalMode));
          switch (normalMode) {
          case CFluidPlaneCPURender::kNM_Normals: {

            CFluidPlaneCPURender::SHFieldSample* topSamples = curSamples;

            CFluidPlaneCPURender::SHFieldSample* bottomSamples =
                curSamples + CFluidPlaneCPURender::numSubdivisionsInTile * 45;
            float stripX = curPos.GetX();
            for (int n = stripCount; n > 0; --n) {
              GXPosition3f32(stripX, curPos.GetY(), topSamples->height);
              GXNormal3s8(topSamples->nx, topSamples->ny, topSamples->nz);
              int wavecap = topSamples->wavecapIntensity;
              topSamples = topSamples + CFluidPlaneCPURender::numSubdivisionsInTile;
              GXColor4u8(static_cast< u8 >(wavecap >> info.mRedShift),
                         static_cast< u8 >(wavecap >> info.mGreenShift),
                         static_cast< u8 >(wavecap >> info.mBlueShift), 0xff);
              GXPosition3f32(stripX, info.mTileSize + curPos.GetY(), bottomSamples->height);
              GXNormal3s8(bottomSamples->nx, bottomSamples->ny, bottomSamples->nz);
              wavecap = bottomSamples->wavecapIntensity;
              bottomSamples = bottomSamples + CFluidPlaneCPURender::numSubdivisionsInTile;
              GXColor4u8(static_cast< u8 >(wavecap >> info.mRedShift),
                         static_cast< u8 >(wavecap >> info.mGreenShift),
                         static_cast< u8 >(wavecap >> info.mBlueShift), 0xff);
              stripX += info.mTileSize;
            }
            break;
          }
          case CFluidPlaneCPURender::kNM_NBT: {

            CFluidPlaneCPURender::SHFieldSample* topSamples = curSamples;

            CFluidPlaneCPURender::SHFieldSample* bottomSamples =
                curSamples + CFluidPlaneCPURender::numSubdivisionsInTile * 45;
            float stripX = curPos.GetX();
            for (int n = stripCount; n > 0; --n) {
              // Top vertex
              GXPosition3f32(stripX, curPos.GetY(), topSamples->height);
              GXNormal3s8(topSamples->nx, topSamples->ny, topSamples->nz);
              GXNormal3s8(topSamples->nx, topSamples->nz, -topSamples->ny);
              GXNormal3s8(topSamples->nz, topSamples->ny, -topSamples->nx);
              int wavecap = topSamples->wavecapIntensity;
              topSamples = topSamples + CFluidPlaneCPURender::numSubdivisionsInTile;
              GXColor4u8(static_cast< u8 >(wavecap >> info.mRedShift),
                         static_cast< u8 >(wavecap >> info.mGreenShift),
                         static_cast< u8 >(wavecap >> info.mBlueShift), 0xff);
              // Bottom vertex
              GXPosition3f32(stripX, info.mTileSize + curPos.GetY(), bottomSamples->height);
              GXNormal3s8(bottomSamples->nx, bottomSamples->ny, bottomSamples->nz);
              GXNormal3s8(bottomSamples->nx, bottomSamples->nz, -bottomSamples->ny);
              GXNormal3s8(bottomSamples->nz, bottomSamples->ny, -bottomSamples->nx);
              wavecap = bottomSamples->wavecapIntensity;
              bottomSamples = bottomSamples + CFluidPlaneCPURender::numSubdivisionsInTile;
              GXColor4u8(static_cast< u8 >(wavecap >> info.mRedShift),
                         static_cast< u8 >(wavecap >> info.mGreenShift),
                         static_cast< u8 >(wavecap >> info.mBlueShift), 0xff);
              stripX += info.mTileSize;
            }
            break;
          }
          default:
            break;
          }
          CGX::End();
        } else {
          // Triangle fan rendering
          CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, static_cast< ushort >(totalVerts));

          int normalMode = static_cast< int >(static_cast< signed char >(info.mNormalMode));

          switch (normalMode) {
          case CFluidPlaneCPURender::kNM_None: {
            GXPosition3f32(halfRes + curPos.GetX(), centerY, curSamples[centerOffset].height);

            {
              CFluidPlaneCPURender::SHFieldSample* samples = curSamples + numSubTimesStride;
              float edgeX = curPos.GetX();
              float edgeY = info.mTileSize + curPos.GetY();
              int count = hasBelow ? CFluidPlaneCPURender::numSubdivisionsInTile : 1;
              for (; count > 0; --count) {
                GXPosition3f32(edgeX, edgeY, samples->height);
                edgeX += info.mRippleResolution;
                ++samples;
              }
            }

            {
              CFluidPlaneCPURender::SHFieldSample* samples =
                  curSamples + (numSubdivisions + numSubTimesStride);
              float edgeX = info.mTileSize + curPos.GetX();
              float edgeY = info.mTileSize + curPos.GetY();
              int count = hasRight ? CFluidPlaneCPURender::numSubdivisionsInTile : 1;
              for (; count > 0; --count) {
                GXPosition3f32(edgeX, edgeY, samples->height);
                edgeY -= info.mRippleResolution;
                samples -= 45;
              }
            }

            {
              CFluidPlaneCPURender::SHFieldSample* samples = curSamples + numSubdivisions;
              float edgeX = info.mTileSize + curPos.GetX();
              int count = hasAbove ? CFluidPlaneCPURender::numSubdivisionsInTile : 1;
              for (; count > 0; --count) {
                GXPosition3f32(edgeX, curPos.GetY(), samples->height);
                edgeX -= info.mRippleResolution;
                --samples;
              }
            }

            RenderStripEnd(curSamples, info, isLeftEdge, curPos.GetX(), curPos.GetY());
            break;
          }
          case CFluidPlaneCPURender::kNM_NoNormals: {
            // Center vertex with color
            CFluidPlaneCPURender::SHFieldSample* centerSample = curSamples + centerOffset;
            float centerX = halfRes + curPos.GetX();
            GXPosition3f32(centerX, centerY, centerSample->height);
            GXColor4u8(static_cast< u8 >(centerSample->wavecapIntensity >> info.mRedShift),
                       static_cast< u8 >(centerSample->wavecapIntensity >> info.mGreenShift),
                       static_cast< u8 >(centerSample->wavecapIntensity >> info.mBlueShift),
                       0xff);

            // Bottom edge
            {
              float endY = info.mTileSize + curPos.GetY();
              CFluidPlaneCPURender::SHFieldSample* s = curSamples + numSubTimesStride;
              float stripX = curPos.GetX();
              int count = 1;
              if (hasBelow) {
                count = CFluidPlaneCPURender::numSubdivisionsInTile;
              }
              for (; count > 0; --count) {
                GXPosition3f32(stripX, endY, s->height);
                GXColor4u8(static_cast< u8 >(s->wavecapIntensity >> info.mRedShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mGreenShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mBlueShift), 0xff);
                stripX += info.mRippleResolution;
                ++s;
              }
            }

            // Right edge
            {
              float endY = info.mTileSize + curPos.GetY();
              float endX = info.mTileSize + curPos.GetX();
              CFluidPlaneCPURender::SHFieldSample* s =
                  curSamples + (numSubdivisions + numSubTimesStride);
              int count = 1;
              if (hasRight) {
                count = CFluidPlaneCPURender::numSubdivisionsInTile;
              }
              for (; count > 0; --count) {
                GXPosition3f32(endX, endY, s->height);
                GXColor4u8(static_cast< u8 >(s->wavecapIntensity >> info.mRedShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mGreenShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mBlueShift), 0xff);
                endY -= info.mRippleResolution;
                s -= 45;
              }
            }

            // Top edge
            {
              float endX = info.mTileSize + curPos.GetX();
              CFluidPlaneCPURender::SHFieldSample* s = curSamples + numSubdivisions;
              int count = 1;
              if (hasAbove) {
                count = CFluidPlaneCPURender::numSubdivisionsInTile;
              }
              for (; count > 0; --count) {
                GXPosition3f32(endX, curPos.GetY(), s->height);
                GXColor4u8(static_cast< u8 >(s->wavecapIntensity >> info.mRedShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mGreenShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mBlueShift), 0xff);
                endX -= info.mRippleResolution;
                --s;
              }
            }

            // Left edge
            RenderStripEndNoNormals(curSamples, info, isLeftEdge, curPos.GetX(), curPos.GetY());
            break;
          }
          case CFluidPlaneCPURender::kNM_Normals: {
            // Center vertex with normals and color
            CFluidPlaneCPURender::SHFieldSample* centerSample = curSamples + centerOffset;
            float centerX = halfRes + curPos.GetX();
            GXPosition3f32(centerX, centerY, centerSample->height);
            GXNormal3s8(centerSample->nx, centerSample->ny, centerSample->nz);
            GXColor4u8(static_cast< u8 >(centerSample->wavecapIntensity >> info.mRedShift),
                       static_cast< u8 >(centerSample->wavecapIntensity >> info.mGreenShift),
                       static_cast< u8 >(centerSample->wavecapIntensity >> info.mBlueShift),
                       0xff);

            // Bottom edge
            {
              float endY = info.mTileSize + curPos.GetY();
              CFluidPlaneCPURender::SHFieldSample* s = curSamples + numSubTimesStride;
              float stripX = curPos.GetX();
              int count = 1;
              if (hasBelow) {
                count = CFluidPlaneCPURender::numSubdivisionsInTile;
              }
              for (; count > 0; --count) {
                GXPosition3f32(stripX, endY, s->height);
                GXNormal3s8(s->nx, s->ny, s->nz);
                GXColor4u8(static_cast< u8 >(s->wavecapIntensity >> info.mRedShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mGreenShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mBlueShift), 0xff);
                stripX += info.mRippleResolution;
                ++s;
              }
            }

            // Right edge
            {
              float endY = info.mTileSize + curPos.GetY();
              float endX = info.mTileSize + curPos.GetX();
              CFluidPlaneCPURender::SHFieldSample* s =
                  curSamples + (numSubdivisions + numSubTimesStride);
              int count = 1;
              if (hasRight) {
                count = CFluidPlaneCPURender::numSubdivisionsInTile;
              }
              for (; count > 0; --count) {
                GXPosition3f32(endX, endY, s->height);
                GXNormal3s8(s->nx, s->ny, s->nz);
                GXColor4u8(static_cast< u8 >(s->wavecapIntensity >> info.mRedShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mGreenShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mBlueShift), 0xff);
                endY -= info.mRippleResolution;
                s -= 45;
              }
            }

            // Top edge
            {
              float endX = info.mTileSize + curPos.GetX();
              CFluidPlaneCPURender::SHFieldSample* s = curSamples + numSubdivisions;
              int count = 1;
              if (hasAbove) {
                count = CFluidPlaneCPURender::numSubdivisionsInTile;
              }
              for (; count > 0; --count) {
                GXPosition3f32(endX, curPos.GetY(), s->height);
                GXNormal3s8(s->nx, s->ny, s->nz);
                GXColor4u8(static_cast< u8 >(s->wavecapIntensity >> info.mRedShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mGreenShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mBlueShift), 0xff);
                endX -= info.mRippleResolution;
                --s;
              }
            }

            // Left edge
            RenderStripEndNormals(curSamples, info, isLeftEdge, curPos.GetX(), curPos.GetY());
            break;
          }
          case CFluidPlaneCPURender::kNM_NBT: {
            // Center vertex with NBT and color
            CFluidPlaneCPURender::SHFieldSample* centerSample = curSamples + centerOffset;
            float centerX = halfRes + curPos.GetX();
            GXPosition3f32(centerX, centerY, centerSample->height);
            {
              GXNormal3s8(centerSample->nx, centerSample->ny, centerSample->nz);
              // Binormal
              GXNormal3s8(centerSample->nx, centerSample->nz, -centerSample->ny);
              // Tangent
              GXNormal3s8(centerSample->nz, centerSample->ny, -centerSample->nx);
              GXColor4u8(static_cast< u8 >(centerSample->wavecapIntensity >> info.mRedShift),
                         static_cast< u8 >(centerSample->wavecapIntensity >> info.mGreenShift),
                         static_cast< u8 >(centerSample->wavecapIntensity >> info.mBlueShift),
                         0xff);
            }

            // Bottom edge
            {
              float endY = info.mTileSize + curPos.GetY();
              CFluidPlaneCPURender::SHFieldSample* s = curSamples + numSubTimesStride;
              float stripX = curPos.GetX();
              int count = 1;
              if (hasBelow) {
                count = CFluidPlaneCPURender::numSubdivisionsInTile;
              }
              for (; count > 0; --count) {
                GXPosition3f32(stripX, endY, s->height);
                GXNormal3s8(s->nx, s->ny, s->nz);
                GXNormal3s8(s->nx, s->nz, -s->ny);
                GXNormal3s8(s->nz, s->ny, -s->nx);
                GXColor4u8(static_cast< u8 >(s->wavecapIntensity >> info.mRedShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mGreenShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mBlueShift), 0xff);
                stripX += info.mRippleResolution;
                ++s;
              }
            }

            // Right edge
            {
              float endY = info.mTileSize + curPos.GetY();
              float endX = info.mTileSize + curPos.GetX();
              CFluidPlaneCPURender::SHFieldSample* s =
                  curSamples + (numSubdivisions + numSubTimesStride);
              int count = 1;
              if (hasRight) {
                count = CFluidPlaneCPURender::numSubdivisionsInTile;
              }
              for (; count > 0; --count) {
                GXPosition3f32(endX, endY, s->height);
                GXNormal3s8(s->nx, s->ny, s->nz);
                GXNormal3s8(s->nx, s->nz, -s->ny);
                GXNormal3s8(s->nz, s->ny, -s->nx);
                GXColor4u8(static_cast< u8 >(s->wavecapIntensity >> info.mRedShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mGreenShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mBlueShift), 0xff);
                endY -= info.mRippleResolution;
                s -= 45;
              }
            }

            // Top edge
            {
              float endX = info.mTileSize + curPos.GetX();
              CFluidPlaneCPURender::SHFieldSample* s = curSamples + numSubdivisions;
              int count = 1;
              if (hasAbove) {
                count = CFluidPlaneCPURender::numSubdivisionsInTile;
              }
              for (; count > 0; --count) {
                GXPosition3f32(endX, curPos.GetY(), s->height);
                GXNormal3s8(s->nx, s->ny, s->nz);
                GXNormal3s8(s->nx, s->nz, -s->ny);
                GXNormal3s8(s->nz, s->ny, -s->nx);
                GXColor4u8(static_cast< u8 >(s->wavecapIntensity >> info.mRedShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mGreenShift),
                           static_cast< u8 >(s->wavecapIntensity >> info.mBlueShift), 0xff);
                endX -= info.mRippleResolution;
                --s;
              }
            }

            // Left edge
            RenderStripEndNBT(curSamples, info, isLeftEdge, curPos.GetX(), curPos.GetY());
            break;
          }
          default:
            break;
          }
          CGX::End();
        }
      }
    }

    tileIdx += numCombined;
    gridOffset += numCombined;
    xPos += CFluidPlaneCPURender::numSubdivisionsInTile * numCombined;
    curSamples += CFluidPlaneCPURender::numSubdivisionsInTile * numCombined;
    curPos.SetX(curPos.GetX() + info.mTileSize * static_cast< float >(numCombined));
  }
}

void ApplyRipple(const CFluidPlaneCPURender::SRippleInfo& rippleInfo,
                 CFluidPlaneCPURender::SHFieldSample (&heights)[45][45],
                 unsigned char (&flags)[9][9], const float (&sineWave)[256],
                 CFluidPlaneCPURender::SPatchInfo& info) {
  const CRipple& rip = *rippleInfo.mRipple;

  float timeRatio = rip.GetTime() * rip.GetOoTimeFalloff();
  float lookupT = 256.f * ((1.f - timeRatio * rip.GetOoTimeFalloff()) * rip.GetFrequency());
  int lifeIdx = static_cast< int >(64.f * timeRatio);
  float distMul = (1.f / 255.f) * rip.GetDistFalloff();

  float minDist = distMul * static_cast< float >(static_cast< int >(sRippleMins[lifeIdx]));
  const float minDistSq = minDist * minDist;
  const float minDistR = fast_sqrt(minDistSq);

  float maxDist = distMul * static_cast< float >(static_cast< int >(sRippleMaxs[lifeIdx]));
  const float maxDistSq = maxDist * maxDist;
  const float maxDistR = fast_sqrt(maxDistSq);

  int subdivsM1 = CFluidPlaneCPURender::numSubdivisionsInTile - 1;

  int fromY = (rippleInfo.mGfromY + subdivsM1) / CFluidPlaneCPURender::numSubdivisionsInTile;
  int fromX = (rippleInfo.mGfromX + subdivsM1) / CFluidPlaneCPURender::numSubdivisionsInTile;
  int toX = (rippleInfo.mGtoX + subdivsM1) / CFluidPlaneCPURender::numSubdivisionsInTile;
  int toY = (rippleInfo.mGtoY + subdivsM1) / CFluidPlaneCPURender::numSubdivisionsInTile;

  float curY = (rip.GetCenter().GetY() - info.mGlobalMinY) -
               (0.5f * info.mTileSize + static_cast< float >(fromY - 1) * info.mTileSize);

  int curGridY =
      static_cast< int >(info.mGridDimX) * (static_cast< int >(info.mTileY) + fromY - 1);
  const int startGridX = static_cast< int >(info.mTileX) + fromX - 1;
  const int gridCells =
      static_cast< int >(info.mGridDimX) * static_cast< int >(info.mGridDimY);
  float distFalloff = 64.f * rip.GetOoDistFalloff();
  int curYDiv = rippleInfo.mFromY;

  bool hasGridFlags = info.mGridFlags != 0;

  unsigned char* flagsRowStart = &flags[fromY][fromX];

  for (int i = fromY; i <= toY; ++i, curY -= info.mTileSize) {
    int nextYDiv = (i + 1) * CFluidPlaneCPURender::numSubdivisionsInTile;
    int curGridX = startGridX;
    int curXDiv = rippleInfo.mFromX;
    float curYSq = curY * curY;
    float curX = (rip.GetCenter().GetX() - info.mGlobalMinX) -
                 (0.5f * info.mTileSize + static_cast< float >(fromX - 1) * info.mTileSize);
    unsigned char* flagsBase = flagsRowStart;

    for (int j = fromX; j <= toX; ++j, curX -= info.mTileSize, ++curGridX, ++flagsBase) {
      const float dist = fast_sqrt(curX * curX + curYSq);
      if (maxDistR < dist - info.mTileHypRadius || minDistR > dist + info.mTileHypRadius) {
        continue;
      }

      {
        bool addedRipple = false;
        int nextXDiv = (j + 1) * CFluidPlaneCPURender::numSubdivisionsInTile;
        float curXMod = (rip.GetCenter().GetX() - info.mGlobalMinX) -
                        info.mRippleResolution * static_cast< float >(curXDiv);
        float curYMod = (rip.GetCenter().GetY() - info.mGlobalMinY) -
                        info.mRippleResolution * static_cast< float >(curYDiv);

        if (!hasGridFlags || (hasGridFlags && curGridY >= 0 && curGridY < gridCells &&
                              curGridX >= 0 && curGridX < static_cast< int >(info.mGridDimX) &&
                              info.mGridFlags[curGridY + curGridX])) {
          int k = curYDiv;
          for (; k <= rstl::min_val(nextYDiv - 1, rippleInfo.mToY);
               ++k, curYMod -= info.mRippleResolution) {
            float curYModSq = curYMod * curYMod;
            float tmpXMod = curXMod;
            int l = curXDiv;
            for (; l <= rstl::min_val(nextXDiv - 1, rippleInfo.mToX);
                 ++l, tmpXMod -= info.mRippleResolution) {
              const float divDistSq = tmpXMod * tmpXMod + curYModSq;
              if (divDistSq < minDistSq || divDistSq > maxDistSq) {
                continue;
              }
              const float divDist = fast_sqrt(divDistSq);
              int distIdx = static_cast< int >(divDist * distFalloff);
              const u8 rippleV = sRippleValues[lifeIdx][distIdx];
              float height;
              if (rippleV != 0) {
                float phase = divDist * rip.GetLookupPhase() + lookupT;
                int sineIdx = static_cast< int >(phase) & 0xFF;
                height = sineWave[sineIdx] * (static_cast< float >(static_cast< int >(rippleV)) *
                                              rip.GetLookupAmplitude());
              } else {
                height = 0.f;
              }
              heights[k][l].height += height;
              addedRipple = true;
            }
          }

          if (addedRipple) {
            *flagsBase = 0x1f;
          }
        } else {
          int yMax = nextYDiv - CFluidPlaneCPURender::numSubdivisionsInTile + 1;
          int yMin = nextYDiv - 1;
          int xMax = nextXDiv - CFluidPlaneCPURender::numSubdivisionsInTile + 1;
          int xMin = nextXDiv - 1;

          if (curGridX >= 0 && curGridX < static_cast< int >(info.mGridDimX) &&
              curGridY - static_cast< int >(info.mGridDimX) >= 0 &&
              !info.mGridFlags[curGridX + (curGridY - static_cast< int >(info.mGridDimX))]) {
            yMax -= 2;
          }
          if (curGridX >= 0 && curGridX < static_cast< int >(info.mGridDimX) &&
              curGridY + static_cast< int >(info.mGridDimX) < gridCells &&
              !info.mGridFlags[curGridY + (curGridX + static_cast< int >(info.mGridDimX))]) {
            yMin += 2;
          }
          if (curGridY >= 0 && curGridY < static_cast< int >(info.mGridDimY) && curGridX > 0 &&
              !info.mGridFlags[curGridY + curGridX - 1]) {
            xMax -= 2;
          }
          if (curGridY >= 0 && curGridY < static_cast< int >(info.mGridDimY) &&
              curGridX + 1 < static_cast< int >(info.mGridDimX) &&
              !info.mGridFlags[curGridY + curGridX + 1]) {
            xMin += 2;
          }

          int k = curYDiv;
          for (; k <= rstl::min_val(nextYDiv - 1, rippleInfo.mToY);
               ++k, curYMod -= info.mRippleResolution) {
            float curYModSq = curYMod * curYMod;
            float tmpXMod = curXMod;
            int l = curXDiv;
            for (; l <= rstl::min_val(nextXDiv - 1, rippleInfo.mToX);
                 ++l, tmpXMod -= info.mRippleResolution) {
              if (k > yMax && k < yMin && l > xMax && l < xMin) {
                continue;
              }
              const float divDistSq = tmpXMod * tmpXMod + curYModSq;
              if (divDistSq < minDistSq || divDistSq > maxDistSq) {
                continue;
              }
              const float divDist = fast_sqrt(divDistSq);
              int distIdx = static_cast< int >(divDist * distFalloff);
              const u8 rippleV = sRippleValues[lifeIdx][distIdx];
              float height;
              if (rippleV != 0) {
                float phase = divDist * rip.GetLookupPhase() + lookupT;
                int sineIdx = static_cast< int >(phase) & 0xFF;
                height = sineWave[sineIdx] * (static_cast< float >(static_cast< int >(rippleV)) *
                                              rip.GetLookupAmplitude());
              } else {
                height = 0.f;
              }
              heights[k][l].height += height;
              addedRipple = true;
            }
          }

          if (addedRipple) {
            *flagsBase = 0x0f;
          }
        }
        curXDiv = nextXDiv;
      }
    }
    curYDiv = nextYDiv;
    curGridY += static_cast< int >(info.mGridDimX);
    flagsRowStart += 9;
  }
}

void ApplyRipples(const rstl::reserved_vector< CFluidPlaneCPURender::SRippleInfo, 32 >& ripples,
                  CFluidPlaneCPURender::SHFieldSample (&heights)[45][45],
                  unsigned char (&flags)[9][9], const float (&sineWave)[256],
                  CFluidPlaneCPURender::SPatchInfo& info) {
  LCQueueWait(0);

  typedef CFluidPlaneCPURender::SRippleInfo SRippleInfo;
  rstl::reserved_vector< SRippleInfo, 32 >::const_iterator it = ripples.begin();
  for (; it != ripples.end(); ++it) {
    ApplyRipple(*it, heights, flags, sineWave, info);
  }

  unsigned char* ptr;
  int count;

  count = CFluidPlaneCPURender::numTilesInHField;
  ptr = &flags[0][1];
  for (; count != 0; --count) {
    *ptr |= 1;
    ++ptr;
  }
  count = CFluidPlaneCPURender::numTilesInHField;
  ptr = &flags[1][0];
  for (; count != 0; --count) {
    *ptr |= 8;
    ptr += 9;
  }
  count = CFluidPlaneCPURender::numTilesInHField;
  ptr = &flags[1][count + 1];
  for (; count != 0; --count) {
    *ptr |= 4;
    ptr += 9;
  }
  count = CFluidPlaneCPURender::numTilesInHField;
  ptr = &flags[count + 1][1];
  for (; count != 0; --count) {
    *ptr |= 2;
    ++ptr;
  }
}

void RenderPatch(const CFluidPlaneCPURender::SPatchInfo& info, bool noRipples,
                 bool flaggedGridGen) {
  if (noRipples) {
    const char ySubdivs = info.mYSubdivs;
    int xSubdivs = info.mXSubdivs;
    int normalMode = static_cast< int >(static_cast< signed char >(info.mNormalMode));
    const CVector3f localMin(info.mLocalMinX, info.mLocalMinY, 0.f);
    const CVector3f localMax(
        info.mRippleResolution * static_cast< float >(xSubdivs - 2) + info.mLocalMinX,
        info.mRippleResolution * static_cast< float >(ySubdivs - 2) + info.mLocalMinY, 0.f);

    switch (normalMode) {
    case 0:
      CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
      GXPosition3f32(localMin.GetX(), localMin.GetY(), 0.f);
      GXPosition3f32(localMin.GetX(), localMax.GetY(), 0.f);
      GXPosition3f32(localMax.GetX(), localMin.GetY(), 0.f);
      GXPosition3f32(localMax.GetX(), localMax.GetY(), 0.f);
      CGX::End();
      break;
    case 1:
      CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
      GXPosition3f32(localMin.GetX(), localMin.GetY(), 0.f);
      GXColor4u8(0, 0, 0, 0xff);
      GXPosition3f32(localMin.GetX(), localMax.GetY(), 0.f);
      GXColor4u8(0, 0, 0, 0xff);
      GXPosition3f32(localMax.GetX(), localMin.GetY(), 0.f);
      GXColor4u8(0, 0, 0, 0xff);
      GXPosition3f32(localMax.GetX(), localMax.GetY(), 0.f);
      GXColor4u8(0, 0, 0, 0xff);
      CGX::End();
      break;
    case 2: {
      int nextX;
      int numTilesY = (ySubdivs - 3) / CFluidPlaneCPURender::numSubdivisionsInTile + 1;
      int numTilesXM1 = (xSubdivs - 3) / CFluidPlaneCPURender::numSubdivisionsInTile;
      int numTilesXP1 = numTilesXM1 + 1;
      int gridOffset = static_cast< int >(info.mTileY) * static_cast< int >(info.mGridDimX) +
                       static_cast< int >(info.mTileX);
      float curY = localMin.GetY();

      for (int iY = numTilesY; iY > 0; --iY) {
        bool isFirstRow = (numTilesY - iY) == 0;
        bool isLastRow = (1 - iY) == 0;
        float curX = localMin.GetX();
        int iX = 0;

        while (iX < numTilesXP1) {

          if (info.mGridFlags == NULL ||
              (info.mGridFlags != NULL && info.mGridFlags[gridOffset + iX] != 0)) {
            bool isLeftEdge = iX == 0;
            bool isRightEdge = (numTilesXP1 - 1 - iX) == 0;

            if (isFirstRow || isLastRow || isLeftEdge || isRightEdge) {
              int totalVerts = (isLastRow ? CFluidPlaneCPURender::numSubdivisionsInTile : 1) + 2;
              totalVerts += (isRightEdge ? CFluidPlaneCPURender::numSubdivisionsInTile : 1);
              totalVerts += (isFirstRow ? CFluidPlaneCPURender::numSubdivisionsInTile : 1);
              totalVerts += (isLeftEdge ? CFluidPlaneCPURender::numSubdivisionsInTile : 1);

              CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, static_cast< ushort >(totalVerts));

              GXPosition3f32(curX + 0.5f * info.mTileSize, curY + 0.5f * info.mTileSize, 0.f);
              GXNormal3s8(0, 0, 0x3f);
              GXColor4u8(0, 0, 0, 0xff);

              {
                int count = isLastRow ? CFluidPlaneCPURender::numSubdivisionsInTile : 1;
                float edgeX = curX;
                for (int e = count; e > 0; --e) {
                  GXPosition3f32(edgeX, curY + info.mTileSize, 0.f);
                  GXNormal3s8(0, 0, 0x3f);
                  GXColor4u8(0, 0, 0, 0xff);
                  edgeX += info.mRippleResolution;
                }
              }

              {
                int count = isRightEdge ? CFluidPlaneCPURender::numSubdivisionsInTile : 1;
                float edgeY = curY + info.mTileSize;
                for (int e = count; e > 0; --e) {
                  GXPosition3f32(curX + info.mTileSize, edgeY, 0.f);
                  GXNormal3s8(0, 0, 0x3f);
                  GXColor4u8(0, 0, 0, 0xff);
                  edgeY -= info.mRippleResolution;
                }
              }

              {
                int count = isFirstRow ? CFluidPlaneCPURender::numSubdivisionsInTile : 1;
                float edgeX = curX + info.mTileSize;
                for (int e = count; e > 0; --e) {
                  GXPosition3f32(edgeX, curY, 0.f);
                  GXNormal3s8(0, 0, 0x3f);
                  GXColor4u8(0, 0, 0, 0xff);
                  edgeX -= info.mRippleResolution;
                }
              }

              {
                int count = isLeftEdge ? CFluidPlaneCPURender::numSubdivisionsInTile : 1;
                float edgeY = curY;
                for (int e = count; e > 0; --e) {
                  GXPosition3f32(curX, edgeY, 0.f);
                  GXNormal3s8(0, 0, 0x3f);
                  GXColor4u8(0, 0, 0, 0xff);
                  edgeY += info.mRippleResolution;
                }
              }

              GXPosition3f32(curX, curY + info.mTileSize, 0.f);
              GXNormal3s8(0, 0, 0x3f);
              GXColor4u8(0, 0, 0, 0xff);
              CGX::End();
              nextX = iX + 1;
              curX += info.mTileSize;
            } else {
              for (nextX = iX + 1;
                   nextX < numTilesXP1 - 1 &&
                   (info.mGridFlags == NULL ||
                    (info.mGridFlags != NULL && info.mGridFlags[gridOffset + nextX] != 0));
                   ++nextX) {
              }
              int runLen = (nextX - iX) + 1;
              CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, static_cast< ushort >(runLen * 2));
              for (int v = runLen; v > 0; --v) {
                GXPosition3f32(curX, curY, 0.f);
                GXNormal3s8(0, 0, 0x3f);
                GXColor4u8(0, 0, 0, 0xff);
                GXPosition3f32(curX, curY + info.mTileSize, 0.f);
                GXNormal3s8(0, 0, 0x3f);
                GXColor4u8(0, 0, 0, 0xff);
                curX += info.mTileSize;
              }
              CGX::End();
              ++nextX;
              if (nextX == numTilesXP1) {
                --nextX;
                curX -= info.mTileSize;
              }
            }
          } else {
            curX += info.mTileSize;
            for (nextX = iX + 1; nextX < numTilesXP1 && info.mGridFlags[gridOffset + nextX] == 0;
                 ++nextX) {
              curX += info.mTileSize;
            }
          }
          iX = nextX;
        }
        curY += info.mTileSize;
        gridOffset += static_cast< int >(info.mGridDimX);
      }
      break;
    }
    case 3: {
      if (flaggedGridGen || info.mGridFlags == NULL) {
        CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
        GXPosition3f32(localMin.GetX(), localMin.GetY(), 0.f);
        GXNormal3s8(0, 0, 0x3f); // Normal
        GXNormal3s8(0, 0x3f, 0); // Binormal
        GXNormal3s8(0x3f, 0, 0); // Tangent
        GXColor4u8(0, 0, 0, 0xff);
        GXPosition3f32(localMin.GetX(), localMax.GetY(), 0.f);
        GXNormal3s8(0, 0, 0x3f);
        GXNormal3s8(0, 0x3f, 0);
        GXNormal3s8(0x3f, 0, 0);
        GXColor4u8(0, 0, 0, 0xff);
        GXPosition3f32(localMax.GetX(), localMin.GetY(), 0.f);
        GXNormal3s8(0, 0, 0x3f);
        GXNormal3s8(0, 0x3f, 0);
        GXNormal3s8(0x3f, 0, 0);
        GXColor4u8(0, 0, 0, 0xff);
        GXPosition3f32(localMax.GetX(), localMax.GetY(), 0.f);
        GXNormal3s8(0, 0, 0x3f);
        GXNormal3s8(0, 0x3f, 0);
        GXNormal3s8(0x3f, 0, 0);
        GXColor4u8(0, 0, 0, 0xff);
        CGX::End();
      } else {
        int numTilesX = (xSubdivs - 3) / CFluidPlaneCPURender::numSubdivisionsInTile + 1;
        int gridOffset = static_cast< int >(info.mTileX) +
                         static_cast< int >(info.mTileY) * static_cast< int >(info.mGridDimX);
        float curY = localMin.GetY();
        for (int iY = (ySubdivs - 3) / CFluidPlaneCPURender::numSubdivisionsInTile + 1; iY > 0;
             --iY) {
          float curX = localMin.GetX();
          int iX = 0;
          while (iX < numTilesX) {
            int endIX;
            if (info.mGridFlags[gridOffset + iX] != 0) {
              endIX = iX + 1;
              while (endIX < numTilesX && info.mGridFlags[gridOffset + endIX] != 0) {
                ++endIX;
              }
              int runLen = (endIX - iX) + 1;
              CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, static_cast< ushort >(runLen * 2));
              for (int v = runLen; v > 0; --v) {
                GXPosition3f32(curX, curY, 0.f);
                GXNormal3s8(0, 0, 0x3f);
                GXNormal3s8(0, 0x3f, 0);
                GXNormal3s8(0x3f, 0, 0);
                GXColor4u8(0, 0, 0, 0xff);
                GXPosition3f32(curX, curY + info.mTileSize, 0.f);
                GXNormal3s8(0, 0, 0x3f);
                GXNormal3s8(0, 0x3f, 0);
                GXNormal3s8(0x3f, 0, 0);
                GXColor4u8(0, 0, 0, 0xff);
                curX += info.mTileSize;
              }
              CGX::End();
              ++endIX;
            } else {
              curX += info.mTileSize;
              endIX = iX + 1;
              while (endIX < numTilesX && info.mGridFlags[gridOffset + endIX] == 0) {
                curX += info.mTileSize;
                ++endIX;
              }
            }
            iX = endIX;
          }
          curY += info.mTileSize;
          gridOffset += static_cast< int >(info.mGridDimX);
        }
      }
      break;
    }
    default:
      break;
    }
  } else {
    float curY = info.mLocalMinY;
    int startYDiv = 1;
    for (; startYDiv < static_cast< int >(static_cast< signed char >(info.mYSubdivs)) - 2;
         startYDiv += CFluidPlaneCPURender::numSubdivisionsInTile) {
      RenderStripWithRipples(*reinterpret_cast< CFluidPlaneCPURender::SHFieldSample(*)[45][45] >(
                                 static_cast< uchar* >(LCGetBase()) + 0xa0),
                             *reinterpret_cast< const unsigned char (*)[9][9] >(
                                 static_cast< uchar* >(LCGetBase()) + 0x40),
                             startYDiv, curY, info);
      curY += info.mTileSize;
    }
  }
}
