#ifndef __URDE_CHIERARCHYPOSEBUILDER_HPP__
#define __URDE_CHIERARCHYPOSEBUILDER_HPP__

#include "CSegId.hpp"
#include "TSegIdMap.hpp"
#include "zeus/CQuaternion.hpp"
#include "CLayoutDescription.hpp"

namespace urde
{
class CLayoutDescription;
class CCharLayoutInfo;
class CPoseAsTransforms;

class CHierarchyPoseBuilder
{
    CLayoutDescription x0_layoutDesc;
    bool x34_ = 0;


    struct CTreeNode
    {
        CSegId x0_child = 0;
        CSegId x1_sibling = 0;
        zeus::CQuaternion x4_rotation;
        zeus::CVector3f x14_offset;
        CTreeNode() = default;
        CTreeNode(const zeus::CVector3f& offset) : x14_offset(offset) {}
        CTreeNode(const zeus::CQuaternion& quat) : x4_rotation(quat) {}
        CTreeNode(const zeus::CQuaternion& quat, const zeus::CVector3f& offset)
        : x4_rotation(quat), x14_offset(offset) {}
    };
    TSegIdMap<CTreeNode> x38_treeMap;

    CSegId xcec_rootId;
    bool xcf0_hasRoot = false;

    void BuildIntoHierarchy(const CCharLayoutInfo& layout,
                            const CSegId& boneId, const CSegId& nullId);
    void RecursivelyBuildNoScale(const CSegId& boneId, const CTreeNode& node,
                                 CPoseAsTransforms& pose, const zeus::CQuaternion& rot,
                                 const zeus::CMatrix3f& scale, const zeus::CVector3f& offset) const;
    void RecursivelyBuild(const CSegId& boneId, const CTreeNode& node,
                          CPoseAsTransforms& pose, const zeus::CQuaternion& rot,
                          const zeus::CMatrix3f& scale, const zeus::CVector3f& offset) const;
public:
    CHierarchyPoseBuilder(const CLayoutDescription& layout);

    bool HasRoot() const { return xcf0_hasRoot; }
    void BuildTransform(const CSegId& boneId, zeus::CTransform& xfOut) const;
    void BuildNoScale(CPoseAsTransforms& pose);
    void Insert(const CSegId& boneId, const zeus::CQuaternion& quat);
    void Insert(const CSegId& boneId, const zeus::CQuaternion& quat, const zeus::CVector3f& offset);
};

}

#endif // __URDE_CHIERARCHYPOSEBUILDER_HPP__
