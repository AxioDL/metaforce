#include "CGuiTextSupport.hpp"
#include "CSimplePool.hpp"
#include "CFontImageDef.hpp"
#include "Graphics/CGraphicsPalette.hpp"

namespace urde
{

CGuiTextSupport::CGuiTextSupport(TResId fontId, const CGuiTextProperties& props,
                                 const zeus::CColor& col1, const zeus::CColor& col2,
                                 const zeus::CColor& col3, int, int, CSimplePool* store)
: x10_props(props), x1c_(col1), x20_(col2), x24_(col3)
{
    x2c0_font = store->GetObj({SBIG('FONT'), fontId});
}

void CGuiTextSupport::GetCurrentAnimationOverAge() const
{
    if (!x2ac_ || !x44_)
        return;


}

int CGuiTextSupport::GetNumCharsPrinted() const
{
}

int CGuiTextSupport::GetTotalAnimationTime() const
{
}

void CGuiTextSupport::SetTypeWriteEffectOptions(bool, float, float)
{
}

void CGuiTextSupport::Update(float dt)
{
}

void CGuiTextSupport::ClearBuffer()
{
}

void CGuiTextSupport::CheckAndRebuildTextRenderBuffer()
{
}

void CGuiTextSupport::Render() const
{
}

void CGuiTextSupport::SetGeometryColor(const zeus::CColor& col)
{
}

void CGuiTextSupport::SetOutlineColor(const zeus::CColor& col)
{
}

void CGuiTextSupport::SetFontColor(const zeus::CColor& col)
{
}

void CGuiTextSupport::AddText(const std::wstring& str)
{
}

void CGuiTextSupport::SetText(const std::wstring& str)
{
}

void CGuiTextSupport::SetText(const std::string& str)
{
}

bool CGuiTextSupport::GetIsTextSupportFinishedLoading() const
{
}

}
