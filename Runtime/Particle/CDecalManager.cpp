#include "CDecalManager.hpp"
#include "CDecalDescription.hpp"
#include "CDecal.hpp"
#include "CStateManager.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{
bool   CDecalManager::m_PoolInitialized = false;
s32    CDecalManager::m_FreeIndex = 63;
float  CDecalManager::m_DeltaTimeSinceLastDecalCreation = 0.f;
s32    CDecalManager::m_LastDecalCreatedIndex = -1;
CAssetId CDecalManager::m_LastDecalCreatedAssetId = -1;
rstl::reserved_vector<CDecalManager::SDecal, 64> CDecalManager::m_DecalPool;
rstl::reserved_vector<s32, 64> CDecalManager::m_ActiveIndexList;

void CDecalManager::Initialize()
{
    if (m_PoolInitialized)
        return;

    m_DecalPool.clear();
    m_DecalPool.resize(64);

    m_FreeIndex = 63;
    m_PoolInitialized = true;
    m_DeltaTimeSinceLastDecalCreation = 0.f;
    m_LastDecalCreatedIndex = -1;
    m_LastDecalCreatedAssetId = -1;
}

void CDecalManager::Shutdown()
{

}

void CDecalManager::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr)
{
    for (s32 idx : m_ActiveIndexList)
    {
        CDecalManager::SDecal& decal = m_DecalPool[idx];
        if (decal.x75_flags & 0x2 || mgr.GetParticleFlags())
        {
            const zeus::CVector3f& point = decal.x0_decal->xc_transform.origin;
            zeus::CAABox aabb(point, point);
            g_Renderer->AddDrawable(&*decal.x0_decal, point, aabb, 2,
                                    IRenderer::EDrawableSorting::SortedCallback);
        }
    }
}

void CDecalManager::Update(float dt, CStateManager& mgr)
{

}

}
