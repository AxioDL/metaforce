#include "MetroidPrime/Cameras/CCameraBlurPass.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"

#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "MetaRender/CCubeRenderer.hpp"

#include "rstl/math.hpp"
#include "rstl/optional_object.hpp"

#include "dolphin/gx/GXFrameBuffer.h"
#include "dolphin/gx/GXManage.h"
#include "dolphin/gx/GXTransform.h"
#include "dolphin/gx/GXVert.h"

#include "math.h"
#include "stdlib.h"
#include "string.h"

const float CCameraBlurPass::skXRayViewportScaleX = 0.9f;
const float CCameraBlurPass::skXRayViewportScaleY = 0.9f;

static const char* const skDebugFilterTypeNames[] = {
    "PassThru   ", "Multiply   ", "Invert     ", "Add        ", "Subtract   ",
    "Blend      ", "WideScreen ", "SceneAdd   ", "NoColor    ",
};

static const char* const skDebugFilterShapeNames[] = {
    "FullScreen                      ", "FullScreenHalvesLeftRight       ",
    "FullScreenHalvesTopBottom       ", "FullScreenQuarters              ",
    "CinemaBars                      ", "ScanLinesEven                   ",
    "ScanLinesOdd                    ", "RandomStatic                    ",
    "CookieCutterDepthRandomStatic   ",
};

static const char* const skDebugBlurTypeNames[] = {
    "NoBlur  ",
    "LoBlur  ",
    "HiBlur  ",
    "XRay    ",
};

static const char* const skBlurTextureAssetNames[] = {
    "",
    "",
    "",
    "TXTR_XRayPalette",
};

static const GXTexFmt skBlurFbCopyTextureFormats[] = {
    GX_TF_RGB565,
    GX_TF_RGB565,
    GX_TF_RGB565,
    GX_TF_I8,
};

static const CColor& skIdentityColorMultiply = CColor::White();

CCameraFilterPass::CCameraFilterPass()
: x0_curType(kFT_Passthru)
, x4_nextType(kFT_Passthru)
, x8_shape(kFS_Fullscreen)
, xc_duration(0.f)
, x10_remTime(0.f)
, x14_prevColor(0xFFFFFFFF)
, x18_curColor(0xFFFFFFFF)
, x1c_nextColor(0xFFFFFFFF)
, x20_nextTxtr(kInvalidAssetId) {}

void CCameraFilterPass::SetFilter(const EFilterType type, const EFilterShape shape,
                                  const float time, const CColor& color, const CAssetId txtr) {
  if (time == 0.f) {
    xc_duration = 0.f;
    x10_remTime = 0.f;
    x8_shape = shape;
    x4_nextType = type;
    x0_curType = type;
    x1c_nextColor = color;
    x18_curColor = x1c_nextColor;
    x14_prevColor = x18_curColor;
    x20_nextTxtr = txtr;

    if (x20_nextTxtr != kInvalidAssetId) {
      x24_texObj = rs_new TLockedToken< CTexture >(gpSimplePool->GetObj(SObjectTag('TXTR', txtr)));
    }
  } else {
    x1c_nextColor = color;
    x14_prevColor = x18_curColor;
    x8_shape = shape;
    x20_nextTxtr = txtr;

    if (x20_nextTxtr != kInvalidAssetId) {
      x24_texObj = rs_new TLockedToken< CTexture >(gpSimplePool->GetObj(SObjectTag('TXTR', txtr)));
    }

    x10_remTime = time;
    xc_duration = time;
    x0_curType = x4_nextType;
    x4_nextType = type;

    if (type == kFT_Passthru) {
      if (x0_curType == kFT_Multiply) {
        x1c_nextColor = skIdentityColorMultiply;
      } else if (x0_curType == kFT_Add || x0_curType == kFT_Blend) {
        x1c_nextColor =
            CColor(x1c_nextColor.GetRed(), x1c_nextColor.GetGreen(), x1c_nextColor.GetBlue(), 0.f);
      }
    } else {
      if (x0_curType == kFT_Passthru) {
        if (type == kFT_Multiply) {
          x18_curColor = skIdentityColorMultiply;
        } else if (type == kFT_Add || type == kFT_Blend) {
          x18_curColor = CColor(x1c_nextColor.GetRed(), x1c_nextColor.GetGreen(),
                                x1c_nextColor.GetBlue(), 0.f);
          x14_prevColor = x18_curColor;
        }
      }
      x0_curType = x4_nextType;
    }
  }
}

void CCameraFilterPass::DisableFilter(float time) {
  SetFilter(kFT_Passthru, x8_shape, time, 0xFFFFFFFF, kInvalidAssetId);
}

void CCameraFilterPass::Update(float dt) {
  if (x10_remTime > 0.f) {
    EFilterType origType = x0_curType;

    x10_remTime = rstl::max_val(0.f, x10_remTime - dt);
    x18_curColor = CColor::Lerp(x1c_nextColor, x14_prevColor, x10_remTime / xc_duration);

    if (x10_remTime == 0.f) {
      x0_curType = x4_nextType;
      if (x0_curType == kFT_Passthru) {
        x24_texObj = nullptr;
        x20_nextTxtr = kInvalidAssetId;
      }
    }
  }
}

void CCameraFilterPass::DrawFullScreenColoredQuad(const CColor& color) {
  rstl::pair< CVector2f, CVector2f > vp = gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  const CVector2f& lt = vp.first;
  const CVector2f& rb = vp.second;
  gpRender->SetDepthReadWrite(false, false);
  gpRender->BeginTriangleStrip(4);
  gpRender->PrimColor(color);
  gpRender->PrimVertex(CVector3f(lt.GetX() - 1.f, 0.f, 1.f + rb.GetY()));
  gpRender->PrimVertex(CVector3f(lt.GetX() - 1.f, 0.f, lt.GetY() - 1.f));
  gpRender->PrimVertex(CVector3f(1.f + rb.GetX(), 0.f, 1.f + rb.GetY()));
  gpRender->PrimVertex(CVector3f(1.f + rb.GetX(), 0.f, lt.GetY() - 1.f));
  gpRender->EndPrimitive();
}

void CCameraFilterPass::DrawFullScreenTexturedQuad(const CColor& color, const CTexture* tex,
                                                   float lod) {
  const float u = 0.5f - 0.5f * lod;
  const float v = 0.5f + 0.5f * lod;
  rstl::pair< CVector2f, CVector2f > vp = gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  const CVector2f& lt = vp.first;
  const CVector2f& rb = vp.second;
  gpRender->SetDepthReadWrite(false, false);
  if (tex != nullptr) {
    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  }
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGraphics::StreamBegin(kP_TriangleStrip);
  CGraphics::StreamColor(color);
  CGraphics::StreamTexcoord(u, v);
  CGraphics::StreamVertex(CVector3f(lt.GetX() - 1.f, 0.f, 1.f + rb.GetY()));
  CGraphics::StreamTexcoord(u, u);
  CGraphics::StreamVertex(CVector3f(lt.GetX() - 1.f, 0.f, lt.GetY() - 1.f));
  CGraphics::StreamTexcoord(v, v);
  CGraphics::StreamVertex(CVector3f(1.f + rb.GetX(), 0.f, 1.f + rb.GetY()));
  CGraphics::StreamTexcoord(v, u);
  CGraphics::StreamVertex(CVector3f(1.f + rb.GetX(), 0.f, lt.GetY() - 1.f));
  CGraphics::StreamEnd();
}

void CCameraFilterPass::DrawFullScreenTexturedQuadQuarters(const CColor& color, const CTexture* tex,
                                                           float lod) {
  rstl::pair< CVector2f, CVector2f > vp = gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  const CVector2f& lt = vp.first;
  const CVector2f& rb = vp.second;
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  gpRender->SetDepthReadWrite(false, false);
  if (tex != nullptr) {
    tex->Load(GX_TEXMAP0, CTexture::kCM_Clamp);
  }
  CGraphics::SetCullMode(kCM_None);
  for (int i = 0; i < 4; ++i) {
    float x = (i & 1) > 0 ? 1.f : -1.f;
    float z = (i & 2) > 0 ? 1.f : -1.f;
    gpRender->SetModelMatrix(CTransform4f::Scale(x, 0.f, z));
    CGraphics::StreamBegin(kP_TriangleStrip);
    CGraphics::StreamColor(color);
    CGraphics::StreamTexcoord(lod, lod);
    CGraphics::StreamVertex(CVector3f(lt.GetX(), 0.f, rb.GetY()));
    CGraphics::StreamTexcoord(lod, 0.f);
    CGraphics::StreamVertex(CVector3f(lt.GetX(), 0.f, 0.f));
    CGraphics::StreamTexcoord(0.f, lod);
    CGraphics::StreamVertex(CVector3f(0.f, 0.f, rb.GetY()));
    CGraphics::StreamTexcoord(0.f, 0.f);
    CGraphics::StreamVertex(CVector3f(0.f, 0.f, 0.f));
    CGraphics::StreamEnd();
  }
  CGraphics::SetCullMode(kCM_Front);
}

void CCameraFilterPass::DrawScanLines(const CColor& color, bool even) {
  rstl::pair< CVector2f, CVector2f > vp = gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  const CVector2f& lt = vp.first;
  const CVector2f& rb = vp.second;
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  float offset = even ? 0.f : 2.f;
  int count = static_cast< int >((rb.GetY() - lt.GetY()) / 4.f);
  CGraphics::SetLineWidth(2.f, kTO_One);
  gpRender->BeginLines(count * 2);
  gpRender->PrimColor(color);
  for (int i = 0; i < count; ++i) {
    float fi = 4.f * CCast::ToReal32(i);
    gpRender->PrimVertex(CVector3f(lt.GetX(), 0.f, fi + lt.GetY() + offset));
    gpRender->PrimVertex(CVector3f(rb.GetX(), 0.f, fi + lt.GetY() + offset));
  }
  gpRender->EndPrimitive();
  CGraphics::SetLineWidth(1.f, kTO_One);
}

float CCameraFilterPass::GetT(bool invert) const {
  float tmp;
  if (xc_duration == 0.f) {
    tmp = 1.f;
  } else {
    tmp = 1.f - x10_remTime / xc_duration;
  }
  if (invert) {
    return 1.f - tmp;
  }
  return tmp;
}

void CCameraFilterPass::DrawWideScreen(const CColor& color, const CTexture* tex, float lod) {
  const rstl::pair< CVector2f, CVector2f > vp = gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  const CVector2f& lt = vp.first;
  const CVector2f& rb = vp.second;
  float barHeight =
      -((vp.second.GetX() - vp.first.GetX()) / 16.f * 9.f - (vp.second.GetY() - vp.first.GetY())) *
      0.5f;
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  if (tex != nullptr) {
    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  }
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);

  {
    CGraphics::StreamBegin(kP_TriangleStrip);
    float v = static_cast< float >(rand() % 16384) / 16384.f;
    CGraphics::StreamColor(color);
    CGraphics::StreamTexcoord(v, 1.f);
    CGraphics::StreamVertex(CVector3f(lt.GetX() - 10.f, 0.f, lt.GetY() - -(barHeight * lod)));
    CGraphics::StreamTexcoord(v, 0.f);
    CGraphics::StreamVertex(CVector3f(lt.GetX() - 10.f, 0.f, lt.GetY()));
    CGraphics::StreamTexcoord(1.f + v, 1.f);
    CGraphics::StreamVertex(CVector3f(10.f + rb.GetX(), 0.f, lt.GetY() - -(barHeight * lod)));
    CGraphics::StreamTexcoord(1.f + v, 0.f);
    CGraphics::StreamVertex(CVector3f(10.f + rb.GetX(), 0.f, lt.GetY()));
    CGraphics::StreamEnd();
  }
  {
    CGraphics::StreamBegin(kP_TriangleStrip);
    float v = static_cast< float >(rand() % 16384) / 16384.f;
    CGraphics::StreamColor(color);
    CGraphics::StreamTexcoord(v, 0.f);
    CGraphics::StreamVertex(CVector3f(lt.GetX() - 10.f, 0.f, rb.GetY()));
    CGraphics::StreamTexcoord(v, 1.f);
    CGraphics::StreamVertex(CVector3f(lt.GetX() - 10.f, 0.f, rb.GetY() - (barHeight * lod)));
    CGraphics::StreamTexcoord(1.f + v, 0.f);
    CGraphics::StreamVertex(CVector3f(10.f + rb.GetX(), 0.f, rb.GetY()));
    CGraphics::StreamTexcoord(1.f + v, 1.f);
    CGraphics::StreamVertex(CVector3f(10.f + rb.GetX(), 0.f, rb.GetY() - (barHeight * lod)));
    CGraphics::StreamEnd();
  }
}

void CCameraFilterPass::DrawRandomStatic(const CColor& color, float alpha, bool cookieCutterDepth) {
  rstl::pair< CVector2f, CVector2f > vp = gpRender->SetViewportOrtho(true, 0.f, 1.f);
  const CVector2f& lt = vp.first;
  const CVector2f& rb = vp.second;
#ifdef TARGET_PC
  int size = 0;
#define BASE reinterpret_cast< const uintptr_t >(DVDGetDOLLocation(&size))
#define OFFSET 0x4f60
#define RAND (rand() & 0x7fff)
#else
#define BASE 0
#define OFFSET 0x8000
#define RAND ((rand() + 0x1f) & ~0x1f)
#endif

  if (cookieCutterDepth) {
    CGraphics::SetAlphaCompare(kAF_GEqual, CCast::ToUint8((1.f - alpha) * 255.f), kAO_And,
                               kAF_Always, 0);
    gpRender->SetDepthReadWrite(true, true);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    CGraphics::LoadDolphinSpareTexture(static_cast< int >(2.f + (rb.GetX() - lt.GetX())),
                                       static_cast< int >(2.f + (rb.GetY() - lt.GetY())), GX_TF_IA4,
                                       reinterpret_cast< void* >(BASE + RAND + OFFSET), GX_TEXMAP0);
  } else {
    gpRender->SetDepthReadWrite(false, false);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulateColor);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    CGraphics::LoadDolphinSpareTexture(static_cast< int >(2.f + (rb.GetX() - lt.GetX())),
                                       static_cast< int >(2.f + (rb.GetY() - lt.GetY())), GX_TF_IA4,
                                       reinterpret_cast< void* >(BASE + RAND + OFFSET), GX_TEXMAP0);
  }

  CGraphics::StreamBegin(kP_TriangleStrip);
  CGraphics::StreamColor(color);
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(CVector3f(lt.GetX() - 1.f, 0.01f, 1.f + rb.GetY()));
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(CVector3f(lt.GetX() - 1.f, 0.01f, lt.GetY() - 1.f));
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(CVector3f(1.f + rb.GetX(), 0.01f, 1.f + rb.GetY()));
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(CVector3f(1.f + rb.GetX(), 0.01f, lt.GetY() - 1.f));
  CGraphics::StreamEnd();

  if (cookieCutterDepth) {
    CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  }
}

void CCameraFilterPass::Draw() const {
  float t = GetT(x4_nextType == kFT_Passthru);
  CTexture* tex = x24_texObj.null() ? nullptr : **x24_texObj;
  DrawFilter(x0_curType, x8_shape, x18_curColor, tex, t);
}

void CCameraFilterPass::DrawFilter(EFilterType type, EFilterShape shape, const CColor& color,
                                   const CTexture* tex, float lod) {
  if (type == kFT_Passthru) {
    return;
  }
  switch (type) {
  case kFT_Passthru:
  default:
    return;
  case kFT_Multiply:
    gpRender->SetBlendMode_ColorMultiply();
    break;
  case kFT_Invert:
    gpRender->SetBlendMode_InvertDst();
    break;
  case kFT_Add:
    gpRender->SetBlendMode_AdditiveAlpha();
    break;
  case kFT_Blend:
    gpRender->SetBlendMode_AlphaBlended();
    break;
  case kFT_Subtract:
    CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
    break;
  case kFT_Widescreen:
    return;
  case kFT_SceneAdd:
    gpRender->SetBlendMode_AdditiveDestColor();
    break;
  case kFT_NoColor:
    gpRender->SetBlendMode_NoColorWrite();
    break;
  }
  DrawFilterShape(shape, color, tex, lod);
  gpRender->SetBlendMode_AlphaBlended();
}

void CCameraFilterPass::DrawFilterShape(EFilterShape shape, const CColor& color,
                                        const CTexture* tex, float lod) {
  switch (shape) {
  case kFS_ScanLinesEven:
    DrawScanLines(color, true);
    break;
  case kFS_ScanLinesOdd:
    DrawScanLines(color, false);
    break;
  case kFS_Fullscreen:
  case kFS_FullscreenHalvesLeftRight:
  case kFS_FullscreenHalvesTopBottom:
  default:
    if (tex != nullptr) {
      DrawFullScreenTexturedQuad(color, tex, lod);
    } else {
      DrawFullScreenColoredQuad(color);
    }
    break;
  case kFS_FullscreenQuarters:
    if (tex != nullptr) {
      DrawFullScreenTexturedQuadQuarters(color, tex, lod);
    } else {
      DrawFullScreenColoredQuad(color);
    }
    break;
  case kFS_CinemaBars:
    DrawWideScreen(color, tex, lod);
    break;
  case kFS_RandomStatic:
    DrawRandomStatic(color, 1.f, false);
    break;
  case kFS_CookieCutterDepthRandomStatic:
    DrawRandomStatic(color, lod, true);
    break;
  }
}

// CCameraBlurPass functions

CCameraBlurPass::CCameraBlurPass()
: x0_paletteTex(rstl::optional_object_null())
, x10_curType(kBT_NoBlur)
, x14_endType(kBT_NoBlur)
, x18_endValue(0.f)
, x1c_curValue(0.f)
, x20_startValue(0.f)
, x24_totalTime(0.f)
, x28_remainingTime(0.f)
, x2c_usePersistent(false)
, x2d_noPersistentCopy(false)
, x30_persistentBuf(0) {}

void CCameraBlurPass::Update(float dt) {
  if (x28_remainingTime > 0.f) {
    float remaining = x28_remainingTime - dt;
    x28_remainingTime = rstl::max_val(0.f, remaining);

    float t = x28_remainingTime / x24_totalTime;
    x1c_curValue = x20_startValue * (1.f - t) + x18_endValue * t;

    if (x28_remainingTime == 0.f) {
      if (x14_endType == kBT_NoBlur && x10_curType != kBT_NoBlur && x2c_usePersistent) {
        FreePersistentFbTexture();
      }

      x0_paletteTex = rstl::optional_object_null();

      const char* texName = skBlurTextureAssetNames[x14_endType];
      if (strcmp(texName, "") != 0) {
        x0_paletteTex = TToken< CTexture >(gpSimplePool->GetObj(texName));
      }

      x10_curType = x14_endType;
    }
  }
}

void CCameraBlurPass::SetBlur(EBlurType type, float amount, float duration, bool usePersistentFb) {
  if (duration == 0.f) {
    x28_remainingTime = 0.f;
    x24_totalTime = 0.f;
    x20_startValue = amount;
    x1c_curValue = amount;
    x18_endValue = amount;

    if (x10_curType == kBT_NoBlur) {
      if (type != kBT_NoBlur) {
        if (usePersistentFb) {
          AllocatePersistentFbTexture();
        }
        x0_paletteTex = rstl::optional_object_null();
        const char* texName = skBlurTextureAssetNames[type];
        if (strcmp(texName, "") != 0) {
          x0_paletteTex =
              TCachedToken< CTexture >(TToken< CTexture >(gpSimplePool->GetObj(texName)));
          x0_paletteTex->Lock();
        }
      }
    } else if (type == kBT_NoBlur) {
      if (x2c_usePersistent) {
        FreePersistentFbTexture();
      }
      x0_paletteTex = rstl::optional_object_null();
    }

    x14_endType = type;
    x10_curType = type;
    x2c_usePersistent = usePersistentFb;
  } else {
    x2c_usePersistent = usePersistentFb;
    x24_totalTime = duration;
    x28_remainingTime = duration;
    x18_endValue = x1c_curValue;
    x20_startValue = amount;

    if (type != x14_endType) {
      if (x10_curType == kBT_NoBlur) {
        if (x2c_usePersistent) {
          AllocatePersistentFbTexture();
        }
        if (strcmp(skBlurTextureAssetNames[x14_endType], "") != 0) {
          x0_paletteTex =
              TToken< CTexture >(gpSimplePool->GetObj(skBlurTextureAssetNames[x14_endType]));
          x0_paletteTex->Lock();
        }
        x10_curType = type;
      }
      x14_endType = type;
    }
  }
}

void CCameraBlurPass::DisableBlur(float duration) {
  SetBlur(kBT_NoBlur, 0.f, duration, x2c_usePersistent);
}

static inline float get_stretched_t(float t, float linearWeight) {
  float r = 2.f * (t - 0.5f);
  float rcube = r * (r * r);
  return (1.f - linearWeight) * ((1.f + rcube) / 2.f) + linearWeight * t;
}

void CCameraBlurPass::Draw() const {
  if (x10_curType == kBT_NoBlur)
    return;

  uchar* data;
  u32 texMtxId;
  GXTexFmt fmt = skBlurFbCopyTextureFormats[x10_curType];
  if (x2c_usePersistent) {
    data = static_cast< uchar* >(x30_persistentBuf);
  } else {
    data = static_cast< uchar* >(CGraphics::GetDolphinSpareBuffer());
  }

  ushort width = 640;
  ushort height = 448;
  if (x10_curType != kBT_XRay) {
    width >>= 1;
    height >>= 1;
  }

  if (!x2d_noPersistentCopy || !x2c_usePersistent) {
    GetFbCopy(fmt, data);
  }

  if (x10_curType == kBT_XRay) {
    const_cast< TCachedToken< CTexture >& >(*x0_paletteTex).TryCache();
    CTexture* paletteTex = x0_paletteTex->GetObject();
    if (paletteTex != nullptr) {
      paletteTex->GetPalette()->Load();
    } else {
      return;
    }
    CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_C8, GX_TLUT0, data,
                                       CGraphics::kSpareBufferTexMapID);
  } else {
    CGraphics::LoadDolphinSpareTexture(width, height, fmt, data, CGraphics::kSpareBufferTexMapID);
  }

  // TODO: demo map shows CGraphics::GetViewport(int&, int&, int&, int&)
  const CViewport& viewport = CGraphics::GetViewport();
  float vWidth = static_cast< float >(viewport.mWidth);
  float vHeight = static_cast< float >(viewport.mHeight);
  float vLeft = static_cast< float >(viewport.mLeft);
  float vTop = static_cast< float >(viewport.mTop);
  float right = vLeft + vWidth;
  float bottom = vTop + vHeight;

  CGraphics::SetOrtho(vLeft, right, bottom, vTop, -1.f, 1.f);
  gpRender->SetDepthReadWrite(false, false);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  gpRender->SetModelMatrix(CTransform4f::Identity());

  if (x10_curType == kBT_XRay) {
    gpRender->SetBlendMode_Replace();

    const GXVtxDescList xrayVtxDescList[] = {
        {GX_VA_POS, GX_DIRECT},
        {GX_VA_TEX0, GX_DIRECT},
        {GX_VA_NULL, GX_NONE},
    };
    CGX::SetVtxDescv(xrayVtxDescList);
    CGX::SetNumChans(0);
    CGX::SetNumTexGens(4);
    CGX::SetNumTevStages(4);

    for (GXTevStageID stage = GX_TEVSTAGE0; stage < GX_TEVSTAGE4;
         stage = static_cast< GXTevStageID >(stage + 1)) {
      CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
      CGX::SetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
      CGX::SetTevKAlphaSel(stage, GX_TEV_KASEL_8_8);
      CGX::SetStandardTevColorAlphaOp(stage);
      CGX::SetTevKColorSel(stage, GX_TEV_KCSEL_K0);
    }
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
    CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);

    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD2, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD3, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD4, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD5, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE6, GX_TEXCOORD6, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE7, GX_TEXCOORD7, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);

    CColor xrayKColor(0.25f, 0.25f, 0.25f, 1.f);
    CGX::SetTevKColor(GX_KCOLOR0, xrayKColor.GetGXColor());

    float tweakLinear = gpTweakGui->GetXrayBlurScaleLinear() / 4.f;
    float tweakQuad = gpTweakGui->GetXrayBlurScaleQuadratic() / 4.f;
    float linearCoeff = x1c_curValue * tweakLinear;
    float quadCoeff = x1c_curValue * tweakQuad;

    for (int i = 0; i < 4; ++i) {
      float val = linearCoeff * static_cast< float >(i) +
                  quadCoeff * static_cast< float >(i) * static_cast< float >(i);
      float amplitude = 1.0f - val;
      float mtx[2][4] = {
          {amplitude, 0.0f, 0.0f, -0.5f * amplitude + 0.5f},
          {0.0f, amplitude, 0.0f, -0.5f * amplitude + 0.5f},
      };
      GXLoadTexMtxImm(mtx, GX_TEXMTX0 + i * 3, GX_MTX2x4);
    }

    CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX3, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD4, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX4, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD5, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX5, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD6, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX6, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD7, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX7, false, GX_PTIDENTITY);

    const float step = 1.0f / 6.0f;
    const float uStep = 1.0f / 7.0f;

    for (float v = 0.f; v < 1.f; v += step) {
      float posYBot = vHeight * get_stretched_t(1.0f - v - step, 0.9f) + vTop;
      float posYTop = vHeight * get_stretched_t(1.0f - v, 0.9f) + vTop;
      float texVTop = 0.9f * (v - 0.5f) + 0.5f;
      float texVBot = 0.9f * (v + step - 0.5f) + 0.5f;

      for (float u = 0.f; u < 1.f; u += uStep) {
        float posXLeft = vWidth * get_stretched_t(u, 0.9f) + vLeft;
        float posXRight = vWidth * get_stretched_t(uStep + u, 0.9f) + vLeft;
        float texULeft = 0.9f * (u - 0.5f) + 0.5f;
        float texURight = 0.9f * (uStep + u - 0.5f) + 0.5f;

        CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
        GXPosition3f32(posXLeft, 0.f, posYTop);
        GXTexCoord2f32(texULeft, texVTop);
        GXPosition3f32(posXLeft, 0.f, posYBot);
        GXTexCoord2f32(texULeft, texVBot);
        GXPosition3f32(posXRight, 0.f, posYTop);
        GXTexCoord2f32(texURight, texVTop);
        GXPosition3f32(posXRight, 0.f, posYBot);
        GXTexCoord2f32(texURight, texVBot);
        CGX::End();
      }
    }

    CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX2, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX3, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD4, GX_TG_MTX2x4, GX_TG_TEX4, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD5, GX_TG_MTX2x4, GX_TG_TEX5, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD6, GX_TG_MTX2x4, GX_TG_TEX6, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD7, GX_TG_MTX2x4, GX_TG_TEX7, GX_IDENTITY, false, GX_PTIDENTITY);
  } else {
    gpRender->SetBlendMode_AlphaBlended();

    const GXVtxDescList blurVtxDescList[] = {
        {GX_VA_POS, GX_DIRECT},
        {GX_VA_TEX0, GX_DIRECT},
        {GX_VA_NULL, GX_NONE},
    };
    CGX::SetVtxDescv(blurVtxDescList);
    CGX::SetNumChans(0);
    CGX::SetNumTexGens(7);
    CGX::SetNumTevStages(7);

    for (GXTevStageID stage = GX_TEVSTAGE0; stage < GX_TEVSTAGE7;
         stage = static_cast< GXTevStageID >(stage + 1)) {
      CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
      CGX::SetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
      CGX::SetStandardTevColorAlphaOp(stage);
      CGX::SetTevKColorSel(stage, GX_TEV_KCSEL_K0);
    }

    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
    CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);

    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD2, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD3, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD4, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD5, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE6, GX_TEXCOORD6, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE7, GX_TEXCOORD7, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);

    float alpha;
    if (x2c_usePersistent) {
      alpha = 1.f;
    } else {
      alpha = rstl::min_val(1.f, 0.5f * x1c_curValue);
    }
    CColor blurKColor(1.f / 7.f, 1.f / 7.f, 1.f / 7.f, alpha);
    CGX::SetTevKColor(GX_KCOLOR0, blurKColor.GetGXColor());

    int i = 0;
    texMtxId = GX_TEXMTX0;
    for (; i < 7; ++i) {
      double offsetX;
      if (i == 0) {
        offsetX = 0.0;
      } else {
        float angle = 6.2831855f * static_cast< float >(i - 1) / 6.f;
        offsetX = (x1c_curValue / 640.f) * cos(angle);
      }
      float fOffsetX = static_cast< float >(offsetX);
      double offsetY;
      if (i == 0) {
        offsetY = 0.0;
      } else {
        float angle = 6.2831855f * static_cast< float >(i - 1) / 6.f;
        offsetY = (x1c_curValue / 448.f) * sin(angle);
      }
      float fOffsetY = static_cast< float >(offsetY);
      float mtx[2][4] = {
          {1.0f, 0.0f, 0.0f, fOffsetX},
          {0.0f, 1.0f, 0.0f, fOffsetY},
      };
      GXLoadTexMtxImm(mtx, texMtxId, GX_MTX2x4);
      texMtxId += 3;
    }

    CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX3, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD4, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX4, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD5, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX5, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD6, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX6, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD7, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX7, false, GX_PTIDENTITY);

    CGraphics::LoadDolphinSpareTexture(width, height, fmt, data, GX_TEXMAP0);

    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
    GXPosition3f32(vLeft - 1.f, 0.f, bottom + 1.f);
    GXTexCoord2f32(0.f, 0.f);
    GXPosition3f32(vLeft - 1.f, 0.f, vTop - 1.f);
    GXTexCoord2f32(0.f, 1.f);
    GXPosition3f32(right + 1.f, 0.f, bottom + 1.f);
    GXTexCoord2f32(1.f, 0.f);
    GXPosition3f32(right + 1.f, 0.f, vTop - 1.f);
    GXTexCoord2f32(1.f, 1.f);
    CGX::End();

    CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX2, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX3, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD4, GX_TG_MTX2x4, GX_TG_TEX4, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD5, GX_TG_MTX2x4, GX_TG_TEX5, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD6, GX_TG_MTX2x4, GX_TG_TEX6, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD7, GX_TG_MTX2x4, GX_TG_TEX7, GX_IDENTITY, false, GX_PTIDENTITY);
  }

  gpRender->SetBlendMode_AlphaBlended();
  gpRender->SetDepthReadWrite(true, true);
}

void CCameraBlurPass::GetFbCopy(GXTexFmt fmt, uchar* buf) const {
  GXSetTexCopySrc(static_cast< u16 >(CGraphics::GetViewport().mLeft),
                  static_cast< u16 >(CGraphics::GetViewport().mTop), 640, 448);
  if (fmt == GX_TF_RGB565) {
    GXGetTexBufferSize(320, 224, GX_TF_RGB565, GX_FALSE, 0);
  } else {
    GXGetTexBufferSize(640, 448, GX_TF_I8, GX_FALSE, 0);
  }
  if (fmt == GX_TF_RGB565) {
    GXSetTexCopyDst(320, 224, fmt, GX_TRUE);
  } else {
    GXSetTexCopyDst(640, 448, fmt, GX_FALSE);
  }
  GXCopyTex(buf, GX_FALSE);
  GXPixModeSync();
  x2d_noPersistentCopy = true;
}

void CCameraBlurPass::AllocatePersistentFbTexture() {
  GXGetTexBufferSize(320, 224, GX_TF_RGB565, GX_FALSE, 0);
  x30_persistentBuf = CGraphics::GetDolphinSpareBuffer();
}

void CCameraBlurPass::FreePersistentFbTexture() { x2d_noPersistentCopy = false; }
