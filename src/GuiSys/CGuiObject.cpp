#include "GuiSys/CGuiObject.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "dolphin/os.h"

CGuiObject::CGuiObject()
: mLocalXF(CTransform4f::Identity())
, mWorldXF(CTransform4f::Identity())
, mParent(nullptr)
, mChild(nullptr)
, mNextSibling(nullptr) {}

CGuiObject::~CGuiObject() {
  delete mChild;
  mChild = nullptr;
  delete mNextSibling;
  mNextSibling = nullptr;
}

void CGuiObject::Update(float dt) {
  if (mChild) {
    mChild->Update(dt);
  }
  if (mNextSibling) {
    mNextSibling->Update(dt);
  }
}

void CGuiObject::Draw(const CGuiWidgetDrawParms& parms) const {
  if (mChild) {
    mChild->Draw(parms);
  }

  if (mNextSibling) {
    mNextSibling->Draw(parms);
  }
}

void CGuiObject::MoveInWorld(const CVector3f& offset) {
  if (GetParent() != nullptr) {
    GetParent()->RotateW2O(offset);
  }
  mLocalXF.AddTranslation(offset);
  RecalculateTransforms();
}

CVector3f CGuiObject::GetWorldPosition() const { return mWorldXF.GetTranslation(); }

CVector3f CGuiObject::GetLocalPosition() const { return mLocalXF.GetTranslation(); }

void CGuiObject::SetLocalPosition(const CVector3f& pos) {
  MoveInWorld(pos - mLocalXF.GetTranslation());
}

void CGuiObject::RotateReset() {
  const CVector3f tmpPos = mLocalXF.GetTranslation();
  mLocalXF = CTransform4f::Identity();
  mLocalXF.SetTranslation(tmpPos);
  RecalculateTransforms();
}

CVector3f CGuiObject::RotateO2P(const CVector3f& vec) const {
  const CVector3f tmp = mLocalXF.Rotate(vec);
  return tmp;
}

inline CVector3f StupidSubtract(const CVector3f& lhs, const CVector3f& rhs) {
  return CVector3f(lhs.GetX() - rhs.GetX(), lhs.GetY() - rhs.GetY(), lhs.GetZ() - rhs.GetZ());
}

CVector3f CGuiObject::RotateTranslateW2O(const CVector3f& vec) const {
  const CVector3f tmp =
      mWorldXF.TransposeRotate(StupidSubtract(vec, mWorldXF.GetTranslation()));
  return tmp;
}

void CGuiObject::MultiplyO2P(const CTransform4f& xf) {
  mLocalXF = xf * mLocalXF;
  RecalculateTransforms();
}

void CGuiObject::AddChildObject(CGuiObject* child, const bool makeWorldLocal, const bool atEnd) {
  child->SetParent(this);

  CGuiObject* cur = mChild;
  if (cur == nullptr) {
    mChild = child;
  } else if (atEnd) {
    do {
      CGuiObject* next = cur->mNextSibling;
      if (next == nullptr) {
        cur->mNextSibling = child;
        break;
      }
      cur = next;
    } while (true);
  } else {
    child->mNextSibling = mChild;
    mChild = child;
  }

  if (makeWorldLocal) {
    const CGuiObject* parent = child->GetParent();
    CTransform4f worldLocalXf = CTransform4f::Identity();
    CVector3f position = parent->mWorldXF.GetTranslation() * -1.f;
    const CVector3f scale(parent->mWorldXF.GetColumn(0).Magnitude(),
                         parent->mWorldXF.GetColumn(1).Magnitude(),
                         parent->mWorldXF.GetColumn(2).Magnitude());
    const CVector3f& m2 = (1.f / scale.GetZ()) * parent->mWorldXF.GetColumn(2);
    const CVector3f& m1 = (1.f / scale.GetY()) * parent->mWorldXF.GetColumn(1);
    const CVector3f& m0 = (1.f / scale.GetX()) * parent->mWorldXF.GetColumn(0);
    const CMatrix3f tmpMtx(m0, m1, m2);
    const CVector3f pos = tmpMtx * position;

    worldLocalXf = CTransform4f(
        tmpMtx.GetColumn(0).GetX(), tmpMtx.GetColumn(1).GetX(), tmpMtx.GetColumn(2).GetX(), pos.GetX(),
        tmpMtx.GetColumn(0).GetY(), tmpMtx.GetColumn(1).GetY(), tmpMtx.GetColumn(2).GetY(), pos.GetY(),
        tmpMtx.GetColumn(0).GetZ(), tmpMtx.GetColumn(1).GetZ(), tmpMtx.GetColumn(2).GetZ(), pos.GetZ());
    child->mLocalXF = worldLocalXf * child->mWorldXF;
  }

  RecalculateTransforms();
}

const CGuiObject* CGuiObject::GetChildObject() const { return mChild; }

CGuiObject* CGuiObject::ChildObject() { return mChild; }
const CGuiObject* CGuiObject::GetNextSibling() const { return mNextSibling; }
CGuiObject* CGuiObject::NextSibling() { return mNextSibling; }

CGuiObject* CGuiObject::Parent() { return mParent; }

void CGuiObject::RecalculateTransforms() {
  if (mParent) {
    mWorldXF = mParent->mWorldXF * mLocalXF;
  } else {
    mWorldXF = mLocalXF;
  }
  if (mNextSibling) {
    mNextSibling->RecalculateTransforms();
  }
  if (mChild) {
    mChild->RecalculateTransforms();
  }
}

void CGuiObject::SetLocalTransform(const CTransform4f& xf) {
  mLocalXF = xf;
  RecalculateTransforms();
}

void CGuiObject::SetO2WTransform(const CTransform4f& worldXf) {
  const CTransform4f invXf = mParent->mWorldXF.GetQuickInverse();
  const CTransform4f localXf = invXf * worldXf;
  SetLocalTransform(localXf);
}
