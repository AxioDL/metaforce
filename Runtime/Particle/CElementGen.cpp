#include "CElementGen.hpp"
#include "CGenDescription.hpp"
#include "CLight.hpp"
#include "CParticleGlobals.hpp"

namespace Retro
{
static LogVisor::LogModule Log("Retro::CElementGen");

static bool s_inCreateNewParticles = false;

int CElementGen::g_ParticleAliveCount;
int CElementGen::g_ParticleSystemAliveCount;
s32 CElementGen::g_FreeIndex;
bool CElementGen::g_StaticListInitialized = false;
static rstl::reserved_vector<CElementGen::CParticle, 2560> g_StaticParticleList;
static rstl::reserved_vector<u16, 2560> g_StaticFreeList;
void CElementGen::Initialize()
{
    if (g_StaticListInitialized)
        return;
    Log.report(LogVisor::Info, "Initialize - Static Particle List - ");

    g_ParticleAliveCount = 0;
    g_ParticleSystemAliveCount = 0;

    g_StaticParticleList.clear();
    g_StaticParticleList.insert(g_StaticParticleList.end(), 2560, CParticle());

    g_StaticFreeList.clear();
    int c=0;
    for (int i=0 ; i<512 ; ++i)
    {
        g_StaticFreeList.push_back(c++);
        g_StaticFreeList.push_back(c++);
        g_StaticFreeList.push_back(c++);
        g_StaticFreeList.push_back(c++);
        g_StaticFreeList.push_back(c++);
    }

    g_FreeIndex = 2559;
    Log.report(LogVisor::Info, "size %d (%d each part).", (56 + 2) * 2560, 56);
    g_StaticListInitialized = true;
}

CElementGen::CElementGen(const TToken<CGenDescription>& gen,
                         EModelOrientationType orientType,
                         EOptionalSystemFlags flags)
: x1c_genDesc(gen), x28_orientType(orientType),
  x226((flags & EOptionalSystemFlags::Two) != EOptionalSystemFlags::None), x230_randState(x74)
{
    CIntElement* pmedElem = x1c_genDesc.GetObj()->x1c_PMED.get();
    if (pmedElem)
    {
        int pmedVal;
        pmedElem->GetValue(x50_curFrame, pmedVal);
        x74 = pmedVal;
    }
    x230_randState.SetSeed(x74);
    ++g_ParticleSystemAliveCount;
    x224_25_LIT_ = x1c_genDesc.GetObj()->x44_29_LIT_;
    x224_26_AAPH = x1c_genDesc.GetObj()->x44_26_AAPH;
    x224_27_ZBUF = x1c_genDesc.GetObj()->x44_27_ZBUF;
    x224_29_MBLR = x1c_genDesc.GetObj()->x44_30_MBLR;

    CIntElement* mbspElem = x1c_genDesc.GetObj()->x48_MBSP.get();
    if (mbspElem)
        mbspElem->GetValue(x50_curFrame, x228_MBSP);

    x224_30_VMD1 = x1c_genDesc.GetObj()->x45_26_VMD1;
    x224_31_VMD2 = x1c_genDesc.GetObj()->x45_27_VMD2;
    x225_24_VMD3 = x1c_genDesc.GetObj()->x45_28_VMD3;
    x225_25_VMD4 = x1c_genDesc.GetObj()->x45_29_VMD4;

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
        maxpElem->GetValue(x50_curFrame, x70_MAXP);

    x2c_particleLists.reserve(std::min(256, x70_MAXP));
    if (x28_orientType == EModelOrientationType::One)
    {
        x3c_parentMatrices.insert(x3c_parentMatrices.end(), x70_MAXP,
                                  Zeus::CMatrix3f::skIdentityMatrix3f);
    }

    x225_26_LINE = x1c_genDesc.GetObj()->x44_24_LINE;
    x225_27_FXLL = x1c_genDesc.GetObj()->x44_25_FXLL;

    CVectorElement* pofsElem = x1c_genDesc.GetObj()->x18_POFS.get();
    if (pofsElem)
        pofsElem->GetValue(x50_curFrame, x94_POFS);

    CIntElement* ltypElem = x1c_genDesc.GetObj()->x100_LTYP.get();
    if (ltypElem)
    {
        int ltyp;
        ltypElem->GetValue(x50_curFrame, ltyp);
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
        lfotElem->GetValue(x50_curFrame, lfot);
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

void CElementGen::Update(double t)
{
    CIntElement* pswtElem = x1c_genDesc.GetObj()->x10_PSWT.get();
    if (pswtElem && !x225_28_warmedUp)
    {
        int pswt = 0;
        pswtElem->GetValue(x50_curFrame, pswt);
        if (pswt > 32)
        {
            Log.report(LogVisor::Info,
                       "Running warmup on particle system 0x%08x for %d ticks.",
                       x1c_genDesc.GetObj(), pswt);
            InternalUpdate(pswt / 60.0);
            x225_28_warmedUp = true;
        }
    }
    InternalUpdate(t);
}

bool CElementGen::InternalUpdate(double dt)
{
    CGlobalRandom gr(x230_randState);

    double dt1 = 1 / 60.0;
    if (fabs(dt - 1 / 60.0) >= 1 / 60000.0)
        dt1 = dt;
    double t = x50_curFrame / 60.0;
    CParticleGlobals::SetEmitterTime(x50_curFrame);

    CRealElement* pstsElem = x1c_genDesc.GetObj()->x14_PSTS.get();
    if (pstsElem)
    {
        float psts;
        pstsElem->GetValue(x50_curFrame, psts);
        double dt1Scaled = psts * dt1;
        dt1 = std::max(0.0, dt1Scaled);
    }

    x58_curSeconds += dt1;

    if (x224_29_MBLR && dt > 0.0)
    {
        CIntElement* mbspElem = x1c_genDesc.GetObj()->x48_MBSP.get();
        if (mbspElem)
            mbspElem->GetValue(x50_curFrame, x228_MBSP);
    }

    int frameUpdateCount = 0;
    while (t < x58_curSeconds && fabs(t - x58_curSeconds) >= 1 / 60000.0)
    {
        x2a8_aabbMin.splat(FLT_MAX);
        x2b4_aabbMax.splat(FLT_MIN);
        x2c0_maxSize = 0.f;
        float grte = 0.f;
        CParticleGlobals::SetEmitterTime(x50_curFrame);
        CRealElement* grteElem = x1c_genDesc.GetObj()->x2c_GRTE.get();
        if (grteElem->GetValue(x50_curFrame, grte))
        {
            x2c_particleLists.clear();
            return true;
        }

        grte = std::max(0.f, grte * x78_generatorRate);
        x6c_generatorRemainder += grte;
        int genCount = floorf(x6c_generatorRemainder);
        x6c_generatorRemainder = x6c_generatorRemainder - genCount;

        if (x50_curFrame < x214_PSLT)
        {
            if (!x68_particleEmission)
                genCount = 0;
        }
        else
            genCount = 0;

        CIntElement* maxpElem = x1c_genDesc.GetObj()->x28_MAXP.get();
        if (maxpElem)
            maxpElem->GetValue(x50_curFrame, x70_MAXP);

        UpdateExistingParticles();

        CParticleGlobals::SetParticleLifetime(x214_PSLT);
        bool oldBoolVal = s_inCreateNewParticles;
        s_inCreateNewParticles = true;
        CreateNewParticles(genCount);
        s_inCreateNewParticles = oldBoolVal;

        UpdatePSTranslationAndOrientation();
        UpdateChildParticleSystems(1 / 60.0);

        if (x2dc_lightType != ELightType::LocalAmbient)
            UpdateLightParameters();

        ++frameUpdateCount;
        ++x50_curFrame;
        t += 1 / 60.0;
    }

    UpdateChildParticleSystems(-(frameUpdateCount / 60.0 - dt1));
    if (fabs(t - x58_curSeconds) < 1 / 60000.0)
        x58_curSeconds = t;

    BuildParticleSystemBounds();
    x224_24 = false;

    double passedTime = t - x58_curSeconds;
    x60 = 1.0 - passedTime * 60.0;

    return false;
}

void CElementGen::AccumulateBounds(Zeus::CVector3f& pos, float size)
{
    x2b4_aabbMax[0] = std::max(pos[0], x2b4_aabbMax[0]);
    x2b4_aabbMax[1] = std::max(pos[1], x2b4_aabbMax[1]);
    x2b4_aabbMax[2] = std::max(pos[2], x2b4_aabbMax[2]);
    x2a8_aabbMin[0] = std::min(pos[0], x2a8_aabbMin[0]);
    x2a8_aabbMin[1] = std::min(pos[1], x2a8_aabbMin[1]);
    x2a8_aabbMin[2] = std::min(pos[2], x2a8_aabbMin[2]);
    x2c0_maxSize = std::max(size, x2c0_maxSize);
}

void CElementGen::UpdateExistingParticles()
{
    x208_activeParticleCount = 0;
    CParticleGlobals::SetEmitterTime(x50_curFrame);
    for (std::vector<CParticleListItem>::iterator p = x2c_particleLists.begin();
         p != x2c_particleLists.end();)
    {
        CElementGen::CParticle& particle = g_StaticParticleList[p->x0_partIdx];
        if (particle.x0_endFrame < x50_curFrame)
        {
            --g_ParticleAliveCount;
            g_StaticFreeList[++g_FreeIndex] = p->x0_partIdx;
            if (p+1 == x2c_particleLists.end())
            {
                x2c_particleLists.pop_back();
                break;
            }
            else
            {
                *p = *(x2c_particleLists.end()-1);
                if (x28_orientType == EModelOrientationType::One)
                {
                    size_t i = p - x2c_particleLists.begin();
                    x3c_parentMatrices[i] = x3c_parentMatrices[x2c_particleLists.size()-1];
                }
                x2c_particleLists.pop_back();
                if (p != x2c_particleLists.end())
                    if (particle.x0_endFrame < x50_curFrame)
                        continue;
            }
        }
        else
        {
            particle.x10_prevPos = particle.x4_pos;
            particle.x4_pos += particle.x1c_vel;
        }

        ++x208_activeParticleCount;
        CParticleGlobals::SetParticleLifetime(particle.x0_endFrame - particle.x28_startFrame);
        int particleFrame = x50_curFrame - particle.x28_startFrame;
        CParticleGlobals::UpdateParticleLifetimeTweenValues(particleFrame);

        bool err;
        CModVectorElement* vel1 = x1c_genDesc.GetObj()->x7c_VEL1.get();
        if (vel1)
        {
            if (x224_30_VMD1)
            {
                Zeus::CVector3f xfVel = x1a8 * particle.x1c_vel;
                Zeus::CVector3f xfPos = x1a8 * (particle.x4_pos - x7c);
                err = vel1->GetValue(particleFrame, xfVel, xfPos);
                particle.x1c_vel = x178 * xfVel;
                particle.x4_pos = x178 * xfPos + x7c;
            }
            else
            {
                err = vel1->GetValue(particleFrame, particle.x1c_vel, particle.x4_pos);
            }
        }

        CModVectorElement* vel2 = x1c_genDesc.GetObj()->x80_VEL2.get();
        if (vel2)
        {
            if (x224_31_VMD2)
            {
                Zeus::CVector3f xfVel = x1a8 * particle.x1c_vel;
                Zeus::CVector3f xfPos = x1a8 * (particle.x4_pos - x7c);
                err |= vel2->GetValue(particleFrame, xfVel, xfPos);
                particle.x1c_vel = x178 * xfVel;
                particle.x4_pos = x178 * xfPos + x7c;
            }
            else
            {
                err |= vel2->GetValue(particleFrame, particle.x1c_vel, particle.x4_pos);
            }
        }

        CModVectorElement* vel3 = x1c_genDesc.GetObj()->x84_VEL3.get();
        if (vel3)
        {
            if (x225_24_VMD3)
            {
                Zeus::CVector3f xfVel = x1a8 * particle.x1c_vel;
                Zeus::CVector3f xfPos = x1a8 * (particle.x4_pos - x7c);
                err |= vel3->GetValue(particleFrame, xfVel, xfPos);
                particle.x1c_vel = x178 * xfVel;
                particle.x4_pos = x178 * xfPos + x7c;
            }
            else
            {
                err |= vel3->GetValue(particleFrame, particle.x1c_vel, particle.x4_pos);
            }
        }

        CModVectorElement* vel4 = x1c_genDesc.GetObj()->x88_VEL4.get();
        if (vel4)
        {
            if (x225_25_VMD4)
            {
                Zeus::CVector3f xfVel = x1a8 * particle.x1c_vel;
                Zeus::CVector3f xfPos = x1a8 * (particle.x4_pos - x7c);
                err |= vel4->GetValue(particleFrame, xfVel, xfPos);
                particle.x1c_vel = x178 * xfVel;
                particle.x4_pos = x178 * xfPos + x7c;
            }
            else
            {
                err |= vel4->GetValue(particleFrame, particle.x1c_vel, particle.x4_pos);
            }
        }

        if (x225_26_LINE)
        {
            CRealElement* leng = x1c_genDesc.GetObj()->x20_LENG.get();
            if (leng)
                err |= leng->GetValue(particleFrame, particle.x2c_lineLengthOrSize);
            CRealElement* widt = x1c_genDesc.GetObj()->x24_WIDT.get();
            if (widt)
                err |= widt->GetValue(particleFrame, particle.x30_lineWidthOrRota);
        }
        else
        {
            CRealElement* rota = x1c_genDesc.GetObj()->x50_ROTA.get();
            if (rota)
                err |= rota->GetValue(particleFrame, particle.x30_lineWidthOrRota);
            CRealElement* size = x1c_genDesc.GetObj()->x4c_SIZE.get();
            if (size)
                err |= size->GetValue(particleFrame, particle.x2c_lineLengthOrSize);
        }

        CColorElement* colr = x1c_genDesc.GetObj()->x30_COLR.get();
        if (colr)
            err |= colr->GetValue(particleFrame, particle.x34_color);

        if (err)
            particle.x0_endFrame = -1;

        AccumulateBounds(particle.x4_pos, particle.x2c_lineLengthOrSize);
        ++p;
    }
}

void CElementGen::CreateNewParticles(int count)
{
    if (!g_StaticListInitialized)
        Initialize();
    if (x2c_particleLists.size() >= x70_MAXP)
        return;

    if (count + x2c_particleLists.size() > x70_MAXP)
        count = x70_MAXP - x2c_particleLists.size();
    CGlobalRandom gr(x230_randState);
    x2c_particleLists.reserve(x70_MAXP);

    for (int i=0 ; i<count ; ++i)
    {
        if (g_FreeIndex < 0)
            return;

        s16 staticIdx = g_StaticFreeList[g_FreeIndex];
        x2c_particleLists.push_back(staticIdx);
        ++x208_activeParticleCount;
        if (x28_orientType == EModelOrientationType::One)
            x3c_parentMatrices[x2c_particleLists.size()-1] = x178.buildMatrix3f();

        CElementGen::CParticle& particle = g_StaticParticleList[staticIdx];
        particle.x28_startFrame = x50_curFrame;
        CIntElement* ltme = x1c_genDesc.GetObj()->x34_LTME.get();
        if (ltme)
            ltme->GetValue(0, particle.x0_endFrame);
        CParticleGlobals::SetParticleLifetime(particle.x0_endFrame);
        CParticleGlobals::UpdateParticleLifetimeTweenValues(0);
        particle.x0_endFrame += x50_curFrame;

        CColorElement* colr = x1c_genDesc.GetObj()->x30_COLR.get();
        if (colr)
            colr->GetValue(0, particle.x34_color);
        else
            particle.x34_color = Zeus::CColor::skWhite;

        CEmitterElement* emtr = x1c_genDesc.GetObj()->x40_EMTR.get();
        if (emtr)
        {
            emtr->GetValue(x210_curEmitterFrame, particle.x4_pos, particle.x1c_vel);
            Zeus::CVector3f compXf1 = (xdc * x148) * x7c;
            Zeus::CVector3f compXf2 = x178 * particle.x4_pos;
            particle.x4_pos = compXf1 + compXf2 + x94_POFS;
            particle.x1c_vel = x178 * particle.x1c_vel;
        }
        else
        {
            Zeus::CVector3f compXf1 = (xdc * x148) * x7c;
            particle.x4_pos = compXf1 + x94_POFS;
            particle.x1c_vel.zeroOut();
        }
        particle.x10_prevPos = particle.x4_pos;

        if (x225_26_LINE)
        {
            CRealElement* leng = x1c_genDesc.GetObj()->x20_LENG.get();
            if (leng)
                leng->GetValue(0, particle.x2c_lineLengthOrSize);
            else
                particle.x2c_lineLengthOrSize = 1.f;

            CRealElement* widt = x1c_genDesc.GetObj()->x24_WIDT.get();
            if (widt)
                widt->GetValue(0, particle.x30_lineWidthOrRota);
            else
                particle.x30_lineWidthOrRota = 1.f;
        }
        else
        {
            CRealElement* rota = x1c_genDesc.GetObj()->x50_ROTA.get();
            if (rota)
                rota->GetValue(0, particle.x30_lineWidthOrRota);
            else
                particle.x30_lineWidthOrRota = 0.f;

            CRealElement* size = x1c_genDesc.GetObj()->x4c_SIZE.get();
            if (size)
                size->GetValue(0, particle.x2c_lineLengthOrSize);
            else
                particle.x2c_lineLengthOrSize = 0.1f;
        }

        AccumulateBounds(particle.x4_pos, particle.x2c_lineLengthOrSize);
        ++x210_curEmitterFrame;
        --g_FreeIndex;
    }
}

void CElementGen::UpdatePSTranslationAndOrientation()
{
    CGlobalRandom gr(x230_randState);
    if (x214_PSLT < x50_curFrame)
        return;

    CModVectorElement* psvm = x1c_genDesc.GetObj()->x4_PSVM.get();
    if (psvm)
    {
        Zeus::CVector3f vel = x7c;
        psvm->GetValue(x50_curFrame, x218_PSIV, vel);
        if (vel != x7c)
        {
            x224_24 = true;
            x7c = vel;
        }
    }

    Zeus::CVector3f v = x178 * x218_PSIV;
    if (v != Zeus::CVector3f::skZero)
        x224_24 = true;
    x7c += v;

    CVectorElement* psov = x1c_genDesc.GetObj()->x8_PSOV.get();
    if (psov)
    {
        Zeus::CVector3f angles;
        psov->GetValue(x50_curFrame, angles);
        Zeus::CTransform xf(x178);
        xf.rotateLocalX(angles[0] * M_PI / 180.f);
        xf.rotateLocalY(angles[1] * M_PI / 180.f);
        xf.rotateLocalZ(angles[2] * M_PI / 180.f);
        SetOrientation(xf);
    }

    CVectorElement* pofs = x1c_genDesc.GetObj()->x18_POFS.get();
    if (pofs)
        pofs->GetValue(x50_curFrame, x94_POFS);

    CVectorElement* sspo = x1c_genDesc.GetObj()->xe8_SSPO.get();
    if (sspo)
        sspo->GetValue(x50_curFrame, x274_SSPO);

    CVectorElement* sepo = x1c_genDesc.GetObj()->xfc_SEPO.get();
    if (sepo)
        sspo->GetValue(x50_curFrame, x294_SEPO);
}

CElementGen* CElementGen::ConstructChildParticleSystem(const TToken<CGenDescription>& desc)
{

}

void CElementGen::UpdateChildParticleSystems(double dt)
{
    CGlobalRandom gr(x230_randState);
    SChildGeneratorDesc& icts = x1c_genDesc.GetObj()->x8c_ICTS;
    if (icts.m_found && x64 != x50_curFrame && x244_CSSD == x50_curFrame)
    {
        int ncsyVal = 1;
        CIntElement* ncsy = x1c_genDesc.GetObj()->x9c_NCSY.get();
        if (ncsy)
            ncsy->GetValue(x50_curFrame, ncsyVal);

        CGenDescription* ictsDesc = icts.m_gen.GetObj();
        if (!(x226 && ictsDesc->x45_31_OPTS))
        {
            x234_children.reserve(ncsyVal + x234_children.size());
            for (int i=0 ; i<ncsyVal ; ++i)
            {
                CElementGen* chGen = ConstructChildParticleSystem(icts.m_gen);
                x234_children.emplace_back(chGen);
            }
        }
    }

    SChildGeneratorDesc& iits = x1c_genDesc.GetObj()->xb8_IITS;
    if (iits.m_found && x64 != x50_curFrame && x50_curFrame < x214_PSLT &&
        x68_particleEmission == 1 && x50_curFrame >= x258_SISY &&
        ((x50_curFrame - x258_SISY) % x25c_PISY) == 0)
    {
        CGenDescription* iitsDesc = iits.m_gen.GetObj();
        if (!(x226 && iitsDesc->x45_31_OPTS))
        {
            CElementGen* chGen = ConstructChildParticleSystem(iits.m_gen);
            x234_children.emplace_back(chGen);
        }
    }

    CSpawnSystemKeyframeData* kssm = x1c_genDesc.GetObj()->xd0_KSSM.get();
    if (kssm && x64 != x50_curFrame && x50_curFrame < x214_PSLT)
    {
    }

}

void CElementGen::UpdateLightParameters()
{
}

void CElementGen::BuildParticleSystemBounds()
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
