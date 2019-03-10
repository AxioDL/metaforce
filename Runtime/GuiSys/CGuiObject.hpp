#pragma once

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CQuaternion.hpp"
#include "zeus/CTransform.hpp"

namespace urde {
struct CGuiWidgetDrawParms;

class CGuiObject : public std::enable_shared_from_this<CGuiObject> {
protected:
  zeus::CTransform m_initLocalXF;
  zeus::CTransform x4_localXF;
  zeus::CTransform x34_worldXF;
  CGuiObject* x64_parent = nullptr;
  CGuiObject* x68_child = nullptr;
  CGuiObject* x6c_nextSibling = nullptr;

public:
  virtual ~CGuiObject() = default;
  virtual void Update(float dt);
  virtual void Draw(const CGuiWidgetDrawParms& parms) const;
  virtual bool TestCursorHit(const zeus::CMatrix4f& vp, const zeus::CVector2f& point) const { return false; }
  virtual void Initialize() = 0;

  void MoveInWorld(const zeus::CVector3f& vec);
  const zeus::CVector3f& GetInitialLocalPosition() const { return m_initLocalXF.origin; }
  const zeus::CTransform& GetInitialLocalTransform() const { return m_initLocalXF; }
  const zeus::CVector3f& GetLocalPosition() const { return x4_localXF.origin; }
  const zeus::CTransform& GetLocalTransform() const { return x4_localXF; }
  void SetLocalPosition(const zeus::CVector3f& pos);
  const zeus::CVector3f& GetWorldPosition() const { return x34_worldXF.origin; }
  const zeus::CTransform& GetWorldTransform() const { return x34_worldXF; }
  void RotateReset();
  zeus::CVector3f RotateW2O(const zeus::CVector3f& vec) const;
  zeus::CVector3f RotateO2P(const zeus::CVector3f& vec) const;
  zeus::CVector3f RotateTranslateW2O(const zeus::CVector3f& vec) const;
  void MultiplyO2P(const zeus::CTransform& xf);
  void AddChildObject(CGuiObject* obj, bool makeWorldLocal, bool atEnd);
  CGuiObject* RemoveChildObject(CGuiObject* obj, bool makeWorldLocal);
  CGuiObject* GetParent() const { return x64_parent; }
  CGuiObject* GetChildObject() const { return x68_child; }
  CGuiObject* GetNextSibling() const { return x6c_nextSibling; }
  void RecalculateTransforms();
  void SetO2WTransform(const zeus::CTransform& xf);
  void SetLocalTransform(const zeus::CTransform& xf);
};

} // namespace urde
