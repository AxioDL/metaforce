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
    CLightParameters x0_lightParms;
    CScannableParameters x40_scanParms;
    std::pair<CAssetId, CAssetId> x44_xrayAssets = {};
    std::pair<CAssetId, CAssetId> x4c_thermalAssets = {};
    CVisorParameters x54_visorParms;
    union
    {
        struct
        {
            bool x58_24_ : 1;
            bool x58_25_thermalHeat : 1;
            bool x58_26_ : 1;
            bool x58_27_noSortThermal : 1;
        };
        u32 _dummy = 0;
    };
    float x5c_ = 0.f;
    float x60_ = 0.f;
    float x64_thermalMag = 0.f;

public:
    CActorParameters() : x58_24_(true), x58_25_thermalHeat(false), x58_26_(false), x58_27_noSortThermal(false) {}
    CActorParameters(const CLightParameters& lightParms, const CScannableParameters& scanParms,
                     const std::pair<CAssetId, CAssetId>& xrayAssets, const std::pair<CAssetId, CAssetId>& thermalAssets,
                     const CVisorParameters& visorParms, bool b1, bool thermalHeat, bool c, bool d)
    : x0_lightParms(lightParms)
    , x40_scanParms(scanParms)
    , x44_xrayAssets(xrayAssets)
    , x4c_thermalAssets(thermalAssets)
    , x54_visorParms(visorParms)
    , x58_24_(b1)
    , x58_25_thermalHeat(thermalHeat)
    , x58_26_(c)
    , x58_27_noSortThermal(d)
    {
    }
    CActorParameters Scannable(const CScannableParameters& sParms) const
    {
        CActorParameters aParms = *this;
        aParms.x40_scanParms = sParms;
        return aParms;
    }

    static CActorParameters None() { return CActorParameters(); }

    void SetVisorParameters(const CVisorParameters& vParams) { x54_visorParms = vParams; }
    const CVisorParameters& GetVisorParameters() const { return x54_visorParms; }
    const CLightParameters& GetLightParameters() const { return x0_lightParms; }
    bool HasThermalHeat() const { return x58_25_thermalHeat; }
};
}

#endif // __URDE_CACTORPARAMETERS_HPP__
