#ifndef __PSHAG_CSEGIDLIST_HPP__
#define __PSHAG_CSEGIDLIST_HPP__

#include "IOStreams.hpp"
#include "CSegId.hpp"

namespace urde
{

class CSegIdList
{
    std::vector<CSegId> x0_list;
public:
    CSegIdList(CInputStream& in);
};

}

#endif // __PSHAG_CSEGIDLIST_HPP__
