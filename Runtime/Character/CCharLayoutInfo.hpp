#ifndef __URDE_CCHARLAYOUTINFO_HPP__
#define __URDE_CCHARLAYOUTINFO_HPP__

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
    CSegId x0_boneCount = 0;
    CSegId x1_curPrevBone = 0;
    CSegId x8_prevBones[100];
    Bone x6c_bones[100];
public:
    CCharLayoutNode(CInputStream& in);

    const Bone& GetBone(const CSegId& id) const {return x6c_bones[id];}
    const CSegId& GetPrevBone(const CSegId& id) const {return x8_prevBones[id];}
};

class CCharLayoutInfo
{
    std::shared_ptr<CCharLayoutNode> x0_node;
    CSegIdList x8_segIdList;
    std::map<std::string, CSegId> x18_segIdMap;
public:
    CCharLayoutInfo(CInputStream& in);
    const std::shared_ptr<CCharLayoutNode>& GetRootNode() const {return x0_node;}
    const CSegIdList& GetSegIdList() const {return x8_segIdList;}
    zeus::CVector3f GetFromParentUnrotated(const CSegId& id) const;
};

CFactoryFnReturn FCharLayoutInfo(const SObjectTag&, CInputStream&, const CVParamTransfer&);

}

#endif // __URDE_CCHARLAYOUTINFO_HPP__
