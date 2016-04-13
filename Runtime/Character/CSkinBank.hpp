#ifndef __URDE_CSKINBANK_HPP__
#define __URDE_CSKINBANK_HPP__

#include "IOStreams.hpp"
#include "CSegId.hpp"

namespace urde
{

class CSkinBank
{
    std::vector<CSegId> x0_segments;
public:
    CSkinBank(CInputStream& in);
};

}

#endif // __URDE_CSKINBANK_HPP__
