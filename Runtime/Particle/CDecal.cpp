#include "CDecal.hpp"

namespace urde
{
CRandom16 CDecal::sDecalRandom(99);
bool CDecal::sMoveRedToAphaBuffer = false;

CDecal::CDecal(const TToken<CDecalDescription>& desc, const zeus::CTransform& xf)
    : x0_description(desc),
      xc_transform(xf),
      x3c_decalQuad1(0, 0.f),
      x48_decalQuad2(0, 0.f)
{
    CGlobalRandom gr(sDecalRandom);

    x5c_31_quad1Invalid = InitQuad(x3c_decalQuad1, x0_description.GetObj()->x0_Quad);
    x5c_30_quad2Invalid = InitQuad(x48_decalQuad2, x0_description.GetObj()->x1c_Quad);

    CDecalDescription* d = x0_description.GetObj();
    if (d->x38_DMDL)
    {
        if (d->x48_DLFT)
            d->x48_DLFT->GetValue(0, x54_lifetime);
        else
            x54_lifetime = 0x7FFFFF;

        if (d->x50_DMRT)
            d->x50_DMRT->GetValue(0, x60_rotation);
    }
    else
        x5c_29_modelInvalid = true;
}

bool CDecal::InitQuad(CDecal::CQuadDecal& quad, const CDecalDescription::SQuadDescr& desc)
{
    if (desc.x14_TEX)
    {
        if (desc.x0_LFT)
            desc.x0_LFT->GetValue(0, quad.x4_lifetime);
        else
            quad.x4_lifetime = 0x7FFFFF;
        if (desc.x8_ROT)
        {
            desc.x8_ROT->GetValue(0, quad.x8_rotation);
            quad.x0_24_invalid = desc.x8_ROT->IsConstant();
        }

        if (desc.x4_SZE)
        {
            quad.x0_24_invalid = desc.x4_SZE->IsConstant();
            float size = 1.f;
            desc.x4_SZE->GetValue(0, size);
            quad.x0_24_invalid = size <= 1.f;
        }

        if (desc.xc_OFF)
            quad.x0_24_invalid = desc.xc_OFF->IsFastConstant();
        return false;
    }

    quad.x0_24_invalid = false;
    return true;
}


void CDecal::SetGlobalSeed(u16 seed)
{
    sDecalRandom.SetSeed(seed);
}

void CDecal::SetMoveRedToAlphaBuffer(bool move)
{
    sMoveRedToAphaBuffer = move;
}

void CDecal::Render() const
{

}
}
