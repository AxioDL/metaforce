#ifndef __DNAMP1_SNOWFORCES_HPP__
#define __DNAMP1_SNOWFORCES_HPP__

#include <vector>

#include "DNAMP1.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct SnowForces : BigYAML
{
    DECL_YAML
    struct Force : BigYAML
    {
        DECL_YAML
        Value<float> gravity;
        Value<float> wind;
    };

    Value<Force> forces[256];
};
}
}

#endif // __DNAMP1_SNOWFORCES_HPP__
