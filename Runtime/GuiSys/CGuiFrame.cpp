#include "CGuiFrame.hpp"
#include "CGuiWidget.hpp"
#include "CGuiSys.hpp"
#include "CGuiHeadWidget.hpp"
#include "CGuiLight.hpp"
#include "CGuiCamera.hpp"
#include "Graphics/CGraphics.hpp"
#include "Input/CFinalInput.hpp"
#include "zeus/CColor.hpp"
#include "CSimplePool.hpp"

namespace urde
{

CGuiFrame::CGuiFrame(ResId id, CGuiSys& sys, int a, int b, int c, CSimplePool* sp)
: x0_id(id), x8_guiSys(sys), x4c_a(a), x50_b(b), x54_c(c), x58_24_loaded(false)
{
    x3c_lights.resize(8);
    x10_rootWidget.reset(new CGuiWidget(
        CGuiWidget::CGuiWidgetParms(this, false, 0, 0, false, false, false, zeus::CColor::skWhite,
                                    CGuiWidget::EGuiModelDrawFlags::Alpha, false,
                                    x8_guiSys.x8_mode != CGuiSys::EUsageMode::Zero)));
}

CGuiWidget* CGuiFrame::FindWidget(const std::string& name) const
{
    s16 id = x18_idDB.FindWidgetID(name);
    if (id == -1)
        return nullptr;
    return FindWidget(id);
}

CGuiWidget* CGuiFrame::FindWidget(s16 id) const
{
    return x10_rootWidget->FindWidget(id);
}

void CGuiFrame::SortDrawOrder()
{
    std::sort(x2c_widgets.begin(), x2c_widgets.end(),
    [](const std::shared_ptr<CGuiWidget>& a, const std::shared_ptr<CGuiWidget>& b) -> bool
    {
        return a->GetWorldPosition().y < b->GetWorldPosition().y;
    });
}

void CGuiFrame::EnableLights(u32 lights) const
{
    CGraphics::DisableAllLights();
    zeus::CColor accumColor(zeus::CColor::skBlack);
    ERglLight lightId = ERglLight::Zero;
    int idx = 0;
    for (auto& light : x3c_lights)
    {
        if ((lights & (1 << idx)) != 0)
        {
            // accumulate color
            accumColor += light->GetColor();
            CGraphics::LoadLight(lightId, light->BuildLight());
            CGraphics::EnableLight(lightId);
        }
        ++reinterpret_cast<std::underlying_type_t<ERglLight>&>(lightId);
        ++idx;
    }
    if (x3c_lights.empty())
        CGraphics::SetAmbientColor(zeus::CColor::skWhite);
    else
        CGraphics::SetAmbientColor(accumColor);
}

void CGuiFrame::DisableLights() const
{
    CGraphics::DisableAllLights();
}

void CGuiFrame::RemoveLight(CGuiLight* light)
{
    x3c_lights[light->GetLoadedIdx()].reset();
}

void CGuiFrame::AddLight(std::shared_ptr<CGuiLight>&& light)
{
    x3c_lights[light->GetLoadedIdx()] = std::move(light);
}

bool CGuiFrame::GetIsFinishedLoading() const
{
    if (x58_24_loaded)
        return true;
    for (const auto& widget : x2c_widgets)
    {
        if (widget->GetIsFinishedLoading())
            continue;
        return false;
    }
    const_cast<CGuiFrame*>(this)->x58_24_loaded = true;
    return true;
}

void CGuiFrame::Touch() const
{
    for (const auto& widget : x2c_widgets)
        widget->Touch();
}

void CGuiFrame::Update(float dt)
{
    xc_headWidget->Update(dt);
}

void CGuiFrame::Draw(const CGuiWidgetDrawParms& parms) const
{
    CGraphics::SetCullMode(ERglCullMode::None);
    CGraphics::SetAmbientColor(zeus::CColor::skWhite);
    DisableLights();
    x14_camera->Draw(parms);
    // Set one-stage modulate
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha,
                            ERglBlendFactor::InvSrcAlpha, ERglLogicOp::Clear);

    for (const auto& widget : x2c_widgets)
        if (widget->GetIsVisible())
            widget->Draw(parms);

    CGraphics::SetCullMode(ERglCullMode::Front);
}

void CGuiFrame::Initialize()
{
    SortDrawOrder();
    xc_headWidget->SetColor(xc_headWidget->xa4_color);
    xc_headWidget->DispatchInitialize();
}

void CGuiFrame::LoadWidgetsInGame(CInputStream& in, CSimplePool* sp)
{
    u32 count = in.readUint32Big();
    x2c_widgets.reserve(count);
    for (u32 i=0 ; i<count ; ++i)
    {
        DataSpec::DNAFourCC type;
        type.read(in);
        std::shared_ptr<CGuiWidget> widget = CGuiSys::CreateWidgetInGame(type, in, this, sp);
        type = widget->GetWidgetTypeID();
        switch (type)
        {
        case SBIG('CAMR'):
        case SBIG('LITE'):
        case SBIG('BGND'):
            break;
        default:
            x2c_widgets.push_back(std::move(widget));
            break;
        }
    }
    Initialize();
}

void CGuiFrame::ProcessUserInput(const CFinalInput& input) const
{
    if (input.ControllerIdx() != 0)
        return;
    for (auto& widget : x2c_widgets)
    {
        if (widget->GetIsActive())
            widget->ProcessUserInput(input);
    }
}

std::unique_ptr<CGuiFrame> CGuiFrame::CreateFrame(ResId frmeId, CGuiSys& sys, CInputStream& in, CSimplePool* sp)
{
    in.readInt32Big();
    int a = in.readInt32Big();
    int b = in.readInt32Big();
    int c = in.readInt32Big();

    std::unique_ptr<CGuiFrame> ret = std::make_unique<CGuiFrame>(frmeId, sys, a, b, c, sp);
    ret->LoadWidgetsInGame(in, sp);
    return ret;
}

std::unique_ptr<IObj> RGuiFrameFactoryInGame(const SObjectTag& tag, CInputStream& in,
                                             const CVParamTransfer& cvParms,
                                             CObjectReference* selfRef)
{
    CSimplePool* sp = cvParms.GetOwnedObj<CSimplePool*>();
    std::unique_ptr<CGuiFrame> frame(CGuiFrame::CreateFrame(tag.id, *g_GuiSys, in, sp));
    return TToken<CGuiFrame>::GetIObjObjectFor(std::move(frame));
}


}
