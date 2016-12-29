#include "CGuiTextSupport.hpp"
#include "CSimplePool.hpp"
#include "CFontImageDef.hpp"
#include "CGuiSys.hpp"
#include "CTextExecuteBuffer.hpp"
#include "CTextParser.hpp"
#include "Graphics/CGraphicsPalette.hpp"
#include "Graphics/CGraphics.hpp"
#include "GuiSys/CRasterFont.hpp"

namespace urde
{

CGuiTextSupport::CGuiTextSupport(ResId fontId, const CGuiTextProperties& props,
                                 const zeus::CColor& fontCol, const zeus::CColor& outlineCol,
                                 const zeus::CColor& geomCol, s32 padX, s32 padY, CSimplePool* store)
: x14_props(props), x24_fontColor(fontCol), x28_outlineColor(outlineCol),
  x2c_geometryColor(geomCol), x34_extentX(padX), x38_extentY(padY)
{
    x2cc_font = store->GetObj({SBIG('FONT'), fontId});
}

CTextRenderBuffer* CGuiTextSupport::GetCurrentLineRenderBuffer() const
{
    if (x60_renderBuf && !x308_multipageFlag)
        return const_cast<CTextRenderBuffer*>(&*x60_renderBuf);
    if (!x308_multipageFlag || x300_ <= x304_pageCounter)
        return nullptr;
    int idx = 0;
    for (const CTextRenderBuffer& buf : x2f0_pageRenderBufs)
        if (idx++ == x304_pageCounter)
            return const_cast<CTextRenderBuffer*>(&buf);
    return nullptr;
}

float CGuiTextSupport::GetCurrentAnimationOverAge() const
{
    float ret = 0.f;
    if (CTextRenderBuffer* buf = GetCurrentLineRenderBuffer())
    {
        if (x50_typeEnable)
        {
            if (x40_primStartTimes.size())
            {
                auto& lastTime = x40_primStartTimes.back();
                ret = std::max(ret, (buf->GetPrimitiveCount() - lastTime.second) / x58_chRate + lastTime.first);
            }
            else
            {
                ret = std::max(ret, buf->GetPrimitiveCount() / x58_chRate);
            }
        }
    }
    return ret;
}

float CGuiTextSupport::GetNumCharsPrinted() const
{
    if (CTextRenderBuffer* buf = GetCurrentLineRenderBuffer())
    {
        if (x50_typeEnable)
        {
            float charsPrinted = x3c_curTime * x58_chRate;
            return std::min(charsPrinted, float(buf->GetPrimitiveCount()));
        }
    }
    return 0.f;
}

float CGuiTextSupport::GetTotalAnimationTime() const
{
    if (CTextRenderBuffer* buf = GetCurrentLineRenderBuffer())
        if (x50_typeEnable)
            return buf->GetPrimitiveCount() / x58_chRate;
    return 0.f;
}

bool CGuiTextSupport::IsAnimationDone() const
{
    return x3c_curTime >= GetTotalAnimationTime();
}

void CGuiTextSupport::SetTypeWriteEffectOptions(bool enable, float chFadeTime, float chRate)
{
    x50_typeEnable = enable;
    x54_chFadeTime = std::max(chFadeTime, 0.0001f);
    x58_chRate = std::max(chRate, 1.f);
}

void CGuiTextSupport::Update(float dt)
{
    if (x50_typeEnable)
    {
        if (CTextRenderBuffer* buf = GetCurrentLineRenderBuffer())
        {
            for (s32 i=0 ; i<buf->GetPrimitiveCount() ; ++i)
            {
                float chStartTime = 0.f;
                for (const std::pair<float, int>& p : x40_primStartTimes)
                {
                    if (p.second < i)
                        continue;
                    if (p.second != i)
                        break;
                    chStartTime = p.first;
                    break;
                }

#if 0
                CTextRenderBuffer::Primitive prim = x54_renderBuf->GetPrimitive(i);
                prim.x0_color1.a = std::min(std::max(0.f, (x30_curTime - chStartTime) / x48_chFadeTime), 1.f);
                x54_renderBuf->SetPrimitive(prim, i);
#else
                buf->SetPrimitiveOpacity(i,
                    std::min(std::max(0.f, (x3c_curTime - chStartTime) / x54_chFadeTime), 1.f));
#endif
            }
        }
        x3c_curTime += dt;
    }

    x10_curTimeMod900 = std::fmod(x10_curTimeMod900 + dt, 900.f);
}

void CGuiTextSupport::ClearRenderBuffer()
{
    x60_renderBuf = std::experimental::nullopt;
}

void CGuiTextSupport::CheckAndRebuildTextRenderBuffer()
{
    g_TextExecuteBuf->Clear();
    g_TextExecuteBuf->x18_textState.x7c_enableWordWrap = x14_props.x0_wordWrap;
    g_TextExecuteBuf->BeginBlock(0, 0, x34_extentX, x38_extentY, x14_props.xc_direction,
                                 x14_props.x4_justification, x14_props.x8_vertJustification);
    g_TextExecuteBuf->AddColor(EColorType::Main, x24_fontColor);
    g_TextExecuteBuf->AddColor(EColorType::Outline, x28_outlineColor);

    std::wstring initStr;
    if ((x5c_fontId & 0xffff) != 0xffff)
        initStr = hecl::WideFormat(L"&font=%08X;", u32(x5c_fontId));
    initStr += x0_string;

    g_TextParser->ParseText(*g_TextExecuteBuf, initStr.c_str(), initStr.size());

    g_TextExecuteBuf->EndBlock();
}

void CGuiTextSupport::Render() const
{
    if (CTextRenderBuffer* buf = GetCurrentLineRenderBuffer())
    {
        zeus::CTransform oldModel = CGraphics::g_GXModelMatrix;
        CGraphics::SetModelMatrix(oldModel * zeus::CTransform::Scale(1.f, 1.f, -1.f));
        buf->Render(x2c_geometryColor, x10_curTimeMod900);
        CGraphics::SetModelMatrix(oldModel);
    }
}

void CGuiTextSupport::SetGeometryColor(const zeus::CColor& col)
{
    x2c_geometryColor = col;
}

void CGuiTextSupport::SetOutlineColor(const zeus::CColor& col)
{
    if (col != x28_outlineColor)
    {
        ClearRenderBuffer();
        x28_outlineColor = col;
    }
}

void CGuiTextSupport::SetFontColor(const zeus::CColor& col)
{
    if (col != x24_fontColor)
    {
        ClearRenderBuffer();
        x24_fontColor = col;
    }
}

void CGuiTextSupport::AddText(const std::wstring& str)
{
    if (x60_renderBuf)
    {
        float t = GetCurrentAnimationOverAge();
        x40_primStartTimes.push_back(std::make_pair(std::max(t, x3c_curTime),
                                                    x60_renderBuf->GetPrimitiveCount()));
    }
    x0_string += str;
    ClearRenderBuffer();
}

void CGuiTextSupport::SetText(const std::wstring& str, bool multipage)
{
    if (x0_string.compare(str))
    {
        x40_primStartTimes.clear();
        x3c_curTime = 0.f;
        x0_string = str;
        ClearRenderBuffer();
        x308_multipageFlag = multipage;
        x304_pageCounter = 0;
    }
}

void CGuiTextSupport::SetText(const std::string& str, bool multipage)
{
    SetText(hecl::UTF8ToWide(str), multipage);
}

bool CGuiTextSupport::GetIsTextSupportFinishedLoading() const
{
    for (const CToken& tok : x2bc_assets)
    {
        ((CToken&)tok).Lock();
        if (!tok.IsLoaded())
            return false;
    }
    return x2cc_font.IsLoaded();
}

}
