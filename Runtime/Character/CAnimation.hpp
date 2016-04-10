#ifndef __PSHAG_CANIMATION_HPP__
#define __PSHAG_CANIMATION_HPP__

#include "IOStreams.hpp"
#include "CMetaAnimFactory.hpp"

namespace urde
{
class IMetaAnim;

class CAnimation
{
    std::string x0_name;
    std::shared_ptr<IMetaAnim> x10_anim;
public:
    CAnimation(CInputStream& in);
};

}

#endif // __PSHAG_CANIMATION_HPP__
