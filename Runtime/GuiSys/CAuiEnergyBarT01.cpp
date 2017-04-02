#include "CAuiEnergyBarT01.hpp"
#include "CGuiSys.hpp"
#include "CSimplePool.hpp"
#include "CGuiWidgetDrawParms.hpp"

namespace urde
{

CAuiEnergyBarT01::CAuiEnergyBarT01(const CGuiWidgetParms& parms, CSimplePool* sp, ResId txtrId)
: CGuiWidget(parms), xb8_txtrId(txtrId)
{
    if (g_GuiSys->GetUsageMode() != CGuiSys::EUsageMode::Two)
        xbc_tex = sp->GetObj(SObjectTag{FOURCC('TXTR'), xb8_txtrId});
}

std::pair<zeus::CVector3f, zeus::CVector3f> CAuiEnergyBarT01::DownloadBarCoordFunc(float t)
{
    float x = 12.5 * t - 6.25;
    return {zeus::CVector3f{x, 0.f, -0.2f}, zeus::CVector3f{x, 0.f, 0.2f}};
}

void CAuiEnergyBarT01::Update(float dt)
{
    if (x100_shadowDrainDelayTimer > 0.f)
        x100_shadowDrainDelayTimer = std::max(x100_shadowDrainDelayTimer - dt, 0.f);

    if (xf8_filledEnergy < xf4_setEnergy)
    {
        if (xf1_wrapping)
        {
            xf8_filledEnergy -= dt * xe4_filledSpeed;
            if (xf8_filledEnergy < 0.f)
            {
                xf8_filledEnergy = std::max(xf4_setEnergy, xf8_filledEnergy + xe0_maxEnergy);
                xf1_wrapping = false;
                xfc_shadowEnergy = xe0_maxEnergy;
            }
        }
        else
        {
            xf8_filledEnergy = std::min(xf4_setEnergy, xf8_filledEnergy + dt * xe4_filledSpeed);
        }
    }
    else if (xf8_filledEnergy > xf4_setEnergy)
    {
        if (xf1_wrapping)
        {
            xf8_filledEnergy += dt * xe4_filledSpeed;
            if (xf8_filledEnergy > xe0_maxEnergy)
            {
                xf8_filledEnergy = std::min(xf4_setEnergy, xf8_filledEnergy - xe0_maxEnergy);
                xf1_wrapping = false;
                xfc_shadowEnergy = xf8_filledEnergy;
            }
        }
        else
        {
            xf8_filledEnergy = std::max(xf4_setEnergy, xf8_filledEnergy - dt * xe4_filledSpeed);
        }
    }

    if (xfc_shadowEnergy < xf8_filledEnergy)
        xfc_shadowEnergy = xf8_filledEnergy;
    else if (xfc_shadowEnergy > xf8_filledEnergy && x100_shadowDrainDelayTimer == 0.f)
        xfc_shadowEnergy = std::max(xf8_filledEnergy, xfc_shadowEnergy - dt * xe8_shadowSpeed);

    CGuiWidget::Update(dt);
}

void CAuiEnergyBarT01::Draw(const CGuiWidgetDrawParms& drawParms) const
{
    if (!xbc_tex || !xbc_tex.IsLoaded() || !xd8_coordFunc)
        return;

    CGraphics::SetModelMatrix(x34_worldXF);
    const_cast<CEnergyBarShader&>(m_energyBarShader).updateModelMatrix();

    float filledT = xe0_maxEnergy > 0.f ? xf8_filledEnergy / xe0_maxEnergy : 0.f;
    float shadowT = xe0_maxEnergy > 0.f ? xfc_shadowEnergy / xe0_maxEnergy : 0.f;

    zeus::CColor filledColor = xd0_filledColor;
    filledColor.a *= drawParms.x0_alphaMod;
    filledColor *= xa8_color2;

    zeus::CColor shadowColor = xd4_shadowColor;
    shadowColor.a *= drawParms.x0_alphaMod;
    shadowColor *= xa8_color2;

    zeus::CColor emptyColor = xcc_emptyColor;
    emptyColor.a *= drawParms.x0_alphaMod;
    emptyColor *= xa8_color2;

    for (int i=0 ; i<3 ; ++i)
    {
        std::vector<CEnergyBarShader::Vertex>& verts = const_cast<CAuiEnergyBarT01&>(*this).m_verts[i];
        verts.clear();

        float start;
        float end;
        switch (i)
        {
        case 0:
        default:
            start = 0.f;
            end = filledT;
            break;
        case 1:
            start = filledT;
            end = shadowT;
            break;
        case 2:
            start = shadowT;
            end = 1.f;
            break;
        }

        if (start == end)
            continue;

        std::pair<zeus::CVector3f, zeus::CVector3f> coords = xd8_coordFunc(start);
        while (start < end)
        {
            verts.push_back({coords.first, zeus::CVector2f(start, 0.f)});
            verts.push_back({coords.second, zeus::CVector2f(start, 1.f)});
            start += xdc_tesselation;
            if (start >= end)
            {
                coords = xd8_coordFunc(end);
                verts.push_back({coords.first, zeus::CVector2f(end, 0.f)});
                verts.push_back({coords.second, zeus::CVector2f(end, 1.f)});
            }
            else
            {
                coords = xd8_coordFunc(start);
            }
        }
    }

    const_cast<CEnergyBarShader&>(m_energyBarShader).draw(filledColor, m_verts[0],
                                                          shadowColor, m_verts[1],
                                                          emptyColor, m_verts[2],
                                                          xbc_tex.GetObj());
}

void CAuiEnergyBarT01::SetCurrEnergy(float e, ESetMode mode)
{
    e = zeus::clamp(0.f, e, xe0_maxEnergy);
    if (e == xf4_setEnergy)
        return;
    if (xf0_alwaysResetDelayTimer || xf8_filledEnergy == xfc_shadowEnergy)
        x100_shadowDrainDelayTimer = xec_shadowDrainDelay;
    xf1_wrapping = mode == ESetMode::Wrapped;
    xf4_setEnergy = e;
    if (mode == ESetMode::Insta)
        xf8_filledEnergy = xf4_setEnergy;
}

void CAuiEnergyBarT01::SetMaxEnergy(float maxEnergy)
{
    xe0_maxEnergy = maxEnergy;
    xf4_setEnergy = std::min(xe0_maxEnergy, xf4_setEnergy);
    xf8_filledEnergy = std::min(xe0_maxEnergy, xf8_filledEnergy);
    xfc_shadowEnergy = std::min(xe0_maxEnergy, xfc_shadowEnergy);
}

std::shared_ptr<CGuiWidget> CAuiEnergyBarT01::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
    ResId tex = in.readUint32Big();
    std::shared_ptr<CGuiWidget> ret = std::make_shared<CAuiEnergyBarT01>(parms, sp, tex);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
