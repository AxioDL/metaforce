#include "Runtime/GuiSys/CGuiFrame.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/GuiSys/CGuiCamera.hpp"
#include "Runtime/GuiSys/CGuiHeadWidget.hpp"
#include "Runtime/GuiSys/CGuiLight.hpp"
#include "Runtime/GuiSys/CGuiSys.hpp"
#include "Runtime/GuiSys/CGuiTableGroup.hpp"
#include "Runtime/GuiSys/CGuiWidget.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"
#include "Runtime/Input/CFinalInput.hpp"

#include <zeus/CColor.hpp>

namespace metaforce {

CGuiFrame::CGuiFrame(CAssetId id, CGuiSys& sys, int a, int b, int c, CSimplePool* sp)
: x0_id(id), x8_guiSys(sys), x4c_a(a), x50_b(b), x54_c(c) {
  x3c_lights.reserve(8);
  m_indexedLights.reserve(8);
  x10_rootWidget = std::make_unique<CGuiWidget>(CGuiWidget::CGuiWidgetParms(
      this, false, 0, 0, false, false, false, zeus::skWhite, CGuiWidget::EGuiModelDrawFlags::Alpha, false,
      x8_guiSys.x8_mode != CGuiSys::EUsageMode::Zero, "<root>"s));
  x8_guiSys.m_registeredFrames.insert(this);
}

CGuiFrame::~CGuiFrame() { x8_guiSys.m_registeredFrames.erase(this); }

CGuiWidget* CGuiFrame::FindWidget(std::string_view name) const {
  s16 id = x18_idDB.FindWidgetID(name);
  if (id == -1)
    return nullptr;
  return FindWidget(id);
}

CGuiWidget* CGuiFrame::FindWidget(s16 id) const { return x10_rootWidget->FindWidget(id); }

void CGuiFrame::SortDrawOrder() {
  std::sort(x2c_widgets.begin(), x2c_widgets.end(),
            [](const std::shared_ptr<CGuiWidget>& a, const std::shared_ptr<CGuiWidget>& b) -> bool {
              return a->GetWorldPosition().y() > b->GetWorldPosition().y();
            });
}

void CGuiFrame::EnableLights(ERglLight lights) const {
  CGraphics::DisableAllLights();

  zeus::CColor ambColor(zeus::skBlack);
  ERglLight lightId = 0;
  int enabledLights = 0;
  for (CGuiLight* light : m_indexedLights) {
    if (light == nullptr || !light->GetIsVisible()) {
      ++lightId;
      continue;
    }
    if ((lights & (1 << lightId)) != 0) {
      const auto& geomCol = light->GetGeometryColor();
      if (geomCol.r() != 0.f || geomCol.g() != 0.f || geomCol.b() != 0.f) {
        CGraphics::LoadLight(lightId, light->BuildLight());
        CGraphics::EnableLight(lightId);
      }
      // accumulate ambient color
      ambColor += light->GetAmbientLightColor();
      ++enabledLights;
    }
    ++lightId;
  }
  if (enabledLights == 0) {
    CGraphics::SetAmbientColor(zeus::skWhite);
  } else {
    CGraphics::SetAmbientColor(ambColor);
  }
}

void CGuiFrame::DisableLights() const { CGraphics::DisableAllLights(); }

void CGuiFrame::RemoveLight(CGuiLight* light) {
  if (m_indexedLights.empty())
    return;
  m_indexedLights[light->GetLightId()] = nullptr;
}

void CGuiFrame::AddLight(CGuiLight* light) {
  if (m_indexedLights.empty())
    m_indexedLights.resize(8);
  m_indexedLights[light->GetLightId()] = light;
}

void CGuiFrame::RegisterLight(std::shared_ptr<CGuiLight>&& light) { x3c_lights.push_back(std::move(light)); }

bool CGuiFrame::GetIsFinishedLoading() const {
  if (x58_24_loaded)
    return true;
  for (const auto& widget : x2c_widgets) {
    if (widget->GetIsFinishedLoading())
      continue;
    return false;
  }
  x58_24_loaded = true;
  return true;
}

void CGuiFrame::Touch() const {
  for (const auto& widget : x2c_widgets)
    widget->Touch();
}

void CGuiFrame::SetAspectConstraint(float c) {
  m_aspectConstraint = c;
  CGuiSys::ViewportResizeFrame(this);
}

void CGuiFrame::SetMaxAspect(float c) {
  m_maxAspect = c;
  CGuiSys::ViewportResizeFrame(this);
}

void CGuiFrame::Reset() { x10_rootWidget->Reset(ETraversalMode::Children); }

void CGuiFrame::Update(float dt) { xc_headWidget->Update(dt); }

void CGuiFrame::Draw(const CGuiWidgetDrawParms& parms) const {
  SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format(FMT_STRING("CGuiFrame::Draw FRME_{}"), x0_id).c_str(), zeus::skMagenta);
  CGraphics::SetCullMode(ERglCullMode::None);
  CGraphics::ResetGfxStates();
  CGraphics::SetAmbientColor(zeus::skWhite);
  DisableLights();
  x14_camera->Draw(parms);
  CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::sTevPass805a5ebc);
  CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                          ERglLogicOp::Clear);

  for (const auto& widget : x2c_widgets)
    if (widget->GetIsVisible())
      widget->Draw(parms);

  CGraphics::SetCullMode(ERglCullMode::Front);
}

CGuiWidget* CGuiFrame::BestCursorHit(const zeus::CVector2f& point, const CGuiWidgetDrawParms& parms) const {
  x14_camera->Draw(parms);
  zeus::CMatrix4f vp = CGraphics::GetPerspectiveProjectionMatrix() * CGraphics::g_CameraMatrix.toMatrix4f();
  CGuiWidget* ret = nullptr;
  for (const auto& widget : x2c_widgets)
    if (widget->GetMouseActive() && widget->TestCursorHit(vp, point))
      ret = widget.get();
  return ret;
}

void CGuiFrame::Initialize() {
  SortDrawOrder();
  xc_headWidget->SetColor(xc_headWidget->xa4_color);
  xc_headWidget->DispatchInitialize();
}

void CGuiFrame::LoadWidgetsInGame(CInputStream& in, CSimplePool* sp, u32 version) {
  u32 count = in.ReadLong();
  x2c_widgets.reserve(count);
  for (u32 i = 0; i < count; ++i) {
    FourCC type;
    in.Get(reinterpret_cast<u8*>(&type), 4);
    std::shared_ptr<CGuiWidget> widget = CGuiSys::CreateWidgetInGame(type.toUint32(), in, this, sp, version);
    switch (widget->GetWidgetTypeID().toUint32()) {
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

void CGuiFrame::ProcessUserInput(const CFinalInput& input) const {
  if (input.ControllerIdx() != 0) {
    return;
  }

  for (const auto& widget : x2c_widgets) {
    if (widget->GetIsActive()) {
      widget->ProcessUserInput(input);
    }
  }
}

bool CGuiFrame::ProcessMouseInput(const CFinalInput& input, const CGuiWidgetDrawParms& parms) {
  if (const auto& kbm = input.GetKBM()) {
    zeus::CVector2f point(kbm->m_mouseCoord.norm[0] * 2.f - 1.f, kbm->m_mouseCoord.norm[1] * 2.f - 1.f);
    CGuiWidget* hit = BestCursorHit(point, parms);
    if (hit != m_lastMouseOverWidget) {
      if (m_inMouseDown && m_mouseDownWidget != hit) {
        m_inCancel = true;
        if (m_mouseUpCb)
          m_mouseUpCb(m_mouseDownWidget, true);
      } else if (m_inCancel && m_mouseDownWidget == hit) {
        m_inCancel = false;
        if (m_mouseDownCb)
          m_mouseDownCb(m_mouseDownWidget, true);
      }
      if (m_mouseOverChangeCb)
        m_mouseOverChangeCb(m_lastMouseOverWidget, hit);
      if (hit)
        hit->m_lastScroll.emplace(kbm->m_accumScroll);
      m_lastMouseOverWidget = hit;
    }
    if (hit && hit->m_lastScroll) {
      SScrollDelta delta = kbm->m_accumScroll - *hit->m_lastScroll;
      hit->m_lastScroll.emplace(kbm->m_accumScroll);
      if (!delta.isZero()) {
        hit->m_integerScroll += delta;
        if (m_mouseScrollCb)
          m_mouseScrollCb(hit, delta, int(hit->m_integerScroll.delta[0]), int(hit->m_integerScroll.delta[1]));
        hit->m_integerScroll.delta[0] -= std::trunc(hit->m_integerScroll.delta[0]);
        hit->m_integerScroll.delta[1] -= std::trunc(hit->m_integerScroll.delta[1]);
      }
    }
    if (!m_inMouseDown && kbm->m_mouseButtons[size_t(EMouseButton::Primary)]) {
      m_inMouseDown = true;
      m_inCancel = false;
      m_mouseDownWidget = hit;
      if (m_mouseDownCb)
        m_mouseDownCb(hit, false);
      if (hit)
        return true;
    } else if (m_inMouseDown && !kbm->m_mouseButtons[size_t(EMouseButton::Primary)]) {
      m_inMouseDown = false;
      m_inCancel = false;
      if (m_mouseDownWidget == m_lastMouseOverWidget) {
        if (m_mouseDownWidget) {
          if (CGuiTableGroup* p = static_cast<CGuiTableGroup*>(m_mouseDownWidget->GetParent())) {
            if (p->GetWidgetTypeID() == FOURCC('TBGP')) {
              s16 workerIdx = m_mouseDownWidget->GetWorkerId();
              if (workerIdx >= 0)
                p->DoSelectWorker(workerIdx);
            }
          }
        }
        if (m_mouseUpCb)
          m_mouseUpCb(m_mouseDownWidget, false);
      }
    }
  }
  return false;
}

void CGuiFrame::ResetMouseState() {
  m_inMouseDown = false;
  m_inCancel = false;
  m_mouseDownWidget = nullptr;
  m_lastMouseOverWidget = nullptr;
}

std::unique_ptr<CGuiFrame> CGuiFrame::CreateFrame(CAssetId frmeId, CGuiSys& sys, CInputStream& in, CSimplePool* sp) {
  u32 version = in.ReadLong();
  int a = in.ReadLong();
  int b = in.ReadLong();
  int c = in.ReadLong();

  std::unique_ptr<CGuiFrame> ret = std::make_unique<CGuiFrame>(frmeId, sys, a, b, c, sp);
  ret->LoadWidgetsInGame(in, sp, version);
  return ret;
}

std::unique_ptr<IObj> RGuiFrameFactoryInGame(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& cvParms,
                                             CObjectReference* selfRef) {
  CSimplePool* sp = cvParms.GetOwnedObj<CSimplePool*>();
  std::unique_ptr<CGuiFrame> frame(CGuiFrame::CreateFrame(tag.id, *g_GuiSys, in, sp));
  return TToken<CGuiFrame>::GetIObjObjectFor(std::move(frame));
}

} // namespace metaforce
