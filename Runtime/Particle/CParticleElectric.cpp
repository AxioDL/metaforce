#include "CParticleElectric.hpp"
#include "CGraphics.hpp"

namespace pshag
{

CParticleElectric::CParticleElectric(const TToken<CElectricDescription>& desc)
    : x1c_elecDesc(desc)
{
}

void CParticleElectric::RenderLines()
{
    CGraphics::DisableAllLights();
    CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, false);
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One, ERglLogicOp::Clear);

    Zeus::CTransform viewXfrm = CGraphics::g_ViewMatrix;
    Zeus::CTransform scale;
    scale.Scale(xec_localScale);
    Zeus::CTransform globalScale;
    globalScale.Scale(xe0_globalScale);
    Zeus::CTransform localTranslate;
    localTranslate.Translate(x38_translation);
    Zeus::CTransform globalTranslate;
    globalTranslate.Translate(xa4_globalTranslation);
    Zeus::CTransform global = globalTranslate * xb0_globalOrientation;
}

void CParticleElectric::Update(double)
{
}

void CParticleElectric::Render()
{
}

void CParticleElectric::SetOrientation(const Zeus::CTransform& orientation)
{
    x44_orientation = orientation;
    x438_28 = true;
}

void CParticleElectric::SetTranslation(const Zeus::CVector3f& translation)
{
    x38_translation = translation;
    x438_28 = true;
}

void CParticleElectric::SetGlobalOrientation(const Zeus::CTransform& orientation)
{
    xb0_globalOrientation = orientation;
    x438_28 = true;
}

void CParticleElectric::SetGlobalTranslation(const Zeus::CVector3f& translation)
{
    xa4_globalTranslation = translation;
    x438_28 = true;
}

void CParticleElectric::SetGlobalScale(const Zeus::CVector3f& scale)
{
    xe0_globalScale = scale;
    x438_28 = true;
}

void CParticleElectric::SetLocalScale(const Zeus::CVector3f& scale)
{
    xec_localScale = scale;
    x438_28 = true;
    if (x438_26)
    {

    }
}

void CParticleElectric::SetParticleEmission(bool)
{
}

void CParticleElectric::SetModulationColor(const Zeus::CColor& color)
{
    if (!x1bc_hasModuColor)
        x1bc_hasModuColor = true;

    x1b8_moduColor = color;
    /* TODO: Add child particle systems */
}

const Zeus::CTransform& CParticleElectric::GetOrientation() const
{
    return x44_orientation;
}

const Zeus::CVector3f& CParticleElectric::GetTranslation() const
{
    return x38_translation;
}

const Zeus::CTransform& CParticleElectric::GetGlobalOrientation() const
{
    return xb0_globalOrientation;
}

const Zeus::CVector3f& CParticleElectric::GetGlobalTranslation() const
{
    return xa4_globalTranslation;
}

const Zeus::CVector3f& CParticleElectric::GetGlobalScale() const
{
    return xe0_globalScale;
}

const Zeus::CColor& CParticleElectric::GetModulationColor() const
{
    if (!x1bc_hasModuColor)
        return Zeus::CColor::skWhite;
    return x1b8_moduColor;
}

bool CParticleElectric::IsSystemDeletable() const
{
    return false;
}

std::pair<Zeus::CAABox, bool> CParticleElectric::GetBounds() const
{
    return std::make_pair(Zeus::CAABox(), false);
}

u32 CParticleElectric::GetParticleCount() const
{
    return 0;
}

bool CParticleElectric::SystemHasLight() const
{
    return false;
}

CLight CParticleElectric::GetLight() const
{
    return CLight();
}

bool CParticleElectric::GetParticleEmission() const
{
    return false;
}

void CParticleElectric::DestroyParticles()
{
}

}
