#include "Kyoto/Graphics/CGraphics.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphicsSys.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CRelAngle.hpp"

#include "GameVersions.h"
#include "dolphin/types.h"
#include "rstl/math.hpp"

#include "dolphin/vi.h"

#include <string.h>

bool CGraphicsSys::mGraphicsInitialized;
static CStopwatch sFPSTimer;
static uchar sSpareFrameBuffer[640 * 448] ATTRIBUTE_ALIGN(32);

// clang-format off
CTevCombiners::CTevPass CGraphics::kEnvModulateConstColor(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_RasterColor,
    CTevCombiners::kCS_RegisterC0,
    CTevCombiners::kCS_Zero
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_RasterAlpha,
    CTevCombiners::kAS_RegisterA0,
    CTevCombiners::kAS_Zero
  )
);
CTevCombiners::CTevPass CGraphics::kEnvConstColor(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_RegisterC0
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_RegisterA0
  )
);
CTevCombiners::CTevPass CGraphics::kEnvModulate(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_RasterColor,
    CTevCombiners::kCS_TextureColor,
    CTevCombiners::kCS_Zero
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_RasterAlpha,
    CTevCombiners::kAS_TextureAlpha,
    CTevCombiners::kAS_Zero
  )
);
CTevCombiners::CTevPass CGraphics::kEnvDecal(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_RasterColor,
    CTevCombiners::kCS_TextureColor,
    CTevCombiners::kCS_TextureAlpha,
    CTevCombiners::kCS_Zero
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_RasterAlpha
  )
);
CTevCombiners::CTevPass CGraphics::kEnvBlend(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_RasterColor,
    CTevCombiners::kCS_One,
    CTevCombiners::kCS_TextureColor,
    CTevCombiners::kCS_Zero
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_TextureAlpha,
    CTevCombiners::kAS_RasterAlpha,
    CTevCombiners::kAS_Zero
  )
);
CTevCombiners::CTevPass CGraphics::kEnvReplace(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_TextureColor
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_TextureAlpha
  )
);
static CTevCombiners::CTevPass kEnvBlendCTandCConCF(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_RegisterC0,
    CTevCombiners::kCS_TextureColor,
    CTevCombiners::kCS_RasterColor,
    CTevCombiners::kCS_Zero
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_RasterAlpha
  )  
);
CTevCombiners::CTevPass CGraphics::kEnvModulateAlpha(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_RasterColor
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_TextureAlpha,
    CTevCombiners::kAS_RasterAlpha,
    CTevCombiners::kAS_Zero
  )
);
CTevCombiners::CTevPass CGraphics::kEnvModulateColor(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_TextureColor,
    CTevCombiners::kCS_RasterColor,
    CTevCombiners::kCS_Zero
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Konst,
    CTevCombiners::kAS_RasterAlpha,
    CTevCombiners::kAS_Zero
  )
);
CTevCombiners::CTevPass CGraphics::kEnvModulateColorByAlpha(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_PreviousColor,
    CTevCombiners::kCS_PreviousAlpha,
    CTevCombiners::kCS_Zero
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_PreviousAlpha
  )
);
// clang-format on

CGraphics::CRenderState CGraphics::sRenderState;
VecPtr CGraphics::vtxBuffer;
VecPtr CGraphics::nrmBuffer;
Vec2Ptr CGraphics::txtBuffer0;
Vec2Ptr CGraphics::txtBuffer1;
uint* CGraphics::clrBuffer;
bool CGraphics::mJustReset;
ERglCullMode CGraphics::mCullMode;
int CGraphics::mNumLightsActive;
float CGraphics::mDepthNear;
VecPtr CGraphics::mpVtxBuffer;
VecPtr CGraphics::mpNrmBuffer;
Vec2Ptr CGraphics::mpTxtBuffer0;
Vec2Ptr CGraphics::mpTxtBuffer1;
uint* CGraphics::mpClrBuffer;

struct CGXLightParams {
  int x0_;
  int x4_;
  int x8_;
  int xc_;
  int x10_;

  CGXLightParams() : x0_(4), x4_(0), x8_(0), xc_(2), x10_(2) {}
};
CGXLightParams mLightParams[8];

struct {
  Vec vtx;
  Vec nrm;
  Vec2 uv0;
  Vec2 uv1;
  uint color;
  ushort textureUsed;
  uchar streamFlags;
} vtxDescr;

CVector3f CGraphics::kDefaultPositionVector(0.f, 0.f, 0.f);
CVector3f CGraphics::kDefaultDirectionVector(0.f, 1.f, 0.f);
CGraphics::CProjectionState CGraphics::mProj(true, -1.f, 1.f, 1.f, -1.f, 1.f, 100.f);
CTransform4f CGraphics::mViewMatrix = CTransform4f::Identity();
CTransform4f CGraphics::mModelMatrix = CTransform4f::Identity();
CColor CGraphics::mClearColor = CColor::Black();
CVector3f CGraphics::mViewPoint(0.f, 0.f, 0.f);
GXLightObj CGraphics::mLightObj[8];
GXTexRegion CGraphics::mTexRegions[GX_MAX_TEXMAP];
GXTexRegion CGraphics::mTexRegionsCI[GX_MAX_TEXMAP / 2];
GXRenderModeObj CGraphics::mRenderModeObj;
Mtx CGraphics::mGXViewPointMatrix;
Mtx CGraphics::mGXModelMatrix;
Mtx CGraphics::mGxModelView;
Mtx CGraphics::mCameraMtx;

int CGraphics::mNumPrimitives;
int CGraphics::mFrameCounter;
float CGraphics::mFramesPerSecond;
float CGraphics::mLastFramesPerSecond;
int CGraphics::mNumBreakpointsWaiting;
int CGraphics::mFlippingState;
bool CGraphics::mLastFrameUsedAbove;
bool CGraphics::mInterruptLastFrameUsedAbove;
uchar CGraphics::mLightActive;
uchar CGraphics::mLightsWereOn;
void* CGraphics::mpFrameBuf1;
void* CGraphics::mpFrameBuf2;
void* CGraphics::mpCurrenFrameBuf;
int CGraphics::mSpareBufferSize;
void* CGraphics::mpSpareBuffer;
int CGraphics::mSpareBufferTexCacheSize;
GXTexRegionCallback CGraphics::mGXDefaultTexRegionCallback;
void* CGraphics::mpFifo;
GXFifoObj* CGraphics::mpFifoObj;
#if VERSION >= VERSION_GM8J_00
uint CGraphics::mFifoSize = 0;
#endif
uint CGraphics::mRenderTimings;
float CGraphics::mSecondsMod900;
CTimeProvider* CGraphics::mpExternalTimeProvider;
int CGraphics::mScreenStretch;
int CGraphics::mScreenPositionX;
int CGraphics::mScreenPositionY;

CViewport CGraphics::mViewport = {0, 0, 640, 480, 320.f, 240.f};
ELightType CGraphics::mLightTypes[8] = {
    kLT_Directional, kLT_Directional, kLT_Directional, kLT_Directional,
    kLT_Directional, kLT_Directional, kLT_Directional, kLT_Directional,
};

const CTevCombiners::CTevPass& CGraphics::kEnvPassthru = CTevCombiners::kEnvPassthru;
bool CGraphics::mIsBeginSceneClearFb = true;
ERglEnum CGraphics::mDepthFunc = kE_LEqual;
ERglPrimitive CGraphics::mCurrentPrimitive = kP_Points;
float CGraphics::mDepthFar = 1.f;
u32 CGraphics::mClearDepthValue = GX_MAX_Z24;
bool CGraphics::mIsGXModelMatrixIdentity = true;
bool CGraphics::mFirstFrame = true;
bool CGraphics::mUseVideoFilter = true;
float CGraphics::mBrightness = 1.f;

const GXTexMapID CGraphics::kSpareBufferTexMapID = GX_TEXMAP7;
#if VERSION >= VERSION_GM8J_00
void CGraphics::InitGraphicsFifo(GXFifoObj* obj, void* fifo, uint fifoSize) {
  GXFifoObj fifoObj;
  GXInitFifoBase(&fifoObj, fifo, fifoSize);
  GXSetCPUFifo(&fifoObj);
  GXSetGPFifo(&fifoObj);
  GXInitFifoLimits(obj, fifoSize - 0x4000, fifoSize - 0x10000);
  GXSetCPUFifo(obj);
  GXSetGPFifo(obj);
}
#endif

bool CGraphics::Startup(const COsContext& osContext, uint fifoSize, void* fifoBase) {
  mpFifo = fifoBase;
  mpFifoObj = GXInit(fifoBase, fifoSize);
#if VERSION >= VERSION_GM8J_00
  mFifoSize = fifoSize;
  InitGraphicsFifo(mpFifoObj, mpFifo, fifoSize);
#else
  GXFifoObj fifoObj;
  GXInitFifoBase(&fifoObj, mpFifo, fifoSize);
  GXSetCPUFifo(&fifoObj);
  GXSetGPFifo(&fifoObj);
  GXInitFifoLimits(mpFifoObj, fifoSize - 0x4000, fifoSize - 0x10000);
  GXSetCPUFifo(mpFifoObj);
  GXSetGPFifo(mpFifoObj);
  GXSetMisc(GX_MT_XF_FLUSH, 8);
  GXSetDither(GX_FALSE);
#endif

  CGX::ResetGXStates();
  InitGraphicsVariables();
  ConfigureFrameBuffer(osContext);
  for (int i = 0; i < ARRAY_SIZE(mTexRegions); i++) {
    GXInitTexCacheRegion(&mTexRegions[i], false, 0x8000 * i, GX_TEXCACHE_32K,
                         0x80000 + (0x8000 * i), GX_TEXCACHE_32K);
  }
  for (int i = 0; i < ARRAY_SIZE(mTexRegionsCI); i++) {
    GXInitTexCacheRegion(&mTexRegionsCI[i], false, (8 + (2 * i)) << 0xF, GX_TEXCACHE_32K,
                         (9 + (2 * i)) << 0xF, GX_TEXCACHE_32K);
  }
  mGXDefaultTexRegionCallback = GXSetTexRegionCallback(TexRegionCallback);
  mSpareBufferSize = ARRAY_SIZE(sSpareFrameBuffer);
  mpSpareBuffer = sSpareFrameBuffer;
  mSpareBufferTexCacheSize = 0x10000;
  return true;
}

GXTexRegion* CGraphics::TexRegionCallback(const GXTexObj* obj, GXTexMapID id) {
  static int nextTexRgn = 0;
  static int nextTexRgnCI = 0;
  if (id == GX_TEXMAP7) {
    return &mTexRegions[0];
  } else {
    GXTexFmt fmt = GXGetTexObjFmt(obj);
    if (fmt != GX_TF_C4 && fmt != GX_TF_C8 && fmt != GX_TF_C14X2) {
      if (nextTexRgn == 0) {
        ++nextTexRgn;
      }
      return &mTexRegions[nextTexRgn++ & 7];
    } else {
      return &mTexRegionsCI[nextTexRgnCI++ & 3];
    }
  }
}

void CGraphics::InitGraphicsVariables() {
  for (int i = 0; i < ARRAY_SIZE(mLightTypes); ++i) {
    mLightTypes[i] = kLT_Directional;
  }
  mLightActive = 0;
  SetDepthWriteMode(false, mDepthFunc, false);
  SetCullMode(kCM_None);
  SetAmbientColor(CColor(0.2f, 0.2f, 0.2f, 1.f));
  mIsGXModelMatrixIdentity = false;
  SetIdentityViewPointMatrix();
  SetIdentityModelMatrix();
  SetViewport(0, 0, mViewport.mWidth, mViewport.mHeight);
  SetPerspective(60.f,
                 static_cast< float >(mViewport.mWidth) / static_cast< float >(mViewport.mHeight),
                 mProj.GetNear(), mProj.GetFar());
  SetCopyClear(mClearColor, 1.f);
  const GXColor white = {0xFF, 0xFF, 0xFF, 0xFF};
  CGX::SetChanMatColor(CGX::Channel0, white);
  sRenderState.ResetFlushAll();
}

void CGraphics::Shutdown() { GXSetTexRegionCallback(mGXDefaultTexRegionCallback); }

void CGraphics::InitGraphicsDefaults() {
  SetDepthRange(0.f, 1.f);
  mIsGXModelMatrixIdentity = false;
  SetModelMatrix(mModelMatrix);
  SetViewPointMatrix(mViewMatrix);
  SetDepthWriteMode(false, mDepthFunc, false);
  SetCullMode(mCullMode);
  SetViewport(mViewport.mLeft, mViewport.mTop, mViewport.mWidth, mViewport.mHeight);
  FlushProjection();
  CTevCombiners::Init();
  DisableAllLights();
  SetDefaultVtxAttrFmt();
}

void CGraphics::ConfigureFrameBuffer(const COsContext& osContext) {
  mRenderModeObj = osContext.GetRenderModeObj();
  mpFrameBuf1 = osContext.GetFramebuf1();
  mpFrameBuf2 = osContext.GetFramebuf2();
  VIConfigure(&mRenderModeObj);
  VISetNextFrameBuffer(mpFrameBuf1);
  mpCurrenFrameBuf = mpFrameBuf2;
  GXSetViewport(0.f, 0.f, static_cast< float >(mRenderModeObj.fbWidth),
                static_cast< float >(mRenderModeObj.efbHeight), 0.f, 1.f);
  GXSetScissor(0, 0, mRenderModeObj.fbWidth, mRenderModeObj.efbHeight);
  GXSetDispCopySrc(0, 0, mRenderModeObj.fbWidth, mRenderModeObj.efbHeight);
  GXSetDispCopyDst(mRenderModeObj.fbWidth, mRenderModeObj.efbHeight);
  GXSetDispCopyYScale(static_cast< float >(mRenderModeObj.xfbHeight) /
                      static_cast< float >(mRenderModeObj.efbHeight));
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, GX_ENABLE,
                  mRenderModeObj.vfilter);
  if (mRenderModeObj.aa) {
    GXSetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
  } else {
    GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  }
  GXSetDispCopyGamma(GX_GM_1_0);
  GXCopyDisp(mpCurrenFrameBuf, true);
  VIFlush();
  VIWaitForRetrace();
  VIWaitForRetrace();
  mViewport.mWidth = mRenderModeObj.fbWidth;
  mViewport.mHeight = mRenderModeObj.efbHeight;
  InitGraphicsDefaults();
}

void CGraphics::EnableLight(ERglLight light) {
  CGX::SetNumChans(1);
  int lightsWereOn = mLightActive;
  GXLightID lightId = static_cast< GXLightID >(1 << light);
  if ((lightsWereOn & lightId) == GX_LIGHT_NULL) {
    mLightActive |= lightId;
    CGX::SetChanCtrl(CGX::Channel0, true, GX_SRC_REG, GX_SRC_REG,
                     static_cast< GXLightID >((lightsWereOn | lightId) & (GX_MAX_LIGHT - 1)),
                     GX_DF_CLAMP, GX_AF_SPOT);
    ++mNumLightsActive;
  }
  mLightsWereOn = mLightActive;
}

static inline GXLightID get_hw_light_index(ERglLight light) {
#if NONMATCHING
  // one instruction, no branches
  return static_cast< GXLightID >((light << 1) & (GX_MAX_LIGHT - 1));
#else
  if (light == kLight0) {
    return GX_LIGHT0;
  } else if (light == kLight1) {
    return GX_LIGHT1;
  } else if (light == kLight2) {
    return GX_LIGHT2;
  } else if (light == kLight3) {
    return GX_LIGHT3;
  } else if (light == kLight4) {
    return GX_LIGHT4;
  } else if (light == kLight5) {
    return GX_LIGHT5;
  } else if (light == kLight6) {
    return GX_LIGHT6;
  }
  // wtf?
  return static_cast< GXLightID >(light == kLight7 ? GX_LIGHT7 : 0);
#endif
}

void CGraphics::LoadLight(ERglLight light, const CLight& info) {
  GXLightID lightId = get_hw_light_index(light);
  ELightType type = info.GetType();
  CVector3f pos = info.GetPosition();
  CVector3f dir = info.GetDirection();

  switch (type) {
  case kLT_Spot: {
    MTXMultVec(mCameraMtx, reinterpret_cast< VecPtr >(&pos), reinterpret_cast< VecPtr >(&pos));
    GXLightObj* obj = &mLightObj[light];
    GXInitLightPos(obj, pos.GetX(), pos.GetY(), pos.GetZ());
    MTXMultVecSR(mCameraMtx, reinterpret_cast< VecPtr >(&dir), reinterpret_cast< VecPtr >(&dir));
    GXInitLightDir(obj, dir.GetX(), dir.GetY(), dir.GetZ());
    GXInitLightAttn(obj, 1.f, 0.f, 0.f, info.GetAttenuationConstant(), info.GetAttenuationLinear(),
                    info.GetAttenuationQuadratic());
    GXInitLightSpot(obj, info.GetSpotCutoff(), GX_SP_COS2);
    break;
  }
  case kLT_Point:
  case kLT_LocalAmbient: {
    MTXMultVec(mCameraMtx, reinterpret_cast< VecPtr >(&pos), reinterpret_cast< VecPtr >(&pos));
    GXInitLightPos(&mLightObj[light], pos.GetX(), pos.GetY(), pos.GetZ());
    GXInitLightAttn(&mLightObj[light], 1.f, 0.f, 0.f, info.GetAttenuationConstant(),
                    info.GetAttenuationLinear(), info.GetAttenuationQuadratic());
    break;
  }
  case kLT_Directional: {
    MTXMultVecSR(mCameraMtx, reinterpret_cast< VecPtr >(&dir), reinterpret_cast< VecPtr >(&dir));
    dir = -dir;
    GXInitLightPos(&mLightObj[light], dir.GetX() * 1048576.f, dir.GetY() * 1048576.f,
                   dir.GetZ() * 1048576.f);
    GXInitLightAttn(&mLightObj[light], 1.f, 0.f, 0.f, 1.f, 0.f, 0.f);
    break;
  }
  case kLT_Custom: {
    MTXMultVec(mCameraMtx, reinterpret_cast< VecPtr >(&pos), reinterpret_cast< VecPtr >(&pos));
    GXLightObj* obj = &mLightObj[light];
    GXInitLightPos(obj, pos.GetX(), pos.GetY(), pos.GetZ());
    MTXMultVecSR(mCameraMtx, reinterpret_cast< VecPtr >(&dir), reinterpret_cast< VecPtr >(&dir));
    GXInitLightDir(obj, dir.GetX(), dir.GetY(), dir.GetZ());
    GXInitLightAttn(obj, info.GetAngleAttenuationConstant(), info.GetAngleAttenuationLinear(),
                    info.GetAngleAttenuationQuadratic(), info.GetAttenuationConstant(),
                    info.GetAttenuationLinear(), info.GetAttenuationQuadratic());
    break;
  }
  default:
    break;
  }

  GXInitLightColor(&mLightObj[light], info.GetColor().GetGXColor());
  GXLoadLightObjImm(&mLightObj[light], lightId);
  mLightTypes[light] = info.GetType();
}

void CGraphics::DisableAllLights() {
  mNumLightsActive = 0;
  mLightActive = 0;
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
}

// https://en.wikipedia.org/wiki/Hamming_weight
static inline uint popcount8(uint b) {
  b = (b & 0x55) + ((b & 0xAA) >> 1);
  b = (b & 0x33) + ((b & 0xCC) >> 2);
  return (static_cast< uchar >(b) & 0xF) + (static_cast< uchar >(b) >> 4);
}

void CGraphics::SetLightState(uchar lights) {
  GXAttnFn attnFn = GX_AF_NONE;
  if (lights != 0) {
    attnFn = GX_AF_SPOT;
  }
  GXDiffuseFn diffFn = GX_DF_NONE;
  if (lights != 0) {
    diffFn = GX_DF_CLAMP;
  }
  CGX::SetChanCtrl(CGX::Channel0, lights != 0 ? GX_ENABLE : GX_DISABLE, GX_SRC_REG,
                   (vtxDescr.streamFlags & 2) != 0 ? GX_SRC_VTX : GX_SRC_REG,
                   static_cast< GXLightID >(lights), diffFn, attnFn);
  mLightActive = lights;
  mNumLightsActive = popcount8(lights);
}

void CGraphics::SetViewMatrix() {
  Mtx mtx;
  MTXTrans(mtx, -mViewPoint.GetX(), -mViewPoint.GetY(), -mViewPoint.GetZ());
  MTXConcat(mGXViewPointMatrix, mtx, mCameraMtx);
  if (mIsGXModelMatrixIdentity) {
    MTXCopy(mCameraMtx, mGxModelView);
  } else {
    MTXConcat(mCameraMtx, mGXModelMatrix, mGxModelView);
  }
  GXLoadPosMtxImm(mGxModelView, GX_PNMTX0);

  Mtx nrmMtx;
  MTXInvXpose(mGxModelView, nrmMtx);
  GXLoadNrmMtxImm(nrmMtx, GX_PNMTX0);
}

void CGraphics::SetViewPointMatrix(const CTransform4f& xf) {
  mViewMatrix = xf;
  mGXViewPointMatrix[0][0] = xf.Get00();
  mGXViewPointMatrix[0][1] = xf.Get10();
  mGXViewPointMatrix[0][2] = xf.Get20();
  mGXViewPointMatrix[0][3] = 0.f;
  mGXViewPointMatrix[1][0] = xf.Get02();
  mGXViewPointMatrix[1][1] = xf.Get12();
  mGXViewPointMatrix[1][2] = xf.Get22();
  mGXViewPointMatrix[1][3] = 0.f;
  mGXViewPointMatrix[2][0] = -xf.Get01();
  mGXViewPointMatrix[2][1] = -xf.Get11();
  mGXViewPointMatrix[2][2] = -xf.Get21();
  mGXViewPointMatrix[2][3] = 0.f;
  mViewPoint = xf.GetTranslation();
  SetViewMatrix();
}

void CGraphics::SetIdentityViewPointMatrix() {
  mViewMatrix = CTransform4f::Identity();
  MTXIdentity(mGXViewPointMatrix);
  mGXViewPointMatrix[2][2] = 0.f;
  mGXViewPointMatrix[1][1] = 0.f;
  mGXViewPointMatrix[1][2] = 1.f;
  mGXViewPointMatrix[2][1] = -1.f;
  mViewPoint = CVector3f::Zero();
  SetViewMatrix();
}

void CGraphics::SetModelMatrix(const CTransform4f& xf) {
  if (&xf == &CTransform4f::Identity()) {
    if (!mIsGXModelMatrixIdentity) {
      mModelMatrix = xf;
      mIsGXModelMatrixIdentity = true;
      SetViewMatrix();
    }
    return;
  }

  mModelMatrix = xf;
  mIsGXModelMatrixIdentity = false;
  mGXModelMatrix[0][0] = xf.Get00();
  mGXModelMatrix[0][1] = xf.Get01();
  mGXModelMatrix[0][2] = xf.Get02();
  mGXModelMatrix[0][3] = xf.Get03();
  mGXModelMatrix[1][0] = xf.Get10();
  mGXModelMatrix[1][1] = xf.Get11();
  mGXModelMatrix[1][2] = xf.Get12();
  mGXModelMatrix[1][3] = xf.Get13();
  mGXModelMatrix[2][0] = xf.Get20();
  mGXModelMatrix[2][1] = xf.Get21();
  mGXModelMatrix[2][2] = xf.Get22();
  mGXModelMatrix[2][3] = xf.Get23();
  SetViewMatrix();
}

void CGraphics::SetIdentityModelMatrix() {
  if (!mIsGXModelMatrixIdentity) {
    mModelMatrix = CTransform4f::Identity();
    mIsGXModelMatrixIdentity = true;
    SetViewMatrix();
  }
}

void CGraphics::SetOrtho(float left, float right, float top, float bottom, float znear,
                         float zfar) {
  mProj = CProjectionState(false, left, right, top, bottom, znear, zfar);
  FlushProjection();
}

void CGraphics::SetPerspective(float fovy, float aspect, float znear, float zfar) {
  float t = tan(CRelAngle::FromDegrees(fovy).AsRadians() / 2.f);
  mProj = CProjectionState(true,                               // Is Projection
                           -(aspect * 2.f * znear * t * 0.5f), // Left
                           (aspect * 2.f * znear * t * 0.5f),  // Right
                           (znear * 2.f * t * 0.5f),           // Top
                           -(znear * 2.f * t * 0.5f),          // Bottom
                           znear, zfar);
  FlushProjection();
}

CMatrix4f CGraphics::GetPerspectiveProjectionMatrix() {
#if NONMATCHING
  // construct in place
  return CMatrix4f(
#else
  CMatrix4f mtx(
#endif
      // clang-format off
    (mProj.GetNear() * 2.f) / (mProj.GetRight() - mProj.GetLeft()),
    -(mProj.GetRight() + mProj.GetLeft()) / (mProj.GetRight() - mProj.GetLeft()),
    0.f,
    0.f,
    0.f,
    -(mProj.GetTop() + mProj.GetBottom()) / (mProj.GetTop() - mProj.GetBottom()),
    (mProj.GetNear() * 2.f) / (mProj.GetTop() - mProj.GetBottom()),
    0.f,
    0.f,
    (mProj.GetFar() + mProj.GetNear()) / (mProj.GetFar() - mProj.GetNear()),
    0.f,
    -(mProj.GetFar() * 2.f * mProj.GetNear()) / (mProj.GetFar() - mProj.GetNear()),
    0.f,
    1.f,
    0.f,
    0.f
      // clang-format on
  );
#if !NONMATCHING
  return mtx;
#endif
}

CMatrix4f CGraphics::CalculatePerspectiveMatrix(float fovy, float aspect, float znear, float zfar) {
  float t = tan(CRelAngle::FromDegrees(fovy).AsRadians() / 2.f);
  float right = aspect * 2.f * znear * t * 0.5f;
  float left = -right;
  float top = znear * 2.f * t * 0.5f;
  float bottom = -top;
#if NONMATCHING
  // construct in place
  return CMatrix4f(
#else
  CMatrix4f mtx(
#endif
      // clang-format off
    (2.f * znear) / (right - left),
    -(right + left) / (right - left),
    0.f,
    0.f,
    0.f,
    -(top + bottom) / (top - bottom),
    (2.f * znear) / (top - bottom),
    0.f,
    0.f,
    (zfar + znear) / (zfar - znear),
    0.f,
    -(2.f * zfar * znear) / (zfar - znear),
    0.f,
    1.f,
    0.f,
    0.f
      // clang-format on
  );
#if !NONMATCHING
  return mtx;
#endif
}

void CGraphics::SetViewport(int left, int bottom, int width, int height) {
  mViewport.mLeft = left;
  mViewport.mTop = mRenderModeObj.efbHeight - (bottom + height);
  mViewport.mWidth = width;
  mViewport.mHeight = height;
  mViewport.mHalfWidth = static_cast< float >(width / 2);
  mViewport.mHalfHeight = static_cast< float >(height / 2);
  GXSetViewport(static_cast< float >(mViewport.mLeft), static_cast< float >(mViewport.mTop),
                static_cast< float >(mViewport.mWidth), static_cast< float >(mViewport.mHeight),
                mDepthNear, mDepthFar);
}

void CGraphics::SetScissor(int left, int bottom, int width, int height) {
  GXSetScissor(left, mRenderModeObj.efbHeight - (bottom + height), width, height);
}

void CGraphics::SetAmbientColor(const CColor& color) {
  CGX::SetChanAmbColor(CGX::Channel0, color.GetGXColor());
  CGX::SetChanAmbColor(CGX::Channel1, color.GetGXColor());
}

void CGraphics::SetCopyClear(const CColor& color, float depth) {
  mClearColor = color;
  mClearDepthValue = static_cast< u32 >(depth * GX_MAX_Z24);
  GXSetCopyClear(color.GetGXColor(), mClearDepthValue);
}

void CGraphics::SetClearColor(const CColor& color) {
  mClearColor = color;
  GXSetCopyClear(mClearColor.GetGXColor(), mClearDepthValue);
}

void CGraphics::ClearBackAndDepthBuffers() {
  GXInvalidateTexAll();
  if (mRenderModeObj.field_rendering) {
    GXSetViewportJitter(0.f, 0.f, static_cast< float >(mRenderModeObj.fbWidth),
                        static_cast< float >(mRenderModeObj.xfbHeight), 0.f, 1.f, VIGetNextField());
  } else {
    GXSetViewport(0.f, 0.f, static_cast< float >(mRenderModeObj.fbWidth),
                  static_cast< float >(mRenderModeObj.xfbHeight), 0.f, 1.f);
  }
  GXInvalidateVtxCache();
}

void CGraphics::BeginScene() { ClearBackAndDepthBuffers(); }

void CGraphics::SwapBuffers() {
  GXDisableBreakPt();
  mFlippingState = 1;
}

void CGraphics::VideoPreCallback(u32 retraceCount) {
  if (mNumBreakpointsWaiting != 0 && mFlippingState == 1) {
    if (mFirstFrame) {
      VISetBlack(GX_FALSE);
      mFirstFrame = false;
    }
    VISetNextFrameBuffer(mpCurrenFrameBuf);
    VIFlush();
    void* frameBuf = mpCurrenFrameBuf == mpFrameBuf1 ? mpFrameBuf2 : mpFrameBuf1;
    mFlippingState = 2;
    mpCurrenFrameBuf = frameBuf;
  }
}

void CGraphics::VideoPostCallback(u32 retraceCount) {
  if (mNumBreakpointsWaiting != 0) {
    if (mFlippingState == 2) {
      --mNumBreakpointsWaiting;
      mFlippingState = 0;
      CStopwatch& timer = sFPSTimer;
      float elapsed = timer.GetElapsedTime();
      mLastFramesPerSecond = mFramesPerSecond;
      mFramesPerSecond = 1.f / elapsed;
      timer.Reset();
      mInterruptLastFrameUsedAbove = VIGetNextField() == 1;
    }
  }
}

void CGraphics::EndScene() {
  CGX::SetZMode(true, GX_LEQUAL, true);
  volatile int& numBreakPt = const_cast< volatile int& >(mNumBreakpointsWaiting);
  while (numBreakPt > 0) {
    OSYieldThread();
  }
  ++mNumBreakpointsWaiting;
  void*& frameBuf = mpCurrenFrameBuf;
  float brightness = CMath::Clamp(0.f, mBrightness, 2.f);
  static const u8 copyFilter[7] = {0x00, 0x00, 0x15, 0x16, 0x15, 0x00, 0x00};
  const u8* inFilter = mUseVideoFilter ? mRenderModeObj.vfilter : copyFilter;
  u8 vfilter[7];
  for (int i = 0; i < 7; i++) {
    vfilter[i] = static_cast< u8 >(static_cast< float >(inFilter[i]) * brightness);
  }
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, true, vfilter);
  GXCopyDisp(frameBuf, mIsBeginSceneClearFb ? GX_TRUE : GX_FALSE);
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern,
                  mUseVideoFilter ? GX_ENABLE : GX_DISABLE, mRenderModeObj.vfilter);
  GXSetBreakPtCallback(SwapBuffers);
  VISetPreRetraceCallback(VideoPreCallback);
  VISetPostRetraceCallback(VideoPostCallback);
  GXFlush();
  GXFifoObj* fifo = GXGetGPFifo();
  void* readPtr;
  void* writePtr;
  GXGetFifoPtrs(fifo, &readPtr, &writePtr);
  GXEnableBreakPt(writePtr);
  mLastFrameUsedAbove = mInterruptLastFrameUsedAbove;
  ++mFrameCounter;
  CFrameDelayedKiller::FlushAllocationsForFrame();
}

void CGraphics::SetDepthWriteMode(const bool test, ERglEnum comp, const bool write) {
  mDepthFunc = comp;
  CGX::SetZMode(test, static_cast< GXCompare >(comp), write);
}

void CGraphics::SetCullMode(ERglCullMode cullMode) {
  mCullMode = cullMode;
  GXSetCullMode(static_cast< GXCullMode >(cullMode));
}

void CGraphics::SetBlendMode(ERglBlendMode mode, ERglBlendFactor src, ERglBlendFactor dst,
                             ERglLogicOp op) {
  CGX::SetBlendMode(static_cast< GXBlendMode >(mode), static_cast< GXBlendFactor >(src),
                    static_cast< GXBlendFactor >(dst), static_cast< GXLogicOp >(op));
}

void CGraphics::SetAlphaCompare(ERglAlphaFunc comp0, uchar ref0, ERglAlphaOp op,
                                ERglAlphaFunc comp1, uchar ref1) {
  CGX::SetAlphaCompare(static_cast< GXCompare >(comp0), static_cast< uchar >(ref0),
                       static_cast< GXAlphaOp >(op), static_cast< GXCompare >(comp1),
                       static_cast< uchar >(ref1));
}

static const GXVtxDescList skPosColorTexDirect[] = {
    {GX_VA_POS, GX_DIRECT},
    {GX_VA_CLR0, GX_DIRECT},
    {GX_VA_TEX0, GX_DIRECT},
    {GX_VA_NULL, GX_NONE},
};

void CGraphics::Render2D(const CTexture& tex, int x, int y, int w, int h, const CColor& col) {
  Mtx44 proj;
  MTXOrtho(proj, mViewport.mHeight / 2, -mViewport.mHeight / 2, -mViewport.mWidth / 2,
           mViewport.mWidth / 2, -1.f, -10.f);
  GXSetProjection(proj, GX_ORTHOGRAPHIC);
  uint c = col.GetColor_u32();

  Mtx mtx;
  MTXIdentity(mtx);
  GXLoadPosMtxImm(mtx, GX_PNMTX0);

  float x2, y2, x1, y1;
  x1 = x - mViewport.mWidth / 2;
  y1 = y - mViewport.mHeight / 2;
  x2 = x1 + w;
  y2 = y1 + h;

  // Save state + setup
  CGX::SetVtxDescv(skPosColorTexDirect);
  SetTevStates(6);
  mLightsWereOn = mLightActive;
  if (mLightActive != 0) {
    DisableAllLights();
  }
  ERglCullMode cullMode = mCullMode;
  SetCullMode(kCM_None);
  tex.Load(GX_TEXMAP0, CTexture::kCM_Repeat);

  // Draw
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition3f32(x1, y1, 1.f);
  GXColor1u32(c);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(x2, y1, 1.f);
  GXColor1u32(c);
  GXTexCoord2f32(1.f, 0.f);
  GXPosition3f32(x1, y2, 1.f);
  GXColor1u32(c);
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(x2, y2, 1.f);
  GXColor1u32(c);
  GXTexCoord2f32(1.f, 1.f);
  CGX::End();

  // Restore state
  if (mLightsWereOn != 0) {
    SetLightState(mLightsWereOn);
  }
  FlushProjection();
  mIsGXModelMatrixIdentity = false;
  SetModelMatrix(mModelMatrix);
  SetCullMode(cullMode);
}

void CGraphics::DrawPrimitive(ERglPrimitive primitive, const float* pos, const CVector3f& normal,
                              const CColor& col, int numVerts) {
  StreamBegin(primitive);
  StreamNormal(reinterpret_cast< const float* >(&normal));
  StreamColor(col);
  for (int i = 0; i < numVerts; ++i) {
    StreamVertex(pos + i * 3);
  }
  StreamEnd();
}

#define STREAM_PRIM_BUFFER_SIZE 240

#define VTX_BUFFER_ADDR static_cast< uchar* >(LCGetBase())
#if NONMATCHING
// Bug fix: these should be 3 times larger to avoid overflow
// Likely the result of bad pointer arithmetic
#define NRM_BUFFER_ADDR (VTX_BUFFER_ADDR + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(Vec)))
#define TXT0_BUFFER_ADDR (NRM_BUFFER_ADDR + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(Vec)))
#else
#define NRM_BUFFER_ADDR (VTX_BUFFER_ADDR + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(float)))
#define TXT0_BUFFER_ADDR (NRM_BUFFER_ADDR + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(float)))
#endif
#define TXT1_BUFFER_ADDR (TXT0_BUFFER_ADDR + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(Vec2)))
#define CLR_BUFFER_ADDR (TXT1_BUFFER_ADDR + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(Vec2)))

static const uchar kHasNormals = 1;
static const uchar kHasColor = 2;
static const uchar kHasTexture = 4;

void CGraphics::StreamBegin(ERglPrimitive primitive) {
  vtxBuffer = reinterpret_cast< VecPtr >(VTX_BUFFER_ADDR);
  nrmBuffer = reinterpret_cast< VecPtr >(NRM_BUFFER_ADDR);
  txtBuffer0 = reinterpret_cast< Vec2Ptr >(TXT0_BUFFER_ADDR);
  txtBuffer1 = reinterpret_cast< Vec2Ptr >(TXT1_BUFFER_ADDR);
  clrBuffer = reinterpret_cast< uint* >(CLR_BUFFER_ADDR);
  ResetVertexDataStream(true);
  mCurrentPrimitive = primitive;
  vtxDescr.streamFlags = kHasColor;
}

void CGraphics::StreamVertex(float x, float y, float z) {
  vtxDescr.vtx.x = x;
  vtxDescr.vtx.y = y;
  vtxDescr.vtx.z = z;
  UpdateVertexDataStream();
}

void CGraphics::StreamVertex(const float* vtx) {
  vtxDescr.vtx.x = vtx[0];
  vtxDescr.vtx.y = vtx[1];
  vtxDescr.vtx.z = vtx[2];
  UpdateVertexDataStream();
}

void CGraphics::StreamVertex(const CVector3f& vtx) {
  vtxDescr.vtx.x = vtx.GetX();
  vtxDescr.vtx.y = vtx.GetY();
  vtxDescr.vtx.z = vtx.GetZ();
  UpdateVertexDataStream();
}

void CGraphics::StreamNormal(const float* nrm) {
  vtxDescr.nrm.x = nrm[0];
  vtxDescr.nrm.y = nrm[1];
  vtxDescr.nrm.z = nrm[2];
  vtxDescr.streamFlags |= kHasNormals;
}

void CGraphics::StreamColor(uint color) {
  vtxDescr.color = color;
  vtxDescr.streamFlags |= kHasColor;
}

void CGraphics::StreamColor(const CColor& color) {
  vtxDescr.color = color.GetColor_u32();
  vtxDescr.streamFlags |= kHasColor;
}

void CGraphics::StreamColor(float r, float g, float b, float a) {
  // clang-format off
  vtxDescr.color = (static_cast< uchar >(r * 255.f) << 24) |
                   (static_cast< uchar >(g * 255.f) << 16) |
                   (static_cast< uchar >(b * 255.f) << 8) |
                   static_cast< uchar >(a * 255.f);
  // clang-format on
  vtxDescr.streamFlags |= kHasColor;
}

void CGraphics::StreamTexcoord(const CVector2f& uv) {
  vtxDescr.uv0.x = uv.GetX();
  vtxDescr.uv0.y = uv.GetY();
  vtxDescr.streamFlags |= kHasTexture;
  vtxDescr.textureUsed |= 1;
}

void CGraphics::StreamTexcoord(float u, float v) {
  vtxDescr.uv0.x = u;
  vtxDescr.uv0.y = v;
  vtxDescr.streamFlags |= kHasTexture;
  vtxDescr.textureUsed |= 1;
}

void CGraphics::StreamEnd() {
  if (mNumPrimitives != 0) {
    FlushStream();
  }
  vtxBuffer = nullptr;
  vtxDescr.streamFlags = 0;
  vtxDescr.textureUsed = 0;
  nrmBuffer = nullptr;
  txtBuffer0 = nullptr;
  txtBuffer1 = nullptr;
  clrBuffer = nullptr;
}

void CGraphics::SetLineWidth(float w, ERglTexOffset offs) {
  CGX::SetLineWidth(static_cast< uchar >(w * 6.f), static_cast< GXTexOffset >(offs));
}

void CGraphics::UpdateVertexDataStream() {
  ++mNumPrimitives;
  mpVtxBuffer->x = vtxDescr.vtx.x;
  mpVtxBuffer->y = vtxDescr.vtx.y;
  mpVtxBuffer->z = vtxDescr.vtx.z;
  ++mpVtxBuffer;
  if ((vtxDescr.streamFlags & kHasNormals) != 0) {
    mpNrmBuffer->x = vtxDescr.nrm.x;
    mpNrmBuffer->y = vtxDescr.nrm.y;
    mpNrmBuffer->z = vtxDescr.nrm.z;
    ++mpNrmBuffer;
  }
  if ((vtxDescr.streamFlags & kHasTexture) != 0) {
    mpTxtBuffer0->x = vtxDescr.uv0.x;
    mpTxtBuffer0->y = vtxDescr.uv0.y;
    ++mpTxtBuffer0;

    mpTxtBuffer1->x = vtxDescr.uv1.x;
    mpTxtBuffer1->y = vtxDescr.uv1.y;
    ++mpTxtBuffer1;
  }
  if ((vtxDescr.streamFlags & kHasColor) != 0) {
    *mpClrBuffer = vtxDescr.color;
    ++mpClrBuffer;
  }
  mJustReset = 0;
  if (mNumPrimitives == STREAM_PRIM_BUFFER_SIZE) {
    FlushStream();
    ResetVertexDataStream(false);
  }
}

void CGraphics::ResetVertexDataStream(bool initial) {
  mpVtxBuffer = vtxBuffer;
  mpNrmBuffer = nrmBuffer;
  mpTxtBuffer0 = txtBuffer0;
  mpTxtBuffer1 = txtBuffer1;
  mpClrBuffer = clrBuffer;
  mNumPrimitives = 0;

  if (initial) {
    return;
  }

  switch (mCurrentPrimitive) {
  case kP_TriangleFan:
    mpVtxBuffer = vtxBuffer + 1;
    memcpy(mpVtxBuffer, &vtxDescr.vtx, sizeof(Vec));
    ++mpVtxBuffer;

    if ((vtxDescr.streamFlags & kHasNormals) != 0) {
      ++mpNrmBuffer;
      memcpy(mpNrmBuffer, &vtxDescr.nrm, sizeof(Vec));
      ++mpNrmBuffer;
    }

    if ((vtxDescr.streamFlags & kHasTexture) != 0) {
      ++mpTxtBuffer0;
      memcpy(mpTxtBuffer0, &vtxDescr.uv0, sizeof(Vec2));
      ++mpTxtBuffer0;

      ++mpTxtBuffer1;
      memcpy(mpTxtBuffer1, &vtxDescr.uv1, sizeof(Vec2));
      ++mpTxtBuffer1;
    }

    if ((vtxDescr.streamFlags & kHasColor) != 0) {
      ++mpClrBuffer;
      *mpClrBuffer = vtxDescr.color;
      ++mpClrBuffer;
    }

    mNumPrimitives += 2;
    break;

  default:
    break;
  }

  mJustReset = 1;
}

void CGraphics::FlushStream() {
  GXVtxDescList vtxDesc[10];

  GXVtxDescList* curDesc = vtxDesc;
  const GXVtxDescList vtxDescPos = {GX_VA_POS, GX_DIRECT};
  *curDesc++ = vtxDescPos;

  if ((vtxDescr.streamFlags & kHasNormals) != 0) {
    const GXVtxDescList vtxDescNrm = {GX_VA_NRM, GX_DIRECT};
    *curDesc++ = vtxDescNrm;
  }

  if ((vtxDescr.streamFlags & kHasColor) != 0) {
    const GXVtxDescList vtxDescClr0 = {GX_VA_CLR0, GX_DIRECT};
    *curDesc++ = vtxDescClr0;
  }

  if ((vtxDescr.streamFlags & kHasTexture) != 0) {
    const GXVtxDescList vtxDescTex0 = {GX_VA_TEX0, GX_DIRECT};
    *curDesc++ = vtxDescTex0;
  }

  const GXVtxDescList vtxDescNull = {GX_VA_NULL, GX_NONE};
  *curDesc = vtxDescNull;

  CGX::SetVtxDescv(vtxDesc);
  SetTevStates(vtxDescr.streamFlags);
  FullRender();
}

void CGraphics::SetTevStates(uchar flags) {
  switch (flags) {
  case 0:
  case kHasNormals:
  case kHasColor:
  case kHasNormals | kHasColor:
    CGX::SetNumChans(1);
    CGX::SetNumTexGens(0);
    CGX::SetNumTevStages(1);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    break;
  case kHasTexture:
  case kHasNormals | kHasTexture:
  case kHasColor | kHasTexture:
  case kHasNormals | kHasColor | kHasTexture:
    CGX::SetNumChans(1);
    if ((vtxDescr.textureUsed & 3) != 0) {
      CGX::SetNumTexGens(2);
    } else {
      CGX::SetNumTexGens(1);
    }
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
    break;
  }
  CGX::SetNumIndStages(0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);

  uint light = mLightActive;
  GXAttnFn attnFn = GX_AF_NONE;
  if (light != 0) {
    attnFn = GX_AF_SPOT;
  }
  GXDiffuseFn diffFn = GX_DF_NONE;
  if (light != 0) {
    diffFn = GX_DF_CLAMP;
  }
  CGX::SetChanCtrl(CGX::Channel0, light ? GX_ENABLE : GX_DISABLE, GX_SRC_REG,
                   (flags & kHasColor) ? GX_SRC_VTX : GX_SRC_REG, static_cast< GXLightID >(light),
                   diffFn, attnFn);
}

void CGraphics::FullRender() {
  CGX::Begin(static_cast< GXPrimitive >(mCurrentPrimitive), GX_VTXFMT0, mNumPrimitives);
  switch (vtxDescr.streamFlags) {
  case 0:
    for (int i = 0; i < mNumPrimitives; i++) {
      const Vec& vtx = vtxBuffer[i];
      GXPosition3f32(vtx.x, vtx.y, vtx.z);
    }
    break;
  case kHasNormals:
    for (int i = 0; i < mNumPrimitives; i++) {
      const Vec& vtx = vtxBuffer[i];
      GXPosition3f32(vtx.x, vtx.y, vtx.z);
      const Vec& nrm = nrmBuffer[i];
      GXNormal3f32(nrm.x, nrm.y, nrm.z);
    }
    break;
  case kHasColor:
    for (int i = 0; i < mNumPrimitives; i++) {
      const Vec& vtx = vtxBuffer[i];
      GXPosition3f32(vtx.x, vtx.y, vtx.z);
      GXColor1u32(clrBuffer[i]);
    }
    break;
  case kHasTexture:
    for (int i = 0; i < mNumPrimitives; i++) {
      const Vec& vtx = vtxBuffer[i];
      GXPosition3f32(vtx.x, vtx.y, vtx.z);
      const Vec2& uv = txtBuffer0[i];
      GXTexCoord2f32(uv.x, uv.y);
    }
    break;
  case kHasNormals | kHasTexture:
    for (int i = 0; i < mNumPrimitives; i++) {
      const Vec& vtx = vtxBuffer[i];
      GXPosition3f32(vtx.x, vtx.y, vtx.z);
      const Vec& nrm = nrmBuffer[i];
      GXNormal3f32(nrm.x, nrm.y, nrm.z);
      const Vec2& uv = txtBuffer0[i];
      GXTexCoord2f32(uv.x, uv.y);
    }
    break;
  case kHasNormals | kHasColor:
    for (int i = 0; i < mNumPrimitives; i++) {
      const Vec& vtx = vtxBuffer[i];
      GXPosition3f32(vtx.x, vtx.y, vtx.z);
      const Vec& nrm = nrmBuffer[i];
      GXNormal3f32(nrm.x, nrm.y, nrm.z);
      GXColor1u32(clrBuffer[i]);
    }
    break;
  case kHasColor | kHasTexture:
    for (int i = 0; i < mNumPrimitives; i++) {
      const Vec& vtx = vtxBuffer[i];
      GXPosition3f32(vtx.x, vtx.y, vtx.z);
      GXColor1u32(clrBuffer[i]);
      const Vec2& uv = txtBuffer0[i];
      GXTexCoord2f32(uv.x, uv.y);
    }
    break;
  case kHasNormals | kHasColor | kHasTexture:
    for (int i = 0; i < mNumPrimitives; i++) {
      const Vec& vtx = vtxBuffer[i];
      GXPosition3f32(vtx.x, vtx.y, vtx.z);
      const Vec& nrm = nrmBuffer[i];
      GXNormal3f32(nrm.x, nrm.y, nrm.z);
      GXColor1u32(clrBuffer[i]);
      const Vec2& uv = txtBuffer0[i];
      GXTexCoord2f32(uv.x, uv.y);
    }
    break;
  }
  CGX::End();
}

void CGraphics::SetDepthRange(float near, float far) {
  mDepthNear = near;
  mDepthFar = far;
  GXSetViewport(static_cast< float >(mViewport.mLeft), static_cast< float >(mViewport.mTop),
                static_cast< float >(mViewport.mWidth), static_cast< float >(mViewport.mHeight),
                mDepthNear, mDepthFar);
}

static inline GXTevStageID get_texture_unit(const ERglTevStage stage) {
#if VERSION >= VERSION_GM8P_00
  return static_cast< GXTevStageID >(stage);
#else
  if (stage == kTS_Stage0) {
    return GX_TEVSTAGE0;
  } else if (stage == kTS_Stage1) {
    return GX_TEVSTAGE1;
  } else if (stage == kTS_Stage2) {
    return GX_TEVSTAGE2;
  } else if (stage == kTS_Stage3) {
    return GX_TEVSTAGE3;
  } else if (stage == kTS_Stage4) {
    return GX_TEVSTAGE4;
  } else if (stage == kTS_Stage5) {
    return GX_TEVSTAGE5;
  } else if (stage == kTS_Stage6) {
    return GX_TEVSTAGE6;
  } else if (stage == kTS_Stage7) {
    return GX_TEVSTAGE7;
  } else if (stage == kTS_Stage8) {
    return GX_TEVSTAGE8;
  } else if (stage == kTS_Stage9) {
    return GX_TEVSTAGE9;
  } else if (stage == kTS_Stage10) {
    return GX_TEVSTAGE10;
  } else if (stage == kTS_Stage11) {
    return GX_TEVSTAGE11;
  } else if (stage == kTS_Stage12) {
    return GX_TEVSTAGE12;
  } else if (stage == kTS_Stage13) {
    return GX_TEVSTAGE13;
  } else if (stage == kTS_Stage14) {
    return GX_TEVSTAGE14;
  }
  // wtf?
  return static_cast< GXTevStageID >(stage == kTS_Stage15 ? GX_TEVSTAGE15 : 0);
#endif
}

void CGraphics::SetTevOp(ERglTevStage stage, const CTevCombiners::CTevPass& pass) {
  CTevCombiners::SetupPass(get_texture_unit(stage), pass);
}

void CGraphics::SetFog(ERglFogMode mode, float startz, float endz, const CColor& color) {
  CGX::SetFog(static_cast< GXFogType >(mode), startz, endz, mProj.GetNear(), mProj.GetFar(),
              color.GetGXColor());
}

void CGraphics::ResetGfxStates() { sRenderState.Set(0); }

void CGraphics::SetDefaultVtxAttrFmt() {
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 14);
  GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 14);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U16, 15);
  for (int i = 1; i <= 7; ++i) {
    GXAttr attr = static_cast< GXAttr >(GX_VA_TEX0 + i);
    GXSetVtxAttrFmt(GX_VTXFMT0, attr, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, attr, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT2, attr, GX_TEX_ST, GX_F32, 0);
  }
}

void CGraphics::LoadDolphinSpareTexture(int width, int height, GXTexFmt fmt, void* data,
                                        GXTexMapID texId) {
  GXTexObj texObj;
  GXInitTexObj(&texObj, data != nullptr ? data : mpSpareBuffer, width, height, fmt, GX_CLAMP,
               GX_CLAMP, GX_DISABLE);
  GXInitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);
  GXLoadTexObj(&texObj, texId);
  CTexture::InvalidateTexmap(texId);
  if (texId == GX_TEXMAP7) {
    GXInvalidateTexRegion(&mTexRegions[0]);
  }
}

void CGraphics::LoadDolphinSpareTexture(int width, int height, GXCITexFmt fmt, GXTlut tlut,
                                        void* data, GXTexMapID texId) {
  GXTexObj texObj;
  GXInitTexObjCI(&texObj, data != nullptr ? data : mpSpareBuffer, width, height, fmt, GX_CLAMP,
                 GX_CLAMP, GX_DISABLE, tlut);
  GXInitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);
  GXLoadTexObj(&texObj, texId);
  CTexture::InvalidateTexmap(texId);
  if (texId == GX_TEXMAP7) {
    GXInvalidateTexRegion(&mTexRegions[0]);
  }
}

void CGraphics::TickRenderTimings() {
  mRenderTimings = (mRenderTimings + 1) % (900 * 60);
  mSecondsMod900 = static_cast< float >(mRenderTimings) / 60.f;
}

float CGraphics::GetSecondsMod900() {
  if (mpExternalTimeProvider != nullptr) {
    return mpExternalTimeProvider->GetSecondsMod900();
  }
  return mSecondsMod900;
}

void CGraphics::SetExternalTimeProvider(CTimeProvider* timeProvider) {
  mpExternalTimeProvider = timeProvider;
}

void CGraphics::FlushProjection() {
  float right = mProj.GetRight();
  float left = mProj.GetLeft();
  float top = mProj.GetTop();
  float bottom = mProj.GetBottom();
  float near = mProj.GetNear();
  float far = mProj.GetFar();
  if (mProj.IsPerspective()) {
    Mtx44 mtx;
    MTXFrustum(mtx, top, bottom, left, right, near, far);
    GXSetProjection(mtx, GX_PERSPECTIVE);
  } else {
    Mtx44 mtx;
    MTXOrtho(mtx, top, bottom, left, right, near, far);
    GXSetProjection(mtx, GX_ORTHOGRAPHIC);
  }
}

const CGraphics::CProjectionState& CGraphics::GetProjectionState() { return mProj; }

void CGraphics::SetProjectionState(const CProjectionState& proj) {
  mProj = proj;
  FlushProjection();
}

// TODO non-matching (regswaps)
CGraphics::CClippedScreenRect
CGraphics::ClipScreenRectFromVS(const CVector3f& p1, const CVector3f& p2, ETexelFormat fmt) {
  if (!p1.IsNonZero() || !p2.IsNonZero()) {
    return CClippedScreenRect();
  }
  if (p1.GetY() < GetProjectionState().GetNear() || p2.GetY() < GetProjectionState().GetNear()) {
    return CClippedScreenRect();
  }
  if (p1.GetY() > GetProjectionState().GetFar() || p2.GetY() > GetProjectionState().GetFar()) {
    return CClippedScreenRect();
  }

  CVector2i p1p = ProjectPoint(p1);
  CVector2i p2p = ProjectPoint(p2);

  int minX = rstl::min_val(p1p.GetX(), p2p.GetX());
  int minY = rstl::min_val(p1p.GetY(), p2p.GetY());

  int maxX = __abs(p1p.GetX() - p2p.GetX());
  int maxY = __abs(p1p.GetY() - p2p.GetY());

  int left = minX & ~1;
  if (left >= mViewport.mLeft + mViewport.mWidth) {
    return CClippedScreenRect();
  }

  int right = minX + maxX + 2 & ~1;
  if (right <= mViewport.mLeft) {
    return CClippedScreenRect();
  }
  left = rstl::max_val(left, mViewport.mLeft) & ~1;
  right = rstl::min_val(right, mViewport.mLeft + mViewport.mWidth) + 1 & ~1;

  int top = minY & ~1;
  if (top >= mViewport.mTop + mViewport.mHeight) {
    return CClippedScreenRect();
  }

  int bottom = minY + maxY + 2 & ~1;
  if (bottom <= mViewport.mTop) {
    return CClippedScreenRect();
  }
  top = rstl::max_val(top, mViewport.mTop) & ~1;
  bottom = rstl::min_val(bottom, mViewport.mTop + mViewport.mHeight) + 1 & ~1;

  float minV = CCast::LtoF(minY - top) / CCast::LtoF(bottom - top);
  float maxV = CCast::LtoF(maxY + (minY - top) + 1) / CCast::LtoF(bottom - top);

  int texAlign = 4;
  switch (fmt) {
  case kTF_I8:
    texAlign = 8;
    break;
  case kTF_IA8:
  case kTF_RGB565:
  case kTF_RGB5A3:
    texAlign = 4;
    break;
  case kTF_RGBA8:
    texAlign = 2;
    break;
  }

  int texWidth = texAlign + (right - left) - 1 & ~(texAlign - 1);
  float minU = CCast::LtoF(minX - left) / CCast::LtoF(texWidth);
  float maxU = CCast::LtoF(maxX + (minX - left) + 1) / CCast::LtoF(texWidth);
  return CClippedScreenRect(left, top, right - left, bottom - top, texWidth, minU, maxU, minV,
                            maxV);
}

CGraphics::CClippedScreenRect
CGraphics::ClipScreenRectFromMS(const CVector3f& p1, const CVector3f& p2, ETexelFormat fmt) {
  return ClipScreenRectFromVS(mViewMatrix.TransposeMultiply(mModelMatrix * p1),
                              mViewMatrix.TransposeMultiply(mModelMatrix * p2), fmt);
}

float CGraphics::GetFPS() {
  BOOL level = OSDisableInterrupts();
  float value = rstl::min_val(mFramesPerSecond, mLastFramesPerSecond);
  OSRestoreInterrupts(level);
  return value;
}

void CGraphics::SetUseVideoFilter(const bool b) {
  mUseVideoFilter = b;
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, b ? GX_ENABLE : GX_DISABLE,
                  mRenderModeObj.vfilter);
}

bool CGraphics::GetUseVideoFilter() { return mUseVideoFilter; }

int CGraphics::GetFrameCounter() { return mFrameCounter; }

CVector2i CGraphics::ProjectPoint(const CVector3f& point) {
  CVector3f vec = GetPerspectiveProjectionMatrix().MultiplyOneOverW(point);
  vec.SetX(vec.GetX() * GetViewport().mHalfWidth + GetViewport().mHalfWidth);
  vec.SetY(-vec.GetY() * GetViewport().mHalfHeight + GetViewport().mHalfHeight);
  return CVector2i(CCast::FtoL(vec.GetX()), CCast::FtoL(vec.GetY()));
}

void CGraphics::SetProgressiveMode(bool b) {
  bool isProgressive = GetProgressiveMode();
  OSSetProgressiveMode(b ? OS_PROGRESSIVE_MODE_ON : OS_PROGRESSIVE_MODE_OFF);
  if (b == isProgressive) {
    return;
  }

  VISetBlack(TRUE);
  VIFlush();
  VIWaitForRetrace();
  for (int i = 0; i < 10; ++i) {
    VIWaitForRetrace();
  }

  if (b) {
    mRenderModeObj.viTVmode = VI_TVMODE_NTSC_PROG;
    mRenderModeObj.xFBmode = VI_XFBMODE_SF;
    const u8 vfilter[7] = {0x04, 0x04, 0x10, 0x10, 0x10, 0x04, 0x04};
    memcpy(mRenderModeObj.vfilter, vfilter, 7);
  } else {
    mRenderModeObj.viTVmode = VI_TVMODE_NTSC_INT;
    mRenderModeObj.xFBmode = VI_XFBMODE_DF;
    memcpy(mRenderModeObj.vfilter, GXNtsc480IntDf.vfilter, 7);
  }
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, GX_ENABLE,
                  mRenderModeObj.vfilter);

  VIConfigure(&mRenderModeObj);
  VISetBlack(TRUE);
  VIFlush();
  for (int i = 0; i < 100; ++i) {
    VIWaitForRetrace();
  }
  VISetBlack(FALSE);
  VIFlush();
  for (int i = 0; i < 2; ++i) {
    VIWaitForRetrace();
  }
}

bool CGraphics::GetProgressiveMode() { return mRenderModeObj.viTVmode == VI_TVMODE_NTSC_PROG; }

bool CGraphics::CanSetProgressiveMode() { return VIGetDTVStatus() != 0; }

bool CGraphics::GetProgressiveDefault() { return OSGetProgressiveMode() == OS_PROGRESSIVE_MODE_ON; }

void CGraphics::GetScreenPosition(int* stretch, int* xOffset, int* yOffset) {
  if (stretch != nullptr) {
    *stretch = mScreenStretch;
  }
  if (xOffset != nullptr) {
    *xOffset = mScreenPositionX;
  }
  if (yOffset != nullptr) {
    *yOffset = mScreenPositionY;
  }
}

void CGraphics::SetScreenPosition(int stretch, int xOffset, int yOffset) {
  int stretchChange = stretch - mScreenStretch;
  int xChange = xOffset - mScreenPositionX;
  int yChange = yOffset - mScreenPositionY;
  if (stretchChange == 0 && xChange == 0 && yChange == 0) {
    return;
  }
  mRenderModeObj.viWidth += stretchChange * 2;
  mRenderModeObj.viXOrigin += xChange - stretchChange;
  mRenderModeObj.viYOrigin += yChange;
  VIConfigure(&mRenderModeObj);
  VIFlush();
  mScreenStretch = stretch;
  mScreenPositionX = xOffset;
  mScreenPositionY = yOffset;
}

void CGraphics::SetIsBeginSceneClearFb(bool b) { mIsBeginSceneClearFb = b; }

CGraphicsSys::CGraphicsSys(const COsContext& osContext, const CMemorySys& memorySys, uint fifoSize,
                           void* fifoBase) {
  if (mGraphicsInitialized != true) {
    mGraphicsInitialized = CGraphics::Startup(osContext, fifoSize, fifoBase);
  }
}

CGraphicsSys::~CGraphicsSys() {
  if (mGraphicsInitialized == true) {
    CGraphics::Shutdown();
    mGraphicsInitialized = false;
  }
}

CGraphics::CRenderState::CRenderState() {
  x0_ = 0;
  x4_ = 0;
}

void CGraphics::CRenderState::Flush() {}

int CGraphics::CRenderState::SetVtxState(const float* pos, const float* nrm, const uint* clr) {
  CGX::SetArray(GX_VA_POS, pos, 12);
  CGX::SetArray(GX_VA_NRM, nrm, 12);
  CGX::SetArray(GX_VA_CLR0, clr, 4);
  int result = 1;
  if (nrm != nullptr) {
    result |= 2;
  }
  if (clr != nullptr) {
    result |= 16;
  }
  return result;
}

void CGraphics::CRenderState::ResetFlushAll() {
  x0_ = 0;
  SetVtxState(nullptr, nullptr, nullptr);
  for (int i = 0; i < 8; i++) {
    CGX::SetArray(static_cast< GXAttr >(GX_VA_TEX0 + i), nullptr, 8);
  }
  Flush();
}

// static float stripped(int n) { return static_cast<float>(n); }
