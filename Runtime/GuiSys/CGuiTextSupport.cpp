#include "CGuiTextSupport.hpp"
#include "CSimplePool.hpp"
#include "CFontImageDef.hpp"
#include "CGuiSys.hpp"
#include "CTextExecuteBuffer.hpp"
#include "CTextParser.hpp"
#include "Graphics/CGraphicsPalette.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde
{

CGuiTextSupport::CGuiTextSupport(TResId fontId, const CGuiTextProperties& props,
                                 const zeus::CColor& fontCol, const zeus::CColor& outlineCol,
                                 const zeus::CColor& geomCol, s32 padX, s32 padY, CSimplePool* store)
: x10_props(props), x1c_fontColor(fontCol), x20_outlineColor(outlineCol),
  x24_geometryColor(geomCol), x28_extentX(padX), x2c_extentY(padY)
{
    x2c0_font = store->GetObj({SBIG('FONT'), fontId});
}

float CGuiTextSupport::GetCurrentAnimationOverAge() const
{
    if (!x2ac_active || !x44_typeEnable)
        return 0.f;

    if (x34_primStartTimes.size())
    {
        float val = (x54_renderBuf->GetPrimitiveCount() - x34_primStartTimes.back().second) /
                    x4c_chRate + x34_primStartTimes.back().first;
        return std::max(0.f, val);
    }
    else
    {
        float val = x54_renderBuf->GetPrimitiveCount() / x4c_chRate;
        return std::max(0.f, val);
    }
}

float CGuiTextSupport::GetNumCharsPrinted() const
{
    if (x2ac_active)
        return std::min(x30_curTime * x4c_chRate, float(x54_renderBuf->GetPrimitiveCount()));
    return 0.f;
}

float CGuiTextSupport::GetTotalAnimationTime() const
{
    if (!x2ac_active || !x44_typeEnable)
        return 0.f;

    return x54_renderBuf->GetPrimitiveCount() / x4c_chRate;
}

void CGuiTextSupport::SetTypeWriteEffectOptions(bool enable, float chFadeTime, float chRate)
{
    x44_typeEnable = enable;
    x48_chFadeTime = std::max(chFadeTime, 0.0001f);
    x4c_chRate = std::max(chRate, 1.f);
}

void CGuiTextSupport::Update(float dt)
{
    if (!x2ac_active)
        return;

    if (x44_typeEnable)
    {
        for (int i=0 ; i<x54_renderBuf->GetPrimitiveCount() ; ++i)
        {
            float chStartTime = 0.f;
            for (const std::pair<float, int>& p : x34_primStartTimes)
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
            x54_renderBuf->SetPrimitiveOpacity(i,
                std::min(std::max(0.f, (x30_curTime - chStartTime) / x48_chFadeTime), 1.f));
#endif
        }
    }

    x30_curTime += dt;
}

void CGuiTextSupport::ClearBuffer()
{
    x54_renderBuf = std::experimental::nullopt;
}

void CGuiTextSupport::CheckAndRebuildTextRenderBuffer()
{
    if (x2ac_active)
        return;

    g_TextExecuteBuf->Clear();
    g_TextExecuteBuf->x18_textState.x48_enableWordWrap = x10_props.x0_wordWrap;
    g_TextExecuteBuf->BeginBlock(0, 0, x28_extentX, x2c_extentY, ETextDirection(x10_props.x1_vertical),
                                 x10_props.x4_justification, x10_props.x8_vertJustification);
    g_TextExecuteBuf->AddColor(EColorType::Main, x1c_fontColor);
    g_TextExecuteBuf->AddColor(EColorType::Outline, x20_outlineColor);

    std::wstring initStr;
    if ((x50_fontId & 0xffff) != 0xffff)
        initStr = hecl::WideFormat(L"&font=%08X;", u32(x50_fontId));
    initStr += x0_string;

    g_TextParser->ParseText(*g_TextExecuteBuf, initStr.c_str(), initStr.size());

    g_TextExecuteBuf->EndBlock();
    x2b0_assets = g_TextExecuteBuf->GetAssets();

    if (GetIsTextSupportFinishedLoading())
    {
        x54_renderBuf = g_TextExecuteBuf->CreateTextRenderBuffer();
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
        x54_renderBuf->Render(x24_geometryColor, x30_curTime);
        CGraphics::SetModelMatrix(oldModel);
    }
}

void CGuiTextSupport::SetGeometryColor(const zeus::CColor& col)
{
    x24_geometryColor = col;
}

void CGuiTextSupport::SetOutlineColor(const zeus::CColor& col)
{
    if (col != x20_outlineColor)
    {
        ClearBuffer();
        x20_outlineColor = col;
    }
}

void CGuiTextSupport::SetFontColor(const zeus::CColor& col)
{
    if (col != x1c_fontColor)
    {
        ClearBuffer();
        x1c_fontColor = col;
    }
}

void CGuiTextSupport::AddText(const std::wstring& str)
{
    if (x2ac_active)
    {
        float t = GetCurrentAnimationOverAge();
        x34_primStartTimes.push_back(std::make_pair(std::max(t, x30_curTime),
                                                    x54_renderBuf->GetPrimitiveCount()));
    }
    x0_string += str;
    ClearBuffer();
}

void CGuiTextSupport::SetText(const std::wstring& str)
{
    if (x0_string.compare(str))
    {
        x34_primStartTimes.clear();
        x30_curTime = 0.f;
        x0_string = str;
        ClearBuffer();
    }
}

void CGuiTextSupport::SetText(const std::string& str)
{
    SetText(hecl::UTF8ToWide(str));
}

bool CGuiTextSupport::GetIsTextSupportFinishedLoading() const
{
    for (const CToken& tok : x2b0_assets)
    {
        ((CToken&)tok).Lock();
        if (!tok.IsLoaded())
            return false;
    }
    return x2c0_font.IsLoaded();
}

}
