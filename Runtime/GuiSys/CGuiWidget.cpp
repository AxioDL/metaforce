#include "Runtime/GuiSys/CGuiWidget.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"

#include <logvisor/logvisor.hpp>

namespace urde {
static logvisor::Module Log("urde::CGuiWidget");

CGuiWidget::CGuiWidget(const CGuiWidgetParms& parms)
: x70_selfId(parms.x6_selfId)
, x72_parentId(parms.x8_parentId)
, m_initColor(parms.x10_color)
, xa4_color(parms.x10_color)
, xa8_color2(parms.x10_color)
, xac_drawFlags(parms.x14_drawFlags)
, xb0_frame(parms.x0_frame)
, m_name(parms.m_name) {
  xb6_24_pg = parms.xd_g;
  xb6_25_isVisible = parms.xa_defaultVisible;
  xb6_26_isActive = parms.xb_defaultActive;
  xb6_29_cullFaces = parms.xc_cullFaces;
  RecalcWidgetColor(ETraversalMode::Single);
}

CGuiWidget::CGuiWidgetParms CGuiWidget::ReadWidgetHeader(CGuiFrame* frame, CInputStream& in) {
  std::string name = in.readString(-1);
  s16 selfId = frame->GetWidgetIdDB().AddWidget(name);
  std::string parent = in.readString(-1);
  s16 parentId = frame->GetWidgetIdDB().AddWidget(parent);

  bool useAnimController = in.readBool();
  bool defaultVis = in.readBool();
  bool defaultActive = in.readBool();
  bool cullFaces = in.readBool();
  zeus::CColor color;
  color.readRGBABig(in);
  EGuiModelDrawFlags df = EGuiModelDrawFlags(in.readUint32Big());

  return CGuiWidget::CGuiWidgetParms(frame, useAnimController, selfId, parentId, defaultVis, defaultActive, cullFaces,
                                     color, df, true, false, std::move(name));
}

std::shared_ptr<CGuiWidget> CGuiWidget::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  std::shared_ptr<CGuiWidget> ret = std::make_shared<CGuiWidget>(parms);
  ret->ParseBaseInfo(frame, in, parms);
  return ret;
}

void CGuiWidget::Initialize() {}

void CGuiWidget::ParseBaseInfo(CGuiFrame* frame, CInputStream& in, const CGuiWidgetParms& parms) {
  CGuiWidget* parent = frame->FindWidget(parms.x8_parentId);
  bool isWorker = in.readBool();
  if (isWorker)
    xb4_workerId = in.readInt16Big();
  zeus::CVector3f trans = zeus::CVector3f::ReadBig(in);
  zeus::CMatrix3f orient = zeus::CMatrix3f::ReadBig(in);
  x74_transform = zeus::CTransform(orient, trans);
  m_initTransform = x74_transform;
  ReapplyXform();
  (void)zeus::CVector3f::ReadBig(in); // Unused
  in.readUint32Big();
  in.readUint16Big();
  if (isWorker) {
    if (!parent->AddWorkerWidget(this)) {
      Log.report(logvisor::Warning, FMT_STRING("Warning: Discarding useless worker id. Parent is not a compound widget."));
      xb4_workerId = -1;
    }
  }
  parent->AddChildWidget(this, false, true);
  m_initLocalXF = x4_localXF;
}

void CGuiWidget::Reset(ETraversalMode mode) {
  xa4_color = m_initColor;
  xa8_color2 = m_initColor;
  x74_transform = m_initTransform;
  ReapplyXform();
  x4_localXF = m_initLocalXF;
  RecalculateTransforms();

  switch (mode) {
  case ETraversalMode::Children: {
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    if (child)
      child->Reset(ETraversalMode::ChildrenAndSiblings);
    break;
  }
  case ETraversalMode::ChildrenAndSiblings: {
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    if (child)
      child->Reset(ETraversalMode::ChildrenAndSiblings);
    CGuiWidget* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
    if (nextSib)
      nextSib->Reset(ETraversalMode::ChildrenAndSiblings);
    break;
  }
  default:
    break;
  }
}

void CGuiWidget::Update(float dt) {
  CGuiWidget* ch = static_cast<CGuiWidget*>(GetChildObject());
  if (ch)
    ch->Update(dt);
  CGuiWidget* sib = static_cast<CGuiWidget*>(GetNextSibling());
  if (sib)
    sib->Update(dt);
}

void CGuiWidget::Draw(const CGuiWidgetDrawParms&) {}
void CGuiWidget::ProcessUserInput(const CFinalInput& input) {}
void CGuiWidget::Touch() {}

bool CGuiWidget::GetIsVisible() const { return xb6_25_isVisible; }

bool CGuiWidget::GetIsActive() const { return xb6_26_isActive; }

bool CGuiWidget::GetMouseActive() const { return m_mouseActive; }

void CGuiWidget::InitializeRGBAFactor() {
  CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
  if (child)
    child->InitializeRGBAFactor();
  CGuiWidget* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
  if (nextSib)
    nextSib->InitializeRGBAFactor();
}

bool CGuiWidget::GetIsFinishedLoadingWidgetSpecific() { return true; }

void CGuiWidget::SetTransform(const zeus::CTransform& xf) {
  x74_transform = xf;
  ReapplyXform();
}

void CGuiWidget::SetIdlePosition(const zeus::CVector3f& pos, bool reapply) {
  x74_transform.origin = pos;
  if (reapply)
    ReapplyXform();
}

void CGuiWidget::ReapplyXform() {
  RotateReset();
  SetLocalPosition(zeus::skZero3f);
  MultiplyO2P(x74_transform);
}

void CGuiWidget::AddChildWidget(CGuiWidget* widget, bool makeWorldLocal, bool atEnd) {
  AddChildObject(widget, makeWorldLocal, atEnd);
}

bool CGuiWidget::AddWorkerWidget(CGuiWidget* worker) { return false; }

void CGuiWidget::SetVisibility(bool visible, ETraversalMode mode) {
  switch (mode) {
  case ETraversalMode::Children: {
    auto* child = static_cast<CGuiWidget*>(GetChildObject());
    if (child) {
      child->SetVisibility(visible, ETraversalMode::ChildrenAndSiblings);
    }
    break;
  }
  case ETraversalMode::ChildrenAndSiblings: {
    auto* child = static_cast<CGuiWidget*>(GetChildObject());
    if (child) {
      child->SetVisibility(visible, ETraversalMode::ChildrenAndSiblings);
    }
    auto* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
    if (nextSib) {
      nextSib->SetVisibility(visible, ETraversalMode::ChildrenAndSiblings);
    }
    break;
  }
  default:
    break;
  }
  SetIsVisible(visible);
}

void CGuiWidget::RecalcWidgetColor(ETraversalMode mode) {
  CGuiWidget* parent = static_cast<CGuiWidget*>(GetParent());
  if (parent)
    xa8_color2 = xa4_color * parent->xa8_color2;
  else
    xa8_color2 = xa4_color;

  switch (mode) {
  case ETraversalMode::ChildrenAndSiblings: {
    CGuiWidget* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
    if (nextSib)
      nextSib->RecalcWidgetColor(ETraversalMode::ChildrenAndSiblings);
    [[fallthrough]];
  }
  case ETraversalMode::Children: {
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    if (child)
      child->RecalcWidgetColor(ETraversalMode::ChildrenAndSiblings);
    break;
  }
  default:
    break;
  }
}

CGuiWidget* CGuiWidget::FindWidget(s16 id) {
  if (x70_selfId == id)
    return this;
  CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
  if (child) {
    CGuiWidget* found = child->FindWidget(id);
    if (found)
      return found;
  }
  CGuiWidget* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
  if (nextSib) {
    CGuiWidget* found = nextSib->FindWidget(id);
    if (found)
      return found;
  }
  return nullptr;
}

bool CGuiWidget::GetIsFinishedLoading() { return GetIsFinishedLoadingWidgetSpecific(); }

void CGuiWidget::DispatchInitialize() {
  Initialize();
  CGuiWidget* ch = static_cast<CGuiWidget*>(GetChildObject());
  if (ch)
    ch->DispatchInitialize();
  CGuiWidget* sib = static_cast<CGuiWidget*>(GetNextSibling());
  if (sib)
    sib->DispatchInitialize();
}

void CGuiWidget::SetColor(const zeus::CColor& color) {
  xa4_color = color;
  RecalcWidgetColor(ETraversalMode::Children);
}

void CGuiWidget::OnActiveChange() {}
void CGuiWidget::OnVisibleChange() {}

void CGuiWidget::SetIsVisible(bool visible) {
  xb6_25_isVisible = visible;
  OnVisibleChange();
}

void CGuiWidget::SetIsActive(bool active) {
  if (active == xb6_26_isActive) {
    return;
  }

  xb6_26_isActive = active;
  OnActiveChange();
}

} // namespace urde
