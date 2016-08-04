#include "CDecal.hpp"

namespace urde
{

CDecal::CDecal(const TToken<CDecalDescription>& desc, const zeus::CTransform& xf)
    : x0_description(desc),
      xc_transform(xf),
      x3c_decalQuad1(0, 0.f),
      x48_decalQuad2(0, 0.f)
{
    CGlobalRandom gr(sDecalRandom);

    InitQuad(x3c_decalQuad1, x0_description.GetObj()->x0_Quad);
    InitQuad(x48_decalQuad2, x0_description.GetObj()->x1c_Quad);
}

void CDecal::InitQuad(CDecal::CQuadDecal& quad, const CDecalDescription::SQuadDescr& desc)
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
            u32 r0 = (quad._dummy >> 25) & 1;
            r0 &= desc.x8_ROT->IsConstant();
            quad._dummy = (quad._dummy & ~0x80) | ((r0 << 7) & 0x80);
        }

        if (desc.x4_SZE)
        {

        }
    }
    else
        quad.x0_24_ = false;
}

}
