#include "CActorModelParticles.hpp"
#include "CStateManager.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CDependencyGroup.hpp"
#include "Particle/CElementGen.hpp"
#include "Particle/CParticleElectric.hpp"
#include "Particle/CParticleSwoosh.hpp"
#include "Particle/CGenDescription.hpp"
#include "World/CWorld.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde
{
CActorModelParticles::CItem::CItem(const CEntity& ent, CActorModelParticles& parent)
: x0_id(ent.GetUniqueId()), x4_areaId(ent.GetAreaIdAlways()),
  xdc_ashy(parent.x48_ashy), x128_parent(parent)
{
    x8_.resize(8);
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
            if (occState == CGameArea::EOcclusionState::NotOccluded)
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
            if (item.xc0_)
                g_Renderer->AddParticleGen(*item.xc0_);
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

}
