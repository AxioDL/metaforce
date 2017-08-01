#include "CActorModelParticles.hpp"
#include "CStateManager.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CDependencyGroup.hpp"
#include "Particle/CElementGen.hpp"
#include "Particle/CParticleElectric.hpp"
#include "Particle/CGenDescription.hpp"
#include "World/CWorld.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "Graphics/CSkinnedModel.hpp"

namespace urde
{
CActorModelParticles::CItem::CItem(const CEntity& ent, CActorModelParticles& parent)
: x0_id(ent.GetUniqueId()), x4_areaId(ent.GetAreaIdAlways()),
  xdc_ashy(parent.x48_ashy), x128_parent(parent)
{
    x8_.resize(8);
}

u32 GetNextBestPt(s32 start, const zeus::CVector3f* vecPtr, s32 vecCount, CRandom16& rnd)
{
    const zeus::CVector3f& startVec = vecPtr[start];
    u32 ret;
    float lastMag = 0.f;
    for (s32 i = 0; i < 10; ++i)
    {
        u32 idx = u32(rnd.Range(0, vecCount - 1));
        const zeus::CVector3f& rndVec = vecPtr[idx];
        float mag = (startVec - rndVec).magSquared();
        if (mag > lastMag)
        {
            ret = idx;
            lastMag = mag;
        }
    }
    return ret;
}
void CActorModelParticles::CItem::GeneratePoints(const zeus::CVector3f* v1, const zeus::CVector3f* v2, int w1)
{
    for (std::pair<std::unique_ptr<CElementGen>, u32>& pair: x8_)
    {
        if (pair.first)
        {
            CRandom16 rnd(pair.second);
            zeus::CVector3f vec = v1[u32(rnd.Float() * (w1 - 1))];
            pair.first->SetTranslation(xec_ * vec);
        }
    }

    if (x84_ > 0)
    {
        CRandom16 rnd(x88_seed1);
        u32 count = (x84_ >= 16 ? 16 : x84_);
        zeus::CVector3f uVec = zeus::CVector3f::skUp;
        u32 idx = x80_;
        for (u32 i = 0; i < count; ++i)
        {
            idx = GetNextBestPt(idx, v1, w1, rnd);
            x78_->SetTranslation(xec_ * v1[idx]);
            zeus::CVector3f v = v2[idx];
            if (v.canBeNormalized())
            {
                v.normalize();
                x78_->SetOrientation(zeus::CTransform{zeus::CVector3f::skUp.cross(v), v, zeus::CVector3f::skUp,
                                                      zeus::CVector3f::skZero});
            }
            x78_->ForceParticleCreation(1);
        }
        x84_ -= count;
        x88_seed1 = rnd.GetSeed();
    }

    if (xb0_ != -1)
    {
        CRandom16 rnd(xb4_seed2);

        std::unique_ptr<CElementGen> iceGen = x128_parent.MakeIceGen();
        iceGen->SetGlobalOrientAndTrans(xf8_);

        u32 next = GetNextBestPt(xb0_, v1, w1, rnd);
        iceGen->SetTranslation(xec_ * v1[next]);

        iceGen->SetOrientation(zeus::CTransform::MakeRotationsBasedOnY(zeus::CUnitVector3f(v2[next])));

        x8c_.push_back(std::move(iceGen));
        xb0_ = (x8c_.size() == 4 ? -1 : next);
    }
// TODO: Verify behavior
    if (xc0_particleElectric && xc0_particleElectric->GetParticleEmission())
    {
        CRandom16 rnd(xcc_seed3);
        u32 end = 1;
#if 0
        if (4 < 1)
            end = 4;
#endif
        u32 lastRnd;
        for (u32 i = 0; i < end; ++i)
        {
            xc0_particleElectric->SetOverrideIPos(v1[u32(rnd.Range(0, w1 - 1))] * xec_);
            lastRnd = u32(rnd.Range(0, w1 - 1));
            xc0_particleElectric->SetOverrideFPos(v1[lastRnd] * xec_);
            xc0_particleElectric->ForceParticleCreation(1);
        }

        xcc_seed3 = rnd.GetSeed();
        xc8_ = lastRnd;
    }

    // TODO: Finish
#if 0
    if (xd4_)
    {
        xd4_->sub_8026A5E0(v1, v2, w1);
    }
#endif
}

void CActorModelParticles::CItem::Update(float, CStateManager&)
{

}

static const char* ParticleDGRPs[] =
{
    "Effect_OnFire_DGRP",
    "Effect_Ash_DGRP",
    "Effect_IceBreak_DGRP",
    "Effect_FirePop_DGRP",
    "Effect_IcePop_DGRP",
    "Effect_Electric_DGRP",
};

std::pair<std::vector<CToken>, bool>
CActorModelParticles::GetParticleDGRPTokens(const char* name)
{
    std::pair<std::vector<CToken>, bool> ret = {};
    TToken<CDependencyGroup> dgrp = g_SimplePool->GetObj(name);
    const auto& vector = dgrp->GetObjectTagVector();
    ret.first.reserve(vector.size());
    for (const SObjectTag& tag : vector)
        ret.first.push_back(g_SimplePool->GetObj(tag));
    return ret;
}

void CActorModelParticles::LoadParticleDGRPs()
{
    for (int i=0 ; i<6 ; ++i)
        x50_dgrps.push_back(GetParticleDGRPTokens(ParticleDGRPs[i]));
}

std::unique_ptr<CElementGen> CActorModelParticles::MakeOnFireGen() const
{
    return std::make_unique<CElementGen>(x18_onFire,
                                         CElementGen::EModelOrientationType::Normal,
                                         CElementGen::EOptionalSystemFlags::One);
}

std::unique_ptr<CElementGen> CActorModelParticles::MakeAshGen() const
{
    return std::make_unique<CElementGen>(x20_ash,
                                         CElementGen::EModelOrientationType::Normal,
                                         CElementGen::EOptionalSystemFlags::One);
}

std::unique_ptr<CElementGen> CActorModelParticles::MakeIceGen() const
{
    return std::make_unique<CElementGen>(x28_iceBreak,
                                         CElementGen::EModelOrientationType::Normal,
                                         CElementGen::EOptionalSystemFlags::One);
}

std::unique_ptr<CElementGen> CActorModelParticles::MakeFirePopGen() const
{
    return std::make_unique<CElementGen>(x30_firePop,
                                         CElementGen::EModelOrientationType::Normal,
                                         CElementGen::EOptionalSystemFlags::One);
}

std::unique_ptr<CElementGen> CActorModelParticles::MakeIcePopGen() const
{
    return std::make_unique<CElementGen>(x38_icePop,
                                         CElementGen::EModelOrientationType::Normal,
                                         CElementGen::EOptionalSystemFlags::One);
}

std::unique_ptr<CParticleElectric> CActorModelParticles::MakeElectricGen() const
{
    return std::make_unique<CParticleElectric>(x40_electric);
}

CActorModelParticles::CActorModelParticles()
{
    x18_onFire = g_SimplePool->GetObj("Effect_OnFire");
    x20_ash = g_SimplePool->GetObj("Effect_Ash");
    x28_iceBreak = g_SimplePool->GetObj("Effect_IceBreak");
    x30_firePop = g_SimplePool->GetObj("Effect_FirePop");
    x38_icePop = g_SimplePool->GetObj("Effect_IcePop");
    x40_electric = g_SimplePool->GetObj("Effect_Electric");
    x48_ashy = g_SimplePool->GetObj("TXTR_Ashy");
    LoadParticleDGRPs();
}

void CActorModelParticles::AddStragglersToRenderer(const CStateManager& mgr)
{
    bool isNotOne = mgr.GetParticleFlags() != 1;
    bool isNotZero = mgr.GetParticleFlags() != 0;

    for (CItem& item : x0_items)
    {
        if (item.x4_areaId != kInvalidAreaId)
        {
            const CGameArea* area = mgr.GetWorld()->GetAreaAlways(item.x4_areaId);
            if (!area->IsPostConstructed())
                continue;
            CGameArea::EOcclusionState occState = area->GetPostConstructed()->x10dc_occlusionState;
            if (occState == CGameArea::EOcclusionState::Occluded)
                continue;
        }
        if (mgr.GetObjectById(item.x0_id) &&
            ((isNotOne && item.x12c_24_) || (isNotZero && item.x12c_25_)))
        {
            item.x12c_24_ = false;
            item.x12c_25_ = false;
            continue;
        }
        if (isNotOne)
        {
            for (int i=0 ; i<8 ; ++i)
            {
                std::unique_ptr<CElementGen>& gen = item.x8_[i].first;
                if (gen)
                    g_Renderer->AddParticleGen(*gen);
            }
            if (mgr.GetParticleFlags() && item.x78_)
                g_Renderer->AddParticleGen(*item.x78_);
            if (item.xb8_)
                g_Renderer->AddParticleGen(*item.xb8_);
            if (item.xc0_particleElectric)
                g_Renderer->AddParticleGen(*item.xc0_particleElectric);
        }
        if (isNotZero)
        {
            for (std::unique_ptr<CElementGen>& gen : item.x8c_)
                g_Renderer->AddParticleGen(*gen);
            if (item.xe4_)
                g_Renderer->AddParticleGen(*item.xe4_);
        }
        if (isNotOne)
        {
            item.x12c_24_ = false;
            item.x12c_25_ = false;
        }
    }
}

void CActorModelParticles::Update(float dt, CStateManager& mgr)
{

}

void CActorModelParticles::PointGenerator(void* item, const zeus::CVector3f* v1,
                                          const zeus::CVector3f* v2, int w1)
{
    reinterpret_cast<CItem*>(item)->GeneratePoints(v1, v2, w1);
}

void CActorModelParticles::SetupHook(TUniqueId uid)
{
    auto search = FindSystem(uid);
    if (search != x0_items.cend())
        CSkinnedModel::SetPointGeneratorFunc((void*)&*search, PointGenerator);
}

std::list<CActorModelParticles::CItem>::const_iterator CActorModelParticles::FindSystem(TUniqueId uid) const
{
    for (auto it = x0_items.cbegin() ; it != x0_items.cend() ; ++it)
        if (it->x0_id == uid)
            return it;
    return x0_items.cend();
}

void CActorModelParticles::StartIce(CActor& actor, CStateManager& mgr)
{

}

}
