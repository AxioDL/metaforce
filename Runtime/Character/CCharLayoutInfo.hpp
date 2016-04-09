#ifndef __PSHAG_CCHARLAYOUTINFO_HPP__
#define __PSHAG_CCHARLAYOUTINFO_HPP__

#include "CFactoryMgr.hpp"
#include "IOStreams.hpp"
#include "CSegIdList.hpp"
#include "CSegId.hpp"

namespace urde
{

class CCharLayoutNode
{
public:
    struct Bone
    {
        CSegId x0_parentId;
        zeus::CVector3f x4_origin;
        std::vector<CSegId> x10_children;
        void read(CInputStream& in);
    };
private:
    u8 x0_boneCount;
    CSegId x8_ids[100];
    Bone x108_bones[100];
public:
    CCharLayoutNode(CInputStream& in);
};

class CCharLayoutInfo
{
    std::shared_ptr<CCharLayoutNode> x0_node;
    CSegIdList x8_segIdList;
    std::map<std::string, CSegId> x18_segIdMap;
public:
    CCharLayoutInfo(CInputStream& in);
};

CFactoryFnReturn FCharLayoutInfo(const SObjectTag&, CInputStream&, const CVParamTransfer&);

}

#endif // __PSHAG_CCHARLAYOUTINFO_HPP__
