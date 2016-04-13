#ifndef __URDE_CSEGIDLIST_HPP__
#define __URDE_CSEGIDLIST_HPP__

#include "IOStreams.hpp"
#include "CSegId.hpp"

namespace urde
{

class CSegIdList
{
    std::vector<CSegId> x0_list;
public:
    CSegIdList(CInputStream& in);
    const std::vector<CSegId>& GetList() const {return x0_list;}
};

}

#endif // __URDE_CSEGIDLIST_HPP__
