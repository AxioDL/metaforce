#ifndef __URDE_CDECALMANAGER_HPP__
#define __URDE_CDECALMANAGER_HPP__

#include "RetroTypes.hpp"
#include "rstl.hpp"
#include "optional.hpp"
#include "CToken.hpp"
#include "CDecal.hpp"

namespace urde
{
class CDecalManager
{
    struct SDecal
    {
        TAreaId m_areaId;
        std::experimental::optional<CDecal> x60_decal;
        SDecal() = default;
        SDecal(std::experimental::optional<CDecal>&&, TAreaId);
    };

    static bool  m_PoolInitialized;
    static s32   m_FreeIndex;
    static float m_DeltaTimeSinceLastDecalCreation;
    static s32   m_LastDecalCreatedIndex;
    static ResId m_LastDecalCreatedAssetId;
    static rstl::reserved_vector<SDecal, 64> m_DecalPool;
    static rstl::reserved_vector<s32, 64> m_ActiveIndexList;
public:
    static void Initialize();
    static void Shutdown();
};

}

#endif // __URDE_CDECALMANAGER_HPP__
