#include "CGuiFrame.hpp"
#include "CGuiWidget.hpp"
#include "CGuiSys.hpp"
#include "CGuiHeadWidget.hpp"
#include "CGuiAnimController.hpp"
#include "zeus/CColor.hpp"

namespace urde
{

CGuiFrame::CGuiFrame(TResId id, const std::string& name, CGuiSys& sys, int a, int b, int c)
: x4_name(name), x14_id(id), x1c_transitionOpts(EFrameTransitionOptions::Zero),
  x3c_guiSys(sys), xb0_a(a), xb4_b(b), xb8_c(c), xbc_24_flag1(false)
{
    xa0_lights.resize(8);
    x48_rootWidget.reset(new CGuiWidget(
        CGuiWidget::CGuiWidgetParms(this, false, 0, 0, false, false, false, zeus::CColor::skWhite,
                                    CGuiWidget::EGuiModelDrawFlags::Two, false,
                                    x3c_guiSys.x2c_mode != CGuiSys::EUsageMode::Zero)));
}

CGuiWidget* CGuiFrame::FindWidget(const std::string& name) const
{
    s16 id = x64_idDB.FindWidgetID(name);
    if (id == -1)
        return nullptr;
    return FindWidget(id);
}

CGuiWidget* CGuiFrame::FindWidget(s16 id) const
{
    return x48_rootWidget->FindWidget(id);
}

void CGuiFrame::ResetControllerStatus()
{
    x0_controllerStatus[0] = false;
    x0_controllerStatus[1] = false;
    x0_controllerStatus[2] = false;
    x0_controllerStatus[3] = false;
}

CGuiFrame::WidgetToLogicalEventMap* CGuiFrame::FindWidget2LogicalEventMap(u64 events)
{
    auto search = x7c_messageMap.find(events);
    if (search == x7c_messageMap.cend())
        return nullptr;
    return search->second.get();
}

CGuiFrame::LogicalEventList* CGuiFrame::FindLogicalEventList(u64 events, s16 id)
{
    CGuiFrame::WidgetToLogicalEventMap* map = FindWidget2LogicalEventMap(events);
    if (!map)
        return nullptr;

    auto search = map->find(id);
    if (search == map->cend())
        return nullptr;

    return search->second.get();
}

void CGuiFrame::ClearMessageMap(const CGuiLogicalEventTrigger* trigger, s16 id)
{
    CGuiFrame::LogicalEventList* list =
        FindLogicalEventList(trigger->GetPhysicalMsg().x0_curStates, id);
    if (list)
        list->clear();
}

void CGuiFrame::AddMessageMap(const CGuiLogicalEventTrigger* trigger, s16 id)
{
    u64 events = trigger->GetPhysicalMsg().x0_curStates;
    int triggerId = trigger->GetTriggerId();
    CGuiFrame::WidgetToLogicalEventMap* map = FindWidget2LogicalEventMap(events);
    if (!map)
    {
        auto it =
        x7c_messageMap.emplace(std::make_pair(events, std::make_unique<WidgetToLogicalEventMap>()));
        map = it.first->second.get();
    }

    CGuiFrame::LogicalEventList* list = FindLogicalEventList(events, id);
    if (!list)
    {
        auto it =
        map->emplace(std::make_pair(id, std::make_unique<LogicalEventList>()));
        list = it.first->second.get();
    }

    for (std::unique_ptr<CGuiFrameMessageMapNode>& node : *list)
        if (node->GetTrigger().GetTriggerId() == triggerId)
            return;

    list->push_back(std::make_unique<CGuiFrameMessageMapNode>(trigger->GetPhysicalMsg(), triggerId));
}

void CGuiFrame::SortDrawOrder()
{
    std::sort(x90_widgets.begin(), x90_widgets.end(),
    [](const CGuiWidget* a, const CGuiWidget* b) -> bool
    {
        return a->GetWorldPosition().y < b->GetWorldPosition().y;
    });
}

void CGuiFrame::Initialize()
{
    SortDrawOrder();
    x44_headWidget->SetColor(x44_headWidget->xbc_color);
    x44_headWidget->InitializeRecursive();
}

void CGuiFrame::LoadWidgetsInGame(CInputStream& in)
{
    u32 count = in.readUint32Big();
    x90_widgets.reserve(count);
    for (u32 i=0 ; i<count ; ++i)
    {
        FourCC type = in.readUint32Big();
        CGuiWidget* widget = CGuiSys::CreateWidgetInGame(type, in, this);
        type = widget->GetWidgetTypeID();
        switch (type)
        {
        case SBIG('CAMR'):
        case SBIG('LITE'):
        case SBIG('BGND'):
            break;
        default:
            x90_widgets.push_back(widget);
            break;
        }
    }
    Initialize();
}

CGuiFrame* CGuiFrame::CreateFrame(TResId frmeId, CGuiSys& sys, CInputStream& in)
{
    std::string name = CreateFrameName(frmeId);
    in.readInt32Big();
    int a = in.readInt32Big();
    int b = in.readInt32Big();
    int c = in.readInt32Big();

    CGuiFrame* ret = new CGuiFrame(frmeId, name, sys, a, b, c);
    ret->LoadWidgetsInGame(in);
    return ret;
}

std::string CGuiFrame::CreateFrameName(TResId frmeId)
{
    /* formatting token originally "frame_%x" for 32-bit ids */
    return hecl::Format("frame_%016" PRIX64, frmeId);
}


}
