#ifndef __URDE_CDRAWSTRINGOPTIONS_HPP__
#define __URDE_CDRAWSTRINGOPTIONS_HPP__

#include <vector>
#include "RetroTypes.hpp"

namespace urde
{

class CDrawStringOptions
{
    u32 x0_ = 0;
    std::vector<u32> x4_vec;
public:
    CDrawStringOptions()
    {
        x4_vec.resize(16);
    }
};

}

#endif // __URDE_CDRAWSTRINGOPTIONS_HPP__
