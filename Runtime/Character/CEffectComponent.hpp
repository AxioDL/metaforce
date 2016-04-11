#ifndef __PSHAG_CEFFECTCOMPONENT_HPP__
#define __PSHAG_CEFFECTCOMPONENT_HPP__

#include "IOStreams.hpp"
#include "RetroTypes.hpp"

namespace urde
{

class CEffectComponent
{
    std::string x0_name;
    SObjectTag x10_tag;
    std::string x18_boneName;
    float x28_;
    u32 x2c_;
    u32 x30_;
    static SObjectTag GetSObjectTagFromStream(CInputStream& in);
public:
    CEffectComponent(CInputStream& in);
};

}

#endif // __PSHAG_CEFFECTCOMPONENT_HPP__
