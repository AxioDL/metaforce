#include "CElementGen.hpp"
#include "CGenDescription.hpp"
#include "CParticleGlobals.hpp"
#include "CParticleSwoosh.hpp"
#include "CParticleElectric.hpp"
#include "CModel.hpp"

#define MAX_GLOBAL_PARTICLES 2560

namespace pshag
{
static LogVisor::LogModule Log("Retro::CElementGen");

static bool s_inCreateNewParticles = false;

int CElementGen::g_ParticleAliveCount;
int CElementGen::g_ParticleSystemAliveCount;
s32 CElementGen::g_FreeIndex;
bool CElementGen::g_StaticListInitialized = false;
bool CElementGen::g_MoveRedToAlphaBuffer = false;
static rstl::reserved_vector<CElementGen::CParticle, MAX_GLOBAL_PARTICLES> g_StaticParticleList;
static rstl::reserved_vector<u16, MAX_GLOBAL_PARTICLES> g_StaticFreeList;
void CElementGen::Initialize()
{
    if (g_StaticListInitialized)
        return;
    Log.report(LogVisor::Info, "Initialize - Static Particle List - ");

    g_ParticleAliveCount = 0;
    g_ParticleSystemAliveCount = 0;

    g_StaticParticleList.clear();
    g_StaticParticleList.insert(g_StaticParticleList.end(), MAX_GLOBAL_PARTICLES, CParticle());

    g_StaticFreeList.clear();
    for (int i=0 ; i<MAX_GLOBAL_PARTICLES ; ++i)
        g_StaticFreeList.push_back(i);

    g_FreeIndex = MAX_GLOBAL_PARTICLES - 1;
    Log.report(LogVisor::Info, "size %d (%d each part).",
               (sizeof(CParticle) + sizeof(u16)) * MAX_GLOBAL_PARTICLES, 56);
    g_StaticListInitialized = true;
}

CElementGen::CElementGen(const TToken<CGenDescription>& gen) : x1c_genDesc(gen), x230_randState(x74_randomSeed) {}

CElementGen::CElementGen(const TToken<CGenDescription>& gen,
                         EModelOrientationType orientType,
                         EOptionalSystemFlags flags)
: x1c_genDesc(gen), x28_orientType(orientType),
  x226_enableOPTS((flags & EOptionalSystemFlags::Two) != EOptionalSystemFlags::None), x230_randState(x74_randomSeed)
{
    CGenDescription* desc = x1c_genDesc.GetObj();

    CIntElement* seedElem = desc->x1c_SEED.get();
    if (seedElem)
    {
        int seedVal;
        seedElem->GetValue(x50_curFrame, seedVal);
        x74_randomSeed = seedVal;
    }
    x230_randState.SetSeed(x74_randomSeed);
    ++g_ParticleSystemAliveCount;
    x224_25_LIT_ = desc->x44_29_LIT_;
    x224_26_AAPH = desc->x44_26_AAPH;
    x224_27_ZBUF = desc->x44_27_ZBUF;
    x224_29_MBLR = desc->x44_30_MBLR;

    CIntElement* mbspElem = desc->x48_MBSP.get();
    if (mbspElem)
        mbspElem->GetValue(x50_curFrame, x228_MBSP);

    x224_30_VMD1 = desc->x45_26_VMD1;
    x224_31_VMD2 = desc->x45_27_VMD2;
    x225_24_VMD3 = desc->x45_28_VMD3;
    x225_25_VMD4 = desc->x45_29_VMD4;

    CIntElement* cssdElem = desc->xa0_CSSD.get();
    if (cssdElem)
        cssdElem->GetValue(0, x244_CSSD);

    SChildGeneratorDesc& idts = desc->xa4_IDTS;
    if (idts.m_found)
    {
        int ndsyVal = 1;
        CIntElement* ndsyElem = desc->xb4_NDSY.get();
        if (ndsyElem)
            ndsyElem->GetValue(0, ndsyVal);
        x248_finishPartChildren.reserve(ndsyVal + x248_finishPartChildren.size());
        for (int i=0 ; i<ndsyVal ; ++i)
        {
            CGenDescription* chDesc = desc->xa4_IDTS.m_gen.GetObj();
            if (x226_enableOPTS && chDesc->x45_31_OPTS)
                break;
            x248_finishPartChildren.emplace_back(new CElementGen(desc->xa4_IDTS.m_gen,
                                                 EModelOrientationType::Normal,
                                                 x226_enableOPTS ? EOptionalSystemFlags::Two : EOptionalSystemFlags::One));
        }
    }

    CIntElement* pisyElem = desc->xc8_PISY.get();
    if (pisyElem)
    {
        pisyElem->GetValue(0, x25c_PISY);
        if (x25c_PISY <= 0)
            x25c_PISY = 1;
    }

    CIntElement* sisyElem = desc->xcc_SISY.get();
    if (sisyElem)
        pisyElem->GetValue(0, x258_SISY);

    CIntElement* sssdElem = desc->xe4_SSSD.get();
    if (sssdElem)
        sssdElem->GetValue(0, x270_SSSD);

    CVectorElement* sspoElem = desc->xe8_SSPO.get();
    if (sspoElem)
        sspoElem->GetValue(0, x274_SSPO);

    CIntElement* sesdElem = desc->xf8_SESD.get();
    if (sesdElem)
        sesdElem->GetValue(0, x290_SESD);

    CVectorElement* sepoElem = desc->xfc_SEPO.get();
    if (sepoElem)
        sepoElem->GetValue(0, x294_SEPO);

    CIntElement* psltElem = desc->xc_PSLT.get();
    if (psltElem)
        psltElem->GetValue(0, x214_PSLT);

    CVectorElement* psivElem = desc->x0_PSIV.get();
    if (psivElem)
        psivElem->GetValue(0, x218_PSIV);

    CIntElement* maxpElem = desc->x28_MAXP.get();
    if (maxpElem)
        maxpElem->GetValue(x50_curFrame, x70_MAXP);

    x2c_particleLists.reserve(std::min(256, x70_MAXP));
    if (x28_orientType == EModelOrientationType::One)
    {
        x3c_parentMatrices.insert(x3c_parentMatrices.end(), x70_MAXP,
                                  Zeus::CMatrix3f::skIdentityMatrix3f);
    }

    x225_26_LINE = desc->x44_24_LINE;
    x225_27_FXLL = desc->x44_25_FXLL;

    CVectorElement* pofsElem = desc->x18_POFS.get();
    if (pofsElem)
        pofsElem->GetValue(x50_curFrame, x94_POFS);

    CIntElement* ltypElem = desc->x100_LTYP.get();
    if (ltypElem)
    {
        int ltyp;
        ltypElem->GetValue(x50_curFrame, ltyp);
        switch (LightType(ltyp))
        {
        case LightType::None:
        default:
            x2dc_lightType = LightType::None;
            break;
        case LightType::Directional:
            x2dc_lightType = LightType::Directional;
            break;
        case LightType::Custom:
            x2dc_lightType = LightType::Custom;
            break;
        case LightType::Spot:
            x2dc_lightType = LightType::Spot;
            break;
        }
    }

    CIntElement* lfotElem = desc->x114_LFOT.get();
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
    CGenDescription* desc = x1c_genDesc.GetObj();
    CIntElement* pswtElem = desc->x10_PSWT.get();
    if (pswtElem && !x225_28_warmedUp)
    {
        int pswt = 0;
        pswtElem->GetValue(x50_curFrame, pswt);
        if (pswt > 32)
        {
            Log.report(LogVisor::Info,
                       "Running warmup on particle system 0x%08x for %d ticks.",
                       desc, pswt);
            InternalUpdate(pswt / 60.0);
            x225_28_warmedUp = true;
        }
    }
    InternalUpdate(t);
}

bool CElementGen::InternalUpdate(double dt)
{
    CGlobalRandom gr(x230_randState);
    CGenDescription* desc = x1c_genDesc.GetObj();

    double dt1 = 1 / 60.0;
    if (fabs(dt - 1 / 60.0) >= 1 / 60000.0)
        dt1 = dt;
    double t = x50_curFrame / 60.0;
    CParticleGlobals::SetEmitterTime(x50_curFrame);

    CRealElement* pstsElem = desc->x14_PSTS.get();
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
        CIntElement* mbspElem = desc->x48_MBSP.get();
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
        CRealElement* grteElem = desc->x2c_GRTE.get();
        if (grteElem->GetValue(x50_curFrame, grte))
        {
            x2c_particleLists.clear();
            return true;
        }

        grte = std::max(0.f, grte * x78_generatorRate);
        x6c_generatorRemainder += grte;
        int genCount = floorf(x6c_generatorRemainder);
        x6c_generatorRemainder = x6c_generatorRemainder - genCount;

        if (!x68_particleEmission || x50_curFrame >= x214_PSLT)
            genCount = 0;

        CIntElement* maxpElem = desc->x28_MAXP.get();
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

        if (x2dc_lightType != LightType::None)
            UpdateLightParameters();

        ++frameUpdateCount;
        ++x50_curFrame;
        t += 1 / 60.0;
    }

    UpdateChildParticleSystems(-(frameUpdateCount / 60.0 - dt1));
    if (fabs(t - x58_curSeconds) < 1 / 60000.0)
        x58_curSeconds = t;

    BuildParticleSystemBounds();
    x224_24_translationDirty = false;

    double passedTime = t - x58_curSeconds;
    x60_timeDeltaScale = 1.0 - passedTime * 60.0;

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
    CGenDescription* desc = x1c_genDesc.GetObj();

    x208_activeParticleCount = 0;
    CParticleGlobals::SetEmitterTime(x50_curFrame);
    for (std::vector<CParticleListItem>::iterator p = x2c_particleLists.begin();
         p != x2c_particleLists.end();)
    {
        CElementGen::CParticle& particle = g_StaticParticleList[p->x0_partIdx];
        if (particle.x0_endFrame < x50_curFrame)
        {
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

        bool err = false;
        CModVectorElement* vel1 = desc->x7c_VEL1.get();
        if (vel1)
        {
            if (x224_30_VMD1)
            {
                Zeus::CVector3f xfVel = x1a8_orientationInverse * particle.x1c_vel;
                Zeus::CVector3f xfPos = x1a8_orientationInverse * (particle.x4_pos - x7c_translation);
                err = vel1->GetValue(particleFrame, xfVel, xfPos);
                particle.x1c_vel = x178_orientation * xfVel;
                particle.x4_pos = x178_orientation * xfPos + x7c_translation;
            }
            else
            {
                err = vel1->GetValue(particleFrame, particle.x1c_vel, particle.x4_pos);
            }
        }

        CModVectorElement* vel2 = desc->x80_VEL2.get();
        if (vel2)
        {
            if (x224_31_VMD2)
            {
                Zeus::CVector3f xfVel = x1a8_orientationInverse * particle.x1c_vel;
                Zeus::CVector3f xfPos = x1a8_orientationInverse * (particle.x4_pos - x7c_translation);
                err |= vel2->GetValue(particleFrame, xfVel, xfPos);
                particle.x1c_vel = x178_orientation * xfVel;
                particle.x4_pos = x178_orientation * xfPos + x7c_translation;
            }
            else
            {
                err |= vel2->GetValue(particleFrame, particle.x1c_vel, particle.x4_pos);
            }
        }

        CModVectorElement* vel3 = desc->x84_VEL3.get();
        if (vel3)
        {
            if (x225_24_VMD3)
            {
                Zeus::CVector3f xfVel = x1a8_orientationInverse * particle.x1c_vel;
                Zeus::CVector3f xfPos = x1a8_orientationInverse * (particle.x4_pos - x7c_translation);
                err |= vel3->GetValue(particleFrame, xfVel, xfPos);
                particle.x1c_vel = x178_orientation * xfVel;
                particle.x4_pos = x178_orientation * xfPos + x7c_translation;
            }
            else
            {
                err |= vel3->GetValue(particleFrame, particle.x1c_vel, particle.x4_pos);
            }
        }

        CModVectorElement* vel4 = desc->x88_VEL4.get();
        if (vel4)
        {
            if (x225_25_VMD4)
            {
                Zeus::CVector3f xfVel = x1a8_orientationInverse * particle.x1c_vel;
                Zeus::CVector3f xfPos = x1a8_orientationInverse * (particle.x4_pos - x7c_translation);
                err |= vel4->GetValue(particleFrame, xfVel, xfPos);
                particle.x1c_vel = x178_orientation * xfVel;
                particle.x4_pos = x178_orientation * xfPos + x7c_translation;
            }
            else
            {
                err |= vel4->GetValue(particleFrame, particle.x1c_vel, particle.x4_pos);
            }
        }

        if (x225_26_LINE)
        {
            CRealElement* leng = desc->x20_LENG.get();
            if (leng)
                err |= leng->GetValue(particleFrame, particle.x2c_lineLengthOrSize);
            CRealElement* widt = desc->x24_WIDT.get();
            if (widt)
                err |= widt->GetValue(particleFrame, particle.x30_lineWidthOrRota);
        }
        else
        {
            CRealElement* rota = desc->x50_ROTA.get();
            if (rota)
                err |= rota->GetValue(particleFrame, particle.x30_lineWidthOrRota);
            CRealElement* size = desc->x4c_SIZE.get();
            if (size)
                err |= size->GetValue(particleFrame, particle.x2c_lineLengthOrSize);
        }

        CColorElement* colr = desc->x30_COLR.get();
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
    CGenDescription* desc = x1c_genDesc.GetObj();

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
        x2c_particleLists.emplace_back(staticIdx);
        ++x208_activeParticleCount;
        if (x28_orientType == EModelOrientationType::One)
            x3c_parentMatrices[x2c_particleLists.size()-1] = x178_orientation.buildMatrix3f();

        CElementGen::CParticle& particle = g_StaticParticleList[staticIdx];
        particle.x28_startFrame = x50_curFrame;
        CIntElement* ltme = desc->x34_LTME.get();
        if (ltme)
            ltme->GetValue(0, particle.x0_endFrame);
        CParticleGlobals::SetParticleLifetime(particle.x0_endFrame);
        CParticleGlobals::UpdateParticleLifetimeTweenValues(0);
        particle.x0_endFrame += x50_curFrame;

        CColorElement* colr = desc->x30_COLR.get();
        if (colr)
            colr->GetValue(0, particle.x34_color);
        else
            particle.x34_color = Zeus::CColor::skWhite;

        CEmitterElement* emtr = desc->x40_EMTR.get();
        if (emtr)
        {
            emtr->GetValue(x210_curEmitterFrame, particle.x4_pos, particle.x1c_vel);
            Zeus::CVector3f compXf1 = (xdc_globalScaleTransformInverse * x148_localScaleTransformInverse) * x7c_translation;
            Zeus::CVector3f compXf2 = x178_orientation * particle.x4_pos;
            particle.x4_pos = compXf1 + compXf2 + x94_POFS;
            particle.x1c_vel = x178_orientation * particle.x1c_vel;
        }
        else
        {
            Zeus::CVector3f compXf1 = (xdc_globalScaleTransformInverse * x148_localScaleTransformInverse) * x7c_translation;
            particle.x4_pos = compXf1 + x94_POFS;
            particle.x1c_vel.zeroOut();
        }
        particle.x10_prevPos = particle.x4_pos;

        if (x225_26_LINE)
        {
            CRealElement* leng = desc->x20_LENG.get();
            if (leng)
                leng->GetValue(0, particle.x2c_lineLengthOrSize);
            else
                particle.x2c_lineLengthOrSize = 1.f;

            CRealElement* widt = desc->x24_WIDT.get();
            if (widt)
                widt->GetValue(0, particle.x30_lineWidthOrRota);
            else
                particle.x30_lineWidthOrRota = 1.f;
        }
        else
        {
            CRealElement* rota = desc->x50_ROTA.get();
            if (rota)
                rota->GetValue(0, particle.x30_lineWidthOrRota);
            else
                particle.x30_lineWidthOrRota = 0.f;

            CRealElement* size = desc->x4c_SIZE.get();
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
    CGenDescription* desc = x1c_genDesc.GetObj();

    CGlobalRandom gr(x230_randState);
    if (x214_PSLT < x50_curFrame)
        return;

    CModVectorElement* psvm = desc->x4_PSVM.get();
    if (psvm)
    {
        Zeus::CVector3f vel = x7c_translation;
        psvm->GetValue(x50_curFrame, x218_PSIV, vel);
        if (vel != x7c_translation)
        {
            x224_24_translationDirty = true;
            x7c_translation = vel;
        }
    }

    Zeus::CVector3f v = x178_orientation * x218_PSIV;
    if (v != Zeus::CVector3f::skZero)
        x224_24_translationDirty = true;
    x7c_translation += v;

    CVectorElement* psov = desc->x8_PSOV.get();
    if (psov)
    {
        Zeus::CVector3f angles;
        psov->GetValue(x50_curFrame, angles);
        Zeus::CTransform xf(x178_orientation);
        xf.rotateLocalX(angles[0] * M_PI / 180.f);
        xf.rotateLocalY(angles[1] * M_PI / 180.f);
        xf.rotateLocalZ(angles[2] * M_PI / 180.f);
        SetOrientation(xf);
    }

    CVectorElement* pofs = desc->x18_POFS.get();
    if (pofs)
        pofs->GetValue(x50_curFrame, x94_POFS);

    CVectorElement* sspo = desc->xe8_SSPO.get();
    if (sspo)
        sspo->GetValue(x50_curFrame, x274_SSPO);

    CVectorElement* sepo = desc->xfc_SEPO.get();
    if (sepo)
        sspo->GetValue(x50_curFrame, x294_SEPO);
}

CElementGen* CElementGen::ConstructChildParticleSystem(const TToken<CGenDescription>& desc)
{
    CElementGen* ret = new CElementGen(desc, EModelOrientationType::Normal,
                                       x226_enableOPTS ? EOptionalSystemFlags::Two : EOptionalSystemFlags::One);
    ret->SetGlobalTranslation(x88_globalTranslation);
    ret->SetGlobalOrientation(x1d8_globalOrientation);
    ret->SetGlobalScale(xa0_globalScale);
    ret->SetLocalScale(x10c_localScale);
    ret->SetTranslation(x7c_translation);
    ret->SetOrientation(x178_orientation);
    ret->SetParticleEmission(x68_particleEmission);
    ret->SetModulationColor(x30c_moduColor);
    return ret;
}

void CElementGen::UpdateChildParticleSystems(double dt)
{
    CGenDescription* desc = x1c_genDesc.GetObj();

    CGlobalRandom gr(x230_randState);
    SChildGeneratorDesc& icts = desc->x8c_ICTS;
    if (icts.m_found && x64_prevFrame != x50_curFrame && x244_CSSD == x50_curFrame)
    {
        int ncsyVal = 1;
        CIntElement* ncsy = desc->x9c_NCSY.get();
        if (ncsy)
            ncsy->GetValue(x50_curFrame, ncsyVal);

        CGenDescription* ictsDesc = icts.m_gen.GetObj();
        if (!(x226_enableOPTS && ictsDesc->x45_31_OPTS))
        {
            x234_activePartChildren.reserve(ncsyVal + x234_activePartChildren.size());
            for (int i=0 ; i<ncsyVal ; ++i)
            {
                CElementGen* chGen = ConstructChildParticleSystem(icts.m_gen);
                x234_activePartChildren.emplace_back(chGen);
            }
        }
    }

    SChildGeneratorDesc& iits = desc->xb8_IITS;
    if (iits.m_found && x64_prevFrame != x50_curFrame && x50_curFrame < x214_PSLT &&
        x68_particleEmission == 1 && x50_curFrame >= x258_SISY &&
        ((x50_curFrame - x258_SISY) % x25c_PISY) == 0)
    {
        CGenDescription* iitsDesc = iits.m_gen.GetObj();
        if (!(x226_enableOPTS && iitsDesc->x45_31_OPTS))
        {
            CElementGen* chGen = ConstructChildParticleSystem(iits.m_gen);
            x234_activePartChildren.emplace_back(chGen);
        }
    }

    CSpawnSystemKeyframeData* kssm = desc->xd0_KSSM.get();
    if (kssm && x64_prevFrame != x50_curFrame && x50_curFrame < x214_PSLT)
    {
        std::vector<CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo>& systems =
        kssm->GetSpawnedSystemsAtFrame(x50_curFrame);
        x234_activePartChildren.reserve(x234_activePartChildren.size() + systems.size());
        for (CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo& system : systems)
        {
            TLockedToken<CGenDescription>& token = system.GetToken();
            if (!(x226_enableOPTS && token.GetObj()->x45_31_OPTS))
            {
                CElementGen* chGen = ConstructChildParticleSystem(token);
                x234_activePartChildren.emplace_back(chGen);
            }
        }
    }

    SSwooshGeneratorDesc& sswh = desc->xd4_SSWH;
    if (sswh.m_found && x64_prevFrame != x50_curFrame && x50_curFrame == x270_SSSD)
    {
        CParticleSwoosh* sswhGen = new CParticleSwoosh(sswh.m_swoosh, 0);
        sswhGen->SetGlobalTranslation(x88_globalTranslation);
        sswhGen->SetGlobalScale(xa0_globalScale);
        sswhGen->SetTranslation(x7c_translation);
        sswhGen->SetOrientation(x178_orientation);
        sswhGen->SetParticleEmission(x68_particleEmission);
        x260_swhcChildren.emplace_back(sswhGen);
    }

    SElectricGeneratorDesc& selc = desc->xec_SELC;
    if (selc.m_found && x64_prevFrame != x50_curFrame && x50_curFrame == x290_SESD)
    {
        CParticleElectric* selcGen = new CParticleElectric(selc.m_electric);
        selcGen->SetGlobalTranslation(x88_globalTranslation);
        selcGen->SetGlobalScale(xa0_globalScale);
        selcGen->SetTranslation(x7c_translation);
        selcGen->SetOrientation(x178_orientation);
        selcGen->SetParticleEmission(x68_particleEmission);
        x280_elscChildren.emplace_back(selcGen);
    }

    for (auto p = x234_activePartChildren.begin() ; p != x234_activePartChildren.end() ;)
    {
        std::unique_ptr<CElementGen>& ch = *p;

        if ((x50_curFrame == x4c_internalStartFrame || x224_24_translationDirty) && x64_prevFrame != x50_curFrame)
        {
            ch->SetTranslation(x7c_translation);
            ch->SetOrientation(x178_orientation);
        }

        ch->Update(dt);
        if (ch->IsSystemDeletable())
        {
            p = x234_activePartChildren.erase(p);
            continue;
        }

        ++p;
    }

    for (auto p = x248_finishPartChildren.begin() ; p != x248_finishPartChildren.end() ;)
    {
        std::unique_ptr<CElementGen>& ch = *p;

        if (x214_PSLT <= x50_curFrame)
        {
            if (x214_PSLT == x50_curFrame && x64_prevFrame != x50_curFrame)
            {
                ch->SetTranslation(x7c_translation);
                ch->SetOrientation(x178_orientation);
            }
            ch->Update(dt);
        }

        if (ch->IsSystemDeletable())
        {
            p = x248_finishPartChildren.erase(p);
            continue;
        }

        ++p;
    }

    for (auto p = x260_swhcChildren.begin() ; p != x260_swhcChildren.end() ;)
    {
        std::unique_ptr<CParticleSwoosh>& ch = *p;

        if ((x50_curFrame == x270_SSSD || x224_24_translationDirty) && x64_prevFrame != x50_curFrame)
        {
            Zeus::CVector3f trans = x7c_translation + x274_SSPO;
            ch->SetTranslation(trans);
            ch->SetOrientation(x178_orientation);
        }

        ch->Update(dt);
        if (ch->IsSystemDeletable())
        {
            p = x260_swhcChildren.erase(p);
            continue;
        }

        ++p;
    }

    for (auto p = x280_elscChildren.begin() ; p != x280_elscChildren.end() ;)
    {
        std::unique_ptr<CParticleElectric>& ch = *p;

        if ((x50_curFrame == x290_SESD || x224_24_translationDirty) && x64_prevFrame != x50_curFrame)
        {
            Zeus::CVector3f trans = x7c_translation + x294_SEPO;
            ch->SetTranslation(trans);
            ch->SetOrientation(x178_orientation);
        }

        ch->Update(dt);
        if (ch->IsSystemDeletable())
        {
            p = x280_elscChildren.erase(p);
            continue;
        }

        ++p;
    }

    x64_prevFrame = x50_curFrame;
}

void CElementGen::UpdateLightParameters()
{
    CGenDescription* desc = x1c_genDesc.GetObj();

    CColorElement* lclr = desc->x104_LCLR.get();
    if (lclr)
        lclr->GetValue(x50_curFrame, x2e0_LCLR);

    CRealElement* lint = desc->x108_LINT.get();
    if (lint)
        lint->GetValue(x50_curFrame, x2e4_LINT);

    switch (x2dc_lightType)
    {
    default:
    case LightType::None:
    case LightType::Custom:
    case LightType::Spot:
    {
        CVectorElement* loff = desc->x10c_LOFF.get();
        if (loff)
            loff->GetValue(x50_curFrame, x2e8_LOFF);

        CRealElement* lfor = desc->x118_LFOR.get();
        if (lfor)
            lfor->GetValue(x50_curFrame, x304_LFOR);

        if (x2dc_lightType == LightType::Spot)
        {
            CRealElement* lsla = desc->x11c_LSLA.get();
            if (lsla)
                lsla->GetValue(x50_curFrame, x308_LSLA);
        }
    }
    case LightType::Directional:
    {
        if (x2dc_lightType != LightType::Custom)
        {
            CVectorElement* ldir = desc->x110_LDIR.get();
            if (ldir)
                ldir->GetValue(x50_curFrame, x2f4_LDIR);
        }
    }
    }
}

u32 CElementGen::GetParticleCountAllInternal() const
{
    u32 ret = x208_activeParticleCount;

    for (const std::unique_ptr<CElementGen>& ch : x234_activePartChildren)
        ret += ch->GetParticleCountAll();

    for (const std::unique_ptr<CElementGen>& ch : x248_finishPartChildren)
        ret += ch->GetParticleCountAll();

    return ret;
}

void CElementGen::BuildParticleSystemBounds()
{
    Zeus::CAABox aabb;
    bool accumulated = false;

    for (std::unique_ptr<CElementGen>& ch : x234_activePartChildren)
    {
        std::pair<Zeus::CAABox, bool> chBounds = ch->GetBounds();
        if (chBounds.second)
        {
            accumulated = true;
            aabb.accumulateBounds(chBounds.first);
        }
    }

    for (std::unique_ptr<CElementGen>& ch : x248_finishPartChildren)
    {
        std::pair<Zeus::CAABox, bool> chBounds = ch->GetBounds();
        if (chBounds.second)
        {
            accumulated = true;
            aabb.accumulateBounds(chBounds.first);
        }
    }

    for (std::unique_ptr<CParticleSwoosh>& ch : x260_swhcChildren)
    {
        std::pair<Zeus::CAABox, bool> chBounds = ch->GetBounds();
        if (chBounds.second)
        {
            accumulated = true;
            aabb.accumulateBounds(chBounds.first);
        }
    }

    for (std::unique_ptr<CParticleElectric>& ch : x280_elscChildren)
    {
        std::pair<Zeus::CAABox, bool> chBounds = ch->GetBounds();
        if (chBounds.second)
        {
            accumulated = true;
            aabb.accumulateBounds(chBounds.first);
        }
    }

    x20c_recursiveParticleCount = GetParticleCountAllInternal();
    if (GetParticleCount())
    {
        Zeus::CVector3f scale = xa0_globalScale * x2c0_maxSize;
        Zeus::CTransform xf = (xac_globalScaleTransform * x1d8_globalOrientation) * x118_localScaleTransform;
        Zeus::CAABox box = Zeus::CAABox(x2a8_aabbMin, x2b4_aabbMax).getTransformedAABox(xf);
        Zeus::CVector3f min = box.m_min + x88_globalTranslation - scale;
        Zeus::CVector3f max = box.m_max + x88_globalTranslation + scale;
        x2c4_systemBounds = Zeus::CAABox(min, max);
    }
    else
        x2c4_systemBounds = Zeus::CAABox::skInvertedBox;

    if (accumulated)
        x2c4_systemBounds.accumulateBounds(aabb);
}

u32 CElementGen::GetSystemCount()
{
    u32 ret = 0;
    for (const std::unique_ptr<CElementGen>& child : x234_activePartChildren)
        ret += child->GetSystemCount();

    for (const std::unique_ptr<CElementGen>& child : x248_finishPartChildren)
        ret += child->GetSystemCount();

    return (ret + (x208_activeParticleCount != 0));
}

void CElementGen::Render()
{
    CGenDescription* desc = x1c_genDesc.GetObj();

    x22c_backupLightActive = CGraphics::g_LightActive;
    CGraphics::DisableAllLights();

    for (std::unique_ptr<CElementGen>& child : x234_activePartChildren)
        child->Render();

    if (x214_PSLT <= x50_curFrame)
        for (std::unique_ptr<CElementGen>& child : x248_finishPartChildren)
            child->Render();

    for (std::unique_ptr<CParticleSwoosh>& child : x260_swhcChildren)
        child->Render();

    for (std::unique_ptr<CParticleElectric>& child : x280_elscChildren)
        child->Render();

    if (x2c_particleLists.size())
    {
        SParticleModel& pmdl = desc->x5c_PMDL;
        if (pmdl.m_found || desc->x45_24_PMUS)
            RenderModels();

        if (x225_26_LINE)
            RenderLines();
        else
            RenderParticles();
    }
}

void CElementGen::RenderModels()
{
    CGenDescription* desc = x1c_genDesc.GetObj();

    if (x225_29_modelsUseLights)
        CGraphics::SetLightState(x22c_backupLightActive);
    CGlobalRandom gr(x230_randState);

    SUVElementSet uvs = {0.f, 0.f, 1.f, 1.f};
    CUVElement* texr = desc->x54_TEXR.get();
    CTexture* cachedTex = nullptr;
    bool texConst = true;
    bool moveRedToAlphaBuffer = false;

    if (desc->x45_24_PMUS)
    {
        if (g_MoveRedToAlphaBuffer && desc->x44_31_PMAB && desc->x54_TEXR)
            moveRedToAlphaBuffer = true;

        if (desc->x44_31_PMAB)
        {
            CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, false);
            if (moveRedToAlphaBuffer)
                CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One, ERglBlendFactor::One, ERglLogicOp::Clear);
            else
                CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One, ERglLogicOp::Clear);
        }
        else
        {
            CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, true);
            CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha, ERglLogicOp::Clear);
        }

        CGraphics::SetCullMode(ERglCullMode::None);

        if (texr)
        {
            CParticle& target = g_StaticParticleList[x2c_particleLists[0].x0_partIdx];
            int partFrame = x50_curFrame - target.x28_startFrame;
            cachedTex = texr->GetValueTexture(partFrame).GetObj();
            cachedTex->Load(0, CTexture::EClampMode::One);
            /* Shade as TEXC * RASC and TEXA * RASA */
            if (moveRedToAlphaBuffer)
            {
                /* Color = Prev.rgb * Prev.a */
                /* Alpha = Tex.r * Prev.a */
            }
            texConst = texr->HasConstantTexture();
            texr->GetValueUV(partFrame, uvs);
        }
    }

    Zeus::CTransform orient = Zeus::CTransform::Identity();
    if (desc->x45_25_PMOO)
        orient = x178_orientation;
    orient = orient * x1d8_globalOrientation;

    CVectorElement* pmrt = desc->x70_PMRT.get();
    bool pmrtConst = false;
    if (pmrt)
        pmrtConst = pmrt->IsFastConstant();

    Zeus::CVector3f trans = (xdc_globalScaleTransformInverse * x148_localScaleTransformInverse) * x88_globalTranslation;

    Zeus::CTransform rot = Zeus::CTransform::Identity();
    if (pmrtConst)
    {
        Zeus::CVector3f pmrtVal;
        pmrt->GetValue(x50_curFrame, pmrtVal);
        rot = Zeus::CTransform::RotateZ(pmrtVal[2] * M_PI / 180.f);
        rot.rotateLocalY(pmrtVal[1] * M_PI / 180.f);
        rot.rotateLocalX(pmrtVal[0] * M_PI / 180.f);
    }
    rot = orient * rot;

    CParticleGlobals::SetEmitterTime(x50_curFrame);
    Zeus::CColor col = {1.f, 1.f, 1.f, 1.f};

    Zeus::CVector3f pmopVec;
    auto matrixIt = x3c_parentMatrices.begin();
    for (CParticleListItem& item : x2c_particleLists)
    {
        CParticle& particle = g_StaticParticleList[item.x0_partIdx];
        if (particle.x0_endFrame == -1)
        {
            ++matrixIt;
            continue;
        }
        CParticleGlobals::SetParticleLifetime(particle.x0_endFrame - particle.x28_startFrame);
        int partFrame = x50_curFrame - particle.x28_startFrame - 1;
        CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
        CVectorElement* pmop = desc->x6c_PMOP.get();
        if (pmop)
            pmop->GetValue(partFrame, pmopVec);

        Zeus::CTransform partTrans = Zeus::CTransform::Translate(particle.x4_pos + trans);
        if (x28_orientType == EModelOrientationType::One)
        {
            Zeus::CTransform partRot(*matrixIt);
            Zeus::CVector3f pmopRotateOffset = (orient * partRot) * pmopVec;
            partTrans = partTrans * partRot;
            partTrans += pmopRotateOffset;
        }
        else
        {
            partTrans += orient * pmopVec;
        }

        if (pmrtConst)
        {
            partTrans = partTrans * rot;
        }
        else
        {
            if (pmrt)
            {
                Zeus::CVector3f pmrtVal;
                pmrt->GetValue(partFrame, pmrtVal);
                rot = Zeus::CTransform::RotateZ(pmrtVal[2] * M_PI / 180.f);
                rot.rotateLocalY(pmrtVal[1] * M_PI / 180.f);
                rot.rotateLocalX(pmrtVal[0] * M_PI / 180.f);
                partTrans = partTrans * (orient * rot);
            }
            else
            {
                partTrans = partTrans * rot;
            }
        }

        CVectorElement* pmsc = desc->x74_PMSC.get();
        if (pmsc)
        {
            Zeus::CVector3f pmscVal;
            pmsc->GetValue(partFrame, pmscVal);
            partTrans = partTrans * Zeus::CTransform::Scale(pmscVal);
        }

        CColorElement* pmcl = desc->x78_PMCL.get();
        if (pmcl)
            pmcl->GetValue(partFrame, col);

        CGraphics::SetModelMatrix((xac_globalScaleTransform * partTrans) * x118_localScaleTransform);

        if (desc->x45_24_PMUS)
        {
            if (!texConst)
            {
                CTexture* tex = texr->GetValueTexture(x50_curFrame - particle.x28_startFrame).GetObj();
                if (tex != cachedTex)
                {
                    tex->Load(0, CTexture::EClampMode::One);
                    cachedTex = tex;
                }
            }

            /* Draw: */
            /* Pos: {0.5, 0.0, 0.5}  Color: <col-variable>  UV0: {uv[2], uv[3]} */
            /* Pos: {-0.5, 0.0, 0.5}  Color: <col-variable>  UV0: {uv[0], uv[3]} */
            /* Pos: {-0.5, 0.0, -0.5}  Color: <col-variable>  UV0: {uv[0], uv[1]} */
            /* Pos: {0.5, 0.0, -0.5}  Color: <col-variable>  UV0: {uv[2], uv[1]} */
        }
        else
        {
            CModel* model = desc->x5c_PMDL.m_model.GetObj();
            if (desc->x44_31_PMAB)
            {
                model->Draw({3, 0, 1, col});
            }
            else
            {
                if (1.f == col.a)
                    model->Draw({0, 0, 3, Zeus::CColor::skWhite});
                else
                    model->Draw({4, 0, 1, col});
            }
        }

        ++matrixIt;
    }

    if (x225_29_modelsUseLights)
        CGraphics::DisableAllLights();

    CGraphics::SetCullMode(ERglCullMode::Front);
    if (moveRedToAlphaBuffer)
    {
        /* Restore passthrough */
    }
}

void CElementGen::RenderLines()
{    
}

void CElementGen::RenderParticles()
{
    CGenDescription* desc = x1c_genDesc.GetObj();
    CGlobalRandom gr(x230_randState);

    CUVElement* texr = desc->x54_TEXR.get();
    CUVElement* tind = desc->x58_TIND.get();
    if (texr && tind)
    {
        RenderParticlesIndirectTexture();
        return;
    }

    CRealElement* size = desc->x4c_SIZE.get();
    if (size && size->IsConstant())
    {
        float sizeVal;
        size->GetValue(0, sizeVal);
        if (sizeVal == 0.f)
        {
            size->GetValue(1, sizeVal);
            if (sizeVal == 0.f)
                return;
        }
    }

    Zeus::CTransform systemViewPointMatrix(CGraphics::g_ViewMatrix);
    systemViewPointMatrix.m_origin.zeroOut();
    Zeus::CTransform systemCameraMatrix = systemViewPointMatrix.inverse() * x1d8_globalOrientation;
    systemViewPointMatrix = ((Zeus::CTransform::Translate(x88_globalTranslation) * xac_globalScaleTransform) * systemViewPointMatrix) * x118_localScaleTransform;
    CGraphics::SetModelMatrix(systemViewPointMatrix);

    CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);

    SUVElementSet uvs = {0.f, 0.f, 1.f, 1.f};
    bool constTexr = true;
    bool constUVs = true;
    CTexture* cachedTex = nullptr;

    if (texr)
    {
        CParticle& target = g_StaticParticleList[x2c_particleLists[0].x0_partIdx];
        int partFrame = x50_curFrame - target.x28_startFrame;
        cachedTex = texr->GetValueTexture(partFrame).GetObj();
        cachedTex->Load(0, CTexture::EClampMode::One);

        if (x30c_moduColor != Zeus::CColor::skBlack)
        {
            /* Add RASC * PREVC pass for MODU color loaded into channel mat-color */
        }
        else
        {
            /* Pass-thru */
        }

        constTexr = texr->HasConstantTexture();
        texr->GetValueUV(partFrame, uvs);
        constUVs = texr->HasConstantUV();
    }
    else
    {
        /* Pass-thru */
    }

    if (desc->x44_28_SORT)
    {
        for (CParticleListItem& item : x2c_particleLists)
        {
            CParticle& particle = g_StaticParticleList[item.x0_partIdx];
            item.x4_viewPoint = systemCameraMatrix * ((particle.x4_pos - particle.x10_prevPos) * x60_timeDeltaScale + particle.x10_prevPos);
        }

        std::sort(x2c_particleLists.begin(), x2c_particleLists.end(),
                  [](const CParticleListItem& a, const CParticleListItem& b) -> bool
                  {return a.x4_viewPoint[1] >= b.x4_viewPoint[1];});

#if 0
        bool done = false;
        while (!done)
        {
            done = true;
            for (int i=0 ; i<x2c_particleLists.size()-1 ; ++i)
            {
                CParticleListItem& p1 = x2c_particleLists[i];
                CParticleListItem& p2 = x2c_particleLists[i+1];
                if (p1.x4_viewPoint[1] < p2.x4_viewPoint[1])
                {
                    CParticleListItem tmp = p2;
                    p2 = p1;
                    p1 = tmp;
                    done = false;
                }
            }
        }
#endif
    }

    if (x30c_moduColor != Zeus::CColor::skBlack)
    {
        /* Load mat-color here */
    }
    else
    {
        /* Pass-thru */
    }

    bool moveRedToAlphaBuffer = false;
    if (g_MoveRedToAlphaBuffer && x224_26_AAPH)
        moveRedToAlphaBuffer = true;

    if (moveRedToAlphaBuffer)
    {
        CGraphics::SetDepthWriteMode(x224_28_zTest, ERglEnum::LEqual, false);
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One, ERglBlendFactor::One, ERglLogicOp::Clear);
        /* Color = Prev.rgb * Prev.a */
        /* Alpha = Tex.r * Prev.a */
    }
    else
    {
        if (x224_26_AAPH)
        {
            CGraphics::SetDepthWriteMode(x224_28_zTest, ERglEnum::LEqual, false);
            CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One, ERglLogicOp::Clear);
        }
        else
        {
            CGraphics::SetDepthWriteMode(x224_28_zTest, ERglEnum::LEqual, x224_27_ZBUF);
            CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha, ERglLogicOp::Clear);
        }
    }

    int mbspVal = std::max(1, x228_MBSP);
    if (!x224_29_MBLR)
    {
        /* Begin quad draw of x2c_particleLists.size() * 4 verts */
    }
    else
    {
        /* Begin quad draw of x2c_particleLists.size() * 4 * mbspVal verts */
    }

    CParticleGlobals::SetEmitterTime(x50_curFrame);
    if (!x224_29_MBLR)
    {
        for (CParticleListItem& item : x2c_particleLists)
        {
            CParticle& particle = g_StaticParticleList[item.x0_partIdx];
            int partFrame = x50_curFrame - particle.x28_startFrame - 1;
            Zeus::CVector3f viewPoint;
            if (desc->x44_28_SORT)
                viewPoint = item.x4_viewPoint;
            else
                viewPoint = systemCameraMatrix * ((particle.x4_pos - particle.x10_prevPos) * x60_timeDeltaScale + particle.x10_prevPos);

            if (!constTexr)
            {
                CTexture* tex = texr->GetValueTexture(partFrame).GetObj();
                if (tex != cachedTex)
                {
                    tex->Load(0, CTexture::EClampMode::One);
                    cachedTex = tex;
                }
            }

            if (!constUVs)
            {
                CParticleGlobals::SetParticleLifetime(particle.x0_endFrame - particle.x28_startFrame);
                CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
                texr->GetValueUV(partFrame, uvs);
            }

            float size = 0.5f * particle.x2c_lineLengthOrSize;
            if (0.f == particle.x30_lineWidthOrRota)
            {
                /* Draw: */
                /* Pos: {viewPoint.x + size, 0.0, viewPoint.z + size}  Color: particle.color  UV0: {uv[2], uv[3]} */
                /* Pos: {viewPoint.x - size, 0.0, viewPoint.z + size}  Color: particle.color  UV0: {uv[0], uv[3]} */
                /* Pos: {viewPoint.x - size, 0.0, viewPoint.z - size}  Color: particle.color  UV0: {uv[0], uv[1]} */
                /* Pos: {viewPoint.x + size, 0.0, viewPoint.z - size}  Color: particle.color  UV0: {uv[2], uv[1]} */
            }
            else
            {
                float theta = particle.x30_lineWidthOrRota * M_PI / 180.f;
                float sinT = sinf(theta) * size;
                float cosT = sinf(theta) * size;

                /* Draw:
                viewPoint.x + sinT + cosT;
                0.f;
                viewPoint.z + cosT - sinT;

                viewPoint.x + sinT - cosT;
                0.f;
                viewPoint.z + sinT + cosT;

                viewPoint.x - (sinT + cosT);
                0.f;
                viewPoint.z - (cosT - sinT);

                viewPoint.x + (cosT - sinT);
                0.f;
                viewPoint.z + (-cosT - sinT);
                */
            }
        }
    }
    else
    {
        float mbspFac = 1.f / float(mbspVal);
        for (CParticleListItem& item : x2c_particleLists)
        {
            CParticle& particle = g_StaticParticleList[item.x0_partIdx];
            int partFrame = x50_curFrame - particle.x28_startFrame - 1;

            if (!constTexr)
            {
                CTexture* tex = texr->GetValueTexture(partFrame).GetObj();
                if (tex != cachedTex)
                {
                    tex->Load(0, CTexture::EClampMode::One);
                    cachedTex = tex;
                }
            }

            if (!constUVs)
            {
                CParticleGlobals::SetParticleLifetime(particle.x0_endFrame - particle.x28_startFrame);
                CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
                texr->GetValueUV(partFrame, uvs);
            }

            Zeus::CVector3f dVec = particle.x4_pos - particle.x10_prevPos;
            Zeus::CVector3f vec = dVec * x60_timeDeltaScale + particle.x10_prevPos;
            Zeus::CVector3f mbspVec = dVec * mbspFac;
            float size = 0.5f * particle.x2c_lineLengthOrSize;
            if (0.f == particle.x30_lineWidthOrRota)
            {
                for (int i=0 ; i<mbspVal ; ++i)
                {
                    vec += mbspVec;
                    Zeus::CVector3f vec2 = systemCameraMatrix * vec;
                    /* Draw: */
                    /* Pos: {vec2.x + size, vec2.y, vec2.z + size}  Color: particle.color  UV0: {uv[2], uv[3]} */
                    /* Pos: {vec2.x - size, vec2.y, vec2.z + size}  Color: particle.color  UV0: {uv[0], uv[3]} */
                    /* Pos: {vec2.x - size, vec2.y, vec2.z - size}  Color: particle.color  UV0: {uv[0], uv[1]} */
                    /* Pos: {vec2.x + size, vec2.y, vec2.z - size}  Color: particle.color  UV0: {uv[2], uv[1]} */
                }
            }
            else
            {
                float theta = particle.x30_lineWidthOrRota * M_PI / 180.f;
                float sinT = sinf(theta) * size;
                float cosT = sinf(theta) * size;

                for (int i=0 ; i<mbspVal ; ++i)
                {
                    vec += mbspVec;
                    Zeus::CVector3f vec2 = systemCameraMatrix * vec;
                    /* Draw:
                    vec2.x + sinT + cosT;
                    vec2.y;
                    vec2.z + cosT - sinT;

                    vec2.x + sinT - cosT;
                    vec2.y;
                    vec2.z + sinT + cosT;

                    vec2.x - (sinT + cosT);
                    vec2.y;
                    vec2.z - (cosT - sinT);

                    vec2.x + (cosT - sinT);
                    vec2.y;
                    vec2.z + (-cosT - sinT);
                    */
                }
            }
        }
    }

    if (moveRedToAlphaBuffer)
    {
        /* Restore */
    }
}

void CElementGen::RenderParticlesIndirectTexture()
{
    CGenDescription* desc = x1c_genDesc.GetObj();

    Zeus::CTransform systemViewPointMatrix(CGraphics::g_ViewMatrix);
    systemViewPointMatrix.m_origin.zeroOut();
    Zeus::CTransform systemCameraMatrix = systemViewPointMatrix.inverse() * x1d8_globalOrientation;
    systemViewPointMatrix = ((Zeus::CTransform::Translate(x88_globalTranslation) * xac_globalScaleTransform) * systemViewPointMatrix) * x118_localScaleTransform;
    CGraphics::SetModelMatrix(systemViewPointMatrix);

    CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);

    if (x224_26_AAPH)
    {
        CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, true);
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One, ERglLogicOp::Clear);
    }
    else
    {
        CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, x224_27_ZBUF);
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha, ERglLogicOp::Clear);
    }

    CUVElement* texr = desc->x54_TEXR.get();
    CParticle& firstParticle = g_StaticParticleList[x2c_particleLists[0].x0_partIdx];
    int partFrame = x50_curFrame - firstParticle.x28_startFrame;
    CTexture* cachedTex = texr->GetValueTexture(partFrame).GetObj();
    cachedTex->Load(0, CTexture::EClampMode::One);

    SUVElementSet uvs = {0.f, 0.f, 1.f, 1.f};
    bool constTexr = texr->HasConstantTexture();
    texr->GetValueUV(partFrame, uvs);
    bool constUVs = texr->HasConstantUV();

    CUVElement* tind = desc->x58_TIND.get();
    CTexture* cachedIndTex = tind->GetValueTexture(partFrame).GetObj();
    cachedIndTex->Load(2, CTexture::EClampMode::One);

    SUVElementSet uvsInd = {0.f, 0.f, 1.f, 1.f};
    bool constIndTexr = tind->HasConstantTexture();
    bool constIndUVs = tind->HasConstantUV();
    tind->GetValueUV(partFrame, uvsInd);

}

void CElementGen::SetOrientation(const Zeus::CTransform& orientation)
{
    x178_orientation = orientation;
    x1a8_orientationInverse = x178_orientation.inverse();

    for (const std::unique_ptr<CElementGen>& ch : x234_activePartChildren)
        ch->SetOrientation(orientation);

    for (const std::unique_ptr<CElementGen>& ch : x248_finishPartChildren)
        ch->SetOrientation(orientation);

    for (const std::unique_ptr<CParticleSwoosh>& ch : x260_swhcChildren)
        ch->SetOrientation(orientation);

    for (const std::unique_ptr<CParticleElectric>& ch : x280_elscChildren)
        ch->SetOrientation(orientation);
}

void CElementGen::SetTranslation(const Zeus::CVector3f& translation)
{
    x7c_translation = translation;

    for (const std::unique_ptr<CElementGen>& ch : x234_activePartChildren)
        ch->SetTranslation(translation);

    for (const std::unique_ptr<CElementGen>& ch : x248_finishPartChildren)
        ch->SetTranslation(translation);

    for (const std::unique_ptr<CParticleSwoosh>& ch : x260_swhcChildren)
        ch->SetTranslation(translation + x274_SSPO);

    for (const std::unique_ptr<CParticleElectric>& ch : x280_elscChildren)
        ch->SetTranslation(translation + x294_SEPO);
}

void CElementGen::SetGlobalOrientation(const Zeus::CTransform& rotation)
{
    x1d8_globalOrientation.setRotation(rotation);

    for (const std::unique_ptr<CElementGen>& ch : x234_activePartChildren)
        ch->SetGlobalOrientation(x1d8_globalOrientation);

    for (const std::unique_ptr<CElementGen>& ch : x248_finishPartChildren)
        ch->SetGlobalOrientation(x1d8_globalOrientation);

    for (const std::unique_ptr<CParticleElectric>& ch : x280_elscChildren)
        ch->SetGlobalOrientation(x1d8_globalOrientation);
}

void CElementGen::SetGlobalTranslation(const Zeus::CVector3f& translation)
{
    x88_globalTranslation = translation;

    for (const std::unique_ptr<CElementGen>& ch : x234_activePartChildren)
        ch->SetGlobalTranslation(translation);

    for (const std::unique_ptr<CElementGen>& ch : x248_finishPartChildren)
        ch->SetGlobalTranslation(translation);

    for (const std::unique_ptr<CParticleSwoosh>& ch : x260_swhcChildren)
        ch->SetGlobalTranslation(translation);

    for (const std::unique_ptr<CParticleElectric>& ch : x280_elscChildren)
        ch->SetGlobalTranslation(translation);
}

void CElementGen::SetGlobalScale(const Zeus::CVector3f& scale)
{
    xa0_globalScale = scale;
    xac_globalScaleTransform = Zeus::CTransform::Scale(scale);
    xdc_globalScaleTransformInverse = Zeus::CTransform::Scale(Zeus::CVector3f::skOne / scale);

    for (const std::unique_ptr<CElementGen>& ch : x234_activePartChildren)
        ch->SetGlobalScale(scale);

    for (const std::unique_ptr<CElementGen>& ch : x248_finishPartChildren)
        ch->SetGlobalScale(scale);

    for (const std::unique_ptr<CParticleSwoosh>& ch : x260_swhcChildren)
        ch->SetGlobalScale(scale);

    for (const std::unique_ptr<CParticleElectric>& ch : x280_elscChildren)
        ch->SetGlobalScale(scale);
}

void CElementGen::SetLocalScale(const Zeus::CVector3f& scale)
{
    x10c_localScale = scale;
    x118_localScaleTransform = Zeus::CTransform::Scale(scale);
    x148_localScaleTransformInverse = Zeus::CTransform::Scale(Zeus::CVector3f::skOne / scale);

    for (const std::unique_ptr<CElementGen>& ch : x234_activePartChildren)
        ch->SetLocalScale(scale);

    for (const std::unique_ptr<CElementGen>& ch : x248_finishPartChildren)
        ch->SetLocalScale(scale);
}

void CElementGen::SetParticleEmission(bool enabled)
{
    x68_particleEmission = enabled;

    for (const std::unique_ptr<CElementGen>& ch : x234_activePartChildren)
        ch->SetParticleEmission(enabled);

    for (const std::unique_ptr<CElementGen>& ch : x248_finishPartChildren)
        ch->SetParticleEmission(enabled);

    for (const std::unique_ptr<CParticleSwoosh>& ch : x260_swhcChildren)
        ch->SetParticleEmission(enabled);

    for (const std::unique_ptr<CParticleElectric>& ch : x280_elscChildren)
        ch->SetParticleEmission(enabled);
}

void CElementGen::SetModulationColor(const Zeus::CColor& color)
{
    x30c_moduColor = color;

    for (const std::unique_ptr<CElementGen>& ch : x234_activePartChildren)
        ch->SetModulationColor(color);

    for (const std::unique_ptr<CElementGen>& ch : x248_finishPartChildren)
        ch->SetModulationColor(color);

    for (const std::unique_ptr<CParticleSwoosh>& ch : x260_swhcChildren)
        ch->SetModulationColor(color);

    for (const std::unique_ptr<CParticleElectric>& ch : x280_elscChildren)
        ch->SetModulationColor(color);
}

const Zeus::CTransform& CElementGen::GetOrientation() const
{
    return x178_orientation;
}

const Zeus::CVector3f& CElementGen::GetTranslation() const
{
    return x7c_translation;
}

const Zeus::CTransform& CElementGen::GetGlobalOrientation() const
{
    return x1d8_globalOrientation;
}

const Zeus::CVector3f& CElementGen::GetGlobalTranslation() const
{
    return x88_globalTranslation;
}

const Zeus::CVector3f& CElementGen::GetGlobalScale() const
{
    return xa0_globalScale;
}

const Zeus::CColor& CElementGen::GetModulationColor() const
{
    return x30c_moduColor;
}

bool CElementGen::IsSystemDeletable() const
{
    for (const std::unique_ptr<CElementGen>& ch : x234_activePartChildren)
        if (!ch->IsSystemDeletable())
            return false;

    for (const std::unique_ptr<CElementGen>& ch : x248_finishPartChildren)
        if (!ch->IsSystemDeletable())
            return false;

    for (const std::unique_ptr<CParticleSwoosh>& ch : x260_swhcChildren)
        if (!ch->IsSystemDeletable())
            return false;

    for (const std::unique_ptr<CParticleElectric>& ch : x280_elscChildren)
        if (!ch->IsSystemDeletable())
            return false;

    if (x214_PSLT < x50_curFrame && x208_activeParticleCount == 0)
        return true;

    return false;
}

std::pair<Zeus::CAABox, bool> CElementGen::GetBounds() const
{
    if (GetParticleCountAll() == 0)
        return {Zeus::CAABox(), false};
    else
        return {x2c4_systemBounds, true};
}

u32 CElementGen::GetParticleCount() const
{
    return x208_activeParticleCount;
}

bool CElementGen::SystemHasLight() const
{
    return x2dc_lightType != LightType::None;
}

CLight CElementGen::GetLight() const
{
    switch (x2dc_lightType)
    {
    case LightType::Directional:
        return CLight::BuildDirectional(x2f4_LDIR.normalized(), x2e0_LCLR * x2e4_LINT);
    case LightType::Spot:
        return CLight::BuildSpot(x2e8_LOFF, x2f4_LDIR.normalized(), x2e0_LCLR * x2e4_LINT, x308_LSLA);
    default:
    {
        float quad = x300_falloffType == EFalloffType::Quadratic ? x304_LFOR : 0.f;
        float linear = x300_falloffType == EFalloffType::Linear ? x304_LFOR : 0.f;
        float constant = x300_falloffType == EFalloffType::Constant ? 1.f : 0.f;
        return CLight::BuildCustom(x2e8_LOFF, {1.f, 0.f, 0.f}, x2e0_LCLR,
                                   constant, linear, quad, x2e4_LINT, 0.f, 0.f);
    }
    }
}

bool CElementGen::GetParticleEmission() const
{
    return x68_particleEmission;
}

void CElementGen::DestroyParticles()
{
    for (CParticleListItem& p : x2c_particleLists)
    {
        g_StaticFreeList[++g_FreeIndex] = p.x0_partIdx;
        g_StaticParticleList[p.x0_partIdx].x0_endFrame = -1;
    }

    x2c_particleLists.clear();
    x3c_parentMatrices.clear();

    for (const std::unique_ptr<CElementGen>& ch : x234_activePartChildren)
        ch->DestroyParticles();

    for (const std::unique_ptr<CElementGen>& ch : x248_finishPartChildren)
        ch->DestroyParticles();
}

}
