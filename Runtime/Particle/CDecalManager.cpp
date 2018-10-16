#include "CDecalManager.hpp"
#include "CDecalDescription.hpp"
#include "CDecal.hpp"
#include "CStateManager.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/Shaders/CDecalShaders.hpp"

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
    for (int i=0 ; i<64 ; ++i)
        m_DecalPool.emplace_back(std::experimental::optional<CDecal>{}, 0, i-1, false);

    m_FreeIndex = 63;
    m_PoolInitialized = true;
    m_DeltaTimeSinceLastDecalCreation = 0.f;
    m_LastDecalCreatedIndex = -1;
    m_LastDecalCreatedAssetId = -1;

    /* Compile shaders */
    CDecalShaders::Initialize();
}

void CDecalManager::Reinitialize()
{
    if (!m_PoolInitialized)
        Initialize();

    m_DecalPool.clear();
    for (int i=0 ; i<64 ; ++i)
        m_DecalPool.emplace_back(std::experimental::optional<CDecal>{}, 0, i-1, false);

    m_ActiveIndexList.clear();

    m_FreeIndex = 63;
}

void CDecalManager::Shutdown()
{
    m_ActiveIndexList.clear();
    m_DecalPool.clear();
    CDecalShaders::Shutdown();
}

void CDecalManager::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr)
{
    for (s32 idx : m_ActiveIndexList)
    {
        CDecalManager::SDecal& decal = m_DecalPool[idx];
        if (decal.x75_24_notIce || mgr.GetThermalDrawFlag() != EThermalDrawFlag::Hot)
        {
            const zeus::CVector3f& point = decal.x0_decal->xc_transform.origin;
            zeus::CAABox aabb(point, point);
            g_Renderer->AddDrawable(&*decal.x0_decal, point, aabb, 2,
                                    IRenderer::EDrawableSorting::SortedCallback);
        }
    }
}

rstl::reserved_vector<s32, 64>::iterator
CDecalManager::RemoveFromActiveList(rstl::reserved_vector<s32, 64>::iterator it, s32 idx)
{
    it = m_ActiveIndexList.erase(it);
    m_DecalPool[idx].x74_index = u8(m_FreeIndex);
    m_FreeIndex = idx;
    if (m_LastDecalCreatedIndex == m_FreeIndex)
        m_LastDecalCreatedIndex = -1;
    return it;
}

void CDecalManager::Update(float dt, CStateManager& mgr)
{
    m_DeltaTimeSinceLastDecalCreation += dt;
    for (auto it = m_ActiveIndexList.begin() ; it != m_ActiveIndexList.end() ;)
    {
        SDecal& decal = m_DecalPool[*it];
        if (decal.x70_areaId != mgr.GetNextAreaId() ||
            (decal.x0_decal->x5c_29_modelInvalid &&
             decal.x0_decal->x5c_30_quad2Invalid &&
             decal.x0_decal->x5c_31_quad1Invalid))
        {
            it = RemoveFromActiveList(it, *it);
            continue;
        }
        decal.x0_decal->Update(dt);
        ++it;
    }
}

void CDecalManager::AddDecal(const TToken<CDecalDescription>& decal, const zeus::CTransform& xf,
                             bool notIce, CStateManager& mgr)
{
    if (m_LastDecalCreatedIndex != -1 && m_DeltaTimeSinceLastDecalCreation < 0.75f &&
        m_LastDecalCreatedAssetId == decal.GetObjectTag()->id)
    {
        SDecal& existingDecal = m_DecalPool[m_LastDecalCreatedIndex];
        if ((existingDecal.x0_decal->xc_transform.origin - xf.origin).magSquared() < 0.01f)
            return;
    }

    if (m_FreeIndex == -1)
        RemoveFromActiveList(m_ActiveIndexList.begin(), m_ActiveIndexList[0]);

    s32 thisIndex = m_FreeIndex;
    SDecal& freeDecal = m_DecalPool[thisIndex];
    m_FreeIndex = freeDecal.x74_index;
    freeDecal.x0_decal.emplace(decal, xf);

    freeDecal.x70_areaId = mgr.GetNextAreaId();
    freeDecal.x75_24_notIce = notIce;
    m_DeltaTimeSinceLastDecalCreation = 0.f;
    m_LastDecalCreatedIndex = thisIndex;
    m_LastDecalCreatedAssetId = decal.GetObjectTag()->id;
    m_ActiveIndexList.push_back(thisIndex);
}

}
