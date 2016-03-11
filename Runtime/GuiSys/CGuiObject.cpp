#include "CGuiObject.hpp"
#include "CGuiWidgetDrawParams.hpp"

namespace urde
{

CGuiObject::~CGuiObject()
{
    delete x74_child;
    delete x78_nextSibling;
}

void CGuiObject::Update(float dt)
{
    if (x74_child)
        x74_child->Update(dt);
    if (x78_nextSibling)
        x78_nextSibling->Update(dt);
}

void CGuiObject::Draw(const CGuiWidgetDrawParms& parms) const
{
    if (x74_child)
        x74_child->Draw(parms);
    if (x78_nextSibling)
        x78_nextSibling->Draw(parms);
}

void CGuiObject::MoveInWorld(const zeus::CVector3f& vec)
{
    if (x70_parent)
        x70_parent->RotateW2O(vec);
    x4_localXF.m_origin += vec;
    Reorthogonalize();
    RecalculateTransforms();
}

void CGuiObject::RotateReset()
{
    x4_localXF.m_basis = zeus::CMatrix3f::skIdentityMatrix3f;
    Reorthogonalize();
    RecalculateTransforms();
}

zeus::CVector3f CGuiObject::RotateW2O(const zeus::CVector3f& vec) const
{
    return x34_worldXF.transposeRotate(vec);
}

zeus::CVector3f CGuiObject::RotateO2P(const zeus::CVector3f& vec) const
{
    return x4_localXF.rotate(vec);
}

zeus::CVector3f CGuiObject::RotateTranslateW2O(const zeus::CVector3f& vec) const
{
    return x34_worldXF.transposeRotate(vec - x34_worldXF.m_origin);
}

void CGuiObject::MultiplyO2P(const zeus::CTransform& xf)
{
    x4_localXF.m_origin += x64_rotationCenter;
    x4_localXF = xf * x4_localXF;
    x4_localXF.m_origin -= x64_rotationCenter;
    Reorthogonalize();
    RecalculateTransforms();
}

void CGuiObject::AddChildObject(CGuiObject* obj, bool makeWorldLocal, bool atEnd)
{
    obj->x70_parent = this;

    if (!x74_child)
    {
        x74_child = obj;
    }
    else if (atEnd)
    {
        CGuiObject* prev = nullptr;
        CGuiObject* cur = x74_child;
        for (; cur ; cur = cur->x78_nextSibling) {prev = cur;}
        if (prev)
            prev->x78_nextSibling = obj;
    }
    else
    {
        obj->x78_nextSibling = x74_child;
        x74_child = obj;
    }

    if (makeWorldLocal)
    {
        zeus::CVector3f negParentWorld = -x34_worldXF.m_origin;
        zeus::CMatrix3f basisMat(
            x34_worldXF.m_basis[0] / x34_worldXF.m_basis[0].magnitude(),
            x34_worldXF.m_basis[1] / x34_worldXF.m_basis[1].magnitude(),
            x34_worldXF.m_basis[2] / x34_worldXF.m_basis[2].magnitude());
        zeus::CVector3f xfWorld = basisMat * negParentWorld;
        obj->x4_localXF = zeus::CTransform(basisMat, xfWorld) * obj->x34_worldXF;
    }

    RecalculateTransforms();
}

CGuiObject* CGuiObject::RemoveChildObject(CGuiObject* obj, bool makeWorldLocal)
{
    CGuiObject* prev = nullptr;
    CGuiObject* cur = x74_child;
    for (; cur && cur != obj ; cur = cur->x78_nextSibling) {prev = cur;}
    if (!cur)
        return nullptr;
    if (prev)
        prev->x78_nextSibling = cur->x78_nextSibling;
    cur->x78_nextSibling = nullptr;
    cur->x70_parent = nullptr;

    if (makeWorldLocal)
        cur->x4_localXF = cur->x34_worldXF;
    cur->RecalculateTransforms();

    return cur;
}

void CGuiObject::RecalculateTransforms()
{
    if (x70_parent)
    {
        x4_localXF.m_origin += x64_rotationCenter;
        x34_worldXF = x70_parent->x34_worldXF * x4_localXF;
        x4_localXF.m_origin -= x64_rotationCenter;
        x34_worldXF.m_origin -= x64_rotationCenter;
    }
    else
    {
        x34_worldXF = x4_localXF;
    }

    if (x78_nextSibling)
        x78_nextSibling->RecalculateTransforms();
    if (x74_child)
        x74_child->RecalculateTransforms();
}

void CGuiObject::Reorthogonalize()
{
    static bool Global = false;
    if (Global)
    {
        x4_localXF.orthonormalize();
        RecalculateTransforms();
    }
}

void CGuiObject::SetO2WTransform(const zeus::CTransform& xf)
{
    x4_localXF = GetParent()->x34_worldXF.inverse() * xf;
    RecalculateTransforms();
}

}
