#ifndef __URDE_CPVSVISOCTREE_HPP__
#define __URDE_CPVSVISOCTREE_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{

class CPVSVisOctree
{
public:
    CPVSVisOctree(CInputStream& in);
    u32 GetNumChildren(const unsigned char*) const;
    u32 GetChildIndex(const unsigned char*, const zeus::CVector3f&) const;
    u32 SetTestPoint(const zeus::CVector3f&) const;
};

}

#endif // __URDE_CPVSVISOCTREE_HPP__
