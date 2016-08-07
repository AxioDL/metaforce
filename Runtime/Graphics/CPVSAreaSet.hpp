#ifndef __URDE_CPVSAREASET_HPP__
#define __URDE_CPVSAREASET_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CPVSAreaSet
{
public:
    struct CPVSAreaHolder
    {
        CPVSAreaHolder(CInputStream& in);
    };
private:
    std::vector<CPVSAreaHolder> xunk;
public:
    CPVSAreaSet(CInputStream& in);
};

}

#endif // __URDE_CPVSAREASET_HPP__
