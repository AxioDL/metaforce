#ifndef __URDE_CSCANNABLEPARAMETERS_HPP__
#define __URDE_CSCANNABLEPARAMETERS_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CScannableParameters
{
    CAssetId x0_scanId;

public:
    CScannableParameters() = default;
    CScannableParameters(CAssetId id) : x0_scanId(id) {}
    CAssetId GetScanId() const { return x0_scanId; }
};
}

#endif // __URDE_CSCANNABLEPARAMETERS_HPP__
