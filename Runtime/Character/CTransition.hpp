#ifndef __URDE_CTRANSITION_HPP__
#define __URDE_CTRANSITION_HPP__

#include "IOStreams.hpp"
#include "CMetaTransFactory.hpp"

namespace urde
{

class CTransition
{
    u32 x0_id;
    u32 x4_animA;
    u32 x8_animB;
    std::shared_ptr<IMetaTrans> xc_trans;
public:
    CTransition(CInputStream& in);
    u32 GetAnimA() const {return x4_animA;}
    u32 GetAnimB() const {return x8_animB;}
    std::pair<u32, u32> GetAnimPair() const {return {x4_animA, x8_animB};}
    const std::shared_ptr<IMetaTrans>& GetMetaTrans() const {return xc_trans;}
};

}

#endif // __URDE_CTRANSITION_HPP__
