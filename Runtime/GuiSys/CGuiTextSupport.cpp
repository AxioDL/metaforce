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

float CGuiTextSupport::GetCurrentAnimationOverAge() const
{
    if (!x2ac_active || !x50_typeEnable)
        return 0.f;

    if (x44_primStartTimes.size())
    {
        float val = (x60_renderBuf->GetPrimitiveCount() - x44_primStartTimes.back().second) /
                    x58_chRate + x44_primStartTimes.back().first;
        return std::max(0.f, val);
    }
    else
    {
        float val = x60_renderBuf->GetPrimitiveCount() / x58_chRate;
        return std::max(0.f, val);
    }
}

float CGuiTextSupport::GetNumCharsPrinted() const
{
    if (x2ac_active)
        return std::min(x3c_curTime * x58_chRate, float(x60_renderBuf->GetPrimitiveCount()));
    return 0.f;
}

float CGuiTextSupport::GetTotalAnimationTime() const
{
    if (!x2ac_active || !x50_typeEnable)
        return 0.f;

    return x60_renderBuf->GetPrimitiveCount() / x58_chRate;
}

void CGuiTextSupport::SetTypeWriteEffectOptions(bool enable, float chFadeTime, float chRate)
{
    x50_typeEnable = enable;
    x54_chFadeTime = std::max(chFadeTime, 0.0001f);
    x58_chRate = std::max(chRate, 1.f);
}

void CGuiTextSupport::Update(float dt)
{
    if (!x2ac_active)
        return;

    if (x50_typeEnable)
    {
        for (int i=0 ; i<x60_renderBuf->GetPrimitiveCount() ; ++i)
        {
            float chStartTime = 0.f;
            for (const std::pair<float, int>& p : x44_primStartTimes)
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
            x60_renderBuf->SetPrimitiveOpacity(i,
                std::min(std::max(0.f, (x3c_curTime - chStartTime) / x54_chFadeTime), 1.f));
#endif
        }
    }

    x3c_curTime += dt;
}

void CGuiTextSupport::ClearBuffer()
{
    x60_renderBuf = std::experimental::nullopt;
}

void CGuiTextSupport::CheckAndRebuildTextRenderBuffer()
{
    if (x2ac_active)
        return;

    g_TextExecuteBuf->Clear();
    g_TextExecuteBuf->x18_textState.x48_enableWordWrap = x14_props.x0_wordWrap;
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
    x2b0_assets = g_TextExecuteBuf->GetAssets();

    if (GetIsTextSupportFinishedLoading())
    {
        x60_renderBuf = g_TextExecuteBuf->CreateTextRenderBuffer();
        g_TextExecuteBuf->Clear();
    }

    Update(0.f);
}

void CGuiTextSupport::Render() const
{
    if (x2ac_active)
    {
        zeus::CTransform oldModel = CGraphics::g_GXModelMatrix;
        CGraphics::SetModelMatrix(oldModel * zeus::CTransform::Scale(1.f, 1.f, -1.f));
        x60_renderBuf->Render(x2c_geometryColor, x3c_curTime);
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
        ClearBuffer();
        x28_outlineColor = col;
    }
}

void CGuiTextSupport::SetFontColor(const zeus::CColor& col)
{
    if (col != x24_fontColor)
    {
        ClearBuffer();
        x24_fontColor = col;
    }
}

void CGuiTextSupport::AddText(const std::wstring& str)
{
    if (x2ac_active)
    {
        float t = GetCurrentAnimationOverAge();
        x44_primStartTimes.push_back(std::make_pair(std::max(t, x3c_curTime),
                                                    x60_renderBuf->GetPrimitiveCount()));
    }
    x0_string += str;
    ClearBuffer();
}

void CGuiTextSupport::SetText(const std::wstring& str, bool scanFlag)
{
    if (x0_string.compare(str))
    {
        x44_primStartTimes.clear();
        x3c_curTime = 0.f;
        x0_string = str;
        ClearBuffer();
        x308_scanFlag = scanFlag;
        x304_scanCounter = 0;
    }
}

void CGuiTextSupport::SetText(const std::string& str, bool scanFlag)
{
    SetText(hecl::UTF8ToWide(str), scanFlag);
}

bool CGuiTextSupport::GetIsTextSupportFinishedLoading() const
{
    for (const CToken& tok : x2b0_assets)
    {
        ((CToken&)tok).Lock();
        if (!tok.IsLoaded())
            return false;
    }
    return x2cc_font.IsLoaded();
}

}
