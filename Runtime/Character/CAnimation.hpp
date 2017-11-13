#ifndef __URDE_CANIMATION_HPP__
#define __URDE_CANIMATION_HPP__

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
    const std::shared_ptr<IMetaAnim>& GetMetaAnim() const {return x10_anim;}
    std::string_view GetMetaAnimName() const { return x0_name; }
};

}

#endif // __URDE_CANIMATION_HPP__
