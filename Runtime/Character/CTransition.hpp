#ifndef __PSHAG_CTRANSITION_HPP__
#define __PSHAG_CTRANSITION_HPP__

#include "IOStreams.hpp"
#include "CMetaTransFactory.hpp"

namespace urde
{

class CTransition
{
    u32 x0_id;
    u32 x4_animA;
    u32 x8_animB;
    std::unique_ptr<IMetaTrans> xc_trans;
public:
    CTransition(CInputStream& in);
};

}

#endif // __PSHAG_CTRANSITION_HPP__
