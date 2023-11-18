#include "Runtime/Camera/CCameraFilter.hpp"

#include "Runtime/CDvdFile.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/Graphics/CGX.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include <algorithm>
#include <zeus/CColor.hpp>

namespace metaforce {

void CCameraFilterPass::Update(float dt) {
  if (x10_remTime <= 0.f)
    return;

  EFilterType origType = x0_curType;

  x10_remTime = std::max(0.f, x10_remTime - dt);
  x18_curColor = zeus::CColor::lerp(x1c_nextColor, x14_prevColor, x10_remTime / xc_duration);

  if (x10_remTime == 0.f) {
    x0_curType = x4_nextType;
    if (x0_curType == EFilterType::Passthru) {
      x24_texObj = TLockedToken<CTexture>();
      x20_nextTxtr = {};
    }
  }
}

void CCameraFilterPass::SetFilter(EFilterType type, EFilterShape shape, float time, const zeus::CColor& color,
                                  CAssetId txtr) {
  if (time == 0.f) {
    xc_duration = 0.f;
    x10_remTime = 0.f;

    if (txtr.IsValid())
      x24_texObj = g_SimplePool->GetObj({FOURCC('TXTR'), txtr});

    x4_nextType = type;
    x0_curType = type;
    x8_shape = shape;
    x1c_nextColor = color;
    x18_curColor = color;
    x14_prevColor = color;
    x20_nextTxtr = txtr;
  } else {
    EFilterType origType = x0_curType;
    CAssetId origTxtr = x20_nextTxtr;

    x1c_nextColor = color;
    x14_prevColor = x18_curColor;
    x8_shape = shape;
    x20_nextTxtr = txtr;
    if (txtr.IsValid())
      x24_texObj = g_SimplePool->GetObj({FOURCC('TXTR'), txtr});
    x10_remTime = time;
    xc_duration = time;
    x0_curType = x4_nextType;
    x4_nextType = type;
    if (type == EFilterType::Passthru) {
      if (x0_curType == EFilterType::Multiply)
        x1c_nextColor = zeus::skWhite;
      else if (x0_curType == EFilterType::Add || x0_curType == EFilterType::Blend)
        x1c_nextColor.a() = 0.f;
    } else {
      if (x0_curType == EFilterType::Passthru) {
        if (type == EFilterType::Multiply) {
          x18_curColor = zeus::skWhite;
        } else if (type == EFilterType::Add || type == EFilterType::Blend) {
          x18_curColor = x1c_nextColor;
          x18_curColor.a() = 0.f;
          x14_prevColor = x18_curColor;
        }
      }
      x0_curType = x4_nextType;
    }
  }
}

void CCameraFilterPass::DisableFilter(float time) {
  SetFilter(EFilterType::Passthru, x8_shape, time, zeus::skWhite, {});
}

void CCameraFilterPass::Draw() {
  DrawFilter(x0_curType, x8_shape, x18_curColor, x24_texObj.GetObj(), GetT(x4_nextType == EFilterType::Passthru));
}

float CCameraFilterPass::GetT(bool invert) const {
  float tmp;
  if (xc_duration == 0.f)
    tmp = 1.f;
  else
    tmp = 1.f - x10_remTime / xc_duration;
  if (invert)
    return 1.f - tmp;
  return tmp;
}

void CCameraFilterPass::DrawFilter(EFilterType type, EFilterShape shape, const zeus::CColor& color, CTexture* tex,
                                   float lod) {
  switch (type) {
  case EFilterType::Multiply:
    g_Renderer->SetBlendMode_ColorMultiply();
    break;
  case EFilterType::Invert:
    g_Renderer->SetBlendMode_InvertDst();
    break;
  case EFilterType::Add:
    g_Renderer->SetBlendMode_AdditiveAlpha();
    break;
  case EFilterType::Subtract:
    CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
    break;
  case EFilterType::Blend:
    g_Renderer->SetBlendMode_AlphaBlended();
    break;
  case EFilterType::Widescreen:
    return;
  case EFilterType::SceneAdd:
    g_Renderer->SetBlendMode_AdditiveDestColor();
    break;
  case EFilterType::NoColor:
    g_Renderer->SetBlendMode_NoColorWrite();
    break;
  default:
    return;
  }
  DrawFilterShape(shape, color, tex, lod);
  g_Renderer->SetBlendMode_AlphaBlended();
}

void CCameraFilterPass::DrawFullScreenColoredQuad(const zeus::CColor& color) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CCameraFilterPass::DrawFullScreenColoredQuad", zeus::skBlue);
  const auto [lt, rb] = g_Renderer->SetViewportOrtho(true, -4096.f, 4096.f);
  g_Renderer->SetDepthReadWrite(false, false);
  g_Renderer->BeginTriangleStrip(4);
  g_Renderer->PrimColor(color);
  g_Renderer->PrimVertex({lt.x() - 1.f, 0.f, 1.f + rb.y()});
  g_Renderer->PrimVertex({lt.x() - 1.f, 0.f, lt.y() - 1.f});
  g_Renderer->PrimVertex({1.f + rb.x(), 0.f, 1.f + rb.y()});
  g_Renderer->PrimVertex({1.f + rb.x(), 0.f, lt.y() - 1.f});
  g_Renderer->EndPrimitive();
}

void CCameraFilterPass::DrawFilterShape(EFilterShape shape, const zeus::CColor& color, CTexture* tex, float lod) {
  switch (shape) {
  default:
    if (tex == nullptr) {
      DrawFullScreenColoredQuad(color);
    } else {
      DrawFullScreenTexturedQuad(color, tex, lod);
    }
    break;
  case EFilterShape::FullscreenQuarters:
    if (tex == nullptr) {
      DrawFullScreenColoredQuad(color);
    } else {
      DrawFullScreenTexturedQuadQuarters(color, tex, lod);
    }
    break;
  case EFilterShape::CinemaBars:
    DrawWideScreen(color, tex, lod);
    break;
  case EFilterShape::ScanLinesEven:
    DrawScanLines(color, true);
    break;
  case EFilterShape::ScanLinesOdd:
    DrawScanLines(color, false);
    break;
  case EFilterShape::RandomStatic:
    DrawRandomStatic(color, 1.f, false);
    break;
  case EFilterShape::CookieCutterDepthRandomStatic:
    DrawRandomStatic(color, lod, true);
    break;
  }
}

void CCameraFilterPass::DrawFullScreenTexturedQuadQuarters(const zeus::CColor& color, CTexture* tex, float lod) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CCameraFilterPass::DrawFullScreenTexturedQuadQuarters", zeus::skBlue);
  const auto [lt, rb] = g_Renderer->SetViewportOrtho(true, -4096.f, 4096.f);
  CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::kEnvModulate);
  CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::kEnvPassthru);
  g_Renderer->SetDepthReadWrite(false, false);
  if (tex != nullptr) {
    tex->Load(GX_TEXMAP0, EClampMode::Repeat);
  }
  CGraphics::SetCullMode(ERglCullMode::None);
  for (int i = 0; i < 4; ++i) {
    g_Renderer->SetModelMatrix(zeus::CTransform::Scale((i & 1) != 0 ? 1.f : -1.f, 0.f, (i & 2) != 0 ? 1.f : -1.f));
    CGraphics::StreamBegin(GX_TRIANGLESTRIP);
    CGraphics::StreamColor(color);
    CGraphics::StreamTexcoord(lod, lod);
    CGraphics::StreamVertex(lt.x(), 0.f, rb.y());
    CGraphics::StreamTexcoord(lod, 0.f);
    CGraphics::StreamVertex(lt.x(), 0.f, 0.f);
    CGraphics::StreamTexcoord(0.f, lod);
    CGraphics::StreamVertex(0.f, 0.f, rb.y());
    CGraphics::StreamTexcoord(0.f, 0.f);
    CGraphics::StreamVertex(0.f, 0.f, 0.f);
    CGraphics::StreamEnd();
  }
  CGraphics::SetCullMode(ERglCullMode::Front);
}

void CCameraFilterPass::DrawFullScreenTexturedQuad(const zeus::CColor& color, CTexture* tex, float lod) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CCameraFilterPass::DrawFullScreenTexturedQuad", zeus::skBlue);
  const float u = 0.5f - 0.5f * lod;
  const float v = 0.5f + 0.5f * lod;
  const auto [lt, rb] = g_Renderer->SetViewportOrtho(true, -4096.f, 4096.f);
  g_Renderer->SetDepthReadWrite(false, false);
  if (tex != nullptr) {
    tex->Load(GX_TEXMAP0, EClampMode::Repeat);
  }
  CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::kEnvModulate);
  CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::kEnvPassthru);
  CGraphics::StreamBegin(GX_TRIANGLESTRIP);
  CGraphics::StreamColor(color);
  CGraphics::StreamTexcoord(u, v);
  CGraphics::StreamVertex(lt.x() - 1.f, 0.f, 1.f + rb.y());
  CGraphics::StreamTexcoord(u, u);
  CGraphics::StreamVertex(lt.x() - 1.f, 0.f, lt.y() - 1.f);
  CGraphics::StreamTexcoord(v, v);
  CGraphics::StreamVertex(1.f + rb.x(), 0.f, 1.f + rb.y());
  CGraphics::StreamTexcoord(v, u);
  CGraphics::StreamVertex(1.f + rb.x(), 0.f, lt.y() - 1.f);
  CGraphics::StreamEnd();
}

void CCameraFilterPass::DrawRandomStatic(const zeus::CColor& color, float alpha, bool cookieCutterDepth) {
  // TODO this shouldn't be here
  static CTexture m_randomStatic{ETexelFormat::IA4, 640, 448, 1, "Camera Random Static"};

  SCOPED_GRAPHICS_DEBUG_GROUP("CCameraFilterPass::DrawRandomStatic", zeus::skBlue);
  const auto [lb, rt] = g_Renderer->SetViewportOrtho(true, 0.f, 1.f);
  if (cookieCutterDepth) {
    CGraphics::SetAlphaCompare(ERglAlphaFunc::GEqual, static_cast<u8>((1.f - alpha) * 255.f), ERglAlphaOp::And,
                               ERglAlphaFunc::Always, 0);
    g_Renderer->SetDepthReadWrite(true, true);
    CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::kEnvModulate);
    CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::kEnvPassthru);
  } else {
    g_Renderer->SetDepthReadWrite(false, false);
    CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::kEnvModulateColor);
    CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::kEnvPassthru);
  }

  // Upload random static texture (game reads from .text)
  const u8* buf = CDvdFile::GetDolBuf() + 0x4f60;
  u8* out = m_randomStatic.Lock();
  memcpy(out, buf + ROUND_UP_32(rand() & 0x7fff), m_randomStatic.GetMemoryAllocated());
  m_randomStatic.UnLock();
  m_randomStatic.Load(GX_TEXMAP0, EClampMode::Clamp);

  CGraphics::StreamBegin(GX_TRIANGLESTRIP);
  CGraphics::StreamColor(color);
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(lb.x() - 1.f, 0.01f, rt.y() + 1.f);
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(lb.x() - 1.f, 0.01f, lb.y() - 1.f);
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(rt.x() + 1.f, 0.01f, rt.y() + 1.f);
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(rt.x() + 1.f, 0.01f, lb.y() - 1.f);
  CGraphics::StreamEnd();
  if (cookieCutterDepth) {
    CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);
  }
}

void CCameraFilterPass::DrawScanLines(const zeus::CColor& color, bool even) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CCameraFilterPass::DrawScanLines", zeus::skBlue);
  const auto [lt, rb] = g_Renderer->SetViewportOrtho(true, -4096.f, 4096.f);
  g_Renderer->SetDepthReadWrite(false, false);
  g_Renderer->SetModelMatrix({});
  // CGraphics::SetLineWidth(2.f, 5);
  // g_Renderer->BeginLines(...);
  // TODO
  // CGraphics::SetLineWidth(1.f, 5);
}

void CCameraFilterPass::DrawWideScreen(const zeus::CColor& color, CTexture* tex, float lod) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CCameraFilterPass::DrawWideScreen", zeus::skBlue);
  //  const auto vp = g_Renderer->SetViewportOrtho(true, -4096.f, 4096.f);
  //  float f = -((vp.second.x() - vp.first.x()) * 0.0625f * 9.f - (vp.second.y() - vp.first.y())) * 0.5f;
  //  g_Renderer->SetDepthReadWrite(false, false);
  //  g_Renderer->SetModelMatrix({});
  //  if (tex != nullptr) {
  //    tex->Load(GX_TEXMAP0, EClampMode::Repeat);
  //  }
  //  CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::sTevPass805a5ebc);
  //  CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::skPassThru);
  //  CGraphics::StreamBegin(GX_TRIANGLESTRIP);
  //  float x = rand() % 4000;
}

void CCameraBlurPass::Draw(bool clearDepth) {
  if (x10_curType == EBlurType::NoBlur)
    return;

  // TODO
  //  if (x10_curType == EBlurType::Xray) {
  //    if (!m_xrayShader)
  //      m_xrayShader.emplace(x0_paletteTex);
  //    m_xrayShader->draw(x1c_curValue);
  //  } else {
  //    if (!m_shader)
  //      m_shader.emplace();
  //    m_shader->draw(x1c_curValue, clearDepth);
  //    if (clearDepth)
  //      CGraphics::SetDepthRange(DEPTH_NEAR, DEPTH_FAR);
  //  }
}

void CCameraBlurPass::Update(float dt) {
  if (x28_remainingTime > 0.f) {
    x28_remainingTime = std::max(x28_remainingTime - dt, 0.f);
    x1c_curValue = x18_endValue + (x20_startValue - x18_endValue) * x28_remainingTime / x24_totalTime;

    if (x28_remainingTime != 0.f)
      return;

    x10_curType = x14_endType;
  }
}

void CCameraBlurPass::SetBlur(EBlurType type, float amount, float duration, bool usePersistentFb) {
  // TODO impl usePersistentFb
  if (duration == 0.f) {
    x24_totalTime = 0.f;
    x28_remainingTime = 0.f;
    x18_endValue = amount;
    x1c_curValue = amount;
    x20_startValue = amount;

    if (x10_curType == EBlurType::NoBlur) {
      if (type == EBlurType::Xray)
        x0_paletteTex = g_SimplePool->GetObj("TXTR_XRayPalette");
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
      if (x10_curType == EBlurType::NoBlur) {
        if (type == EBlurType::Xray)
          x0_paletteTex = g_SimplePool->GetObj("TXTR_XRayPalette");
        x10_curType = type;
      }
      x14_endType = type;
    }
  }
}

void CCameraBlurPass::DisableBlur(float duration) { SetBlur(EBlurType::NoBlur, 0.f, duration, x2c_usePersistent); }

} // namespace metaforce
