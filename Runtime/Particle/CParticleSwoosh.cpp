#include "CParticleSwoosh.hpp"
#include "CSwooshDescription.hpp"
#include "CParticleGlobals.hpp"
#include <chrono>

namespace urde
{

int CParticleSwoosh::g_ParticleSystemAliveCount = 0;

CParticleSwoosh::CParticleSwoosh(const TToken<CSwooshDescription>& desc, int leng)
    : x1c_desc(desc), x1c0_rand(x1c_desc->x45_26_CRND ?
      std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count() : 99)
{
    x1d0_24_emitting = true;
    ++g_ParticleSystemAliveCount;

    if (leng > 0)
        x1b4_LENG = leng;
    else if (CIntElement* leng = x1c_desc->x10_LENG.get())
        leng->GetValue(0, x1b4_LENG);
    x1b4_LENG += 1;

    if (CIntElement* side = x1c_desc->x18_SIDE.get())
        side->GetValue(0, x1b8_SIDE);

    x1d0_28_LLRD = x1c_desc->x44_24_LLRD;
    x1d0_29_VLS1 = x1c_desc->x44_26_VLS1;
    x1d0_30_VLS2 = x1c_desc->x44_27_VLS2;

    if (IsValid())
    {
        if (CIntElement* pslt = x1c_desc->x0_PSLT.get())
            pslt->GetValue(0, x2c_PSLT);
        else
            x2c_PSLT = INT_MAX;

        x1d0_25_AALP = x1c_desc->x44_31_AALP;

        if (CIntElement* spln = x1c_desc->x38_SPLN.get())
            spln->GetValue(0, x1b0_SPLN);
        if (x1b0_SPLN < 0)
            x1b0_SPLN = 0;

        x15c_swooshes.clear();
        x15c_swooshes.reserve(x1b4_LENG);
        for (int i=0 ; i<x1b4_LENG ; ++i)
            x15c_swooshes.emplace_back(zeus::CVector3f::skZero, zeus::CVector3f::skZero, 0.f, 0.f,
                                       0, false, zeus::CTransform::Identity(), zeus::CVector3f::skZero,
                                       0.f, 0.f, zeus::CColor::skClear);

        SetOrientation(zeus::CTransform::Identity());

        x16c_.resize(x1b8_SIDE);
        x17c_.resize(x1b8_SIDE);
        x18c_.resize(x1b8_SIDE);
        x19c_.resize(x1b8_SIDE);
    }
}

CParticleSwoosh::~CParticleSwoosh()
{
    --g_ParticleSystemAliveCount;
}

void CParticleSwoosh::UpdateMaxRadius(float r)
{
    x208_maxRadius = std::max(x208_maxRadius, r);
}

float CParticleSwoosh::GetLeftRadius(int i) const
{
    float ret = 0.f;
    if (CRealElement* lrad = x1c_desc->x8_LRAD.get())
        lrad->GetValue(x15c_swooshes[i].x68_frame, ret);
    return ret;
}

float CParticleSwoosh::GetRightRadius(int i) const
{
    float ret = 0.f;
    if (CRealElement* rrad = x1c_desc->xc_RRAD.get())
        rrad->GetValue(x15c_swooshes[i].x68_frame, ret);
    return ret;
}

void CParticleSwoosh::UpdateSwooshTranslation(const zeus::CVector3f& translation)
{
    x15c_swooshes[x158_curParticle].xc_translation = x11c_invScaleXf * translation;
}

void CParticleSwoosh::UpdateTranslationAndOrientation()
{

}

bool CParticleSwoosh::Update(double dt)
{
    if (!IsValid())
        return false;

    CParticleGlobals::SetParticleLifetime(x1b4_LENG);
    CParticleGlobals::SetEmitterTime(x28_curFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(0);
    CGlobalRandom gr(x1c0_rand);

    float evalTime = x28_curFrame * (1.f / 60.f);
    float time = 1.f;
    if (CRealElement* timeElem = x1c_desc->x4_TIME.get())
        timeElem->GetValue(x28_curFrame, time);

    x30_curTime += std::max(0.0, dt * time);
    while (!x1d0_26_ && evalTime < x30_curTime)
    {
        x1d0_26_ = false;

        x158_curParticle += 1;
        if (x158_curParticle >= x15c_swooshes.size())
            x158_curParticle = 0;

        if (x1d0_24_emitting && x28_curFrame < x2c_PSLT)
        {
            UpdateSwooshTranslation(x38_translation);

            if (CRealElement* irot = x1c_desc->x1c_IROT.get())
                irot->GetValue(x28_curFrame, x15c_swooshes[x158_curParticle].x30_irot);
            else
                x15c_swooshes[x158_curParticle].x30_irot = 0.f;

            x15c_swooshes[x158_curParticle].x34_ = 0.f;
            x15c_swooshes[x158_curParticle].x70_ = x28_curFrame;

            if (!x15c_swooshes[x158_curParticle].x0_active)
            {
                x1ac_particleCount += 1;
                x15c_swooshes[x158_curParticle].x0_active = true;
            }

            x15c_swooshes[x158_curParticle].x38_orientation = x44_orientation;

            if (CVectorElement* ivel = x1c_desc->x28_IVEL.get())
            {
                ivel->GetValue(x28_curFrame, x15c_swooshes[x158_curParticle].x74_velocity);
                x15c_swooshes[x158_curParticle].x74_velocity = x44_orientation * x15c_swooshes[x158_curParticle].x74_velocity;
            }

            if (CVectorElement* pofs = x1c_desc->x24_POFS.get())
                pofs->GetValue(x28_curFrame, x15c_swooshes[x158_curParticle].x18_offset);

            x15c_swooshes[x158_curParticle].x24_ = x15c_swooshes[x158_curParticle].x18_offset;

            if (CColorElement* colr = x1c_desc->x14_COLR.get())
                colr->GetValue(x28_curFrame, x15c_swooshes[x158_curParticle].x6c_color);
            else
                x15c_swooshes[x158_curParticle].x6c_color = zeus::CColor::skWhite;

            int tspn = 0;
            if (CIntElement* tspnElem = x1c_desc->x40_TSPN.get())
                tspnElem->GetValue(x28_curFrame, tspn);
            x1cc_TSPN = tspn;
        }
        else if (x15c_swooshes[x158_curParticle].x0_active)
        {
            x1ac_particleCount = std::max(0, int(x1ac_particleCount) - 1);
            x15c_swooshes[x158_curParticle].x0_active = false;
        }

        UpdateTranslationAndOrientation();

        evalTime += (1.f / 60.f);
        x28_curFrame += 1;
    }

    return false;
}

void CParticleSwoosh::RenderNSidedSpline()
{

}

void CParticleSwoosh::RenderNSidedNoSpline()
{

}

void CParticleSwoosh::Render3SidedSolidSpline()
{

}

void CParticleSwoosh::Render3SidedSolidNoSplineNoGaps()
{

}

void CParticleSwoosh::Render2SidedSpline()
{

}

void CParticleSwoosh::Render2SidedNoSplineGaps()
{

}

void CParticleSwoosh::Render2SidedNoSplineNoGaps()
{

}

void CParticleSwoosh::Render()
{
}

void CParticleSwoosh::SetOrientation(const zeus::CTransform& xf)
{
    x44_orientation = xf;
    x74_invOrientation = xf.inverse();
    x15c_swooshes[x158_curParticle].x38_orientation = xf;
}

void CParticleSwoosh::SetTranslation(const zeus::CVector3f& translation)
{
    x38_translation = translation;
    UpdateSwooshTranslation(x38_translation);
}

void CParticleSwoosh::SetGlobalOrientation(const zeus::CTransform& xf)
{
    xb0_globalOrientation = xf.getRotation();
}

void CParticleSwoosh::SetGlobalTranslation(const zeus::CVector3f& translation)
{
    xa4_globalTranslation = translation;
}

void CParticleSwoosh::SetGlobalScale(const zeus::CVector3f& scale)
{
    xe0_globalScale = scale;
    xec_scaleXf = zeus::CTransform::Scale(scale);
    x11c_invScaleXf = zeus::CTransform::Scale(1.f / scale);
}

void CParticleSwoosh::SetLocalScale(const zeus::CVector3f& scale)
{
    x14c_localScale = scale;
}

void CParticleSwoosh::SetParticleEmission(bool e)
{
    x1d0_24_emitting = e;
}

void CParticleSwoosh::SetModulationColor(const zeus::CColor& color)
{
    x20c_moduColor = color;
}

const zeus::CTransform& CParticleSwoosh::GetOrientation() const
{
    return x44_orientation;
}

const zeus::CVector3f& CParticleSwoosh::GetTranslation() const
{
    return x38_translation;
}

const zeus::CTransform& CParticleSwoosh::GetGlobalOrientation() const
{
    return xb0_globalOrientation;
}

const zeus::CVector3f& CParticleSwoosh::GetGlobalTranslation() const
{
    return xa4_globalTranslation;
}

const zeus::CVector3f& CParticleSwoosh::GetGlobalScale() const
{
    return xe0_globalScale;
}

const zeus::CColor& CParticleSwoosh::GetModulationColor() const
{
    return x20c_moduColor;
}

bool CParticleSwoosh::IsSystemDeletable() const
{
    if (x1d0_24_emitting && x28_curFrame < x2c_PSLT)
        return false;

    if (GetParticleCount() >= 2)
        return false;

    return true;
}

rstl::optional_object<zeus::CAABox> CParticleSwoosh::GetBounds() const
{
    if (GetParticleCount() <= 1)
    {
        zeus::CVector3f trans = x38_translation + xa4_globalTranslation;
        return zeus::CAABox(trans, trans);
    }
    else
    {
        zeus::CTransform xf = zeus::CTransform::Translate(xa4_globalTranslation) *
            xb0_globalOrientation * xec_scaleXf;
        return zeus::CAABox(x1f0_ - x208_maxRadius, x1fc_ + x208_maxRadius).getTransformedAABox(xf);
    }
}

u32 CParticleSwoosh::GetParticleCount() const
{
    return x1ac_particleCount;
}

bool CParticleSwoosh::SystemHasLight() const
{
    return false;
}

CLight CParticleSwoosh::GetLight() const
{
    return CLight::BuildLocalAmbient(zeus::CVector3f::skZero, zeus::CColor::skWhite);
}

bool CParticleSwoosh::GetParticleEmission() const
{
    return x1d0_24_emitting;
}

void CParticleSwoosh::DestroyParticles()
{
    // Empty
}

}
