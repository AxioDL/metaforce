#include "CParticleDatabase.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

void CParticleDatabase::CacheParticleDesc(const CCharacterInfo::CParticleResData& desc)
{
    for (ResId id : desc.x0_part)
    {
        auto search = x0_particleDescs.find(id);
        if (search == x0_particleDescs.cend())
            x0_particleDescs[id] = std::make_shared<TLockedToken<CGenDescription>>(
                g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), id}));
    }
    for (ResId id : desc.x10_swhc)
    {
        auto search = x14_swooshDescs.find(id);
        if (search == x14_swooshDescs.cend())
            x14_swooshDescs[id] = std::make_shared<TLockedToken<CSwooshDescription>>(
                g_SimplePool->GetObj(SObjectTag{FOURCC('SWHC'), id}));
    }
    for (ResId id : desc.x20_elsc)
    {
        auto search = x28_electricDescs.find(id);
        if (search == x28_electricDescs.cend())
            x28_electricDescs[id] = std::make_shared<TLockedToken<CElectricDescription>>(
                g_SimplePool->GetObj(SObjectTag{FOURCC('ELSC'), id}));
    }
}

void CParticleDatabase::SetModulationColorAllActiveEffects(const zeus::CColor& color)
{
}

void CParticleDatabase::SuspendAllActiveEffects(CStateManager& stateMgr)
{
}

void CParticleDatabase::StartEffect(const std::string& name, u32 flags, const CParticleData& data,
                                    const zeus::CVector3f& scale, CStateManager& stateMgr, TAreaId aid, u32 unk1)
{
}

void CParticleDatabase::Update(float dt, const CPoseAsTransforms& pose, const CCharLayoutInfo& charInfo,
                               const zeus::CTransform& xf, const zeus::CVector3f& vec, CStateManager& stateMgr)
{
}

void CParticleDatabase::AddToRendererClipped(const zeus::CFrustum& frustum)
{
}

void CParticleDatabase::GetActiveParticleLightIds(std::vector<TUniqueId>&)
{

}

}
