#pragma once

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

