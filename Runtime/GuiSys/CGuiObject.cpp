#include "CGuiObject.hpp"
#include "CGuiWidgetDrawParms.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

void CGuiObject::Update(float dt)
{
    if (x68_child)
        x68_child->Update(dt);
    if (x6c_nextSibling)
        x6c_nextSibling->Update(dt);
}

void CGuiObject::Draw(const CGuiWidgetDrawParms& parms) const
{
    if (x68_child)
        x68_child->Draw(parms);
    if (x6c_nextSibling)
        x6c_nextSibling->Draw(parms);
}

void CGuiObject::MoveInWorld(const zeus::CVector3f& vec)
{
    if (x64_parent)
        x64_parent->RotateW2O(vec);
    x4_localXF.origin += vec;
    Reorthogonalize();
    RecalculateTransforms();
}

void CGuiObject::SetLocalPosition(const zeus::CVector3f& pos)
{
    MoveInWorld(pos - x4_localXF.origin);
}

void CGuiObject::RotateReset()
{
    x4_localXF.basis = zeus::CMatrix3f::skIdentityMatrix3f;
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
    return x34_worldXF.transposeRotate(vec - x34_worldXF.origin);
}

void CGuiObject::MultiplyO2P(const zeus::CTransform& xf)
{
    x4_localXF = xf * x4_localXF;
    Reorthogonalize();
    RecalculateTransforms();
}

void CGuiObject::AddChildObject(CGuiObject* obj, bool makeWorldLocal, bool atEnd)
{
    obj->x64_parent = this;

    if (!x68_child)
    {
        x68_child = obj;
    }
    else if (atEnd)
    {
        CGuiObject* prev = nullptr;
        CGuiObject* cur = x68_child;
        for (; cur ; cur = cur->x6c_nextSibling) {prev = cur;}
        if (prev)
            prev->x6c_nextSibling = obj;
    }
    else
    {
        obj->x6c_nextSibling = x68_child;
        x68_child = obj;
    }

    if (makeWorldLocal)
    {
        zeus::CVector3f negParentWorld = -x34_worldXF.origin;
        zeus::CMatrix3f basisMat(
            x34_worldXF.basis[0] / x34_worldXF.basis[0].magnitude(),
            x34_worldXF.basis[1] / x34_worldXF.basis[1].magnitude(),
            x34_worldXF.basis[2] / x34_worldXF.basis[2].magnitude());
        zeus::CVector3f xfWorld = basisMat * negParentWorld;
        obj->x4_localXF = zeus::CTransform(basisMat, xfWorld) * obj->x34_worldXF;
    }

    RecalculateTransforms();
}

CGuiObject* CGuiObject::RemoveChildObject(CGuiObject* obj, bool makeWorldLocal)
{
    CGuiObject* prev = nullptr;
    CGuiObject* cur = x68_child;
    for (; cur && cur != obj ; cur = cur->x6c_nextSibling) {prev = cur;}
    if (!cur)
        return nullptr;
    if (prev)
        prev->x6c_nextSibling = cur->x6c_nextSibling;
    cur->x6c_nextSibling = nullptr;
    cur->x64_parent = nullptr;

    if (makeWorldLocal)
        cur->x4_localXF = cur->x34_worldXF;
    cur->RecalculateTransforms();

    return cur;
}

void CGuiObject::RecalculateTransforms()
{
    if (x64_parent)
        x34_worldXF = x64_parent->x34_worldXF * x4_localXF;
    else
        x34_worldXF = x4_localXF;

    if (x6c_nextSibling)
        x6c_nextSibling->RecalculateTransforms();
    if (x68_child)
        x68_child->RecalculateTransforms();
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

void CGuiObject::SetLocalTransform(const zeus::CTransform& xf)
{
    x4_localXF = xf;
    RecalculateTransforms();
}

}
