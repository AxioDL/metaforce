#ifndef __URDE_TSEGIDMAP_HPP__
#define __URDE_TSEGIDMAP_HPP__

#include <map>
#include "CSegId.hpp"

namespace urde
{

template <class T>
class TSegIdMap
{
    std::map<CSegId, T> x0_map;
public:
    const T& AccessElement(const CSegId& id) const {return x0_map[id];}
};

}

#endif // __URDE_TSEGIDMAP_HPP__
