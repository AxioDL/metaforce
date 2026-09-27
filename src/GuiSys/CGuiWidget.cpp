#include "GuiSys/CGuiWidget.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"

#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "stdio.h"

const short CGuiWidget::gkDummyWidgetID = 0;
const short CGuiWidget::gkInvalidWidgetId = -1;

CGuiWidget::CGuiWidgetParms::CGuiWidgetParms(CGuiFrame* frame, bool useAnimController,
                                             const short selfId, const short parentId,
                                             const bool defaultVisible, const bool defaultActive,
                                             const bool cullFaces, const CColor& color,
                                             EGuiModelDrawFlags drawFlags, const bool g,
                                             const bool h)
: mFrame(frame)
, mUseAnimController(useAnimController)
, mSelfId(selfId)
, mParentId(parentId)
, mDefaultVisible(defaultVisible)
, mDefaultActive(defaultActive)
, mCullFaces(cullFaces)
, mG(g)
, mH(h)
, mColor(color)
, mDrawFlags(drawFlags) {}

CGuiWidget* CGuiWidget::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  CGuiWidget* ret = rs_new CGuiWidget(parms);
  ret->ParseBaseInfo(frame, in, parms);
  return ret;
}

CGuiWidget::CGuiWidgetParms CGuiWidget::ReadWidgetHeader(CGuiFrame* frame, CInputStream& in) {
  rstl::string name(in);
  const short& selfId = frame->WidgetIdDB().AddWidget(name);
  rstl::string parent(in);
  const short& parentId = frame->WidgetIdDB().AddWidget(parent);

  bool useAnimController = in.ReadBool();
  bool defaultVis = in.Get< bool >();
  bool defaultActive = in.Get< bool >();
  bool cullFaces = in.Get< bool >();
  CColor color(in);
  EGuiModelDrawFlags flags = static_cast< EGuiModelDrawFlags >(in.Get< uint >());

  return CGuiWidgetParms(frame, useAnimController, selfId, parentId, defaultVis, defaultActive,
                         cullFaces, color, flags, true, false);
}

CGuiWidget::CGuiWidget(const CGuiWidgetParms& parms)
: mSelfId(parms.mSelfId)
, mParentId(parms.mParentId)
, mTransform(CTransform4f::Identity())
, mColor(parms.mColor)
, mColor2(mColor)
, mDrawFlags(parms.mDrawFlags)
, mFrame(parms.mFrame)
, mWorkerId(-1)
, mPg(parms.mG)
, mIsVisible(parms.mDefaultVisible)
, mIsActive(parms.mDefaultActive)
, mIsSelectable(true)
, mEventLock(false)
, mCullFaces(parms.mCullFaces)
, mDepthGreater(false)
, mDepthTest(true)
, mDepthWrite(false)
, xb7_25_(true) {
  RecalcWidgetColor(kTM_Single);
}

CGuiWidget::~CGuiWidget() { mFrame->RemoveWidgetFromDrawList(this); }

void CGuiWidget::ParseBaseInfo(CGuiFrame* frame, CInputStream& in, const CGuiWidgetParms& parms) {
  CGuiWidget* parent = frame->FindWidget(parms.mParentId);
  bool isWorker = in.ReadBool();
  if (isWorker) {
    mWorkerId = in.ReadShort();
  }
  CVector3f translation(in);
  CMatrix3f orientation(in);
  const CTransform4f transform(orientation, translation);
  mTransform = transform;
  ReapplyXform();
  CVector3f unused(in);
  ReadUnusedThing(in);
  in.ReadShort();
  if (isWorker) {
    if (!parent->AddWorkerWidget(this)) {
      printf("Warning: Discarding useless worker id.  Parent is not a compound widget.");
      mWorkerId = -1;
    }
  }
  parent->AddChildWidget(this, false, true);
}

void CGuiWidget::ReadUnusedThing(CInputStream& in) { in.ReadLong(); }

void CGuiWidget::Draw(const CGuiWidgetDrawParms&) const {}

void CGuiWidget::ProcessUserInput(const CFinalInput&) {}

void CGuiWidget::Update(float dt) {
  if (ChildObject()) {
    ChildObject()->Update(dt);
  }
  if (NextSibling()) {
    NextSibling()->Update(dt);
  }
}

void CGuiWidget::InitializeRecursive() {
  Initialize();
  if (ChildObject()) {
    static_cast< CGuiWidget* >(ChildObject())->InitializeRecursive();
  }
  if (NextSibling()) {
    static_cast< CGuiWidget* >(NextSibling())->InitializeRecursive();
  }
}

bool CGuiWidget::GetIsFinishedLoading() const { return GetIsFinishedLoadingWidgetSpecific(); }

CGuiWidget* CGuiWidget::FindWidget(short id) {
  if (mSelfId == id) {
    return this;
  }
  if (ChildObject()) {
    CGuiWidget* found = static_cast< CGuiWidget* >(ChildObject())->FindWidget(id);
    if (found) {
      return found;
    }
  }
  if (NextSibling()) {
    CGuiWidget* found = static_cast< CGuiWidget* >(NextSibling())->FindWidget(id);
    if (found) {
      return found;
    }
  }
  return nullptr;
}

void CGuiWidget::SetColor(const CColor& color) {
  mColor = color;
  RecalcWidgetColor(kTM_Children);
}

void CGuiWidget::RecalcWidgetColor(ETraversalMode mode) {
  CGuiWidget* parent = static_cast< CGuiWidget* >(Parent());
  if (parent) {
    mColor2 = CColor::Modulate(mColor, parent->GetModifiedColor());
  } else {
    mColor2 = mColor;
  }
  switch (mode) {
  case kTM_Single:
    break;
  case kTM_ChildrenAndSiblings:
    if (NextSibling()) {
      static_cast< CGuiWidget* >(NextSibling())->RecalcWidgetColor(kTM_ChildrenAndSiblings);
    }
  case kTM_Children:
    if (ChildObject()) {
      static_cast< CGuiWidget* >(ChildObject())->RecalcWidgetColor(kTM_ChildrenAndSiblings);
    }
    break;
  }
}

void CGuiWidget::SetVisibility(const bool visible, ETraversalMode mode) {
  switch (mode) {
  case kTM_Single:
    break;
  case kTM_Children:
    if (ChildObject()) {
      static_cast< CGuiWidget* >(ChildObject())->SetVisibility(visible, kTM_ChildrenAndSiblings);
    }
    break;
  case kTM_ChildrenAndSiblings:
    if (ChildObject()) {
      static_cast< CGuiWidget* >(ChildObject())->SetVisibility(visible, kTM_ChildrenAndSiblings);
    }
    if (NextSibling()) {
      static_cast< CGuiWidget* >(NextSibling())->SetVisibility(visible, kTM_ChildrenAndSiblings);
    }
    break;
  }
  SetIsVisible(visible);
}

void CGuiWidget::AddChildWidget(CGuiWidget* widget, bool makeWorldLocal, bool atEnd) {
  AddChildObject(widget, makeWorldLocal, atEnd);
}

CVector3f CGuiWidget::GetIdlePosition() const {
  return CVector3f(mTransform.Get03(), mTransform.Get13(), mTransform.Get23());
}

void CGuiWidget::ReapplyXform() {
  RotateReset();
  SetLocalPosition(CVector3f::Zero());
  MultiplyO2P(mTransform);
}

void CGuiWidget::SetIsVisible(const bool visible) {
  mIsVisible = visible;
  OnVisible();
}

void CGuiWidget::SetIsActive(const bool active) {
  if (mIsActive != active) {
    mIsActive = active;
    OnActivate();
  }
}

void CGuiWidget::OnVisible() {}

void CGuiWidget::OnActivate() {}
