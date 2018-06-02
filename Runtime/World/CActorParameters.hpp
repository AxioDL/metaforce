#ifndef __URDE_CACTORPARAMETERS_HPP__
#define __URDE_CACTORPARAMETERS_HPP__

#include "CLightParameters.hpp"
#include "CScannableParameters.hpp"
#include "CVisorParameters.hpp"

namespace urde
{

class CActorParameters
{
    friend class ScriptLoader;
    friend class CActor;
    friend class CScriptActor;
    CLightParameters x0_lightParms;
    CScannableParameters x40_scanParms;
    std::pair<CAssetId, CAssetId> x44_xrayAssets = {};
    std::pair<CAssetId, CAssetId> x4c_thermalAssets = {};
    CVisorParameters x54_visorParms;
    union
    {
        struct
        {
            bool x58_24_globalTimeProvider : 1;
            bool x58_25_thermalHeat : 1;
            bool x58_26_renderUnsorted : 1;
            bool x58_27_noSortThermal : 1;
        };
        u32 _dummy = 0;
    };
    float x5c_fadeInTime = 0.f;
    float x60_fadeOutTime = 0.f;
    float x64_thermalMag = 0.f;

public:
    CActorParameters() : x58_24_globalTimeProvider(true), x58_25_thermalHeat(false),
                         x58_26_renderUnsorted(false), x58_27_noSortThermal(false) {}
    CActorParameters(const CLightParameters& lightParms, const CScannableParameters& scanParms,
                     const std::pair<CAssetId, CAssetId>& xrayAssets,
                     const std::pair<CAssetId, CAssetId>& thermalAssets,
                     const CVisorParameters& visorParms, bool globalTimeProvider, bool thermalHeat,
                     bool renderUnsorted, bool noSortThermal,
                     float fadeInTime, float fadeOutTime, float thermalMag)
    : x0_lightParms(lightParms)
    , x40_scanParms(scanParms)
    , x44_xrayAssets(xrayAssets)
    , x4c_thermalAssets(thermalAssets)
    , x54_visorParms(visorParms)
    , x58_24_globalTimeProvider(globalTimeProvider)
    , x58_25_thermalHeat(thermalHeat)
    , x58_26_renderUnsorted(renderUnsorted)
    , x58_27_noSortThermal(noSortThermal)
    , x5c_fadeInTime(fadeInTime)
    , x60_fadeOutTime(fadeOutTime)
    , x64_thermalMag(thermalMag)
    {
    }
    CActorParameters Scannable(const CScannableParameters& sParms) const
    {
        CActorParameters aParms = *this;
        aParms.x40_scanParms = sParms;
        return aParms;
    }

    static CActorParameters None() { return CActorParameters(); }
    CActorParameters HotInThermal(bool hot) const
    {
        CActorParameters ret = *this;
        ret.x58_25_thermalHeat = hot;
        return ret;
    }
    void SetVisorParameters(const CVisorParameters& vParams) { x54_visorParms = vParams; }
    const CVisorParameters& GetVisorParameters() const { return x54_visorParms; }
    const CLightParameters& GetLightParameters() const { return x0_lightParms; }
    bool HasThermalHeat() const { return x58_25_thermalHeat; }
};
}

#endif // __URDE_CACTORPARAMETERS_HPP__
