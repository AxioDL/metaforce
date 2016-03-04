#include "CParticleElectric.hpp"
#include "CGenDescription.hpp"
#include "CSwooshDescription.hpp"
#include "CElectricDescription.hpp"
#include "Graphics/CModel.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde
{

CParticleElectric::CParticleElectric(const TToken<CElectricDescription>& desc)
    : x1c_elecDesc(desc)
{
}

void CParticleElectric::SetupLineGXMaterial()
{

}

void CParticleElectric::RenderLines()
{
    CGraphics::DisableAllLights();
    CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, false);
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One, ERglLogicOp::Clear);

    zeus::CTransform viewXfrm = CGraphics::g_ViewMatrix;
    zeus::CTransform localScale;
    localScale.Scale(xec_localScale);
    zeus::CTransform globalScale;
    globalScale.Scale(xe0_globalScale);
    zeus::CTransform localTranslation;
    localTranslation.Translate(x38_translation);
    zeus::CTransform globalTranslation;
    globalTranslation.Translate(xa4_globalTranslation);
    CGraphics::SetModelMatrix(xb0_globalOrientation * globalTranslation * localTranslation * x44_orientation * globalScale);
    CGraphics::SetCullMode(ERglCullMode::None);
    SetupLineGXMaterial();
    /* Iterate line managers */

    CGraphics::SetCullMode(ERglCullMode::Front);
    //CGraphics::SetLineWidth(1.f, ERglTexOffset);
    CGraphics::SetViewPointMatrix(viewXfrm);
}

void CParticleElectric::Update(double)
{
}

void CParticleElectric::Render()
{
}

void CParticleElectric::SetOrientation(const zeus::CTransform& orientation)
{
    x44_orientation = orientation;
    x438_28 = true;
}

void CParticleElectric::SetTranslation(const zeus::CVector3f& translation)
{
    x38_translation = translation;
    x438_28 = true;
}

void CParticleElectric::SetGlobalOrientation(const zeus::CTransform& orientation)
{
    xb0_globalOrientation = orientation;
    x438_28 = true;
}

void CParticleElectric::SetGlobalTranslation(const zeus::CVector3f& translation)
{
    xa4_globalTranslation = translation;
    x438_28 = true;
}

void CParticleElectric::SetGlobalScale(const zeus::CVector3f& scale)
{
    xe0_globalScale = scale;
    x438_28 = true;
}

void CParticleElectric::SetLocalScale(const zeus::CVector3f& scale)
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

void CParticleElectric::SetModulationColor(const zeus::CColor& color)
{
    if (!x1bc_hasModuColor)
        x1bc_hasModuColor = true;

    x1b8_moduColor = color;
    /* TODO: Add child particle systems */
}

const zeus::CTransform& CParticleElectric::GetOrientation() const
{
    return x44_orientation;
}

const zeus::CVector3f& CParticleElectric::GetTranslation() const
{
    return x38_translation;
}

const zeus::CTransform& CParticleElectric::GetGlobalOrientation() const
{
    return xb0_globalOrientation;
}

const zeus::CVector3f& CParticleElectric::GetGlobalTranslation() const
{
    return xa4_globalTranslation;
}

const zeus::CVector3f& CParticleElectric::GetGlobalScale() const
{
    return xe0_globalScale;
}

const zeus::CColor& CParticleElectric::GetModulationColor() const
{
    if (!x1bc_hasModuColor)
        return zeus::CColor::skWhite;
    return x1b8_moduColor;
}

bool CParticleElectric::IsSystemDeletable() const
{
    return false;
}

std::pair<zeus::CAABox, bool> CParticleElectric::GetBounds() const
{
    return std::make_pair(zeus::CAABox(), false);
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
