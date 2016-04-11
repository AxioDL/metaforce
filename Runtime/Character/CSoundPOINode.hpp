#ifndef __PSHAG_CSOUNDPOINODE_HPP__
#define __PSHAG_CSOUNDPOINODE_HPP__

#include "CPOINode.hpp"
#include "CCharAnimTime.hpp"

namespace urde
{

class CSoundPOINode : public CPOINode
{
    u32 x38_sfxId;
    float x3c_falloff;
    float x40_maxDist;
public:
    CSoundPOINode(CInputStream& in);
    CSoundPOINode(const std::string& name, u16 a,
                  const CCharAnimTime& time, u32 b, bool c,
                  float d, u32 e, u32 f, u32 sfxId, float falloff, float maxDist);
};

}

#endif // __PSHAG_CSOUNDPOINODE_HPP__
