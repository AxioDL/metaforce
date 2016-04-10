#include "CAnimation.hpp"
#include "CMetaAnimFactory.hpp"

namespace urde
{

CAnimation::CAnimation(CInputStream& in)
{
    x0_name = in.readString();
    x10_anim = CMetaAnimFactory::CreateMetaAnim(in);
}

}
