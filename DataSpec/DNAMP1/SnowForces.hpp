#ifndef __DNAMP1_SNOWFORCES_HPP__
#define __DNAMP1_SNOWFORCES_HPP__

#include <vector>

#include "DNAMP1.hpp"

namespace DataSpec::DNAMP1
{
struct SnowForces : BigDNA
{
    AT_DECL_DNA_YAML
    struct Force : BigDNA
    {
        AT_DECL_DNA
        Value<float> gravity;
        Value<float> wind;
    };

    Value<Force> forces[256];
};
}

#endif // __DNAMP1_SNOWFORCES_HPP__
