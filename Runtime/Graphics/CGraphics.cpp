#include "Runtime/Graphics/CGraphics.hpp"

#include "Runtime/CTimeProvider.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/GuiSys/CGuiSys.hpp"
#include "Runtime/Graphics/CGX.hpp"
#include "Runtime/Logging.hpp"

#include <dolphin/gx.h>
#include <dolphin/vi.h>
#include <zeus/Math.hpp>

namespace metaforce {
using CVector3f = zeus::CVector3f;
using CVector2i = zeus::CVector2i;
using CTransform4f = zeus::CTransform;
using CColor = zeus::CColor;
using uchar = unsigned char;
using uint = unsigned int;
using ushort = unsigned short;

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

struct {
  Vec vtx;
  Vec nrm;
  Vec2 uv0;
  Vec2 uv1;
  u32 color;
  u16 textureUsed;
  u8 streamFlags;
} vtxDescr;

CVector3f CGraphics::kDefaultPositionVector(0.f, 0.f, 0.f);
CVector3f CGraphics::kDefaultDirectionVector(0.f, 1.f, 0.f);
CGraphics::CProjectionState CGraphics::mProj(true, -1.f, 1.f, 1.f, -1.f, 1.f, 100.f);
CTransform4f CGraphics::mViewMatrix = CTransform4f();
CTransform4f CGraphics::mModelMatrix = CTransform4f();
CColor CGraphics::mClearColor = zeus::skBlack;
CVector3f CGraphics::mViewPoint(0.f, 0.f, 0.f);
GXLightObj CGraphics::mLightObj[8];
// GXTexRegion CGraphics::mTexRegions[GX_MAX_TEXMAP];
// GXTexRegion CGraphics::mTexRegionsCI[GX_MAX_TEXMAP / 2];
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
GX::LightMask CGraphics::mLightActive;
GX::LightMask CGraphics::mLightsWereOn;
void* CGraphics::mpFrameBuf1;
void* CGraphics::mpFrameBuf2;
void* CGraphics::mpCurrenFrameBuf;
int CGraphics::mSpareBufferSize;
void* CGraphics::mpSpareBuffer;
int CGraphics::mSpareBufferTexCacheSize;
// GXTexRegionCallback CGraphics::mGXDefaultTexRegionCallback;
void* CGraphics::mpFifo;
GXFifoObj* CGraphics::mpFifoObj;
uint CGraphics::mRenderTimings;
float CGraphics::mSecondsMod900;
CTimeProvider* CGraphics::mpExternalTimeProvider;
int CGraphics::mScreenStretch;
int CGraphics::mScreenPositionX;
int CGraphics::mScreenPositionY;

CViewport CGraphics::mViewport = {0, 0, 640, 480, 320.f, 240.f};
ELightType CGraphics::mLightTypes[8] = {
    ELightType::Directional, ELightType::Directional, ELightType::Directional, ELightType::Directional,
    ELightType::Directional, ELightType::Directional, ELightType::Directional, ELightType::Directional,
};

// const CTevCombiners::CTevPass& CGraphics::kEnvPassthru = CTevCombiners::kEnvPassthru;
bool CGraphics::mIsBeginSceneClearFb = true;
ERglEnum CGraphics::mDepthFunc = ERglEnum::LEqual;
ERglPrimitive CGraphics::mCurrentPrimitive = ERglPrimitive::Points;
float CGraphics::mDepthFar = 1.f;
u32 CGraphics::mClearDepthValue = GX_MAX_Z24;
bool CGraphics::mIsGXModelMatrixIdentity = true;
bool CGraphics::mFirstFrame = true;
GXBool CGraphics::mUseVideoFilter = GX_ENABLE;
float CGraphics::mBrightness = 1.f;

const GXTexMapID CGraphics::kSpareBufferTexMapID = GX_TEXMAP7;

// We don't actually store anything here
static std::array<uchar, 1> sSpareFrameBuffer;

void CGraphics::DisableAllLights() {
  mNumLightsActive = 0;
  mLightActive.reset();
  CGX::SetChanCtrl(CGX::EChannelId::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
}

static inline GXLightID get_hw_light_index(ERglLight light) {
  return static_cast<GXLightID>((1 << light) & (GX_MAX_LIGHT - 1));
}

void CGraphics::LoadLight(ERglLight light, const CLight& info) {
  GXLightID lightId = get_hw_light_index(light);
  ELightType type = info.GetType();
  CVector3f pos = info.GetPosition();
  CVector3f dir = info.GetDirection();

  switch (type) {
  case ELightType::Spot: {
    MTXMultVec(mCameraMtx, reinterpret_cast<VecPtr>(&pos), reinterpret_cast<VecPtr>(&pos));
    GXLightObj* obj = &mLightObj[light];
    GXInitLightPos(obj, pos.x(), pos.y(), pos.z());
    MTXMultVecSR(mCameraMtx, reinterpret_cast<VecPtr>(&dir), reinterpret_cast<VecPtr>(&dir));
    GXInitLightDir(obj, dir.x(), dir.y(), dir.z());
    GXInitLightAttn(obj, 1.f, 0.f, 0.f, info.GetAttenuationConstant(), info.GetAttenuationLinear(),
                    info.GetAttenuationQuadratic());
    GXInitLightSpot(obj, info.GetSpotCutoff(), GX_SP_COS2);
    break;
  }
  case ELightType::Point:
  case ELightType::LocalAmbient: {
    MTXMultVec(mCameraMtx, reinterpret_cast<VecPtr>(&pos), reinterpret_cast<VecPtr>(&pos));
    GXInitLightPos(&mLightObj[light], pos.x(), pos.y(), pos.z());
    GXInitLightAttn(&mLightObj[light], 1.f, 0.f, 0.f, info.GetAttenuationConstant(), info.GetAttenuationLinear(),
                    info.GetAttenuationQuadratic());
    break;
  }
  case ELightType::Directional: {
    MTXMultVecSR(mCameraMtx, reinterpret_cast<VecPtr>(&dir), reinterpret_cast<VecPtr>(&dir));
    dir = -dir;
    GXInitLightPos(&mLightObj[light], dir.x() * 1048576.f, dir.y() * 1048576.f, dir.z() * 1048576.f);
    GXInitLightAttn(&mLightObj[light], 1.f, 0.f, 0.f, 1.f, 0.f, 0.f);
    break;
  }
  case ELightType::Custom: {
    MTXMultVec(mCameraMtx, reinterpret_cast<VecPtr>(&pos), reinterpret_cast<VecPtr>(&pos));
    GXLightObj* obj = &mLightObj[light];
    GXInitLightPos(obj, pos.x(), pos.y(), pos.z());
    MTXMultVecSR(mCameraMtx, reinterpret_cast<VecPtr>(&dir), reinterpret_cast<VecPtr>(&dir));
    GXInitLightDir(obj, dir.x(), dir.y(), dir.z());
    GXInitLightAttn(obj, info.GetAngleAttenuationConstant(), info.GetAngleAttenuationLinear(),
                    info.GetAngleAttenuationQuadratic(), info.GetAttenuationConstant(), info.GetAttenuationLinear(),
                    info.GetAttenuationQuadratic());
    break;
  }
  default:
    break;
  }

  GXInitLightColor(&mLightObj[light], to_gx_color(info.GetColor()));
  GXLoadLightObjImm(&mLightObj[light], lightId);
  mLightTypes[light] = info.GetType();
}

void CGraphics::EnableLight(ERglLight light) {
  CGX::SetNumChans(1);
  GX::LightMask lightsWereOn = mLightActive;
  if (!lightsWereOn.test(light)) {
    mLightActive.set(light);
    CGX::SetChanCtrl(CGX::EChannelId::Channel0, true, GX_SRC_REG, GX_SRC_REG, mLightActive, GX_DF_CLAMP, GX_AF_SPOT);
    ++mNumLightsActive;
  }
  mLightsWereOn = mLightActive;
}

void CGraphics::SetLightState(GX::LightMask lights) {
  GXAttnFn attnFn = GX_AF_NONE;
  if (lights.any()) {
    attnFn = GX_AF_SPOT;
  }
  GXDiffuseFn diffFn = GX_DF_NONE;
  if (lights.any()) {
    diffFn = GX_DF_CLAMP;
  }
  CGX::SetChanCtrl(CGX::EChannelId::Channel0, lights.any() ? GX_ENABLE : GX_DISABLE, GX_SRC_REG,
                   (vtxDescr.streamFlags & 2) != 0 ? GX_SRC_VTX : GX_SRC_REG, lights, diffFn, attnFn);
  mLightActive = lights;
  mNumLightsActive = lights.count();
}

void CGraphics::SetAmbientColor(const zeus::CColor& col) {
  CGX::SetChanAmbColor(CGX::EChannelId::Channel0, col);
  CGX::SetChanAmbColor(CGX::EChannelId::Channel1, col);
}

void CGraphics::SetFog(ERglFogMode mode, float startz, float endz, const zeus::CColor& color) {
  CGX::SetFog(static_cast<GXFogType>(mode), startz, endz, mProj.GetNear(), mProj.GetFar(), to_gx_color(color));
}

void CGraphics::SetDepthWriteMode(const bool test, ERglEnum comp, const bool write) {
  mDepthFunc = comp;
  CGX::SetZMode(test, static_cast<GXCompare>(comp), write);
}

void CGraphics::SetBlendMode(ERglBlendMode mode, ERglBlendFactor src, ERglBlendFactor dst, ERglLogicOp op) {
  CGX::SetBlendMode(static_cast<GXBlendMode>(mode), static_cast<GXBlendFactor>(src), static_cast<GXBlendFactor>(dst),
                    static_cast<GXLogicOp>(op));
}

void CGraphics::SetCullMode(ERglCullMode cullMode) {
  mCullMode = cullMode;
  GXSetCullMode(static_cast<GXCullMode>(cullMode));
}

void CGraphics::ClearBackAndDepthBuffers() {
  GXInvalidateTexAll();
  GXSetViewport(0.f, 0.f, mRenderModeObj.fbWidth, mRenderModeObj.xfbHeight, 0.f, 1.f);
  GXInvalidateVtxCache();
}

void CGraphics::BeginScene() { ClearBackAndDepthBuffers(); }

void CGraphics::EndScene() {
  CGX::SetZMode(true, GX_LEQUAL, true);
  // volatile int& numBreakPt = const_cast< volatile int& >(mNumBreakpointsWaiting);
  // while (numBreakPt > 0) {
  //   OSYieldThread();
  // }
  ++mNumBreakpointsWaiting;
  void*& frameBuf = mpCurrenFrameBuf;
  float brightness = std::clamp(mBrightness, 0.f, 2.f);
  static const u8 copyFilter[7] = {0x00, 0x00, 0x15, 0x16, 0x15, 0x00, 0x00};
  const u8* inFilter = mUseVideoFilter ? mRenderModeObj.vfilter : copyFilter;
  u8 vfilter[7];
  for (int i = 0; i < 7; i++) {
    vfilter[i] = static_cast<u8>(static_cast<float>(inFilter[i]) * brightness);
  }
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, true, vfilter);
  GXCopyDisp(frameBuf, mIsBeginSceneClearFb ? GX_TRUE : GX_FALSE);
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, mUseVideoFilter ? GX_ENABLE : GX_DISABLE,
                  mRenderModeObj.vfilter);
  // GXSetBreakPtCallback(SwapBuffers);
  // VISetPreRetraceCallback(VideoPreCallback);
  // VISetPostRetraceCallback(VideoPostCallback);
  GXFlush();
  GXFifoObj* fifo = GXGetGPFifo();
  void* readPtr;
  void* writePtr;
  GXGetFifoPtrs(fifo, &readPtr, &writePtr);
  // GXEnableBreakPt(writePtr);
  mLastFrameUsedAbove = mInterruptLastFrameUsedAbove;
  ++mFrameCounter;
  // CFrameDelayedKiller::fn_8036CB90();
}

static constexpr GXVtxDescList skPosColorTexDirect[] = {
    {GX_VA_POS, GX_DIRECT},
    {GX_VA_CLR0, GX_DIRECT},
    {GX_VA_TEX0, GX_DIRECT},
    {GX_VA_NULL, GX_DIRECT},
};

void CGraphics::Render2D(CTexture& tex, int x, int y, int w, int h, const zeus::CColor& col, bool scale) {
  Mtx44 proj;
  if (scale) {
    const float viewportAspect = GetViewportAspect();
    float left = -320.f;
    float right = 320.f;
    float top = 224.f;
    float bottom = -224.f;
    if (viewportAspect > 4.f / 3.f) {
      float width = 224.0f * viewportAspect;
      left = -width;
      right = width;
    } else {
      float height = 320.0f / viewportAspect;
      top = height;
      bottom = -height;
    }
    MTXOrtho(proj, top, bottom, left, right, -1.f, -10.f);
  } else {
    MTXOrtho(proj, mViewport.mHeight / 2, -(mViewport.mHeight / 2), -(mViewport.mWidth / 2), mViewport.mWidth / 2, -1.f,
             -10.f);
  }
  GXSetProjection(proj, GX_ORTHOGRAPHIC);
  uint c = col.toRGBA();

  Mtx mtx;
  MTXIdentity(mtx);
  GXLoadPosMtxImm(mtx, GX_PNMTX0);

  float x2, y2, x1, y1;
  if (scale) {
    x1 = x - 320;
    y1 = y - 224;
    x2 = x1 + w;
    y2 = y1 + h;
  } else {
    x1 = x - mViewport.mWidth / 2;
    y1 = y - mViewport.mHeight / 2;
    x2 = x1 + w;
    y2 = y1 + h;
  }

  // Save state + setup
  CGX::SetVtxDescv(skPosColorTexDirect);
  SetTevStates(6);
  mLightsWereOn = mLightActive;
  if (mLightActive.any()) {
    DisableAllLights();
  }
  ERglCullMode cullMode = mCullMode;
  SetCullMode(ERglCullMode::None);
  tex.Load(GX_TEXMAP0, EClampMode::Repeat);

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
  if (mLightsWereOn.any()) {
    SetLightState(mLightsWereOn);
  }
  FlushProjection();
  mIsGXModelMatrixIdentity = false;
  SetModelMatrix(mModelMatrix);
  SetCullMode(cullMode);
}

void CGraphics::SetAlphaCompare(ERglAlphaFunc comp0, uchar ref0, ERglAlphaOp op, ERglAlphaFunc comp1, uchar ref1) {
  CGX::SetAlphaCompare(static_cast<GXCompare>(comp0), ref0, static_cast<GXAlphaOp>(op), static_cast<GXCompare>(comp1),
                       ref1);
}

void CGraphics::SetViewPointMatrix(const zeus::CTransform& xf) {
  mViewMatrix = xf;
  mGXViewPointMatrix[0][0] = xf.basis[0][0];
  mGXViewPointMatrix[0][1] = xf.basis[0][1];
  mGXViewPointMatrix[0][2] = xf.basis[0][2];
  mGXViewPointMatrix[0][3] = 0.f;
  mGXViewPointMatrix[1][0] = xf.basis[2][0];
  mGXViewPointMatrix[1][1] = xf.basis[2][1];
  mGXViewPointMatrix[1][2] = xf.basis[2][2];
  mGXViewPointMatrix[1][3] = 0.f;
  mGXViewPointMatrix[2][0] = -xf.basis[1][0];
  mGXViewPointMatrix[2][1] = -xf.basis[1][1];
  mGXViewPointMatrix[2][2] = -xf.basis[1][2];
  mGXViewPointMatrix[2][3] = 0.f;
  mViewPoint = xf.origin;
  SetViewMatrix();
}

void CGraphics::SetIdentityViewPointMatrix() {
  mViewMatrix = CTransform4f();
  MTXIdentity(mGXViewPointMatrix);
  mGXViewPointMatrix[2][2] = 0.f;
  mGXViewPointMatrix[1][1] = 0.f;
  mGXViewPointMatrix[1][2] = 1.f;
  mGXViewPointMatrix[2][1] = -1.f;
  mViewPoint = CVector3f();
  SetViewMatrix();
}

void CGraphics::SetViewMatrix() {
  Mtx mtx;
  MTXTrans(mtx, -mViewPoint.x(), -mViewPoint.y(), -mViewPoint.z());
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

void CGraphics::SetModelMatrix(const zeus::CTransform& xf) {
  if (xf == zeus::CTransform()) {
    if (!mIsGXModelMatrixIdentity) {
      mModelMatrix = xf;
      mIsGXModelMatrixIdentity = true;
      SetViewMatrix();
    }
    return;
  }

  mModelMatrix = xf;
  mIsGXModelMatrixIdentity = false;
  mGXModelMatrix[0][0] = xf.basis[0][0];
  mGXModelMatrix[0][1] = xf.basis[1][0];
  mGXModelMatrix[0][2] = xf.basis[2][0];
  mGXModelMatrix[0][3] = xf.origin.x();
  mGXModelMatrix[1][0] = xf.basis[0][1];
  mGXModelMatrix[1][1] = xf.basis[1][1];
  mGXModelMatrix[1][2] = xf.basis[2][1];
  mGXModelMatrix[1][3] = xf.origin.y();
  mGXModelMatrix[2][0] = xf.basis[0][2];
  mGXModelMatrix[2][1] = xf.basis[1][2];
  mGXModelMatrix[2][2] = xf.basis[2][2];
  mGXModelMatrix[2][3] = xf.origin.z();
  SetViewMatrix();
}

void CGraphics::SetIdentityModelMatrix() {
  if (!mIsGXModelMatrixIdentity) {
    mModelMatrix = CTransform4f();
    mIsGXModelMatrixIdentity = true;
    SetViewMatrix();
  }
}

zeus::CMatrix4f CGraphics::CalculatePerspectiveMatrix(float fovy, float aspect, float znear, float zfar) {
  float t = std::tan(zeus::degToRad(fovy) / 2.f);
  float right = aspect * 2.f * znear * t * 0.5f;
  float left = -right;
  float top = znear * 2.f * t * 0.5f;
  float bottom = -top;
  return zeus::CMatrix4f{
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
    0.f,
      // clang-format on
  };
}

zeus::CMatrix4f CGraphics::GetPerspectiveProjectionMatrix() {
  return zeus::CMatrix4f{
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
  };
}

const CGraphics::CProjectionState& CGraphics::GetProjectionState() { return mProj; }

void CGraphics::SetProjectionState(const CProjectionState& proj) {
  mProj = proj;
  FlushProjection();
}

void CGraphics::SetPerspective(float fovy, float aspect, float znear, float zfar) {
  float t = tan(zeus::degToRad(fovy) / 2.f);
  mProj = CProjectionState(true,                               // Is Projection
                           -(aspect * 2.f * znear * t * 0.5f), // Left
                           (aspect * 2.f * znear * t * 0.5f),  // Right
                           (znear * 2.f * t * 0.5f),           // Top
                           -(znear * 2.f * t * 0.5f),          // Bottom
                           znear, zfar);
  FlushProjection();
}

void CGraphics::SetOrtho(float left, float right, float top, float bottom, float znear, float zfar) {
  mProj = CProjectionState(false, left, right, top, bottom, znear, zfar);
  FlushProjection();
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

zeus::CVector2i CGraphics::ProjectPoint(const zeus::CVector3f& point) {
  zeus::CVector3f vec = GetPerspectiveProjectionMatrix().multiplyOneOverW(point);
  vec.x() = vec.x() * mViewport.mHalfWidth + mViewport.mHalfWidth;
  vec.y() = -vec.y() * mViewport.mHalfHeight + mViewport.mHalfHeight;
  return CVector2i(vec.x(), vec.y());
}

static CVector3f TransposeMultiply(const CTransform4f& self, const CVector3f& in) {
  return self.transposeRotate({in.x() - self.origin.x(), in.y() - self.origin.y(), in.z() - self.origin.z()});
}

CGraphics::CClippedScreenRect CGraphics::ClipScreenRectFromMS(const CVector3f& p1, const CVector3f& p2,
                                                              ETexelFormat fmt) {
  return ClipScreenRectFromVS(TransposeMultiply(mViewMatrix, mModelMatrix * p1),
                              TransposeMultiply(mViewMatrix, mModelMatrix * p2), fmt);
}

CGraphics::CClippedScreenRect CGraphics::ClipScreenRectFromVS(const CVector3f& p1, const CVector3f& p2,
                                                              ETexelFormat fmt) {
  if (p1.isZero() || p2.isZero()) {
    return CClippedScreenRect();
  }
  if (p1.y() < GetProjectionState().GetNear() || p2.y() < GetProjectionState().GetNear()) {
    return CClippedScreenRect();
  }
  if (p1.y() > GetProjectionState().GetFar() || p2.y() > GetProjectionState().GetFar()) {
    return CClippedScreenRect();
  }

  CVector2i p1p = ProjectPoint(p1);
  CVector2i p2p = ProjectPoint(p2);

  int minX = std::min(p1p.x, p2p.x);
  int minY = std::min(p1p.y, p2p.y);

  int maxX = abs(p1p.x - p2p.x);
  int maxY = abs(p1p.y - p2p.y);

  int left = minX & ~1;
  if (left >= mViewport.mLeft + mViewport.mWidth) {
    return CClippedScreenRect();
  }

  int right = minX + ((maxX + 2) & ~1);
  if (right <= mViewport.mLeft) {
    return CClippedScreenRect();
  }
  left = std::max(left, mViewport.mLeft) & ~1;
  right = (std::min(right, mViewport.mLeft + mViewport.mWidth) + 1) & ~1;

  int top = minY & ~1;
  if (top >= mViewport.mTop + mViewport.mHeight) {
    return CClippedScreenRect();
  }

  int bottom = minY + ((maxY + 2) & ~1);
  if (bottom <= mViewport.mTop) {
    return CClippedScreenRect();
  }
  top = std::max(top, mViewport.mTop) & ~1;
  bottom = (std::min(bottom, mViewport.mTop + mViewport.mHeight) + 1) & ~1;

  float minV = static_cast<float>(minY - top) / static_cast<float>(bottom - top);
  float maxV = static_cast<float>(maxY + (minY - top) + 1) / static_cast<float>(bottom - top);

  int texAlign = 4;
  switch (fmt) {
  case ETexelFormat::I8:
    texAlign = 8;
    break;
  case ETexelFormat::IA8:
  case ETexelFormat::RGB565:
  case ETexelFormat::RGB5A3:
    texAlign = 4;
    break;
  case ETexelFormat::RGBA8:
    texAlign = 2;
    break;
  default:
    break;
  }

  int texWidth = (texAlign + ((right - left) - 1)) & ~(texAlign - 1);
  float minU = static_cast<float>(minX - left) / static_cast<float>(texWidth);
  float maxU = static_cast<float>(maxX + (minX - left) + 1) / static_cast<float>(texWidth);
  return CClippedScreenRect(left, top, right - left, bottom - top, texWidth, minU, maxU, minV, maxV);
}

void CGraphics::SetViewportResolution(const zeus::CVector2i& res) {
  mRenderModeObj.fbWidth = res.x;
  mRenderModeObj.efbHeight = res.y;
  mRenderModeObj.xfbHeight = res.y;
  SetViewport(0, 0, res.x, res.y);
  if (g_GuiSys)
    g_GuiSys->OnViewportResize();
}

void CGraphics::SetViewport(int left, int bottom, int width, int height) {
  mViewport.mLeft = left;
  mViewport.mTop = mRenderModeObj.efbHeight - (bottom + height);
  mViewport.mWidth = width;
  mViewport.mHeight = height;
  mViewport.mHalfWidth = static_cast<float>(width / 2);
  mViewport.mHalfHeight = static_cast<float>(height / 2);
  GXSetViewport(static_cast<float>(mViewport.mLeft), static_cast<float>(mViewport.mTop),
                static_cast<float>(mViewport.mWidth), static_cast<float>(mViewport.mHeight), mDepthNear, mDepthFar);
}

void CGraphics::SetScissor(int left, int bottom, int width, int height) {
  GXSetScissor(left, mRenderModeObj.efbHeight - (bottom + height), width, height);
}

void CGraphics::SetDepthRange(float near, float far) {
  mDepthNear = near;
  mDepthFar = far;
  GXSetViewport(static_cast<float>(mViewport.mLeft), static_cast<float>(mViewport.mTop),
                static_cast<float>(mViewport.mWidth), static_cast<float>(mViewport.mHeight), mDepthNear, mDepthFar);
}

float CGraphics::GetSecondsMod900() {
  if (mpExternalTimeProvider != nullptr) {
    return mpExternalTimeProvider->GetSecondsMod900();
  }
  return mSecondsMod900;
}

void CGraphics::TickRenderTimings() {
  //OPTICK_EVENT();
  mRenderTimings = (mRenderTimings + 1) % (900 * 60);
  mSecondsMod900 = static_cast<float>(mRenderTimings) / 60.f;
}

void CGraphics::SetUseVideoFilter(bool b) {
  mUseVideoFilter = b;
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, b ? GX_ENABLE : GX_DISABLE, mRenderModeObj.vfilter);
}

void CGraphics::SetClearColor(const CColor& color) {
  mClearColor = color;
  GXSetCopyClear(to_gx_color(color), mClearDepthValue);
}

void CGraphics::SetCopyClear(const CColor& color, float depth) {
  mClearColor = color;
  mClearDepthValue = static_cast<u32>(depth * GX_MAX_Z24);
  GXSetCopyClear(to_gx_color(color), mClearDepthValue);
}

void CGraphics::SetIsBeginSceneClearFb(bool b) { mIsBeginSceneClearFb = b; }

void CGraphics::SetTevOp(ERglTevStage stage, const CTevCombiners::CTevPass& pass) {
  CTevCombiners::SetupPass(stage, pass);
}

#define STREAM_PRIM_BUFFER_SIZE 240

static std::array<Vec, STREAM_PRIM_BUFFER_SIZE + 1> sVtxBuffer;
static std::array<Vec, STREAM_PRIM_BUFFER_SIZE + 1> sNrmBuffer;
static std::array<Vec2, STREAM_PRIM_BUFFER_SIZE + 1> sTxt0Buffer;
static std::array<Vec2, STREAM_PRIM_BUFFER_SIZE + 1> sTxt1Buffer;
static std::array<u32, STREAM_PRIM_BUFFER_SIZE + 1> sClrBuffer;

static const uchar kHasNormals = 1;
static const uchar kHasColor = 2;
static const uchar kHasTexture = 4;

void CGraphics::StreamBegin(ERglPrimitive primitive) {
  vtxBuffer = sVtxBuffer.data();
  nrmBuffer = sNrmBuffer.data();
  txtBuffer0 = sTxt0Buffer.data();
  txtBuffer1 = sTxt1Buffer.data();
  clrBuffer = sClrBuffer.data();
  ResetVertexDataStream(true);
  mCurrentPrimitive = primitive;
  vtxDescr.streamFlags = kHasColor;
}

void CGraphics::StreamNormal(const zeus::CVector3f& nrm) {
  vtxDescr.nrm.x = nrm.x();
  vtxDescr.nrm.y = nrm.y();
  vtxDescr.nrm.z = nrm.z();
  vtxDescr.streamFlags |= kHasNormals;
}

void CGraphics::StreamColor(const zeus::CColor& color) {
  vtxDescr.color = color.toRGBA();
  vtxDescr.streamFlags |= kHasColor;
}

void CGraphics::StreamTexcoord(const zeus::CVector2f& uv) {
  vtxDescr.uv0.x = uv.x();
  vtxDescr.uv0.y = uv.y();
  vtxDescr.streamFlags |= kHasTexture;
  vtxDescr.textureUsed |= 1;
}

void CGraphics::StreamVertex(const zeus::CVector3f& pos) {
  vtxDescr.vtx.x = pos.x();
  vtxDescr.vtx.y = pos.y();
  vtxDescr.vtx.z = pos.z();
  UpdateVertexDataStream();
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
  CGX::SetLineWidth(static_cast<uchar>(w * 6.f), static_cast<GXTexOffset>(offs));
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

void CGraphics::FullRender() {
  CGX::Begin(static_cast<GXPrimitive>(mCurrentPrimitive), GX_VTXFMT0, mNumPrimitives);
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
  case ERglPrimitive::TriangleFan:
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

void CGraphics::DrawPrimitive(ERglPrimitive primitive, const zeus::CVector3f* pos, const zeus::CVector3f& normal,
                              const zeus::CColor& col, s32 numVerts) {
  StreamBegin(primitive);
  StreamNormal(normal);
  StreamColor(col);
  for (u32 i = 0; i < numVerts; ++i) {
    StreamVertex(pos[i]);
  }
  StreamEnd();
}

void CGraphics::SetTevStates(u32 flags) noexcept {
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

  GX::LightMask light = mLightActive;
  GXAttnFn attnFn = GX_AF_NONE;
  if (light.any()) {
    attnFn = GX_AF_SPOT;
  }
  GXDiffuseFn diffFn = GX_DF_NONE;
  if (light.any()) {
    diffFn = GX_DF_CLAMP;
  }
  CGX::SetChanCtrl(CGX::EChannelId::Channel0, light.any() ? GX_ENABLE : GX_DISABLE, GX_SRC_REG,
                   (flags & kHasColor) ? GX_SRC_VTX : GX_SRC_REG, light, diffFn, attnFn);
}

bool CGraphics::Startup() {
  // mpFifo = fifoBase;
  // mpFifoObj = GXInit(fifoBase, fifoSize);
  mpFifoObj = GXInit(nullptr, 0);
  // GXFifoObj fifoObj;
  // GXInitFifoBase(&fifoObj, mpFifo, fifoSize);
  // GXSetCPUFifo(&fifoObj);
  // GXSetGPFifo(&fifoObj);
  // GXInitFifoLimits(mpFifoObj, fifoSize - 0x4000, fifoSize - 0x10000);
  // GXSetCPUFifo(mpFifoObj);
  // GXSetGPFifo(mpFifoObj);
  // GXSetMisc(GX_MT_XF_FLUSH, 8);
  GXSetDither(GX_FALSE);
  CGX::ResetGXStates();
  InitGraphicsVariables();
  ConfigureFrameBuffer(/*osContext*/);
  // for (int i = 0; i < ARRAY_SIZE(mTexRegions); i++) {
  //   GXInitTexCacheRegion(&mTexRegions[i], false, 0x8000 * i, GX_TEXCACHE_32K, 0x80000 + (0x8000 * i),
  //   GX_TEXCACHE_32K);
  // }
  // for (int i = 0; i < ARRAY_SIZE(mTexRegionsCI); i++) {
  //   GXInitTexCacheRegion(&mTexRegionsCI[i], false, (8 + (2 * i)) << 0xF, GX_TEXCACHE_32K, (9 + (2 * i)) << 0xF,
  //                        GX_TEXCACHE_32K);
  // }
  // mGXDefaultTexRegionCallback = GXSetTexRegionCallback(TexRegionCallback);
  mSpareBufferSize = sSpareFrameBuffer.size();
  mpSpareBuffer = sSpareFrameBuffer.data();
  mSpareBufferTexCacheSize = 0x10000;
  return true;
}

#define ARRAY_SIZE(arr) static_cast<int>(sizeof(arr) / sizeof(arr[0]))

void CGraphics::InitGraphicsVariables() {
  for (int i = 0; i < ARRAY_SIZE(mLightTypes); ++i) {
    mLightTypes[i] = ELightType::Directional;
  }
  mLightActive = 0;
  SetDepthWriteMode(false, mDepthFunc, false);
  SetCullMode(ERglCullMode::None);
  SetAmbientColor(CColor(0.2f, 0.2f, 0.2f, 1.f));
  mIsGXModelMatrixIdentity = false;
  SetIdentityViewPointMatrix();
  SetIdentityModelMatrix();
  SetViewport(0, 0, mViewport.mWidth, mViewport.mHeight);
  SetPerspective(60.f, static_cast<float>(mViewport.mWidth) / static_cast<float>(mViewport.mHeight), mProj.GetNear(),
                 mProj.GetFar());
  SetCopyClear(mClearColor, 1.f);
  constexpr GXColor white = {0xFF, 0xFF, 0xFF, 0xFF};
  CGX::SetChanMatColor(CGX::EChannelId::Channel0, white);
  sRenderState.ResetFlushAll();
}

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

void CGraphics::ConfigureFrameBuffer(/*const COsContext& osContext*/) {
  // mRenderModeObj = osContext.GetRenderModeObj();
  // mpFrameBuf1 = osContext.GetFramebuf1();
  // mpFrameBuf2 = osContext.GetFramebuf2();
  // VIConfigure(&mRenderModeObj);
  // VISetNextFrameBuffer(mpFrameBuf1);
  mpCurrenFrameBuf = mpFrameBuf2;
  GXSetViewport(0.f, 0.f, static_cast<float>(mRenderModeObj.fbWidth), static_cast<float>(mRenderModeObj.efbHeight), 0.f,
                1.f);
  GXSetScissor(0, 0, mRenderModeObj.fbWidth, mRenderModeObj.efbHeight);
  GXSetDispCopySrc(0, 0, mRenderModeObj.fbWidth, mRenderModeObj.efbHeight);
  GXSetDispCopyDst(mRenderModeObj.fbWidth, mRenderModeObj.efbHeight);
  GXSetDispCopyYScale(static_cast<float>(mRenderModeObj.xfbHeight) / static_cast<float>(mRenderModeObj.efbHeight));
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, GX_ENABLE, mRenderModeObj.vfilter);
  if (mRenderModeObj.aa) {
    GXSetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
  } else {
    GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  }
  GXSetDispCopyGamma(GX_GM_1_0);
  GXCopyDisp(mpCurrenFrameBuf, true);
  VIFlush();
  // VIWaitForRetrace();
  // VIWaitForRetrace();
  mViewport.mWidth = mRenderModeObj.fbWidth;
  mViewport.mHeight = mRenderModeObj.efbHeight;
  InitGraphicsDefaults();
}

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
    GXAttr attr = static_cast<GXAttr>(GX_VA_TEX0 + i);
    GXSetVtxAttrFmt(GX_VTXFMT0, attr, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, attr, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT2, attr, GX_TEX_ST, GX_F32, 0);
  }
}

void CGraphics::ResetGfxStates() noexcept { sRenderState.Set(0); }

void CGraphics::LoadDolphinSpareTexture(int width, int height, GXTexFmt fmt, void* data, GXTexMapID texId) {
  GXTexObj texObj;
  GXInitTexObj(&texObj, data != nullptr ? data : mpSpareBuffer, width, height, fmt, GX_CLAMP, GX_CLAMP, GX_DISABLE);
  GXInitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);
  GXLoadTexObj(&texObj, texId);
  // CTexture::InvalidateTexmap(texId);
  // if (texId == GX_TEXMAP7) {
  //   GXInvalidateTexRegion(&mTexRegions[0]);
  // }
#ifdef AURORA
  GXDestroyTexObj(&texObj);
#endif
}

void CGraphics::LoadDolphinSpareTexture(int width, int height, GXCITexFmt fmt, GXTlut tlut, void* data,
                                        GXTexMapID texId) {
  GXTexObj texObj;
  GXInitTexObjCI(&texObj, data != nullptr ? data : mpSpareBuffer, width, height, fmt, GX_CLAMP, GX_CLAMP, GX_DISABLE,
                 tlut);
  GXInitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);
  GXLoadTexObj(&texObj, texId);
  // CTexture::InvalidateTexmap(texId);
  // if (texId == GX_TEXMAP7) {
  //   GXInvalidateTexRegion(&mTexRegions[0]);
  // }
#ifdef AURORA
  GXDestroyTexObj(&texObj);
#endif
}

CGraphics::CRenderState::CRenderState() {
  x0_ = 0;
  x4_ = 0;
}

void CGraphics::CRenderState::Flush() {}

int CGraphics::CRenderState::SetVtxState(const float* pos, const float* nrm, const uint* clr) {
  // CGX::SetArray(GX_VA_POS, pos, 12);
  // CGX::SetArray(GX_VA_NRM, nrm, 12);
  // CGX::SetArray(GX_VA_CLR0, clr, 4);
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
    CGX::SetArray(static_cast<GXAttr>(GX_VA_TEX0 + i), std::span<const CVector3f>{});
  }
  Flush();
}
} // namespace metaforce
