#ifndef __PSHAG_CSKINRULES_HPP__
#define __PSHAG_CSKINRULES_HPP__

#include "RetroTypes.hpp"
#include "CVirtualBone.hpp"

namespace urde
{

class CSkinRules
{
    std::vector<CVirtualBone> x0_bones;
public:
    CSkinRules(CInputStream& in);
};

}

#endif // __PSHAG_CSKINRULES_HPP__
