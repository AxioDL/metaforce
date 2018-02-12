#ifndef __URDE_CDECALMANAGER_HPP__
#define __URDE_CDECALMANAGER_HPP__

#include "RetroTypes.hpp"
#include "rstl.hpp"
#include "optional.hpp"
#include "CToken.hpp"
#include "CDecal.hpp"
#include "zeus/CFrustum.hpp"

namespace urde
{
class CStateManager;

class CDecalManager
{
    struct SDecal
    {
        std::experimental::optional<CDecal> x0_decal;
        TAreaId x70_areaId;
        u8 x74_index;
        u8 x75_flags : 2;
    };

    static bool  m_PoolInitialized;
    static s32   m_FreeIndex;
    static float m_DeltaTimeSinceLastDecalCreation;
    static s32   m_LastDecalCreatedIndex;
    static CAssetId m_LastDecalCreatedAssetId;
    static rstl::reserved_vector<SDecal, 64> m_DecalPool;
    static rstl::reserved_vector<s32, 64> m_ActiveIndexList;
public:
    static void Initialize();
    static void Shutdown();
    static void AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr);
    static void Update(float dt, CStateManager& mgr);
    static void AddDecal(const TToken<CDecalDescription>& decal, const zeus::CTransform& xf,
                         bool b1, CStateManager& mgr);
};

}

#endif // __URDE_CDECALMANAGER_HPP__
