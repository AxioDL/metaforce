#ifndef _CGUIOBJECT
#define _CGUIOBJECT

#include "Kyoto/Math/CTransform4f.hpp"

class CGuiWidgetDrawParms;
class CMatrix4f;
class CVector2f;

class CGuiObject {
public:
  CGuiObject();
  virtual ~CGuiObject();
  virtual void Update(float dt);
  virtual void Draw(const CGuiWidgetDrawParms& parms) const;
  virtual void Initialize() = 0;
  void MoveInWorld(const CVector3f& offset);
  CVector3f GetWorldPosition() const;
  CVector3f GetLocalPosition() const;
  const CTransform4f& GetWorldTransform() const { return mWorldXF; }
  void SetLocalPosition(const CVector3f& pos);
  CTransform4f& LocalTransform() { return mLocalXF; }
  const CTransform4f& GetO2PTransform() const { return mLocalXF; }
  void SetLocalTransform(const CTransform4f& xf);
  void SetO2PTransform(const CTransform4f& xf) {
    mLocalXF = xf;
    RecalculateTransforms();
  }
  void SetO2WTransform(const CTransform4f& xf);

  void RotateReset();

  CVector3f RotateO2P(const CVector3f& vec) const;
  CVector3f RotateW2O(const CVector3f& vec) const {
    return mWorldXF.TransposeRotate(vec);
  }
  CVector3f RotateTranslateW2O(const CVector3f& vec) const;
  void MultiplyO2P(const CTransform4f& xf);
  void RecalculateTransforms();

  void AddChildObject(CGuiObject* child, bool makeWorldLocal, bool atEnd);

  void SetParent(CGuiObject* obj) { mParent = obj; }

  CGuiObject* Parent();
  const CGuiObject* GetParent() const { return mParent; }
  const CGuiObject* GetChildObject() const;
  CGuiObject* ChildObject();
  const CGuiObject* GetNextSibling() const;
  CGuiObject* NextSibling();

  CVector3f VectorSomething(const CVector3f& vec) {
    return CVector3f(vec.GetX(), vec.GetY(), vec.GetZ());
  }

private:
  CTransform4f mLocalXF;
  CTransform4f mWorldXF;
  CGuiObject* mParent;
  CGuiObject* mChild;
  CGuiObject* mNextSibling;
};
CHECK_SIZEOF(CGuiObject, 0x70)

#endif // _CGUIOBJECT
