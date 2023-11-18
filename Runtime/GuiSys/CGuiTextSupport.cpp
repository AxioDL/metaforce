#include "Runtime/GuiSys/CGuiTextSupport.hpp"

#include <fmt/xchar.h>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CGraphicsPalette.hpp"
#include "Runtime/GuiSys/CFontImageDef.hpp"
#include "Runtime/GuiSys/CGuiSys.hpp"
#include "Runtime/GuiSys/CRasterFont.hpp"
#include "Runtime/GuiSys/CTextExecuteBuffer.hpp"
#include "Runtime/GuiSys/CTextParser.hpp"
#include "Runtime/CStringExtras.hpp"

namespace metaforce {

CGuiTextSupport::CGuiTextSupport(CAssetId fontId, const CGuiTextProperties& props, const zeus::CColor& fontCol,
                                 const zeus::CColor& outlineCol, const zeus::CColor& geomCol, s32 padX, s32 padY,
                                 CSimplePool* store, CGuiWidget::EGuiModelDrawFlags drawFlags)
: x14_props(props)
, x24_fontColor(fontCol)
, x28_outlineColor(outlineCol)
, x2c_geometryColor(geomCol)
, x34_extentX(padX)
, x38_extentY(padY)
, x5c_fontId(fontId)
, m_drawFlags(drawFlags) {
  x2cc_font = store->GetObj({SBIG('FONT'), fontId});
}

CTextRenderBuffer* CGuiTextSupport::GetCurrentPageRenderBuffer() {
  if (x60_renderBuf && !x308_multipageFlag) {
    return &*x60_renderBuf;
  }

  if (!x308_multipageFlag || x2ec_renderBufferPages.size() <= x304_pageCounter) {
    return nullptr;
  }

  int idx = 0;
  for (CTextRenderBuffer& buf : x2ec_renderBufferPages) {
    if (idx++ == x304_pageCounter) {
      return &buf;
    }
  }

  return nullptr;
}

const CTextRenderBuffer* CGuiTextSupport::GetCurrentPageRenderBuffer() const {
  if (x60_renderBuf && !x308_multipageFlag) {
    return &*x60_renderBuf;
  }

  if (!x308_multipageFlag || x2ec_renderBufferPages.size() <= x304_pageCounter) {
    return nullptr;
  }

  int idx = 0;
  for (const CTextRenderBuffer& buf : x2ec_renderBufferPages) {
    if (idx++ == x304_pageCounter) {
      return &buf;
    }
  }

  return nullptr;
}

float CGuiTextSupport::GetCurrentAnimationOverAge() const {
  float ret = 0.f;
  if (const CTextRenderBuffer* buf = GetCurrentPageRenderBuffer()) {
    if (x50_typeEnable) {
      if (!x40_primStartTimes.empty()) {
        const auto& lastTime = x40_primStartTimes.back();
        ret = std::max(ret, (buf->GetPrimitiveCount() - lastTime.second) / x58_chRate + lastTime.first);
      } else {
        ret = std::max(ret, buf->GetPrimitiveCount() / x58_chRate);
      }
    }
  }
  return ret;
}

float CGuiTextSupport::GetNumCharsTotal() const {
  if (const CTextRenderBuffer* buf = GetCurrentPageRenderBuffer()) {
    if (x50_typeEnable) {
      return buf->GetPrimitiveCount();
    }
  }
  return 0.f;
}

float CGuiTextSupport::GetNumCharsPrinted() const {
  if (const CTextRenderBuffer* buf = GetCurrentPageRenderBuffer()) {
    if (x50_typeEnable) {
      const float charsPrinted = x3c_curTime * x58_chRate;
      return std::min(charsPrinted, float(buf->GetPrimitiveCount()));
    }
  }
  return 0.f;
}

float CGuiTextSupport::GetTotalAnimationTime() const {
  if (const CTextRenderBuffer* buf = GetCurrentPageRenderBuffer()) {
    if (x50_typeEnable) {
      return buf->GetPrimitiveCount() / x58_chRate;
    }
  }
  return 0.f;
}

bool CGuiTextSupport::IsAnimationDone() const { return x3c_curTime >= GetTotalAnimationTime(); }

void CGuiTextSupport::SetTypeWriteEffectOptions(bool enable, float chFadeTime, float chRate) {
  x50_typeEnable = enable;
  x54_chFadeTime = std::max(chFadeTime, 0.0001f);
  x58_chRate = std::max(chRate, 1.f);
  if (enable) {
    if (CTextRenderBuffer* buf = GetCurrentPageRenderBuffer()) {
      float chStartTime = 0.f;
      for (u32 i = 0; i < buf->GetPrimitiveCount(); ++i) {
        for (const std::pair<float, int>& p : x40_primStartTimes) {
          if (p.second < i)
            continue;
          if (p.second != i)
            break;
          chStartTime = p.first;
          break;
        }

        //buf->SetPrimitiveOpacity(i, std::min(std::max(0.f, (x3c_curTime - chStartTime) / x54_chFadeTime), 1.f));
        chStartTime += 1.f / x58_chRate;
      }
    }
  }
}

void CGuiTextSupport::Update(float dt) {
  if (x50_typeEnable) {
    if (CTextRenderBuffer* buf = GetCurrentPageRenderBuffer()) {
      float chStartTime = 0.f;
      for (u32 i = 0; i < buf->GetPrimitiveCount(); ++i) {
        for (const std::pair<float, int>& p : x40_primStartTimes) {
          if (p.second < i)
            continue;
          if (p.second != i)
            break;
          chStartTime = p.first;
          break;
        }

        auto primitive = buf->GetPrimitive(i);
        float alpha = std::clamp((x3c_curTime - chStartTime) / x54_chFadeTime, 0.f, 1.f);
        chStartTime += 1.f / x58_chRate;
        primitive.x0_color1 = zeus::CColor{alpha, alpha};
        buf->SetPrimitive(primitive, i);
      }
    }
    x3c_curTime += dt;
  }

  x10_curTimeMod900 = std::fmod(x10_curTimeMod900 + dt, 900.f);
}

void CGuiTextSupport::ClearRenderBuffer() {
  x60_renderBuf = std::nullopt;
  x2ec_renderBufferPages.clear();
}

void CGuiTextSupport::CheckAndRebuildTextBuffer() {
  g_TextExecuteBuf->Clear();
  g_TextExecuteBuf->x18_textState.x7c_enableWordWrap = x14_props.x0_wordWrap;
  g_TextExecuteBuf->BeginBlock(0, 0, x34_extentX, x38_extentY, x30_imageBaseline,
                               ETextDirection(!x14_props.x1_horizontal), x14_props.x4_justification,
                               x14_props.x8_vertJustification);
  g_TextExecuteBuf->AddColor(EColorType::Main, x24_fontColor);
  g_TextExecuteBuf->AddColor(EColorType::Outline, x28_outlineColor);

  std::u16string initStr;
  if (x5c_fontId.IsValid())
    initStr = fmt::format(FMT_STRING(u"&font={};"), x5c_fontId);
  initStr += x0_string;

  g_TextParser->ParseText(*g_TextExecuteBuf, initStr.c_str(), initStr.size(), x14_props.xc_txtrMap);

  g_TextExecuteBuf->EndBlock();
}

bool CGuiTextSupport::CheckAndRebuildRenderBuffer() {
  if (x308_multipageFlag || x60_renderBuf) {
    if (!x308_multipageFlag || x2ec_renderBufferPages.size()) {
      return true;
    }
  }

  CheckAndRebuildTextBuffer();
  x2bc_assets = g_TextExecuteBuf->GetAssets();

  if (!_GetIsTextSupportFinishedLoading())
    return false;

  CheckAndRebuildTextBuffer();
  if (x308_multipageFlag) {
    zeus::CVector2i extent(x34_extentX, x38_extentY);
    x2ec_renderBufferPages = g_TextExecuteBuf->BuildRenderBufferPages(extent, m_drawFlags);
  } else {
    x60_renderBuf.emplace(g_TextExecuteBuf->BuildRenderBuffer(m_drawFlags));
    x2dc_oneBufBounds = x60_renderBuf->AccumulateTextBounds();
  }
  g_TextExecuteBuf->Clear();
  Update(0.f);

  return true;
}

const std::pair<zeus::CVector2i, zeus::CVector2i>& CGuiTextSupport::GetBounds() {
  CheckAndRebuildRenderBuffer();
  return x2dc_oneBufBounds;
}

void CGuiTextSupport::AutoSetExtent() {
  const auto& bounds = GetBounds();
  x34_extentX = bounds.second.x;
  x38_extentY = bounds.second.y;
}

void CGuiTextSupport::Render() {
  CheckAndRebuildRenderBuffer();
  if (CTextRenderBuffer* buf = GetCurrentPageRenderBuffer()) {
    SCOPED_GRAPHICS_DEBUG_GROUP("CGuiTextSupport::Render", zeus::skBlue);
    zeus::CTransform oldModel = CGraphics::g_GXModelMatrix;
    CGraphics::SetModelMatrix(oldModel * zeus::CTransform::Scale(1.f, 1.f, -1.f));
    buf->Render(x2c_geometryColor, x10_curTimeMod900);
    CGraphics::SetModelMatrix(oldModel);
  }
}

void CGuiTextSupport::SetGeometryColor(const zeus::CColor& col) { x2c_geometryColor = col; }

void CGuiTextSupport::SetOutlineColor(const zeus::CColor& col) {
  if (col == x28_outlineColor) {
    return;
  }

  ClearRenderBuffer();
  x28_outlineColor = col;
}

void CGuiTextSupport::SetFontColor(const zeus::CColor& col) {
  if (col == x24_fontColor) {
    return;
  }

  ClearRenderBuffer();
  x24_fontColor = col;
}

void CGuiTextSupport::AddText(std::u16string_view str) {
  if (x60_renderBuf) {
    const float t = GetCurrentAnimationOverAge();
    x40_primStartTimes.emplace_back(std::max(t, x3c_curTime), x60_renderBuf->GetPrimitiveCount());
  }
  x0_string += str;
  ClearRenderBuffer();
}

void CGuiTextSupport::SetText(std::u16string_view str, bool multipage) {
  if (x0_string == str) {
    return;
  }

  x40_primStartTimes.clear();
  x3c_curTime = 0.f;
  x0_string = str;
  ClearRenderBuffer();
  x308_multipageFlag = multipage;
  x304_pageCounter = 0;
}

void CGuiTextSupport::SetText(std::string_view str, bool multipage) {
  SetText(CStringExtras::ConvertToUNICODE(str), multipage);
}

bool CGuiTextSupport::_GetIsTextSupportFinishedLoading() {
  for (CToken& tok : x2bc_assets) {
    tok.Lock();

    if (!tok.IsLoaded()) {
      return false;
    }
  }

  if (!x2cc_font) {
    return true;
  }

  if (x2cc_font.IsLoaded()) {
    return x2cc_font->IsFinishedLoading();
  }

  return false;
}

void CGuiTextSupport::SetJustification(EJustification j) {
  if (j == x14_props.x4_justification) {
    return;
  }

  x14_props.x4_justification = j;
  ClearRenderBuffer();
}

void CGuiTextSupport::SetVerticalJustification(EVerticalJustification j) {
  if (j == x14_props.x8_vertJustification) {
    return;
  }

  x14_props.x8_vertJustification = j;
  ClearRenderBuffer();
}

void CGuiTextSupport::SetImageBaseline(bool b) {
  if (b == x30_imageBaseline) {
    return;
  }

  x30_imageBaseline = b;
  ClearRenderBuffer();
}

bool CGuiTextSupport::GetIsTextSupportFinishedLoading() {
  CheckAndRebuildRenderBuffer();
  return _GetIsTextSupportFinishedLoading();
}

void CGuiTextSupport::SetControlTXTRMap(const std::vector<std::pair<CAssetId, CAssetId>>* txtrMap) {
  if (x14_props.xc_txtrMap == txtrMap) {
    return;
  }

  x14_props.xc_txtrMap = txtrMap;
  ClearRenderBuffer();
}

int CGuiTextSupport::GetTotalPageCount() {
  if (CheckAndRebuildRenderBuffer())
    return x2ec_renderBufferPages.size();
  return -1;
}

void CGuiTextSupport::SetPage(int page) {
  x304_pageCounter = page;
  x40_primStartTimes.clear();
  x3c_curTime = 0.f;
}

} // namespace metaforce
