#ifndef __URDE_CGUIOBJECT_HPP__
#define __URDE_CGUIOBJECT_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{
class CGuiWidgetDrawParms;
class CGuiMessage;
class CGuiFunctionDef;
struct CGuiControllerInfo;

class CGuiObject
{
    zeus::CTransform x4_localXF;
    zeus::CTransform x34_worldXF;
    zeus::CVector3f x64_rotationCenter;
    CGuiObject* x70_parent = nullptr;
    CGuiObject* x74_child = nullptr;
    CGuiObject* x78_nextSibling = nullptr;
public:
    virtual ~CGuiObject();
    virtual void Update(float dt);
    virtual void Draw(const CGuiWidgetDrawParms& parms) const;
    virtual bool Message(const CGuiMessage& msg)=0;

    void MoveInWorld(const zeus::CVector3f& vec);
    const zeus::CVector3f& GetLocalPosition() const {return x4_localXF.m_origin;}
    void SetLocalPosition(const zeus::CVector3f& pos);
    const zeus::CVector3f& GetWorldPosition() const {return x34_worldXF.m_origin;}
    void SetRotationCenter(const zeus::CVector3f& center) {x64_rotationCenter = center;}
    void RotateReset();
    zeus::CVector3f RotateW2O(const zeus::CVector3f& vec) const;
    zeus::CVector3f RotateO2P(const zeus::CVector3f& vec) const;
    zeus::CVector3f RotateTranslateW2O(const zeus::CVector3f& vec) const;
    void MultiplyO2P(const zeus::CTransform& xf);
    void AddChildObject(CGuiObject* obj, bool makeWorldLocal, bool atEnd);
    CGuiObject* RemoveChildObject(CGuiObject* obj, bool makeWorldLocal);
    CGuiObject* GetParent() {return x70_parent;}
    CGuiObject* GetChildObject() {return x74_child;}
    CGuiObject* GetNextSibling() {return x78_nextSibling;}
    void RecalculateTransforms();
    void Reorthogonalize();
    void SetO2WTransform(const zeus::CTransform& xf);
};

}

#endif // __URDE_CGUIOBJECT_HPP__
