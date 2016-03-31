#include "CParticleDataFactory.hpp"
#include "CToken.hpp"
#include "CSimplePool.hpp"
#include "CGenDescription.hpp"
#include "CRandom16.hpp"
#include "Graphics/CModel.hpp"
#include "CSwooshDescription.hpp"
#include "CElectricDescription.hpp"

namespace urde
{
static logvisor::Module Log("urde::CParticleDataFactory");

float CParticleDataFactory::GetReal(CInputStream& in)
{
    return in.readFloatBig();
}

int32_t CParticleDataFactory::GetInt(CInputStream& in)
{
    return in.readInt32Big();
}

bool CParticleDataFactory::GetBool(CInputStream& in)
{
    FourCC cid = GetClassID(in);
    if (cid != FOURCC('CNST'))
        Log.report(logvisor::Fatal, "bool element does not begin with CNST");
    return in.readBool();
}

FourCC CParticleDataFactory::GetClassID(CInputStream& in)
{
    uint32_t val;
    in.readBytesToBuf(&val, 4);
    return val;
}

SParticleModel CParticleDataFactory::GetModel(CInputStream& in, CSimplePool* resPool)
{
    FourCC clsId = GetClassID(in);
    if (clsId == SBIG('NONE'))
        return {};
    TResId id = in.readUint32Big();
    if (!id)
        return {};
    return {std::move(resPool->GetObj({FOURCC('CMDL'), id})), true};
}

SChildGeneratorDesc CParticleDataFactory::GetChildGeneratorDesc(TResId res, CSimplePool* resPool, const std::vector<TResId>& tracker)
{
    if (std::count(tracker.cbegin(), tracker.cend(), res) == 0)
        return {std::move(resPool->GetObj({FOURCC('PART'), res})), true};
    return {};
}

SChildGeneratorDesc CParticleDataFactory::GetChildGeneratorDesc(CInputStream& in, CSimplePool* resPool, const std::vector<TResId>& tracker)
{
    FourCC clsId = GetClassID(in);
    if (clsId == SBIG('NONE'))
        return {};
    TResId id = in.readUint32Big();
    if (!id)
        return {};
    return GetChildGeneratorDesc(id, resPool, tracker);
}

SSwooshGeneratorDesc CParticleDataFactory::GetSwooshGeneratorDesc(CInputStream& in, CSimplePool* resPool)
{
    FourCC clsId = GetClassID(in);
    if (clsId == SBIG('NONE'))
        return {};
    TResId id = in.readUint32Big();
    if (!id)
        return {};
    return {std::move(resPool->GetObj({FOURCC('SWHC'), id})), true};
}

SElectricGeneratorDesc CParticleDataFactory::GetElectricGeneratorDesc(CInputStream& in, CSimplePool* resPool)
{
    FourCC clsId = GetClassID(in);
    if (clsId == SBIG('NONE'))
        return {};
    TResId id = in.readUint32Big();
    if (!id)
        return {};
    return {std::move(resPool->GetObj({FOURCC('ELSC'), id})), true};
}

CUVElement* CParticleDataFactory::GetTextureElement(CInputStream& in, CSimplePool* resPool)
{
    FourCC clsId = GetClassID(in);
    switch (clsId)
    {
    case SBIG('CNST'):
    {
        FourCC subId = GetClassID(in);
        if (subId == SBIG('NONE'))
            return nullptr;
        TResId id = in.readUint32Big();
        TToken<CTexture> txtr = resPool->GetObj({FOURCC('TXTR'), id});
        return new CUVEConstant(std::move(txtr));
    }
    case SBIG('ATEX'):
    {
        FourCC subId = GetClassID(in);
        if (subId == SBIG('NONE'))
            return nullptr;
        TResId id = in.readUint32Big();
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        CIntElement* c = GetIntElement(in);
        CIntElement* d = GetIntElement(in);
        CIntElement* e = GetIntElement(in);
        bool f = GetBool(in);
        TToken<CTexture> txtr = resPool->GetObj({FOURCC('TXTR'), id});
        return new CUVEAnimTexture(std::move(txtr), a, b, c, d, e, f);
    }
    default: break;
    }
    return nullptr;
}

CColorElement* CParticleDataFactory::GetColorElement(CInputStream& in)
{
    FourCC clsId = GetClassID(in);
    switch (clsId)
    {
    case SBIG('KEYE'):
    case SBIG('KEYP'):
    {
        return new CCEKeyframeEmitter(in);
    }
    case SBIG('CNST'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        CRealElement* c = GetRealElement(in);
        CRealElement* d = GetRealElement(in);
        if (a->IsConstant() && b->IsConstant() && c->IsConstant() && d->IsConstant())
        {
            float af, bf, cf, df;
            a->GetValue(0, af);
            b->GetValue(0, bf);
            c->GetValue(0, cf);
            d->GetValue(0, df);
            return new CCEFastConstant(af, bf, cf, df);
        }
        else
        {
            return new CCEConstant(a, b, c, d);
        }
    }
    case SBIG('CHAN'):
    {
        CColorElement* a = GetColorElement(in);
        CColorElement* b = GetColorElement(in);
        CIntElement* c = GetIntElement(in);
        return new CCETimeChain(a, b, c);
    }
    case SBIG('CFDE'):
    {
        CColorElement* a = GetColorElement(in);
        CColorElement* b = GetColorElement(in);
        CRealElement* c = GetRealElement(in);
        CRealElement* d = GetRealElement(in);
        return new CCEFadeEnd(a, b, c, d);
    }
    case SBIG('FADE'):
    {
        CColorElement* a = GetColorElement(in);
        CColorElement* b = GetColorElement(in);
        CRealElement* c = GetRealElement(in);
        return new CCEFade(a, b, c);
    }
    case SBIG('PULS'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        CColorElement* c = GetColorElement(in);
        CColorElement* d = GetColorElement(in);
        return new CCEPulse(a, b, c, d);
    }
    case SBIG('PCOL'):
    {
        return new CCEParticleColor();
    }
    default: break;
    }
    return nullptr;
}

CModVectorElement* CParticleDataFactory::GetModVectorElement(CInputStream& in)
{
    FourCC clsId = GetClassID(in);
    switch (clsId)
    {
    case SBIG('IMPL'):
    {
        CVectorElement* a = GetVectorElement(in);
        CRealElement* b = GetRealElement(in);
        CRealElement* c = GetRealElement(in);
        CRealElement* d = GetRealElement(in);
        bool e = GetBool(in);
        return new CMVEImplosion(a, b, c, d, e);
    }
    case SBIG('EMPL'):
    {
        CVectorElement* a = GetVectorElement(in);
        CRealElement* b = GetRealElement(in);
        CRealElement* c = GetRealElement(in);
        CRealElement* d = GetRealElement(in);
        bool e = GetBool(in);
        return new CMVEExponentialImplosion(a, b, c, d, e);
    }
    case SBIG('CHAN'):
    {
        CModVectorElement* a = GetModVectorElement(in);
        CModVectorElement* b = GetModVectorElement(in);
        CIntElement* c = GetIntElement(in);
        return new CMVETimeChain(a, b, c);
    }
    case SBIG('BNCE'):
    {
        CVectorElement* a = GetVectorElement(in);
        CVectorElement* b = GetVectorElement(in);
        CRealElement* c = GetRealElement(in);
        CRealElement* d = GetRealElement(in);
        bool e = GetBool(in);
        return new CMVEBounce(a, b, c, d, e);
    }
    case SBIG('CNST'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        CRealElement* c = GetRealElement(in);
        if (a->IsConstant() && b->IsConstant() && c->IsConstant())
        {
            float af, bf, cf;
            a->GetValue(0, af);
            b->GetValue(0, bf);
            c->GetValue(0, cf);
            return new CMVEFastConstant(af, bf, cf);
        }
        else
        {
            return new CMVEConstant(a, b, c);
        }
    }
    case SBIG('GRAV'):
    {
        CVectorElement* a = GetVectorElement(in);
        return new CMVEGravity(a);
    }
    case SBIG('EXPL'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        return new CMVEExplode(a, b);
    }
    case SBIG('SPOS'):
    {
        CVectorElement* a = GetVectorElement(in);
        return new CMVESetPosition(a);
    }
    case SBIG('LMPL'):
    {
        CVectorElement* a = GetVectorElement(in);
        CRealElement* b = GetRealElement(in);
        CRealElement* c = GetRealElement(in);
        CRealElement* d = GetRealElement(in);
        bool e = GetBool(in);
        return new CMVELinearImplosion(a, b, c, d, e);
    }
    case SBIG('PULS'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        CModVectorElement* c = GetModVectorElement(in);
        CModVectorElement* d = GetModVectorElement(in);
        return new CMVEPulse(a, b, c, d);
    }
    case SBIG('WIND'):
    {
        CVectorElement* a = GetVectorElement(in);
        CRealElement* b = GetRealElement(in);
        return new CMVEWind(a, b);
    }
    case SBIG('SWRL'):
    {
        CVectorElement* a = GetVectorElement(in);
        CVectorElement* b = GetVectorElement(in);
        CRealElement* c = GetRealElement(in);
        CRealElement* d = GetRealElement(in);
        return new CMVESwirl(a, b, c, d);
    }
    default: break;
    }
    return nullptr;
}

CEmitterElement* CParticleDataFactory::GetEmitterElement(CInputStream& in)
{
    FourCC clsId = GetClassID(in);
    switch (clsId)
    {
    case SBIG('SETR'):
    {
        FourCC prop = GetClassID(in);
        if (prop == SBIG('ILOC'))
        {
            CVectorElement* a = GetVectorElement(in);
            prop = GetClassID(in);
            if (prop == SBIG('IVEC'))
            {
                CVectorElement* b = GetVectorElement(in);
                return new CEESimpleEmitter(a, b);
            }
        }
        return nullptr;
    }
    case SBIG('SEMR'):
    {
        CVectorElement* a = GetVectorElement(in);
        CVectorElement* b = GetVectorElement(in);
        return new CEESimpleEmitter(a, b);
    }
    case SBIG('SPHE'):
    {
        CVectorElement* a = GetVectorElement(in);
        CRealElement* b = GetRealElement(in);
        CRealElement* c = GetRealElement(in);
        return new CVESphere(a, b, c);
    }
    case SBIG('ASPH'):
    {
        CVectorElement* a = GetVectorElement(in);
        CRealElement* b = GetRealElement(in);
        CRealElement* c = GetRealElement(in);
        CRealElement* d = GetRealElement(in);
        CRealElement* e = GetRealElement(in);
        CRealElement* f = GetRealElement(in);
        CRealElement* g = GetRealElement(in);
        return new CVEAngleSphere(a, b, c, d, e, f, g);
    }
    default: break;
    }
    return nullptr;
}

CVectorElement* CParticleDataFactory::GetVectorElement(CInputStream& in)
{
    FourCC clsId = GetClassID(in);
    switch (clsId)
    {
    case SBIG('CONE'):
    {
        CVectorElement* a = GetVectorElement(in);
        CRealElement* b = GetRealElement(in);
        return new CVECone(a, b);
    }
    case SBIG('CHAN'):
    {
        CVectorElement* a = GetVectorElement(in);
        CVectorElement* b = GetVectorElement(in);
        CIntElement* c = GetIntElement(in);
        return new CVETimeChain(a, b, c);
    }
    case SBIG('ANGC'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        CRealElement* c = GetRealElement(in);
        CRealElement* d = GetRealElement(in);
        CRealElement* e = GetRealElement(in);
        return new CVEAngleCone(a, b, c, d, e);
    }
    case SBIG('ADD_'):
    {
        CVectorElement* a = GetVectorElement(in);
        CVectorElement* b = GetVectorElement(in);
        return new CVEAdd(a, b);
    }
    case SBIG('CCLU'):
    {
        CVectorElement* a = GetVectorElement(in);
        CVectorElement* b = GetVectorElement(in);
        CIntElement* c = GetIntElement(in);
        CRealElement* d = GetRealElement(in);
        return new CVECircleCluster(a, b, c, d);
    }
    case SBIG('CNST'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        CRealElement* c = GetRealElement(in);
        if (a->IsConstant() && b->IsConstant() && c->IsConstant())
        {
            float af, bf, cf;
            a->GetValue(0, af);
            b->GetValue(0, bf);
            c->GetValue(0, cf);
            return new CVEFastConstant(af, bf, cf);
        }
        else
        {
            return new CVEConstant(a, b, c);
        }
    }
    case SBIG('CIRC'):
    {
        CVectorElement* a = GetVectorElement(in);
        CVectorElement* b = GetVectorElement(in);
        CRealElement* c = GetRealElement(in);
        CRealElement* d = GetRealElement(in);
        CRealElement* e = GetRealElement(in);
        return new CVECircle(a, b, c, d, e);
    }
    case SBIG('KEYE'):
    case SBIG('KEYP'):
    {
        return new CVEKeyframeEmitter(in);
    }
    case SBIG('MULT'):
    {
        CVectorElement* a = GetVectorElement(in);
        CVectorElement* b = GetVectorElement(in);
        return new CVEMultiply(a, b);
    }
    case SBIG('RTOV'):
    {
        CRealElement* a = GetRealElement(in);
        return new CVERealToVector(a);
    }
    case SBIG('PULS'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        CVectorElement* c = GetVectorElement(in);
        CVectorElement* d = GetVectorElement(in);
        return new CVEPulse(a, b, c, d);
    }
    case SBIG('PVEL'):
    {
        return new CVEParticleVelocity;
    }
    case SBIG('PLCO'):
    {
        return new CVEParticleColor;
    }
    case SBIG('PLOC'):
    {
        return new CVEParticleLocation;
    }
    case SBIG('PSOF'):
    {
        return new CVEParticleSystemOrientationFront;
    }
    case SBIG('PSOU'):
    {
        return new CVEParticleSystemOrientationUp;
    }
    case SBIG('PSOR'):
    {
        return new CVEParticleSystemOrientationRight;
    }
    case SBIG('PSTR'):
    {
        return new CVEParticleSystemTranslation;
    }
    case SBIG('SUB_'):
    {
        CVectorElement* a = GetVectorElement(in);
        CVectorElement* b = GetVectorElement(in);
        return new CVESubtract(a, b);
    }
    case SBIG('CTVC'):
    {
        CColorElement* a = GetColorElement(in);
        return new CVEColorToVector(a);
    }
    default: break;
    }
    return nullptr;
}

CRealElement* CParticleDataFactory::GetRealElement(CInputStream& in)
{
    FourCC clsId = GetClassID(in);
    switch (clsId)
    {
    case SBIG('LFTW'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        return new CRELifetimeTween(a, b);
    }
    case SBIG('CNST'):
    {
        float a = GetReal(in);
        return new CREConstant(a);
    }
    case SBIG('CHAN'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        CIntElement* c = GetIntElement(in);
        return new CRETimeChain(a, b, c);
    }
    case SBIG('ADD_'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        return new CREAdd(a, b);
    }
    case SBIG('CLMP'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        CRealElement* c = GetRealElement(in);
        return new CREClamp(a, b, c);
    }
    case SBIG('KEYE'):
    case SBIG('KEYP'):
    {
        return new CREKeyframeEmitter(in);
    }
    case SBIG('IRND'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        return new CREInitialRandom(a, b);
    }
    case SBIG('RAND'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        return new CRERandom(a, b);
    }
    case SBIG('DOTP'):
    {
        CVectorElement* a = GetVectorElement(in);
        CVectorElement* b = GetVectorElement(in);
        return new CREDotProduct(a, b);
    }
    case SBIG('MULT'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        return new CREMultiply(a, b);
    }
    case SBIG('PULS'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        CRealElement* c = GetRealElement(in);
        CRealElement* d = GetRealElement(in);
        return new CREPulse(a, b, c, d);
    }
    case SBIG('SCAL'):
    {
        CRealElement* a = GetRealElement(in);
        return new CRETimeScale(a);
    }
    case SBIG('RLPT'):
    {
        CRealElement* a = GetRealElement(in);
        return new CRELifetimePercent(a);
    }
    case SBIG('SINE'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        CRealElement* c = GetRealElement(in);
        return new CRESineWave(a, b, c);
    }
    case SBIG('ISWT'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        return new CREISWT(a, b);
    }
    case SBIG('CLTN'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        CRealElement* c = GetRealElement(in);
        CRealElement* d = GetRealElement(in);
        return new CRECompareLessThan(a, b, c, d);
    }
    case SBIG('CEQL'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        CRealElement* c = GetRealElement(in);
        CRealElement* d = GetRealElement(in);
        return new CRECompareEquals(a, b, c, d);
    }
    case SBIG('PAP1'):
    {
        return new CREParticleAccessParam1;
    }
    case SBIG('PAP2'):
    {
        return new CREParticleAccessParam2;
    }
    case SBIG('PAP3'):
    {
        return new CREParticleAccessParam3;
    }
    case SBIG('PAP4'):
    {
        return new CREParticleAccessParam4;
    }
    case SBIG('PAP5'):
    {
        return new CREParticleAccessParam5;
    }
    case SBIG('PAP6'):
    {
        return new CREParticleAccessParam6;
    }
    case SBIG('PAP7'):
    {
        return new CREParticleAccessParam7;
    }
    case SBIG('PAP8'):
    {
        return new CREParticleAccessParam8;
    }
    case SBIG('PSLL'):
    {
        return new CREPSLL;
    }
    case SBIG('PRLW'):
    {
        return new CREPRLW;
    }
    case SBIG('SUB_'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        return new CRESubtract(a, b);
    }
    case SBIG('VMAG'):
    {
        CVectorElement* a = GetVectorElement(in);
        return new CREVectorMagnitude(a);
    }
    case SBIG('VXTR'):
    {
        CVectorElement* a = GetVectorElement(in);
        return new CREVectorXToReal(a);
    }
    case SBIG('VYTR'):
    {
        CVectorElement* a = GetVectorElement(in);
        return new CREVectorYToReal(a);
    }
    case SBIG('VZTR'):
    {
        CVectorElement* a = GetVectorElement(in);
        return new CREVectorZToReal(a);
    }
    case SBIG('CEXT'):
    {
        CIntElement* a = GetIntElement(in);
        return new CRECEXT(a);
    }
    case SBIG('ITRL'):
    {
        CIntElement* a = GetIntElement(in);
        CRealElement* b = GetRealElement(in);
        return new CREIntTimesReal(a, b);
    }
    case SBIG('CRNG'):
    {
        CRealElement* a = GetRealElement(in);
        CRealElement* b = GetRealElement(in);
        CRealElement* c = GetRealElement(in);
        CRealElement* d = GetRealElement(in);
        CRealElement* e = GetRealElement(in);
        return new CREConstantRange(a, b, c, d, e);
    }
    case SBIG('GTCR'):
    {
        CColorElement* a = GetColorElement(in);
        return new CREGetComponentRed(a);
    }
    case SBIG('GTCG'):
    {
        CColorElement* a = GetColorElement(in);
        return new CREGetComponentGreen(a);
    }
    case SBIG('GTCB'):
    {
        CColorElement* a = GetColorElement(in);
        return new CREGetComponentBlue(a);
    }
    case SBIG('GTCA'):
    {
        CColorElement* a = GetColorElement(in);
        return new CREGetComponentAlpha(a);
    }
    default: break;
    }
    return nullptr;
}

CIntElement* CParticleDataFactory::GetIntElement(CInputStream& in)
{
    FourCC clsId = GetClassID(in);
    switch (clsId)
    {
    case SBIG('KEYE'):
    case SBIG('KEYP'):
    {
        return new CIEKeyframeEmitter(in);
    }
    case SBIG('DETH'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        return new CIEDeath(a, b);
    }
    case SBIG('CLMP'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        CIntElement* c = GetIntElement(in);
        return new CIEClamp(a, b, c);
    }
    case SBIG('CHAN'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        CIntElement* c = GetIntElement(in);
        return new CIETimeChain(a, b, c);
    }
    case SBIG('ADD_'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        return new CIEAdd(a, b);
    }
    case SBIG('CNST'):
    {
        int a = GetInt(in);
        return new CIEConstant(a);
    }
    case SBIG('IMPL'):
    {
        CIntElement* a = GetIntElement(in);
        return new CIEImpulse(a);
    }
    case SBIG('ILPT'):
    {
        CIntElement* a = GetIntElement(in);
        return new CIELifetimePercent(a);
    }
    case SBIG('IRND'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        return new CIEInitialRandom(a, b);
    }
    case SBIG('PULS'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        CIntElement* c = GetIntElement(in);
        CIntElement* d = GetIntElement(in);
        return new CIEPulse(a, b, c, d);
    }
    case SBIG('MULT'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        return new CIEMultiply(a, b);
    }
    case SBIG('SPAH'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        CIntElement* c = GetIntElement(in);
        return new CIESampleAndHold(a, b, c);
    }
    case SBIG('RAND'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        return new CIERandom(a, b);
    }
    case SBIG('TSCL'):
    {
        CRealElement* a = GetRealElement(in);
        return new CIETimeScale(a);
    }
    case SBIG('GAPC'):
    {
        return new CIEGetActiveParticleCount;
    }
    case SBIG('GTCP'):
    {
        return new CIEGetCumulativeParticleCount;
    }
    case SBIG('GEMT'):
    {
        return new CIEGetEmitterTime;
    }
    case SBIG('MODU'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        return new CIEModulo(a, b);
    }
    case SBIG('SUB_'):
    {
        CIntElement* a = GetIntElement(in);
        CIntElement* b = GetIntElement(in);
        return new CIESubtract(a, b);
    }
    default: break;
    }
    return nullptr;
}

CGenDescription* CParticleDataFactory::GetGeneratorDesc(CInputStream& in, CSimplePool* resPool)
{
    std::vector<TResId> tracker;
    tracker.reserve(8);
    return CreateGeneratorDescription(in, tracker, 0, resPool);
}

CGenDescription* CParticleDataFactory::CreateGeneratorDescription(CInputStream& in, std::vector<TResId>& tracker,
                                                                  TResId resId, CSimplePool* resPool)
{
    if (std::count(tracker.cbegin(), tracker.cend(), resId) == 0)
    {
        tracker.push_back(resId);
        FourCC cid = GetClassID(in);
        if (cid == FOURCC('GPSM'))
        {
            CGenDescription* ret = new CGenDescription;
            CreateGPSM(ret, in, tracker, resPool);
            LoadGPSMTokens(ret);
            return ret;
        }
    }
    return nullptr;
}

bool CParticleDataFactory::CreateGPSM(CGenDescription* fillDesc, CInputStream& in,
                                      std::vector<TResId>& tracker, CSimplePool* resPool)
{
    CRandom16 rand{99};
    CGlobalRandom gr(rand);
    FourCC clsId = GetClassID(in);
    while (clsId != SBIG('_END'))
    {
        switch (clsId)
        {
        case SBIG('PMCL'):
            fillDesc->x78_x64_PMCL.reset(GetColorElement(in));
            break;
        case SBIG('LFOR'):
            fillDesc->x118_x104_LFOR.reset(GetRealElement(in));
            break;
        case SBIG('IDTS'):
            fillDesc->xa4_x90_IDTS = GetChildGeneratorDesc(in, resPool, tracker);
            break;
        case SBIG('EMTR'):
            fillDesc->x40_x2c_EMTR.reset(GetEmitterElement(in));
            break;
        case SBIG('COLR'):
            fillDesc->x30_x24_COLR.reset(GetColorElement(in));
            break;
        case SBIG('CIND'):
            fillDesc->x45_30_x32_24_CIND = GetBool(in);
            break;
        case SBIG('AAPH'):
            fillDesc->x44_26_x30_26_AAPH = GetBool(in);
            break;
        case SBIG('CSSD'):
            fillDesc->xa0_x8c_CSSD.reset(GetIntElement(in));
            break;
        case SBIG('GRTE'):
            fillDesc->x2c_x20_GRTE.reset(GetRealElement(in));
            break;
        case SBIG('FXLL'):
            fillDesc->x44_25_x30_25_FXLL = GetBool(in);
            break;
        case SBIG('ICTS'):
            fillDesc->x8c_x78_ICTS = GetChildGeneratorDesc(in, resPool, tracker);
            break;
        case SBIG('KSSM'):
        {
            fillDesc->xd0_xbc_KSSM.reset();
            FourCC cid = GetClassID(in);
            if (cid != SBIG('CNST'))
                break;
            fillDesc->xd0_xbc_KSSM.reset(new CSpawnSystemKeyframeData(in));
            fillDesc->xd0_xbc_KSSM->LoadAllSpawnedSystemTokens(resPool);
            break;
        }
        case SBIG('ILOC'):
            delete GetVectorElement(in);
            break;
        case SBIG('IITS'):
            fillDesc->xb8_xa4_IITS = GetChildGeneratorDesc(in, resPool, tracker);
            break;
        case SBIG('IVEC'):
            delete GetVectorElement(in);
            break;
        case SBIG('LDIR'):
            fillDesc->x110_xfc_LDIR.reset(GetVectorElement(in));
            break;
        case SBIG('LCLR'):
            fillDesc->x104_xf0_LCLR.reset(GetColorElement(in));
            break;
        case SBIG('LENG'):
            fillDesc->x20_x14_LENG.reset(GetRealElement(in));
            break;
        case SBIG('MAXP'):
            fillDesc->x28_x1c_MAXP.reset(GetIntElement(in));
            break;
        case SBIG('LOFF'):
            fillDesc->x10c_xf8_LOFF.reset(GetVectorElement(in));
            break;
        case SBIG('LINT'):
            fillDesc->x108_xf4_LINT.reset(GetRealElement(in));
            break;
        case SBIG('LINE'):
            fillDesc->x44_24_x30_24_LINE = GetBool(in);
            break;
        case SBIG('LFOT'):
            fillDesc->x114_x100_LFOT.reset(GetIntElement(in));
            break;
        case SBIG('LIT_'):
            fillDesc->x44_29_x30_29_LIT_ = GetBool(in);
            break;
        case SBIG('LTME'):
            fillDesc->x34_x28_LTME.reset(GetIntElement(in));
            break;
        case SBIG('LSLA'):
            fillDesc->x11c_x108_LSLA.reset(GetRealElement(in));
            break;
        case SBIG('LTYP'):
            fillDesc->x100_xec_LTYP.reset(GetIntElement(in));
            break;
        case SBIG('NDSY'):
            fillDesc->xb4_xa0_NDSY.reset(GetIntElement(in));
            break;
        case SBIG('MBSP'):
            fillDesc->x48_x34_MBSP.reset(GetIntElement(in));
            break;
        case SBIG('MBLR'):
            fillDesc->x44_30_x31_24_MBLR = GetBool(in);
            break;
        case SBIG('NCSY'):
            fillDesc->x9c_x88_NCSY.reset(GetIntElement(in));
            break;
        case SBIG('PISY'):
            fillDesc->xc8_xb4_PISY.reset(GetIntElement(in));
            break;
        case SBIG('OPTS'):
            fillDesc->x45_31_x32_25_OPTS = GetBool(in);
            break;
        case SBIG('PMAB'):
            fillDesc->x44_31_x31_25_PMAB = GetBool(in);
            break;
        case SBIG('SESD'):
            fillDesc->xf8_xe4_SESD.reset(GetIntElement(in));
            break;
        case SBIG('SEPO'):
            fillDesc->xfc_xe8_SEPO.reset(GetVectorElement(in));
            break;
        case SBIG('PSLT'):
            fillDesc->xc_x0_PSLT.reset(GetIntElement(in));
            break;
        case SBIG('PMSC'):
            fillDesc->x74_x60_PMSC.reset(GetVectorElement(in));
            break;
        case SBIG('PMOP'):
            fillDesc->x6c_x58_PMOP.reset(GetVectorElement(in));
            break;
        case SBIG('PMDL'):
            fillDesc->x5c_x48_PMDL = GetModel(in, resPool);
            break;
        case SBIG('PMRT'):
            fillDesc->x70_x5c_PMRT.reset(GetVectorElement(in));
            break;
        case SBIG('POFS'):
            fillDesc->x18_xc_POFS.reset(GetVectorElement(in));
            break;
        case SBIG('PMUS'):
            fillDesc->x45_24_x31_26_PMUS = GetBool(in);
            break;
        case SBIG('PSIV'):
            delete GetVectorElement(in);
            break;
        case SBIG('ROTA'):
            fillDesc->x50_x3c_ROTA.reset(GetRealElement(in));
            break;
        case SBIG('PSVM'):
            delete GetModVectorElement(in);
            break;
        case SBIG('PSTS'):
            fillDesc->x14_x8_PSTS.reset(GetRealElement(in));
            break;
        case SBIG('PSOV'):
            delete GetVectorElement(in);
            break;
        case SBIG('PSWT'):
            fillDesc->x10_x4_PSWT.reset(GetIntElement(in));
            break;
        case SBIG('SEED'):
            fillDesc->x1c_x10_SEED.reset(GetIntElement(in));
            break;
        case SBIG('PMOO'):
            fillDesc->x45_25_x31_27_PMOO = GetBool(in);
            break;
        case SBIG('SSSD'):
            fillDesc->xe4_xd0_SSSD.reset(GetIntElement(in));
            break;
        case SBIG('SORT'):
            fillDesc->x44_28_x30_28_SORT = GetBool(in);
            break;
        case SBIG('SIZE'):
            fillDesc->x4c_x38_SIZE.reset(GetRealElement(in));
            break;
        case SBIG('SISY'):
            fillDesc->xcc_xb8_SISY.reset(GetIntElement(in));
            break;
        case SBIG('SSPO'):
            fillDesc->xe8_xd4_SSPO.reset(GetVectorElement(in));
            break;
        case SBIG('TEXR'):
            fillDesc->x54_x40_TEXR.reset(GetTextureElement(in, resPool));
            break;
        case SBIG('SSWH'):
            fillDesc->xd4_xc0_SSWH = GetSwooshGeneratorDesc(in, resPool);
            break;
        case SBIG('TIND'):
            fillDesc->x58_x44_TIND.reset(GetTextureElement(in, resPool));
            break;
        case SBIG('VMD4'):
            fillDesc->x45_29_x31_31_VMD4 = GetBool(in);
            break;
        case SBIG('VMD3'):
            fillDesc->x45_28_x31_30_VMD3 = GetBool(in);
            break;
        case SBIG('VMD2'):
            fillDesc->x45_27_x31_29_VMD2 = GetBool(in);
            break;
        case SBIG('VMD1'):
            fillDesc->x45_26_x31_28_VMD1 = GetBool(in);
            break;
        case SBIG('VEL4'):
            fillDesc->x88_x74_VEL4.reset(GetModVectorElement(in));
            break;
        case SBIG('VEL3'):
            fillDesc->x84_x70_VEL3.reset(GetModVectorElement(in));
            break;
        case SBIG('VEL2'):
            fillDesc->x80_x6c_VEL2.reset(GetModVectorElement(in));
            break;
        case SBIG('VEL1'):
            fillDesc->x7c_x68_VEL1.reset(GetModVectorElement(in));
            break;
        case SBIG('ZBUF'):
            fillDesc->x44_27_x30_27_ZBUF = GetBool(in);
            break;
        case SBIG('WIDT'):
            fillDesc->x24_x18_WIDT.reset(GetRealElement(in));
            break;
        case SBIG('ORNT'):
            fillDesc->x30_30_ORNT = GetBool(in);
            break;
        case SBIG('RSOP'):
            fillDesc->x30_31_RSOP = GetBool(in);
            break;
        case SBIG('ADV1'):
            fillDesc->x10c_ADV1.reset(GetRealElement(in));
            break;
        case SBIG('ADV2'):
            fillDesc->x110_ADV2.reset(GetRealElement(in));
            break;
        case SBIG('ADV3'):
            fillDesc->x114_ADV3.reset(GetRealElement(in));
            break;
        case SBIG('ADV4'):
            fillDesc->x118_ADV4.reset(GetRealElement(in));
            break;
        case SBIG('ADV5'):
            fillDesc->x11c_ADV5.reset(GetRealElement(in));
            break;
        case SBIG('ADV6'):
            fillDesc->x120_ADV6.reset(GetRealElement(in));
            break;
        case SBIG('ADV7'):
            fillDesc->x124_ADV7.reset(GetRealElement(in));
            break;
        case SBIG('ADV8'):
            fillDesc->x128_ADV8.reset(GetRealElement(in));
            break;
        case SBIG('SELC'):
            fillDesc->xec_xd8_SELC = GetElectricGeneratorDesc(in, resPool);
            break;
        default:
        {
            uint32_t clsName = clsId.toUint32();
            Log.report(logvisor::Fatal, "Unknown GPSM class %.4s @%" PRIi64, &clsName, in.position());
            return false;
        }
        }
        clsId = GetClassID(in);
    }

    /* Now for our custom additions, if available */
    if (!in.atEnd())
    {
        clsId = GetClassID(in);
        if (clsId == 0xFFFFFFFF)
            return true;

        while (clsId != SBIG('_END') && !in.atEnd())
        {
            switch(clsId)
            {
            case SBIG('BGCL'):
                fillDesc->m_bevelGradient.reset(GetColorElement(in));
            break;
            }
            clsId = GetClassID(in);
        }
    }
    return true;
}

void CParticleDataFactory::LoadGPSMTokens(CGenDescription* desc)
{
    if (desc->x5c_x48_PMDL.m_found)
        desc->x5c_x48_PMDL.m_model = desc->x5c_x48_PMDL.m_token.GetObj();

    if (desc->x8c_x78_ICTS.m_found)
        desc->x8c_x78_ICTS.m_gen = desc->x8c_x78_ICTS.m_token.GetObj();

    if (desc->xa4_x90_IDTS.m_found)
        desc->xa4_x90_IDTS.m_gen = desc->xa4_x90_IDTS.m_token.GetObj();

    if (desc->xb8_xa4_IITS.m_found)
        desc->xb8_xa4_IITS.m_gen = desc->xb8_xa4_IITS.m_token.GetObj();

    if (desc->xd4_xc0_SSWH.m_found)
        desc->xd4_xc0_SSWH.m_swoosh = desc->xd4_xc0_SSWH.m_token.GetObj();
}

CFactoryFnReturn FParticleFactory(const SObjectTag& tag, CInputStream& in,
                                       const CVParamTransfer& vparms)
{
    CSimplePool* sp = static_cast<CSimplePool*>(static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam());
    return TToken<CGenDescription>::GetIObjObjectFor(std::unique_ptr<CGenDescription>(CParticleDataFactory::GetGeneratorDesc(in, sp)));
}

}
