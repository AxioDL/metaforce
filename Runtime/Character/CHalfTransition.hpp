#ifndef __PSHAG_CHALFTRANSITION_HPP__
#define __PSHAG_CHALFTRANSITION_HPP__

#include "IOStreams.hpp"
#include "IMetaTrans.hpp"

namespace urde
{

class CHalfTransition
{
    u32 x0_id;
    std::shared_ptr<IMetaTrans> x4_trans;
public:
    CHalfTransition(CInputStream& in);
};

}

#endif // __PSHAG_CHALFTRANSITION_HPP__
