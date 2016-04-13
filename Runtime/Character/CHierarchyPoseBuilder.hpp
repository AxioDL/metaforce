#ifndef __URDE_CHIERARCHYPOSEBUILDER_HPP__
#define __URDE_CHIERARCHYPOSEBUILDER_HPP__

#include "CSegId.hpp"
#include "zeus/CQuaternion.hpp"
#include "CLayoutDescription.hpp"

namespace urde
{
class CLayoutDescription;
class CCharLayoutInfo;
class CPoseAsTransforms;

class CHierarchyPoseBuilder
{
    CSegId x0_boneCount = 0;
    CSegId x1_curPrevBone = 0;
    CSegId x8_prevBones[100];

    struct CTreeNode
    {
        CSegId x0_child = 0;
        CSegId x1_sibling = 0;
        zeus::CQuaternion x4_rotation;
        zeus::CVector3f x14_offset;
        CTreeNode() = default;
        CTreeNode(const zeus::CVector3f& offset) : x14_offset(offset) {}
    };
    CTreeNode x6c_nodes[100];
    CSegId xcec_rootId;
    bool xcf0_hasRoot = false;
    CLayoutDescription xcf4_layoutDesc;

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

    void BuildTransform(const CSegId& boneId, zeus::CTransform& xfOut) const;
    void BuildNoScale(CPoseAsTransforms& pose);
};

}

#endif // __URDE_CHIERARCHYPOSEBUILDER_HPP__
