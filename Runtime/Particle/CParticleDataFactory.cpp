#include "CParticleDataFactory.hpp"
#include "CToken.hpp"
#include "CSimplePool.hpp"
#include "CGenDescription.hpp"

namespace Retro
{
static LogVisor::LogModule Log("Retro::CParticleDataFactory");

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
        Log.report(LogVisor::FatalError, "bool element does not begin with CNST");
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
    return {resPool->GetObj({FOURCC('CMDL'), id}), true};
}

SChildGeneratorDesc CParticleDataFactory::GetChildGeneratorDesc(TResId res, CSimplePool* resPool, const std::vector<TResId>& tracker)
{
    if (std::count(tracker.cbegin(), tracker.cend(), res) == 0)
        return {resPool->GetObj({FOURCC('PART'), res}), true};
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
    return {resPool->GetObj({FOURCC('SWHC'), id}), true};
}

SElectricGeneratorDesc CParticleDataFactory::GetElectricGeneratorDesc(CInputStream& in, CSimplePool* resPool)
{
    FourCC clsId = GetClassID(in);
    if (clsId == SBIG('NONE'))
        return {};
    TResId id = in.readUint32Big();
    if (!id)
        return {};
    return {resPool->GetObj({FOURCC('ELSC'), id}), true};
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
    case SBIG('SPOS'):
    {
        CVectorElement* a = GetVectorElement(in);
        return new CVESPOS(a);
    }
    case SBIG('PLCO'):
    {
        return new CVEPLCO;
    }
    case SBIG('PLOC'):
    {
        return new CVEPLOC;
    }
    case SBIG('PSOF'):
    {
        return new CVEPSOF;
    }
    case SBIG('PSOU'):
    {
        return new CVEPSOU;
    }
    case SBIG('PSOR'):
    {
        return new CVEPSOR;
    }
    case SBIG('PSTR'):
    {
        return new CVEPSTR;
    }
    case SBIG('SUB_'):
    {
        CVectorElement* a = GetVectorElement(in);
        CVectorElement* b = GetVectorElement(in);
        return new CVESubtract(a, b);
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
        return new CIEGAPC;
    }
    case SBIG('GTCP'):
    {
        return new CIEGTCP;
    }
    case SBIG('GEMT'):
    {
        return new CIEGEMT;
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
    FourCC clsId = GetClassID(in);
    while (clsId != SBIG('_END'))
    {
        switch (clsId)
        {
        case SBIG('PMCL'):
            fillDesc->x78_PMCL.reset(GetColorElement(in));
            break;
        case SBIG('LFOR'):
            fillDesc->x118_LFOR.reset(GetRealElement(in));
            break;
        case SBIG('IDTS'):
            fillDesc->xa4_IDTS = GetChildGeneratorDesc(in, resPool, tracker);
            break;
        case SBIG('EMTR'):
            fillDesc->x40_EMTR.reset(GetEmitterElement(in));
            break;
        case SBIG('COLR'):
            fillDesc->x30_COLR.reset(GetColorElement(in));
            break;
        case SBIG('CIND'):
            fillDesc->x45_30_CIND = GetBool(in);
            break;
        case SBIG('AAPH'):
            fillDesc->x44_26_AAPH = GetBool(in);
            break;
        case SBIG('CSSD'):
            fillDesc->xa0_CSSD.reset(GetIntElement(in));
            break;
        case SBIG('GRTE'):
            fillDesc->x2c_GRTE.reset(GetRealElement(in));
            break;
        case SBIG('FXLL'):
            fillDesc->x44_25_FXLL = GetBool(in);
            break;
        case SBIG('ICTS'):
            fillDesc->x8c_ICTS = GetChildGeneratorDesc(in, resPool, tracker);
            break;
        case SBIG('KSSM'):
        {
            fillDesc->xd0_KSSM.reset();
            FourCC cid = GetClassID(in);
            if (cid != SBIG('CNST'))
                break;
            fillDesc->xd0_KSSM.reset(new CSpawnSystemKeyframeData(in));
            fillDesc->xd0_KSSM->LoadAllSpawnedSystemTokens(resPool);
            break;
        }
        case SBIG('ILOC'):
            fillDesc->x38_ILOC.reset(GetVectorElement(in));
            break;
        case SBIG('IITS'):
            fillDesc->xb8_IITS = GetChildGeneratorDesc(in, resPool, tracker);
            break;
        case SBIG('IVEC'):
            fillDesc->x3c_IVEC.reset(GetVectorElement(in));
            break;
        case SBIG('LDIR'):
            fillDesc->x110_LDIR.reset(GetVectorElement(in));
            break;
        case SBIG('LCLR'):
            fillDesc->x104_LCLR.reset(GetColorElement(in));
            break;
        case SBIG('LENG'):
            fillDesc->x20_LENG.reset(GetRealElement(in));
            break;
        case SBIG('MAXP'):
            fillDesc->x28_MAXP.reset(GetIntElement(in));
            break;
        case SBIG('LOFF'):
            fillDesc->x10c_LOFF.reset(GetVectorElement(in));
            break;
        case SBIG('LINT'):
            fillDesc->x108_LINT.reset(GetRealElement(in));
            break;
        case SBIG('LINE'):
            fillDesc->x44_24_LINE = GetBool(in);
            break;
        case SBIG('LFOT'):
            fillDesc->x114_LFOT.reset(GetIntElement(in));
            break;
        case SBIG('LIT_'):
            fillDesc->x44_29_LIT_ = GetBool(in);
            break;
        case SBIG('LTME'):
            fillDesc->x34_LTME.reset(GetIntElement(in));
            break;
        case SBIG('LSLA'):
            fillDesc->x11c_LSLA.reset(GetRealElement(in));
            break;
        case SBIG('LTYP'):
            fillDesc->x100_LTYP.reset(GetIntElement(in));
            break;
        case SBIG('NDSY'):
            fillDesc->xb4_NDSY.reset(GetIntElement(in));
            break;
        case SBIG('MBSP'):
            fillDesc->x48_MBSP.reset(GetIntElement(in));
            break;
        case SBIG('MBLR'):
            fillDesc->x44_30_MBLR = GetBool(in);
            break;
        case SBIG('NCSY'):
            fillDesc->x9c_NCSY.reset(GetIntElement(in));
            break;
        case SBIG('PISY'):
            fillDesc->xc8_PISY.reset(GetIntElement(in));
            break;
        case SBIG('OPTS'):
            fillDesc->x45_31_OPTS = GetBool(in);
            break;
        case SBIG('PMAB'):
            fillDesc->x44_31_PMAB = GetBool(in);
            break;
        case SBIG('SESD'):
            fillDesc->xf8_SESD.reset(GetIntElement(in));
            break;
        case SBIG('SEPO'):
            fillDesc->xfc_SEPO.reset(GetVectorElement(in));
            break;
        case SBIG('PSLT'):
            fillDesc->xc_PSLT.reset(GetIntElement(in));
            break;
        case SBIG('PMSC'):
            fillDesc->x74_PMSC.reset(GetVectorElement(in));
            break;
        case SBIG('PMOP'):
            fillDesc->x6c_PMOP.reset(GetVectorElement(in));
            break;
        case SBIG('PMDL'):
            fillDesc->x5c_PMDL = GetModel(in, resPool);
            break;
        case SBIG('PMRT'):
            fillDesc->x70_PMRT.reset(GetVectorElement(in));
            break;
        case SBIG('POFS'):
            fillDesc->x18_POFS.reset(GetVectorElement(in));
            break;
        case SBIG('PMUS'):
            fillDesc->x45_24_PMUS = GetBool(in);
            break;
        case SBIG('PSIV'):
            fillDesc->x0_PSIV.reset(GetVectorElement(in));
            break;
        case SBIG('ROTA'):
            fillDesc->x50_ROTA.reset(GetRealElement(in));
            break;
        case SBIG('PSVM'):
            fillDesc->x4_PSVM.reset(GetModVectorElement(in));
            break;
        case SBIG('PSTS'):
            fillDesc->x14_PSTS.reset(GetRealElement(in));
            break;
        case SBIG('PSOV'):
            fillDesc->x8_PSOV.reset(GetVectorElement(in));
            break;
        case SBIG('PSWT'):
            fillDesc->x10_PSWT.reset(GetIntElement(in));
            break;
        case SBIG('PMLC'):
            fillDesc->xec_PMLC = GetChildGeneratorDesc(in, resPool, tracker);
            break;
        case SBIG('SEED'):
            fillDesc->x1c_SEED.reset(GetIntElement(in));
            break;
        case SBIG('PMOO'):
            fillDesc->x45_25_PMOO = GetBool(in);
            break;
        case SBIG('SSSD'):
            fillDesc->xe4_SSSD.reset(GetIntElement(in));
            break;
        case SBIG('SORT'):
            fillDesc->x44_28_SORT = GetBool(in);
            break;
        case SBIG('SIZE'):
            fillDesc->x4c_SIZE.reset(GetRealElement(in));
            break;
        case SBIG('SISY'):
            fillDesc->xcc_SISY.reset(GetIntElement(in));
            break;
        case SBIG('SSPO'):
            fillDesc->xe8_SSPO.reset(GetVectorElement(in));
            break;
        case SBIG('TEXR'):
            fillDesc->x54_TEXR.reset(GetTextureElement(in, resPool));
            break;
        case SBIG('SSWH'):
            fillDesc->xd4_SSWH = GetSwooshGeneratorDesc(in, resPool);
            break;
        case SBIG('TIND'):
            fillDesc->x58_TIND.reset(GetTextureElement(in, resPool));
            break;
        case SBIG('VMD4'):
            fillDesc->x45_29_VMD4 = GetBool(in);
            break;
        case SBIG('VMD3'):
            fillDesc->x45_28_VMD3 = GetBool(in);
            break;
        case SBIG('VMD2'):
            fillDesc->x45_27_VMD2 = GetBool(in);
            break;
        case SBIG('VMD1'):
            fillDesc->x45_26_VMD1 = GetBool(in);
            break;
        case SBIG('VEL4'):
            fillDesc->x88_VEL4.reset(GetModVectorElement(in));
            break;
        case SBIG('VEL3'):
            fillDesc->x84_VEL3.reset(GetModVectorElement(in));
            break;
        case SBIG('VEL2'):
            fillDesc->x80_VEL2.reset(GetModVectorElement(in));
            break;
        case SBIG('VEL1'):
            fillDesc->x7c_VEL1.reset(GetModVectorElement(in));
            break;
        case SBIG('ZBUF'):
            fillDesc->x44_27_ZBUF = GetBool(in);
            break;
        case SBIG('WIDT'):
            fillDesc->x24_WIDT.reset(GetRealElement(in));
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
            fillDesc->xec_SELC = GetElectricGeneratorDesc(in, resPool);
            break;
        default:
        {
            uint32_t clsName = clsId.toUint32();
            Log.report(LogVisor::FatalError, "Unknown GPSM class %.4s @%" PRIi64, &clsName, in.position());
            return false;
        }
        }
        clsId = GetClassID(in);
    }
    return true;
}

void CParticleDataFactory::LoadGPSMTokens(CGenDescription* desc)
{
}

std::unique_ptr<IObj> FParticleFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms)
{
    CSimplePool* sp = static_cast<CSimplePool*>(static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam());
    return TToken<CGenDescription>::GetIObjObjectFor(std::unique_ptr<CGenDescription>(CParticleDataFactory::GetGeneratorDesc(in, sp)));
}

}
