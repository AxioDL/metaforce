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
    CLightParameters x4_lightParms;
    CScannableParameters x40_scanParms;
    std::pair<ResId, ResId> x44_xrayAssets = {};
    std::pair<ResId, ResId> x4c_thermalAssets = {};
    CVisorParameters x54_visorParms;
    bool b1 : 1;
    bool b2 : 1;
    bool b3 : 1;
    bool b4 : 1;
public:
    CActorParameters()
    : b1(true), b2(false), b3(false), b4(false) {}
    CActorParameters(const CLightParameters& lightParms, const CScannableParameters& scanParms,
                     const std::pair<ResId, ResId>& xrayAssets, const std::pair<ResId, ResId>& thermalAssets,
                     const CVisorParameters& visorParms, bool a, bool b, bool c, bool d)
    : x4_lightParms(lightParms), x40_scanParms(scanParms),
      x44_xrayAssets(xrayAssets), x4c_thermalAssets(thermalAssets),
      x54_visorParms(visorParms), b1(a), b2(b), b3(c), b4(d) {}
    static CActorParameters None() {return CActorParameters();}

    void SetVisorParameters(const CVisorParameters& vParams) { x54_visorParms = vParams; }
    CVisorParameters GetVisorParameters() const { return x54_visorParms; }
};

}

#endif // __URDE_CACTORPARAMETERS_HPP__
