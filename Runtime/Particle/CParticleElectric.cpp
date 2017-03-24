#include "CParticleElectric.hpp"
#include "CElectricDescription.hpp"
#include "CGenDescription.hpp"
#include "CSwooshDescription.hpp"
#include "CParticleSwoosh.hpp"
#include "CElementGen.hpp"
#include "Graphics/CModel.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde
{

CRandom16 CParticleElectric::g_GlobalSeed = 99;

void CParticleElectric::RenderSwooshes()
{
}

CParticleElectric::CParticleElectric(const TToken<CElectricDescription>& token)
    : x1c_elecDesc(token)
{
    x450_24 = true;
    /* x438_28_x450_28 = true; demo */
    x450_29 = true; // are 28 and 29 the same between retail and demo?
    CElectricDescription* desc = x1c_elecDesc.GetObj();
    if (desc->x10_SSEG)
        desc->x10_SSEG->GetValue(x28_currentFrame, x150_SSEG);
    else
    {
        x150_SSEG++;
        x154_SCNT = 1;
    }

    if (desc->xc_SCNT)
        desc->xc_SCNT->GetValue(x28_currentFrame, x154_SCNT);
    std::max(0, std::min(x154_SCNT, 32));

    if (desc->x0_LIFE)
        desc->x0_LIFE->GetValue(x28_currentFrame, x2c_LIFE);
    else
        x2c_LIFE = 0x7FFFFF;

    if (desc->x40_SSWH)
    {
        x450_27_HaveSSWH = true;
        for (int i = 0 ; i < x154_SCNT ; i++)
            x1e0_lineManagers[i].SSWH.reset(new CParticleSwoosh(desc->x40_SSWH.m_token, x150_SSEG));
    }

    if (desc->x50_GPSM)
    {
        x450_25_HaveGPSM = true;
        for (int i = 0 ; i < x154_SCNT ; i++)
            x1e0_lineManagers[i].GPSM.reset(new CElementGen(desc->x50_GPSM.m_token,
                                                            CElementGen::EModelOrientationType::Normal,
                                                            CElementGen::EOptionalSystemFlags::One));
    }

    if (desc->x60_EPSM)
    {
        x450_26_HaveEPSM = true;
        for (int i = 0 ; i < x154_SCNT ; i++)
            x1e0_lineManagers[i].EPSM.reset(new CElementGen(desc->x60_EPSM.m_token,
                                                             CElementGen::EModelOrientationType::Normal,
                                                             CElementGen::EOptionalSystemFlags::One));
    }
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
    x450_28 = true;
}

void CParticleElectric::SetTranslation(const zeus::CVector3f& translation)
{
    x38_translation = translation;
    x450_28 = true;
}

void CParticleElectric::SetGlobalOrientation(const zeus::CTransform& orientation)
{
    xb0_globalOrientation = orientation;
    x450_28 = true;
}

void CParticleElectric::SetGlobalTranslation(const zeus::CVector3f& translation)
{
    xa4_globalTranslation = translation;
    x450_28 = true;
}

void CParticleElectric::SetGlobalScale(const zeus::CVector3f& scale)
{
    xe0_globalScale = scale;
    x450_28 = true;
}

void CParticleElectric::SetLocalScale(const zeus::CVector3f& scale)
{
    xec_localScale = scale;
    x450_29 = true;
    if (x450_27_HaveSSWH)
    {
    }
}

void CParticleElectric::SetParticleEmission(bool)
{
}

void CParticleElectric::SetModulationColor(const zeus::CColor& color)
{
    x1b8_moduColor = color;
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
    return x1b8_moduColor;
}

bool CParticleElectric::IsSystemDeletable() const
{
    return false;
}

rstl::optional_object<zeus::CAABox> CParticleElectric::GetBounds() const
{
    return {};
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
    return CLight(zeus::CVector3f::skZero, zeus::CVector3f::skZero,
                  zeus::CColor::skBlack, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f);
}

bool CParticleElectric::GetParticleEmission() const
{
    return false;
}

void CParticleElectric::DestroyParticles()
{
}

}
