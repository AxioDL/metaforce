#include "Graphics/CGraphics.hpp"
#include "CCameraFilter.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "CSimplePool.hpp"
#include "Graphics/Shaders/CColoredQuadFilter.hpp"
#include "Graphics/Shaders/CTexturedQuadFilter.hpp"
#include "Graphics/Shaders/CScanLinesFilter.hpp"
#include "Graphics/Shaders/CRandomStaticFilter.hpp"

namespace urde {

template <class S>
void CCameraFilterPass<S>::Update(float dt) {
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

  if (x0_curType == EFilterType::Passthru)
    m_shader = rstl::nullopt;
  else if (x0_curType != origType)
    m_shader.emplace(x0_curType, x24_texObj);
}

template <class S>
void CCameraFilterPass<S>::SetFilter(EFilterType type, EFilterShape shape, float time, const zeus::CColor& color,
                                     CAssetId txtr) {
  if (time == 0.f) {
    xc_duration = 0.f;
    x10_remTime = 0.f;

    if (txtr.IsValid())
      x24_texObj = g_SimplePool->GetObj({FOURCC('TXTR'), txtr});
    if (type == EFilterType::Passthru)
      m_shader = rstl::nullopt;
    else if (x0_curType != type || (x20_nextTxtr != txtr && txtr.IsValid()))
      m_shader.emplace(type, x24_texObj);

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
        x1c_nextColor = zeus::CColor::skWhite;
      else if (x0_curType == EFilterType::Add || x0_curType == EFilterType::Blend)
        x1c_nextColor.a() = 0.f;
    } else {
      if (x0_curType == EFilterType::Passthru) {
        if (type == EFilterType::Multiply) {
          x18_curColor = zeus::CColor::skWhite;
        } else if (type == EFilterType::Add || type == EFilterType::Blend) {
          x18_curColor = x1c_nextColor;
          x18_curColor.a() = 0.f;
          x14_prevColor = x18_curColor;
        }
      }
      x0_curType = x4_nextType;
    }

    if (x0_curType == EFilterType::Passthru)
      m_shader = rstl::nullopt;
    else if (x0_curType != origType || (x20_nextTxtr != origTxtr && x20_nextTxtr.IsValid()))
      m_shader.emplace(x0_curType, x24_texObj);
  }
}

template <class S>
void CCameraFilterPass<S>::DisableFilter(float time) {
  SetFilter(EFilterType::Passthru, x8_shape, time, zeus::CColor::skWhite, -1);
}

template <class S>
void CCameraFilterPass<S>::Draw() const {
  if (m_shader)
    const_cast<S&>(*m_shader).DrawFilter(x8_shape, x18_curColor, GetT(x4_nextType == EFilterType::Passthru));
}

float CCameraFilterPassBase::GetT(bool invert) const {
  float tmp;
  if (xc_duration == 0.f)
    tmp = 1.f;
  else
    tmp = 1.f - x10_remTime / xc_duration;
  if (invert)
    return 1.f - tmp;
  return tmp;
}

void CCameraFilterPassPoly::SetFilter(EFilterType type, EFilterShape shape, float time, const zeus::CColor& color,
                                      CAssetId txtr) {
  if (!m_filter || m_shape != shape) {
    m_shape = shape;
    switch (shape) {
    case EFilterShape::Fullscreen:
    case EFilterShape::FullscreenHalvesLeftRight:
    case EFilterShape::FullscreenHalvesTopBottom:
    case EFilterShape::FullscreenQuarters:
      if (txtr.IsValid())
        m_filter = std::make_unique<CCameraFilterPass<CTexturedQuadFilterAlpha>>();
      else
        m_filter = std::make_unique<CCameraFilterPass<CColoredQuadFilter>>();
      break;
    case EFilterShape::CinemaBars:
      m_filter = std::make_unique<CCameraFilterPass<CWideScreenFilter>>();
      break;
    case EFilterShape::ScanLinesEven:
      m_filter = std::make_unique<CCameraFilterPass<CScanLinesFilterEven>>();
      break;
    case EFilterShape::ScanLinesOdd:
      m_filter = std::make_unique<CCameraFilterPass<CScanLinesFilterOdd>>();
      break;
    case EFilterShape::RandomStatic:
      m_filter = std::make_unique<CCameraFilterPass<CRandomStaticFilter>>();
      break;
    case EFilterShape::CookieCutterDepthRandomStatic:
      m_filter = std::make_unique<CCameraFilterPass<CCookieCutterDepthRandomStaticFilter>>();
      break;
    default:
      break;
    }
  }
  if (m_filter)
    m_filter->SetFilter(type, shape, time, color, txtr);
}

void CCameraBlurPass::Draw(bool clearDepth) {
  if (x10_curType == EBlurType::NoBlur)
    return;

  if (x10_curType == EBlurType::Xray) {
    if (!m_xrayShader)
      m_xrayShader.emplace(x0_paletteTex);
    m_xrayShader->draw(x1c_curValue);
  } else {
    if (!m_shader)
      m_shader.emplace();
    m_shader->draw(x1c_curValue, clearDepth);
    if (clearDepth)
      CGraphics::SetDepthRange(DEPTH_NEAR, DEPTH_FAR);
  }
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

void CCameraBlurPass::SetBlur(EBlurType type, float amount, float duration) {
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
    // x2c_usePersistent = b1;
  } else {
    // x2c_usePersistent = b1;
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

void CCameraBlurPass::DisableBlur(float duration) { SetBlur(EBlurType::NoBlur, 0.f, duration); }

} // namespace urde
