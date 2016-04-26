#ifndef __URDE_CCOLLIDABLEOBBTREEGROUP_HPP__
#define __URDE_CCOLLIDABLEOBBTREEGROUP_HPP__

#include "IOStreams.hpp"
#include "CFactoryMgr.hpp"
namespace urde
{
class COBBTree;
class CCollidableOBBTreeGroup
{
    std::vector<COBBTree> x0_trees;
public:
    CCollidableOBBTreeGroup(CInputStream& in);
};

CFactoryFnReturn FCollidableOBBTreeGroupFactory(const SObjectTag &tag, CInputStream &in,
                                                const CVParamTransfer &vparms);

}

#endif // __URDE_CCOLLIDABLEOBBTREEGROUP_HPP__
