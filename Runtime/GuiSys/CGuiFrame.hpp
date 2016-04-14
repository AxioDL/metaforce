#ifndef __URDE_CGUIFRAME_HPP__
#define __URDE_CGUIFRAME_HPP__

#include "CGuiWidget.hpp"
#include "CGuiAutoRepeatData.hpp"
#include "CGuiPhysicalMsg.hpp"
#include "CGuiControllerInfo.hpp"
#include "CGuiLogicalEventTrigger.hpp"
#include "CGuiWidgetIdDB.hpp"
#include "IObj.hpp"

namespace urde
{
class CGuiSys;
class CLight;
class CGuiCamera;
class CGuiHeadWidget;
class CFinalInput;
class CGuiLight;
class CVParamTransfer;

enum class EFrameTransitionOptions
{
    Zero
};

struct CGuiFrameTransitionOptions
{
    EFrameTransitionOptions x0_opts;
    bool x4_ = true;
    float x8_ = 1.f;
    float xc_ = 0.f;
    float x10_ = 0.f;
    float x14_ = 0.f;
public:
    CGuiFrameTransitionOptions(EFrameTransitionOptions opts)
    : x0_opts(opts) {}
};

class CGuiFrameMessageMapNode
{
    CGuiLogicalEventTrigger x0_trigger;
public:
    CGuiFrameMessageMapNode(const CGuiPhysicalMsg& msg, int val)
    : x0_trigger(msg, val, false) {}
    const CGuiLogicalEventTrigger& GetTrigger() const {return x0_trigger;}
};

class CGuiFrame
{
public:
    enum class EFrameStates
    {
        Zero = 0,
        One = 1,
        Two = 2,
        Three = 3,
        Four = 4
    };
private:
    bool x0_controllerStatus[4] = {};
    std::string x4_name;
    ResId x14_id;
    u32 x18_ = 0;
    CGuiFrameTransitionOptions x1c_transitionOpts;
    EFrameStates x34_ = EFrameStates::Zero;
    EFrameStates x38_ = EFrameStates::Zero;
    CGuiSys& x3c_guiSys;
    u32 x40_ = 0;
    CGuiHeadWidget* x44_headWidget = nullptr;
    std::unique_ptr<CGuiWidget> x48_rootWidget;
    CGuiCamera* x4c_camera = nullptr;
    CGuiWidget* x50_background = nullptr;
    zeus::CQuaternion x54_;
    CGuiWidgetIdDB x64_idDB;
    using LogicalEventList = std::list<std::unique_ptr<CGuiFrameMessageMapNode>>;
    using WidgetToLogicalEventMap =
    std::unordered_map<s16, std::unique_ptr<LogicalEventList>>;
    std::unordered_map<u64, std::unique_ptr<WidgetToLogicalEventMap>> x7c_messageMap;
    std::vector<CGuiWidget*> x90_widgets;
    std::vector<CGuiLight*> xa0_lights;
    int xb0_a;
    int xb4_b;
    int xb8_c;
    bool xbc_24_loaded : 1;
    bool xbd_flag2 = false;

    static void InterpretGUIControllerState(const CFinalInput& input,
                                            CGuiPhysicalMsg::PhysicalMap& state,
                                            char& lx, char& ly, char& rx, char& ry);

public:
    CGuiFrame(ResId id, const std::string& name, CGuiSys& sys, int a, int b, int c);

    CGuiSys& GetGuiSys() {return x3c_guiSys;}

    CGuiLight* GetFrameLight(int idx) {return xa0_lights[idx];}
    CGuiWidget* FindWidget(const std::string& name) const;
    CGuiWidget* FindWidget(s16 id) const;
    void ResetControllerStatus();
    void SetControllerStatus(int idx, bool set) {x0_controllerStatus[idx] = set;}
    void SetFrameBackground(CGuiWidget* bg) {x50_background = bg;}
    void SetFrameCamera(CGuiCamera* camr) {x4c_camera = camr;}
    void SetHeadWidget(CGuiHeadWidget* hwig) {x44_headWidget = hwig;}
    WidgetToLogicalEventMap* FindWidget2LogicalEventMap(u64 events);
    LogicalEventList* FindLogicalEventList(u64 events, s16 id);
    bool SendWidgetMessage(s16,
        std::list<std::unique_ptr<CGuiFrameMessageMapNode>>&,
        CGuiPhysicalMsg::PhysicalMap& state,
        CGuiControllerInfo::CGuiControllerStateInfo& csInfo);
    void ClearAllMessageMap();
    void ClearMessageMap(const CGuiLogicalEventTrigger* trigger, s16 id);
    void AddMessageMap(const CGuiLogicalEventTrigger* trigger, s16 id);
    void SortDrawOrder();
    void EnableLights(u32 lights) const;
    void DisableLights() const;
    void RemoveLight(CGuiLight* light);
    void AddLight(CGuiLight* light);
    bool GetIsFinishedLoading() const;
    void Touch() const;
    void ProcessControllerInput(const CFinalInput& input);

    bool Update(float dt);
    void Draw(const CGuiWidgetDrawParms& parms) const;
    void Stop(const CGuiFrameTransitionOptions&, EFrameStates, bool);
    void Run(CGuiFrame*, const CGuiFrameTransitionOptions&, EFrameStates, bool);
    void Initialize();
    void LoadWidgetsInGame(CInputStream& in);

    CGuiWidgetIdDB& GetWidgetIdDB() {return x64_idDB;}

    static CGuiFrame* CreateFrame(ResId frmeId, CGuiSys& sys, CInputStream& in);
    static std::string CreateFrameName(ResId frmeId);
};

std::unique_ptr<IObj> RGuiFrameFactoryInGame(const SObjectTag& tag, CInputStream& in,
                                             const CVParamTransfer& vparms);

}

#endif // __URDE_CGUIFRAME_HPP__
