#pragma once

#include "CFactoryMgr.hpp"
#include "IOStreams.hpp"
#include "CSegIdList.hpp"
#include "CSegId.hpp"
#include "TSegIdMap.hpp"

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
    TSegIdMap<Bone> x0_boneMap;
public:
    CCharLayoutNode(CInputStream& in);
    const TSegIdMap<Bone>& GetBoneMap() const {return x0_boneMap;}
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
    zeus::CVector3f GetFromRootUnrotated(const CSegId& id) const;
    CSegId GetSegIdFromString(std::string_view name) const;
};

CFactoryFnReturn FCharLayoutInfo(const SObjectTag&, CInputStream&, const CVParamTransfer&,
                                 CObjectReference* selfRef);

}

