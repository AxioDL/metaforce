#include "CGuiFrame.hpp"
#include "CGuiWidget.hpp"
#include "CGuiSys.hpp"
#include "CGuiHeadWidget.hpp"
#include "CGuiAnimController.hpp"
#include "CGuiMessage.hpp"
#include "CGuiLight.hpp"
#include "CGuiCamera.hpp"
#include "Graphics/CGraphics.hpp"
#include "Input/CFinalInput.hpp"
#include "zeus/CColor.hpp"

namespace urde
{

CGuiFrame::CGuiFrame(TResId id, const std::string& name, CGuiSys& sys, int a, int b, int c)
: x4_name(name), x14_id(id), x1c_transitionOpts(EFrameTransitionOptions::Zero),
  x3c_guiSys(sys), xb0_a(a), xb4_b(b), xb8_c(c), xbc_24_loaded(false)
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

void CGuiFrame::InterpretGUIControllerState(const CFinalInput& input,
                                            CGuiPhysicalMsg::PhysicalMap& state,
                                            char& lx, char& ly, char& rx, char& ry)
{
    zeus::CVector2f left(std::fabs(input.ALeftX()), std::fabs(input.ALeftY()));
    bool leftThreshold = false;
    if (left.magnitude() >= 0.6f)
        leftThreshold = true;

    zeus::CVector2f right(std::fabs(input.ARightX()), std::fabs(input.ARightY()));
    bool rightThreshold = false;
    if (right.magnitude() >= 0.6f)
        rightThreshold = true;

    if (leftThreshold)
    {
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftStickThreshold, false);

        if (input.ALeftX() < 0.f)
            lx = input.ALeftX() * 128.f;
        else
            lx = input.ALeftX() * 127.f;

        if (input.ALeftY() < 0.f)
            ly = input.ALeftY() * 128.f;
        else
            ly = input.ALeftY() * 127.f;

        if (0.7f < input.ALeftY())
        {
            if (input.PLAUp())
            {
                CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftStickUpInst, false);
                CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftUpInst, false);
            }
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftStickUp, false);
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftUp, false);
        }

        if (-0.7f > input.ALeftY())
        {
            if (input.PLADown())
            {
                CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftStickDownInst, false);
                CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftDownInst, false);
            }
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftStickDown, false);
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftDown, false);
        }

        if (0.7f < input.ALeftX())
        {
            if (input.PLARight())
            {
                CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftStickRightInst, false);
                CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftRightInst, false);
            }
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftStickRight, false);
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftRight, false);
        }

        if (-0.7f > input.ALeftX())
        {
            if (input.PLALeft())
            {
                CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftStickLeftInst, false);
                CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftLeftInst, false);
            }
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftStickLeft, false);
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftLeft, false);
        }
    }
    else
    {
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftStickRelease, false);
        lx = 0;
        ly = 0;
    }

    if (rightThreshold)
    {
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::RightStickThreshold, false);

        if (input.ARightX() < 0.f)
            rx = input.ARightX() * 128.f;
        else
            rx = input.ARightX() * 127.f;

        if (input.ARightY() < 0.f)
            ry = input.ARightY() * 128.f;
        else
            ry = input.ARightY() * 127.f;

        if (0.7f < input.ARightY())
        {
            if (input.PRAUp())
                CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::RightStickUpInst, false);
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::RightStickUp, false);
        }

        if (-0.7f > input.ARightY())
        {
            if (input.PRADown())
                CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::RightStickDownInst, false);
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::RightStickDown, false);
        }

        if (0.7f < input.ARightX())
        {
            if (input.PRARight())
                CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::RightStickRightInst, false);
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::RightStickRight, false);
        }

        if (-0.7f > input.ARightX())
        {
            if (input.PRALeft())
                CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::RightStickLeftInst, false);
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::RightStickLeft, false);
        }
    }
    else
    {
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::RightStickRelease, false);
        rx = 0;
        ry = 0;
    }

    if (input.PDPUp())
    {
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::DPadUpInst, false);
        if (leftThreshold)
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftUpInst, false);
    }
    else if (input.DDPUp())
    {
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::DPadUp, false);
        if (leftThreshold)
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftUp, false);
    }

    if (input.PDPDown())
    {
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::DPadDownInst, false);
        if (leftThreshold)
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftDownInst, false);
    }
    else if (input.DDPDown())
    {
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::DPadDown, false);
        if (leftThreshold)
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftDown, false);
    }

    if (input.PDPRight())
    {
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::DPadRightInst, false);
        if (leftThreshold)
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftRightInst, false);
    }
    else if (input.DDPRight())
    {
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::DPadRight, false);
        if (leftThreshold)
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftRight, false);
    }

    if (input.PDPLeft())
    {
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::DPadLeftInst, false);
        if (leftThreshold)
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftLeftInst, false);
    }
    else if (input.DDPLeft())
    {
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::DPadLeft, false);
        if (leftThreshold)
            CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LeftLeft, false);
    }

    if (input.PStart())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::StartInst, false);
    if (input.DStart())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::Start, false);

    if (input.PB())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::BInst, false);
    if (input.DB())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::B, false);

    if (input.PA())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::AInst, false);
    if (input.DA())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::A, false);

    if (input.PX())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::XInst, false);
    if (input.DX())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::X, false);

    if (input.PY())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::YInst, false);
    if (input.DY())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::Y, false);

    if (input.PZ())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::ZInst, false);
    if (input.DZ())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::Z, false);

    if (input.PLTrigger())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::LInst, false);
    if (input.DLTrigger())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::L, false);

    if (input.PRTrigger())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::RInst, false);
    if (input.DRTrigger())
        CGuiPhysicalMsg::AddControllerID(state, EPhysicalControllerID::R, false);
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

bool CGuiFrame::SendWidgetMessage(s16 id,
    std::list<std::unique_ptr<CGuiFrameMessageMapNode>>& list,
    CGuiPhysicalMsg::PhysicalMap& state,
    CGuiControllerInfo::CGuiControllerStateInfo& csInfo)
{
    CGuiWidget* widget = FindWidget(id);
    for (std::unique_ptr<CGuiFrameMessageMapNode>& node : list)
    {
        CGuiMessage msg(CGuiMessage::Type(node->GetTrigger().GetTriggerId()),
                        reinterpret_cast<intptr_t>(&state),
                        reinterpret_cast<intptr_t>(&csInfo));
        if (!widget->Message(msg))
            return false;
    }
    return true;
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

void CGuiFrame::EnableLights(u32 lights) const
{
    CGraphics::DisableAllLights();
    zeus::CColor accumColor(zeus::CColor::skBlack);
    ERglLight lightId = ERglLight::Zero;
    for (CGuiLight* light : xa0_lights)
    {
        // accumulate color
        CGraphics::LoadLight(lightId, light->BuildLight());
        CGraphics::EnableLight(lightId);
        ++reinterpret_cast<std::underlying_type_t<ERglLight>&>(lightId);
    }
    if (xa0_lights.empty())
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
    xa0_lights[light->GetLoadedIdx()] = nullptr;
}

void CGuiFrame::AddLight(CGuiLight* light)
{
    xa0_lights[light->GetLoadedIdx()] = light;
}

bool CGuiFrame::GetIsFinishedLoading() const
{
    if (xbc_24_loaded)
        return true;
    for (const CGuiWidget* widget : x90_widgets)
    {
        if (widget->GetIsFinishedLoading())
            continue;
        return false;
    }
    ((CGuiFrame*)this)->xbc_24_loaded = true;
    return true;
}

void CGuiFrame::Touch() const
{
    for (const CGuiWidget* widget : x90_widgets)
        widget->Touch();
}

void CGuiFrame::ProcessControllerInput(const CFinalInput& input)
{
    if (x18_ & 0x4 && input.ControllerIdx() == 0)
    {
        CGuiPhysicalMsg::PhysicalMap state;
        CGuiControllerInfo::CGuiControllerStateInfo stateInfo;
        stateInfo.cIdx = input.ControllerIdx();
        InterpretGUIControllerState(input, state, stateInfo.lx, stateInfo.ly, stateInfo.rx, stateInfo.ry);
        float eventTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - x3c_guiSys.x40_constructTime).count() / 1000.f;

        for (const std::pair<EPhysicalControllerID, CGuiPhysicalMsg::CPhysicalID>& newPair : state)
        {
            auto search = x3c_guiSys.GetRepeatMap().find(newPair.first);
            if (search != x3c_guiSys.GetRepeatMap().end())
                search->second.SetActive(input.ControllerIdx(), eventTime);
        }

        for (std::pair<const EPhysicalControllerID, CGuiAutoRepeatData>& pair : x3c_guiSys.GetRepeatMap())
        {
            pair.second.AddAutoEvent(input.ControllerIdx(), state, eventTime);
        }

        CGuiPhysicalMsg msg(state);
        SetControllerStatus(input.ControllerIdx(), true);

        for (std::pair<const u64, std::unique_ptr<WidgetToLogicalEventMap>>& outer : x7c_messageMap)
        {
            if (outer.second)
            {
                for (std::pair<const s16, std::unique_ptr<LogicalEventList>>& inner : *outer.second)
                {
                    if (inner.second && inner.second->size())
                    {
                        LogicalEventList& list = *inner.second;
                        if (msg.Exists(list.back()->GetTrigger().GetPhysicalMsg()))
                            if (!SendWidgetMessage(inner.first, list, state, stateInfo))
                                break;
                    }
                }
            }
        }
    }
}

bool CGuiFrame::Update(float dt)
{
    if (x34_ != EFrameStates::Four)
        return false;
    if (x18_ & 2)
    {
        EGuiAnimBehListID listId = EGuiAnimBehListID::NegOne;
        bool something = true;
        x44_headWidget->InitializeRGBAFactor();
        x44_headWidget->Update(dt);
        x44_headWidget->RecalculateAllRGBA();
        switch (x34_)
        {
        case EFrameStates::One:
            if (!xbd_flag2)
            {
                CGuiControllerInfo cInfo;
                x44_headWidget->BroadcastMessage(0, &cInfo);
                xbd_flag2 = true;
            }
            break;
        case EFrameStates::Three:
            listId = EGuiAnimBehListID::One;
            break;
        default:
            break;
        }

        if (listId != EGuiAnimBehListID::NegOne)
            x44_headWidget->IsAllAnimsDone(listId, something, ETraversalMode::Zero);

        if (something)
        {
            switch (x34_)
            {
            case EFrameStates::One:
            {
                x34_ = x38_;
                x44_headWidget->SetAnimUpdateState(EGuiAnimBehListID::Zero, false, ETraversalMode::NonRecursive);
                x44_headWidget->InitializeAnimControllers(EGuiAnimBehListID::Two, 0.f, false,
                                                          EGuiAnimInitMode::Five, ETraversalMode::NonRecursive);
                CGuiWidget* camSib = static_cast<CGuiWidget*>(x4c_camera->GetNextSibling());
                if (camSib)
                {
                    camSib->SetAnimUpdateState(EGuiAnimBehListID::Zero, false, ETraversalMode::Zero);
                    camSib->InitializeAnimControllers(EGuiAnimBehListID::Two, 0.f, false,
                                                      EGuiAnimInitMode::Five, ETraversalMode::Zero);
                }
                xbd_flag2 = false;
                break;
            }
            case EFrameStates::Three:
            {
                CGuiControllerInfo cInfo;
                x44_headWidget->BroadcastMessage(1, &cInfo);
                ClearAllMessageMap();
                x18_ &= ~0x3;
                x44_headWidget->ResetAllAnimUpdateState();
                xbd_flag2 = false;
                break;
            }
            default:
                break;
            }
        }

        return x34_ != EFrameStates::Zero;
    }

    return false;
}

void CGuiFrame::Draw(const CGuiWidgetDrawParms& parms) const
{
    if (x18_)
    {
        CGraphics::SetCullMode(ERglCullMode::None);
        CGraphics::SetAmbientColor(zeus::CColor::skWhite);
        DisableLights();
        x4c_camera->Draw(parms);
        // Set one-stage modulate
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha,
                                ERglBlendFactor::InvSrcAlpha, ERglLogicOp::Clear);
        if (x50_background)
            x50_background->Draw(parms);

        for (const CGuiWidget* widget : x90_widgets)
            if (widget->GetIsVisible())
                widget->Draw(parms);
    }
    CGraphics::SetCullMode(ERglCullMode::Front);
}

void CGuiFrame::Stop(const CGuiFrameTransitionOptions& transOpts, EFrameStates states, bool flag)
{
    x18_ &= 0xFFFFFFFB;
    x38_ = states;
    if (flag)
        x34_ = x38_;
    else
    {
        x44_headWidget->InitializeAnimControllers(EGuiAnimBehListID::One, transOpts.xc_, true,
                                                  EGuiAnimInitMode::Two, ETraversalMode::NonRecursive);
        CGuiWidget* camSib = static_cast<CGuiWidget*>(x4c_camera->GetNextSibling());
        if (camSib)
        {
            camSib->InitializeAnimControllers(EGuiAnimBehListID::One, transOpts.xc_, true,
                                              EGuiAnimInitMode::Two, ETraversalMode::Zero);
        }
    }
}

void CGuiFrame::Run(CGuiFrame* frame, const CGuiFrameTransitionOptions& transOpts,
                    EFrameStates states, bool flag)
{
    ResetControllerStatus();
    x34_ = EFrameStates::One;
    x38_ = EFrameStates::Two;
    float len = 0.f;
    x4c_camera->GetBranchAnimLen(EGuiAnimBehListID::Zero, len, ETraversalMode::NonRecursive);
    len += transOpts.xc_ + transOpts.x10_ + transOpts.x14_;
    x44_headWidget->InitializeAnimControllers(EGuiAnimBehListID::Zero, len, true,
                                              EGuiAnimInitMode::One, ETraversalMode::NonRecursive);
    CGuiWidget* camSib = static_cast<CGuiWidget*>(x4c_camera->GetNextSibling());
    if (camSib)
    {
        camSib->InitializeAnimControllers(EGuiAnimBehListID::Zero, len, true,
                                          EGuiAnimInitMode::One, ETraversalMode::Zero);
    }
    x18_ |= 0x7;
    x44_headWidget->RegisterEventHandler(ETraversalMode::Zero);
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
