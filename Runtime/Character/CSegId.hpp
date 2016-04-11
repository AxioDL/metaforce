#ifndef __PSHAG_CSEGID_HPP__
#define __PSHAG_CSEGID_HPP__

#include "RetroTypes.hpp"
#include "IOStreams.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{

class CSegId
{
    u8 x0_segId = 0xff;
public:
    CSegId() = default;
    CSegId(CInputStream& in) : x0_segId(in.readUint32Big()) {}
    operator bool() const {return x0_segId != 0xff;}
    u8 GetId() const {return x0_segId;}
};

}

#endif // __PSHAG_CSEGID_HPP__
