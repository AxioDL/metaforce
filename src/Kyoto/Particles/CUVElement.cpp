#include "Kyoto/Particles/CUVElement.hpp"

#include "Kyoto/Graphics/CTexture.hpp"

#include "rstl/math.hpp"

CUVEConstant::CUVEConstant(TToken< CTexture > tex) : mTex(tex) {}

CUVEConstant::~CUVEConstant() {}

void CUVEConstant::GetValueUV(int frame, SUVElementSet& valOut) const {
  valOut.xMin = 0.f;
  valOut.yMin = 0.f;
  valOut.xMax = 1.f;
  valOut.yMax = 1.f;
}

TLockedToken< CTexture > CUVEConstant::GetValueTexture(int frame) const { return mTex; }

CUVEAnimTexture::CUVEAnimTexture(TToken< CTexture > tex, CIntElement* tileW, CIntElement* tileH,
                                 CIntElement* strideW, CIntElement* strideH,
                                 CIntElement* cycleFrames, const bool loop)
: mTex(tex), mLoop(loop) {
  int result = 0;
  tileW->GetValue(0, result);
  mTileW = result;
  delete tileW;

  tileH->GetValue(0, result);
  mTileH = result;
  delete tileH;

  strideW->GetValue(0, result);
  mStrideW = result;
  delete strideW;

  strideH->GetValue(0, result);
  mStrideH = result;
  delete strideH;

  mCycleFrames = cycleFrames;

  const int width = mTex->GetWidth();
  const int height = mTex->GetHeight();
  const int xTiles = rstl::max_val(1, width / mStrideW);
  const int yTiles = rstl::max_val(1, height / mStrideH);

  mTiles = xTiles * yTiles;
  mUvElems.reserve(xTiles * yTiles);

  int x;
  int y;
  for (y = yTiles - 1; y >= 0; --y) {
    for (x = 0; x < xTiles; ++x) {
      SUVElementSet uvs;
      uvs.xMin = static_cast< float >(mStrideW * x) / static_cast< float >(width);
      uvs.yMin = static_cast< float >(mStrideH * y) / static_cast< float >(height);
      uvs.xMax = static_cast< float >((mStrideW * x) + mTileW) / static_cast< float >(width);
      uvs.yMax = static_cast< float >((mStrideH * y) + mTileH) / static_cast< float >(height);
      mUvElems.push_back(uvs);
    }
  }
}

CUVEAnimTexture::~CUVEAnimTexture() { delete mCycleFrames; }

void CUVEAnimTexture::GetValueUV(int frame, SUVElementSet& valOut) const {
  int cv = 1;
  mCycleFrames->GetValue(frame, cv);
  float cvf =
      static_cast< float >(frame) / (static_cast< float >(cv) / static_cast< float >(mTiles));

  int tile;
  if (mLoop) {
    tile = rstl::max_val(static_cast< int >(cvf), 0);
    if (tile >= mTiles) {
      tile = tile % mTiles;
    }
  } else {
    tile = static_cast< int >(cvf);
    if (static_cast< int >(cvf) >= mTiles) {
      tile = mTiles - 1;
    }
  }

  valOut = mUvElems[tile];
}

TLockedToken< CTexture > CUVEAnimTexture::GetValueTexture(int frame) const { return mTex; }
