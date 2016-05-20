#ifndef CHUDMEMOPARMS_HPP
#define CHUDMEMOPARMS_HPP

#include "IOStreams.hpp"

namespace urde
{
class CHUDMemoParms
{
    float x0_;
    bool x4_;
    bool x5_ = false;
    bool x6_ = false;
public:
    CHUDMemoParms(CInputStream& in)
        : x0_(in.readFloatBig()),
          x4_(in.readBool())
    {}
};
}

#endif // CHUDMEMOPARMS_HPP
