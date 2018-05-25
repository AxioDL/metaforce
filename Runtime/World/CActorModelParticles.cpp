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
#include "World/CScriptPlayerActor.hpp"

namespace urde
{
CActorModelParticles::CItem::CItem(const CEntity& ent, CActorModelParticles& parent)
: x0_id(ent.GetUniqueId()), x4_areaId(ent.GetAreaIdAlways()),
  xdc_ashy(parent.x48_ashy), x128_parent(parent)
{
    x8_thermalHotParticles.resize(8);
}

u32 GetNextBestPt(s32 start, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn, CRandom16& rnd)
{
    const zeus::CVector3f& startVec = vn[start].first;
    u32 ret;
    float lastMag = 0.f;
    for (s32 i = 0; i < 10; ++i)
    {
        u32 idx = u32(rnd.Range(0, s32(vn.size()) - 1));
        const zeus::CVector3f& rndVec = vn[idx].first;
        float mag = (startVec - rndVec).magSquared();
        if (mag > lastMag)
        {
            ret = idx;
            lastMag = mag;
        }
    }
    return ret;
}
void CActorModelParticles::CItem::GeneratePoints(const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn)
{
    for (std::pair<std::unique_ptr<CElementGen>, u32>& pair: x8_thermalHotParticles)
    {
        if (pair.first)
        {
            CRandom16 rnd(pair.second);
            const zeus::CVector3f& vec = vn[u32(rnd.Float() * (vn.size() - 1))].first;
            pair.first->SetTranslation(xec_ * vec);
        }
    }

    if (x84_ > 0)
    {
        CRandom16 rnd(x88_seed1);
        u32 count = (x84_ >= 16 ? 16 : x84_);
        u32 idx = x80_;
        for (u32 i = 0; i < count; ++i)
        {
            idx = GetNextBestPt(idx, vn, rnd);
            x78_->SetTranslation(xec_ * vn[idx].first);
            zeus::CVector3f v = vn[idx].second;
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

        u32 next = GetNextBestPt(xb0_, vn, rnd);
        iceGen->SetTranslation(xec_ * vn[next].first);

        iceGen->SetOrientation(zeus::CTransform::MakeRotationsBasedOnY(zeus::CUnitVector3f(vn[next].second)));

        x8c_thermalColdParticles.push_back(std::move(iceGen));
        xb0_ = (x8c_thermalColdParticles.size() == 4 ? -1 : next);
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
            xc0_particleElectric->SetOverrideIPos(vn[u32(rnd.Range(0, s32(vn.size()) - 1))].first * xec_);
            lastRnd = u32(rnd.Range(0, s32(vn.size()) - 1));
            xc0_particleElectric->SetOverrideFPos(vn[lastRnd].first * xec_);
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
    return std::make_unique<CElementGen>(x18_onFire);
}

std::unique_ptr<CElementGen> CActorModelParticles::MakeAshGen() const
{
    return std::make_unique<CElementGen>(x20_ash);
}

std::unique_ptr<CElementGen> CActorModelParticles::MakeIceGen() const
{
    return std::make_unique<CElementGen>(x28_iceBreak);
}

std::unique_ptr<CElementGen> CActorModelParticles::MakeFirePopGen() const
{
    return std::make_unique<CElementGen>(x30_firePop);
}

std::unique_ptr<CElementGen> CActorModelParticles::MakeIcePopGen() const
{
    return std::make_unique<CElementGen>(x38_icePop);
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
    bool isNotCold = mgr.GetThermalDrawFlag() != EThermalDrawFlag::Cold;
    bool isNotHot = mgr.GetThermalDrawFlag() != EThermalDrawFlag::Hot;

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
            ((isNotCold && item.x12c_24_thermalCold) || (isNotHot && item.x12c_25_thermalHot)))
        {
            item.x12c_24_thermalCold = false;
            item.x12c_25_thermalHot = false;
            continue;
        }
        if (isNotCold)
        {
            /* Hot Draw */
            for (int i=0 ; i<8 ; ++i)
            {
                std::unique_ptr<CElementGen>& gen = item.x8_thermalHotParticles[i].first;
                if (gen)
                    g_Renderer->AddParticleGen(*gen);
            }
            if (mgr.GetThermalDrawFlag() != EThermalDrawFlag::Hot && item.x78_)
                g_Renderer->AddParticleGen(*item.x78_);
            if (item.xb8_)
                g_Renderer->AddParticleGen(*item.xb8_);
            if (item.xc0_particleElectric)
                g_Renderer->AddParticleGen(*item.xc0_particleElectric);
        }
        if (isNotHot)
        {
            /* Cold Draw */
            for (std::unique_ptr<CElementGen>& gen : item.x8c_thermalColdParticles)
                g_Renderer->AddParticleGen(*gen);
            if (item.xe4_)
                g_Renderer->AddParticleGen(*item.xe4_);
        }
        if (isNotCold)
        {
            /* Thermal Reset */
            item.x12c_24_thermalCold = false;
            item.x12c_25_thermalHot = false;
        }
    }
}

void CActorModelParticles::Update(float dt, CStateManager& mgr)
{

}

void CActorModelParticles::PointGenerator(void* ctx,
                                          const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn)
{
    reinterpret_cast<CItem*>(ctx)->GeneratePoints(vn);
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

std::list<CActorModelParticles::CItem>::iterator CActorModelParticles::FindOrCreateSystem(CActor& act)
{
    if (act.GetPointGeneratorParticles())
    {
        for (auto it = x0_items.begin() ; it != x0_items.end() ; ++it)
            if (it->x0_id == act.GetUniqueId())
                return it;
    }

    act.SetPointGeneratorParticles(true);
    return x0_items.emplace(x0_items.end(), act, *this);
}

void CActorModelParticles::StartIce(CActor& actor, CStateManager& mgr)
{

}

void CActorModelParticles::AddRainSplashGenerator(CScriptPlayerActor& act, CStateManager& mgr, u32 maxSplashes,
                                                  u32 genRate, float minZ)
{
    auto it = FindOrCreateSystem(act);
    if (it->xd4_rainSplashGenerator)
        return;

    if (act.GetModelData() && !act.GetModelData()->IsNull())
        it->xd4_rainSplashGenerator = std::make_unique<CRainSplashGenerator>(act.GetModelData()->GetScale(),
                                                                             maxSplashes, genRate, minZ, 0.1875f);
}

void CActorModelParticles::RemoveRainSplashGenerator(CScriptPlayerActor& act)
{
     auto it = FindOrCreateSystem(act);
     it->xd4_rainSplashGenerator.reset();
}

void CActorModelParticles::Render(const CActor& actor) const
{

}

}
