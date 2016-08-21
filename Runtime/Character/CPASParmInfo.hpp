#ifndef __URDE_CPASPARMINFO_HPP__
#define __URDE_CPASPARMINFO_HPP__

#include "IOStreams.hpp"
#include "CPASAnimParm.hpp"

namespace urde
{

class CPASParmInfo
{
public:
    enum class EWeightFunction
    {
        ExactMatch,
        PercentError,
        AngularPercent,
        Three
    };

    CPASAnimParm::EParmType x0_type;
    EWeightFunction x4_weightFunction;
    float x8_weight;
    CPASAnimParm::UParmValue xc_min;
    CPASAnimParm::UParmValue x10_max;
public:
    CPASParmInfo(CInputStream& in);
    CPASAnimParm::EParmType GetParameterType() const {return x0_type;}
    EWeightFunction GetWeightFunction() const { return x4_weightFunction; }
    float GetParameterWeight() const { return x8_weight; }
    CPASAnimParm::UParmValue GetWeightMinValue() const { return xc_min; }
    CPASAnimParm::UParmValue GetWeightMaxValue() const { return x10_max; }
};

}

#endif // __URDE_CPASPARMINFO_HPP__
