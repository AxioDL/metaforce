#include "GuiSys/CGuiFrame.hpp"

#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFeeHelper.hpp"
#include "GuiSys/CGuiHeadWidget.hpp"
#include "GuiSys/CGuiLight.hpp"
#include "GuiSys/CGuiSys.hpp"
#include "GuiSys/CGuiWidget.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "rstl/algorithm.hpp"

namespace rstl {
class CWidgetFartherFromCamera {
public:
  CWidgetFartherFromCamera() {}
  bool operator()(const CGuiWidget* a, const CGuiWidget* b) const {
    const CVector3f aPos = a->GetWorldPosition();
    const CVector3f bPos = b->GetWorldPosition();
    return aPos.GetY() > bPos.GetY();
  }
};
} // namespace rstl

CGuiFrame::CGuiFrame(uint id, CGuiSys& sys, int a, int b, int c, CSimplePool* sp)
: x0_id(id)
, x4_(0)
, x8_guiSys(sys)
, xc_headWidget(nullptr)
, x10_rootWidget(nullptr)
, x14_camera(nullptr)
, x3c_lights(rstl::vector< CGuiLight* >(8, static_cast< CGuiLight* >(nullptr),
                                       rstl::rmemory_allocator()))
, x4c_a(a)
, x50_b(b)
, x54_c(c)
, x58_24_loaded(false) {
  x10_rootWidget = rs_new CGuiWidget(CGuiWidget::CGuiWidgetParms(
      this, false, CGuiWidget::gkDummyWidgetID, CGuiWidget::gkDummyWidgetID, false, false,
      false, CColor::White(), CGuiWidget::kGMDF_Alpha, false,
      !x8_guiSys.GetIsUsedInGame()));
}

CGuiFrame::~CGuiFrame() {
  if (x10_rootWidget) {
    delete x10_rootWidget;
  }
}

CGuiFrame* CGuiFrame::CreateFrame(uint id, CGuiSys& sys, CInputStream& in, CSimplePool* sp) {
  uint version = in.Get< uint >();
  int a = in.ReadLong();
  int b = in.ReadLong();
  int c = in.ReadLong();
  CGuiFrame* frame = rs_new CGuiFrame(id, sys, a, b, c, sp);
  CGuiFeeHelper::SetCurrentLoadingFrame(frame);
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  frame->LoadWidgetsInGame(in, sp, version);
#else
  frame->LoadWidgetsInGame(in, sp);
#endif
  return frame;
}

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
int CGuiFrame::LoadWidgetsInGame(CInputStream& in, CSimplePool* sp, uint version) {
#else
int CGuiFrame::LoadWidgetsInGame(CInputStream& in, CSimplePool* sp) {
#endif
  int count = in.Get< int >();
  x2c_widgets.reserve(count);
  x18_db.Reserve(count);
  for (int i = 0; i < count; ++i) {
    FourCC type = in.ReadLong();
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
    CGuiWidget* widget = FGuiWidgetFactoryInGame(type, this, in, sp, version);
#else
    CGuiWidget* widget = CGuiSys::CreateWidgetInGame(type, in, this, sp);
#endif
    if (widget->GetWidgetTypeID() != 'CAMR' && widget->GetWidgetTypeID() != 'LITE' &&
        widget->GetWidgetTypeID() != 'BGND') {
      x2c_widgets.push_back(widget);
    }
  }
  Initialize();
  return 0;
}

void CGuiFrame::Initialize() {
  SortDrawOrder();
  CGuiHeadWidget* head = xc_headWidget;
  head->SetColor(head->GetColor());
  head->InitializeRecursive();
}

void CGuiFrame::Draw(const CGuiWidgetDrawParms& parms) const {
  CGraphics::SetCullMode(kCM_None);
  CGraphics::ResetGfxStates();
  CGraphics::SetAmbientColor(CColor::White());
  DisableLights();
  x14_camera->Draw(parms);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  for (AUTO(it, x2c_widgets.begin()); it != x2c_widgets.end(); ++it) {
    CGuiWidget* widget = *it;
    if (widget->GetIsVisible()) {
      widget->Draw(parms);
    }
  }
  CGraphics::SetCullMode(kCM_Front);
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
#endif
}

void CGuiFrame::Update(float dt) { xc_headWidget->Update(dt); }

void CGuiFrame::ProcessUserInput(const CFinalInput& input) {
  if (input.ControllerNumber() == 0) {
    for (AUTO(it, x2c_widgets.begin()); it != x2c_widgets.end(); ++it) {
      CGuiWidget* widget = *it;
      if (widget->GetIsActive()) {
        widget->ProcessUserInput(input);
      }
    }
  }
}

void CGuiFrame::Touch() const {
  for (AUTO(it, x2c_widgets.begin()); it != x2c_widgets.end(); ++it) {
    (*it)->Touch();
  }
}

bool CGuiFrame::GetIsFinishedLoading() const {
  if (x58_24_loaded) {
    return true;
  }
  x58_24_loaded = true;
  for (AUTO(it, x2c_widgets.begin()); it != x2c_widgets.end(); ++it) {
    if (!(*it)->GetIsFinishedLoading()) {
      x58_24_loaded = false;
      return false;
    }
  }
  return true;
}

void CGuiFrame::AddLight(CGuiLight* light) { x3c_lights[light->xd8_lightId] = light; }

void CGuiFrame::RemoveLight(CGuiLight* light) {
  if (x3c_lights[light->xd8_lightId] == light) {
    x3c_lights[light->xd8_lightId] = nullptr;
  }
}

void CGuiFrame::DisableLights() const { CGraphics::DisableAllLights(); }

void CGuiFrame::EnableLights(uint mask) const {
  CGraphics::DisableAllLights();
  CColor ambient = CColor::Black();
  int enabledLights = 0;
  for (int i = 0; i < x3c_lights.size(); ++i) {
    if (mask & (1 << i)) {
      CGuiLight* light = x3c_lights[i];
      if (light && light->GetIsVisible()) {
        const CColor& color = light->GetModifiedColor();
        if (color.GetRedu8() != 0 || color.GetGreenu8() != 0 || color.GetBlueu8() != 0) {
          CGraphics::LoadLight(static_cast< ERglLight >(i), light->BuildLight());
          CGraphics::EnableLight(static_cast< ERglLight >(i));
        }
        ambient = CColor::Add(ambient, CColor(light->xdc_ambColor));
        ++enabledLights;
      }
    }
  }
  if (enabledLights == 0) {
    CGraphics::SetAmbientColor(CColor::White());
  } else {
    CGraphics::SetAmbientColor(ambient);
  }
}

void CGuiFrame::SortDrawOrder() {
  rstl::sort(x2c_widgets.begin(), x2c_widgets.end(), rstl::CWidgetFartherFromCamera());
}

void CGuiFrame::RemoveWidgetFromDrawList(CGuiWidget* widget) {
  AUTO(it, x2c_widgets.begin());
  AUTO(end, x2c_widgets.end());
  for (; it != end; ++it) {
    if (*it == widget) {
      x2c_widgets.erase(it);
      break;
    }
  }
}

CGuiWidget* CGuiFrame::FindWidget(const rstl::string& name) const {
  short id = x18_db.FindWidgetID(name);
  if (id != CGuiWidget::InvalidWidgetId()) {
    return FindWidget(id);
  }
  return nullptr;
}

CGuiWidget* CGuiFrame::FindWidget(short id) const { return x10_rootWidget->FindWidget(id); }

void CGuiFrame::SetHeadWidget(CGuiHeadWidget* widget) { xc_headWidget = widget; }

void CGuiFrame::SetFrameCamera(CGuiCamera* camera) { x14_camera = camera; }

CGuiWidget* CGuiFrame::FindWidget(const char* name) const { return FindWidget(rstl::string_l(name)); }

CGuiLight* CGuiFrame::GetFrameLight(int idx) { return x3c_lights[idx]; }
