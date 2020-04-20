#pragma once

#include <array>
#include <functional>
#include <memory>
#include <vector>

#include "Runtime/IObj.hpp"
#include "Runtime/GuiSys/CGuiHeadWidget.hpp"
#include "Runtime/GuiSys/CGuiWidgetIdDB.hpp"
#include "Runtime/GuiSys/CGuiWidget.hpp"

#include <boo/IWindow.hpp>

namespace urde {
class CBooModel;
class CGuiCamera;
class CGuiLight;
class CGuiSys;
class CLight;
class CObjectReference;
class CSimplePool;
class CVParamTransfer;
struct CFinalInput;

class CGuiFrame {
  friend class CGuiSys;

private:
  std::vector<CGuiLight*> m_indexedLights;

  CAssetId x0_id;
  u32 x4_ = 0;
  CGuiSys& x8_guiSys;
  std::shared_ptr<CGuiHeadWidget> xc_headWidget;
  std::shared_ptr<CGuiWidget> x10_rootWidget;
  std::shared_ptr<CGuiCamera> x14_camera;
  CGuiWidgetIdDB x18_idDB;
  std::vector<std::shared_ptr<CGuiWidget>> x2c_widgets;
  std::vector<std::shared_ptr<CGuiLight>> x3c_lights;
  int x4c_a;
  int x50_b;
  int x54_c;
  mutable bool x58_24_loaded : 1 = false;

  zeus::CTransform m_aspectTransform;
  float m_aspectConstraint = -1.f;
  float m_maxAspect = -1.f;

  bool m_inMouseDown = false;
  bool m_inCancel = false;
  CGuiWidget* m_mouseDownWidget = nullptr;
  CGuiWidget* m_lastMouseOverWidget = nullptr;
  std::function<void(CGuiWidget*, CGuiWidget*)> m_mouseOverChangeCb;
  std::function<void(CGuiWidget*, bool)> m_mouseDownCb;
  std::function<void(CGuiWidget*, bool)> m_mouseUpCb;
  std::function<void(CGuiWidget*, const boo::SScrollDelta&, int, int)> m_mouseScrollCb;

public:
  CGuiFrame(CAssetId id, CGuiSys& sys, int a, int b, int c, CSimplePool* sp);
  ~CGuiFrame();

  CGuiSys& GetGuiSys() { return x8_guiSys; }
  const CGuiSys& GetGuiSys() const { return x8_guiSys; }
  CAssetId GetAssetId() const { return x0_id; }

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
  void SetMouseOverChangeCallback(std::function<void(CGuiWidget*, CGuiWidget*)>&& cb) {
    m_mouseOverChangeCb = std::move(cb);
  }
  void SetMouseDownCallback(std::function<void(CGuiWidget*, bool)>&& cb) {
    m_mouseDownCb = std::move(cb);
  }
  void SetMouseUpCallback(std::function<void(CGuiWidget*, bool)>&& cb) {
    m_mouseUpCb = std::move(cb);
  }
  void SetMouseScrollCallback(std::function<void(CGuiWidget*, const boo::SScrollDelta&, int, int)>&& cb) {
    m_mouseScrollCb = std::move(cb);
  }

  void Reset();
  void Update(float dt);
  void Draw(const CGuiWidgetDrawParms& parms) const;
  CGuiWidget* BestCursorHit(const zeus::CVector2f& point, const CGuiWidgetDrawParms& parms) const;
  void Initialize();
  void LoadWidgetsInGame(CInputStream& in, CSimplePool* sp);
  void ProcessUserInput(const CFinalInput& input) const;
  bool ProcessMouseInput(const CFinalInput& input, const CGuiWidgetDrawParms& parms);
  void ResetMouseState();

  CGuiWidgetIdDB& GetWidgetIdDB() { return x18_idDB; }
  const CGuiWidgetIdDB& GetWidgetIdDB() const { return x18_idDB; }

  static std::unique_ptr<CGuiFrame> CreateFrame(CAssetId frmeId, CGuiSys& sys, CInputStream& in, CSimplePool* sp);
};

std::unique_ptr<IObj> RGuiFrameFactoryInGame(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                             CObjectReference* selfRef);

} // namespace urde
