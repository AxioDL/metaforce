#include "CSegIdList.hpp"

namespace urde
{

CSegIdList::CSegIdList(CInputStream& in)
{
    u32 count = in.readUint32Big();
    x0_list.reserve(count);
    for (u32 i=0 ; i<count ; ++i)
        x0_list.emplace_back(in);
}

}
