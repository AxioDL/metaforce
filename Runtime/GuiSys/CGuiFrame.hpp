#ifndef __URDE_CGUIFRAME_HPP__
#define __URDE_CGUIFRAME_HPP__

#include "CGuiWidget.hpp"
#include "CGuiHeadWidget.hpp"
#include "CGuiWidgetIdDB.hpp"
#include "IObj.hpp"
#include <array>

namespace urde
{
class CGuiSys;
class CLight;
class CGuiCamera;
class CFinalInput;
class CGuiLight;
class CVParamTransfer;
class CObjectReference;
class CSimplePool;
class CBooModel;

class CGuiFrame
{
    friend class CGuiSys;
private:
    CAssetId x0_id;
    u32 x4_ = 0;
    CGuiSys& x8_guiSys;
    std::shared_ptr<CGuiHeadWidget> xc_headWidget;
    std::shared_ptr<CGuiWidget> x10_rootWidget;
    std::shared_ptr<CGuiCamera> x14_camera;
    CGuiWidgetIdDB x18_idDB;
    std::vector<std::shared_ptr<CGuiWidget>> x2c_widgets;
    std::vector<std::shared_ptr<CGuiLight>> x3c_lights;
    std::vector<CGuiLight*> m_indexedLights;
    int x4c_a;
    int x50_b;
    int x54_c;
    bool x58_24_loaded : 1;

    zeus::CTransform m_aspectTransform;
    float m_aspectConstraint = -1.f;
    float m_maxAspect = -1.f;

public:
    CGuiFrame(CAssetId id, CGuiSys& sys, int a, int b, int c, CSimplePool* sp);
    ~CGuiFrame();

    CGuiSys& GetGuiSys() {return x8_guiSys;}
    CAssetId GetAssetId() const {return x0_id;}

    CGuiLight* GetFrameLight(int idx) const { return m_indexedLights[idx]; }
    CGuiCamera* GetFrameCamera() const { return x14_camera.get(); }
    CGuiWidget* FindWidget(std::string_view name) const;
    CGuiWidget* FindWidget(s16 id) const;
    void SetFrameCamera(std::shared_ptr<CGuiCamera>&& camr) { x14_camera = std::move(camr); }
    void SetHeadWidget(std::shared_ptr<CGuiHeadWidget>&& hwig) { xc_headWidget = std::move(hwig); }
    CGuiHeadWidget* GetHeadWidget() const { return xc_headWidget.get(); }
    void SortDrawOrder();
    void EnableLights(u32 lights, CBooModel& model) const;
    void DisableLights() const;
    void RemoveLight(CGuiLight* light);
    void AddLight(CGuiLight* light);
    void RegisterLight(std::shared_ptr<CGuiLight>&& light);
    bool GetIsFinishedLoading() const;
    void Touch() const;
    const zeus::CTransform& GetAspectTransform() const { return m_aspectTransform; }
    void SetAspectConstraint(float c);
    void SetMaxAspect(float c);

    void Update(float dt);
    void Draw(const CGuiWidgetDrawParms& parms) const;
    void Initialize();
    void LoadWidgetsInGame(CInputStream& in, CSimplePool* sp);
    void ProcessUserInput(const CFinalInput& input) const;

    CGuiWidgetIdDB& GetWidgetIdDB() {return x18_idDB;}

    static std::unique_ptr<CGuiFrame> CreateFrame(CAssetId frmeId, CGuiSys& sys, CInputStream& in, CSimplePool* sp);
};

std::unique_ptr<IObj> RGuiFrameFactoryInGame(const SObjectTag& tag, CInputStream& in,
                                             const CVParamTransfer& vparms,
                                             CObjectReference* selfRef);

}

#endif // __URDE_CGUIFRAME_HPP__
