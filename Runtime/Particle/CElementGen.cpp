#include "CElementGen.hpp"
#include "CGenDescription.hpp"
#include "CLight.hpp"

namespace Retro
{

int CElementGen::g_ParticleAliveCount;
int CElementGen::g_ParticleSystemAliveCount;
void CElementGen::Initialize()
{
    g_ParticleAliveCount = 0;
    g_ParticleSystemAliveCount = 0;
}

CElementGen::CElementGen(const TToken<CGenDescription>& gen,
                         EModelOrientationType orientType,
                         EOptionalSystemFlags flags)
: x1c_genDesc(gen), x28_orientType(orientType),
  x226((flags & EOptionalSystemFlags::Two) != EOptionalSystemFlags::None), x230(x74)
{
    CIntElement* pmedElem = x1c_genDesc.GetObj()->x1c_PMED.get();
    if (pmedElem)
    {
        int pmedVal;
        pmedElem->GetValue(x50, pmedVal);
        x74 = pmedVal;
    }
    x230.SetSeed(x74);
    ++g_ParticleSystemAliveCount;
    x224_25_LIT_ = x1c_genDesc.GetObj()->x44_29_LIT_;
    x224_26_AAPH = x1c_genDesc.GetObj()->x44_26_AAPH;
    x224_27_ZBUF = x1c_genDesc.GetObj()->x44_27_ZBUF;
    x224_29_MBLR = x1c_genDesc.GetObj()->x44_30_MBLR;

    CIntElement* mbspElem = x1c_genDesc.GetObj()->x48_MBSP.get();
    if (mbspElem)
        mbspElem->GetValue(x50, x228_MBSP);

    x224_30_VMD1 = x1c_genDesc.GetObj()->x45_26_VMD1;
    x224_31_VMD2 = x1c_genDesc.GetObj()->x45_27_VMD2;
    x225_24_VMD2 = x1c_genDesc.GetObj()->x45_28_VMD3;
    x225_25_VMD2 = x1c_genDesc.GetObj()->x45_29_VMD4;

    CIntElement* cssdElem = x1c_genDesc.GetObj()->xa0_CSSD.get();
    if (cssdElem)
        cssdElem->GetValue(0, x244_CSSD);

    SChildGeneratorDesc& idts = x1c_genDesc.GetObj()->xa4_IDTS;
    if (idts.m_found)
    {
        int ndsyVal = 1;
        CIntElement* ndsyElem = x1c_genDesc.GetObj()->xb4_NDSY.get();
        if (ndsyElem)
            ndsyElem->GetValue(0, ndsyVal);
        x248_children.reserve(ndsyVal + x248_children.size());
        for (int i=0 ; i<ndsyVal ; ++i)
        {
            CGenDescription* chDesc = x1c_genDesc.GetObj()->xa4_IDTS.m_gen.GetObj();
            if (x226 && chDesc->x45_31_OPTS)
                break;
            x248_children.emplace_back(new CElementGen(x1c_genDesc.GetObj()->xa4_IDTS.m_gen,
                                                       EModelOrientationType::Normal,
                                                       x226 ? EOptionalSystemFlags::Two : EOptionalSystemFlags::One));
        }
    }

    CIntElement* pisyElem = x1c_genDesc.GetObj()->xc8_PISY.get();
    if (pisyElem)
    {
        pisyElem->GetValue(0, x25c_PISY);
        if (x25c_PISY <= 0)
            x25c_PISY = 1;
    }

    CIntElement* sisyElem = x1c_genDesc.GetObj()->xcc_SISY.get();
    if (sisyElem)
        pisyElem->GetValue(0, x258_SISY);

    CIntElement* sssdElem = x1c_genDesc.GetObj()->xe4_SSSD.get();
    if (sssdElem)
        sssdElem->GetValue(0, x270_SSSD);

    CVectorElement* sspoElem = x1c_genDesc.GetObj()->xe8_SSPO.get();
    if (sspoElem)
        sspoElem->GetValue(0, x274_SSPO);

    CIntElement* sesdElem = x1c_genDesc.GetObj()->xf8_SESD.get();
    if (sesdElem)
        sesdElem->GetValue(0, x290_SESD);

    CVectorElement* sepoElem = x1c_genDesc.GetObj()->xfc_SEPO.get();
    if (sepoElem)
        sepoElem->GetValue(0, x294_SEPO);

    CIntElement* psltElem = x1c_genDesc.GetObj()->xc_PSLT.get();
    if (psltElem)
        psltElem->GetValue(0, x214_PSLT);

    CVectorElement* psivElem = x1c_genDesc.GetObj()->x0_PSIV.get();
    if (psivElem)
        psivElem->GetValue(0, x218_PSIV);

    CIntElement* maxpElem = x1c_genDesc.GetObj()->x28_MAXP.get();
    if (maxpElem)
        maxpElem->GetValue(x50, x70_MAXP);

    x2c_particleLists.reserve(std::min(256, x70_MAXP));
    if (x28_orientType == EModelOrientationType::One)
    {
        x3c_parentMatrices.insert(x3c_parentMatrices.end(), x70_MAXP,
                                  Zeus::CMatrix3f::skIdentityMatrix3f);
    }

    x225_26_LINE = x1c_genDesc.GetObj()->x44_24_LINE;
    x225_27_FXLL = x1c_genDesc.GetObj()->x44_25_FXLL;

    CRealElement* widtElem = x1c_genDesc.GetObj()->x24_WIDT.get();
    if (widtElem)
        widtElem->GetValue(x50, x94_WIDT);

    CIntElement* ltypElem = x1c_genDesc.GetObj()->x100_LTYP.get();
    if (ltypElem)
    {
        int ltyp;
        ltypElem->GetValue(x50, ltyp);
        switch (ELightType(ltyp))
        {
        case ELightType::LocalAmbient:
        default:
            x2dc_lightType = ELightType::LocalAmbient;
            break;
        case ELightType::Directional:
            x2dc_lightType = ELightType::Directional;
            break;
        case ELightType::Custom:
            x2dc_lightType = ELightType::Custom;
            break;
        case ELightType::Spot:
            x2dc_lightType = ELightType::Spot;
            break;
        }
    }

    CIntElement* lfotElem = x1c_genDesc.GetObj()->x114_LFOT.get();
    if (lfotElem)
    {
        int lfot;
        lfotElem->GetValue(x50, lfot);
        switch (EFalloffType(lfot))
        {
        case EFalloffType::Constant:
            x300_falloffType = EFalloffType::Constant;
            break;
        case EFalloffType::Linear:
        default:
            x300_falloffType = EFalloffType::Linear;
            break;
        case EFalloffType::Quadratic:
            x300_falloffType = EFalloffType::Quadratic;
            break;
        }
    }
}

CElementGen::~CElementGen()
{
    --g_ParticleSystemAliveCount;
}

void CElementGen::Update(double)
{
}

void CElementGen::Render()
{
}

void CElementGen::SetOrientation(const Zeus::CTransform&)
{
}

void CElementGen::SetTranslation(const Zeus::CVector3f&)
{
}

void CElementGen::SetGlobalOrientation(const Zeus::CTransform&)
{
}

void CElementGen::SetGlobalTranslation(const Zeus::CVector3f&)
{
}

void CElementGen::SetGlobalScale(const Zeus::CVector3f&)
{
}

void CElementGen::SetLocalScale(const Zeus::CVector3f&)
{
}

void CElementGen::SetParticleEmission(bool)
{
}

void CElementGen::SetModulationColor(const Zeus::CColor&)
{
}

const Zeus::CTransform& CElementGen::GetOrientation() const
{
}

const Zeus::CVector3f& CElementGen::GetTranslation() const
{
}

const Zeus::CVector3f& CElementGen::GetGlobalScale() const
{
}

const Zeus::CColor& CElementGen::GetModulationColor() const
{
}

bool CElementGen::IsSystemDeletable() const
{
}

Zeus::CAABox CElementGen::GetBounds() const
{
}

u32 CElementGen::GetParticleCount() const
{
}

bool CElementGen::SystemHasLight() const
{
}

CLight CElementGen::GetLight() const
{
}

void CElementGen::DestroyParticles()
{
}

void CElementGen::AddModifier(CWarp*)
{
}

}
