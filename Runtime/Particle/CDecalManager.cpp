#include "CDecalManager.hpp"
#include "CDecalDescription.hpp"

namespace urde
{
bool   CDecalManager::m_PoolInitialized = false;
s32    CDecalManager::m_FreeIndex = 63;
float  CDecalManager::m_DeltaTimeSinceLastDecalCreation = 0.f;
s32    CDecalManager::m_LastDecalCreatedIndex = -1;
ResId CDecalManager::m_LastDecalCreatedAssetId = -1;

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
}
