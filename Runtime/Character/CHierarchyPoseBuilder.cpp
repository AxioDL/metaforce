#include "CHierarchyPoseBuilder.hpp"
#include "CAnimData.hpp"
#include "CCharLayoutInfo.hpp"

namespace urde
{

void CHierarchyPoseBuilder::BuildIntoHierarchy(const CCharLayoutInfo& layout,
                                               const CSegId& boneId, const CSegId& nullId)
{
    if (!x0_treeMap.HasElement(boneId))
    {
        const CCharLayoutNode::Bone& bone = layout.GetRootNode()->GetBoneMap()[boneId];
        if (bone.x0_parentId == nullId)
        {
            xcec_rootId = boneId;
            xcf0_hasRoot = true;
            zeus::CVector3f origin = layout.GetFromParentUnrotated(boneId);
            x0_treeMap[boneId] = CTreeNode(origin);
        }
        else
        {
            BuildIntoHierarchy(layout, bone.x0_parentId, nullId);
            zeus::CVector3f origin = layout.GetFromParentUnrotated(boneId);
            CTreeNode& pNode = x0_treeMap[bone.x0_parentId];
            CTreeNode node(origin);
            node.x1_sibling = pNode.x0_child;
            pNode.x0_child = boneId;
            x0_treeMap[boneId] = node;
        }
    }
}

void CHierarchyPoseBuilder::RecursivelyBuildNoScale(const CSegId& boneId, const CTreeNode& node,
                                                    CPoseAsTransforms& pose, const zeus::CQuaternion& parentRot,
                                                    const zeus::CMatrix3f& parentXf, const zeus::CVector3f& parentOffset) const
{
    zeus::CQuaternion quat = parentRot * node.x4_rotation;
    zeus::CMatrix3f xf = quat;
    zeus::CVector3f xfOffset = parentXf * node.x14_offset + parentOffset;
    pose.Insert(boneId, quat, xfOffset);

    CSegId curBone = node.x0_child;
    while (curBone != 0)
    {
        const CTreeNode& node = x0_treeMap[curBone];
        RecursivelyBuild(curBone, node, pose, quat, xf, xfOffset);
        curBone = node.x1_sibling;
    }
}

void CHierarchyPoseBuilder::RecursivelyBuild(const CSegId& boneId, const CTreeNode& node,
                                             CPoseAsTransforms& pose, const zeus::CQuaternion& parentRot,
                                             const zeus::CMatrix3f& parentXf, const zeus::CVector3f& parentOffset) const
{
    zeus::CQuaternion quat = parentRot * node.x4_rotation;

    float scale;
    if (xcf4_layoutDesc.GetScaledLayoutDescription())
        scale = xcf4_layoutDesc.GetScaledLayoutDescription()->GetScale();
    else
        scale = 1.f;

    zeus::CMatrix3f mtxXf;
    if (scale == 1.f)
        mtxXf = quat;
    else
        mtxXf = parentXf * quat * zeus::CMatrix3f(scale);

    zeus::CVector3f xfOffset = parentXf * node.x14_offset + parentOffset;
    pose.Insert(boneId, mtxXf, xfOffset);

    CSegId curBone = node.x0_child;
    while (curBone != 0)
    {
        const CTreeNode& node = x0_treeMap[curBone];
        RecursivelyBuild(curBone, node, pose, quat, quat, xfOffset);
        curBone = node.x1_sibling;
    }
}

void CHierarchyPoseBuilder::BuildTransform(const CSegId& boneId, zeus::CTransform& xfOut) const
{
    TLockedToken<CCharLayoutInfo> layoutInfoTok;
    float scale;
    if (xcf4_layoutDesc.GetScaledLayoutDescription())
    {
        layoutInfoTok = xcf4_layoutDesc.GetScaledLayoutDescription()->GetCharLayoutInfo();
        scale = xcf4_layoutDesc.GetScaledLayoutDescription()->GetScale();
    }
    else
    {
        layoutInfoTok = xcf4_layoutDesc.GetCharLayoutInfo();
        scale = 1.f;
    }
    const CCharLayoutInfo& layoutInfo = *layoutInfoTok.GetObj();

    u32 idCount = 0;
    CSegId buildIDs[100];
    {
        CSegId curId = boneId;
        while (curId != 2)
        {
            buildIDs[idCount++] = curId;
            curId = layoutInfo.GetRootNode()->GetBoneMap()[curId].x0_parentId;
        }
    }

    zeus::CQuaternion accumRot;
    zeus::CMatrix3f accumXF;
    zeus::CVector3f accumPos;
    for (CSegId* id=&buildIDs[idCount] ; id != buildIDs ; --id)
    {
        CSegId& thisId = id[-1];
        const CTreeNode& node = x0_treeMap[thisId];
        accumRot *= node.x4_rotation;
        accumPos += accumXF * node.x14_offset;
        if (scale == 1.f)
            accumXF = accumRot;
        else
            accumXF = accumXF * zeus::CMatrix3f(node.x4_rotation) * zeus::CMatrix3f(scale);
    }

    xfOut.setRotation(accumXF);
    xfOut.origin = accumPos;
}

void CHierarchyPoseBuilder::BuildNoScale(CPoseAsTransforms& pose)
{
    pose.Clear();
    const CTreeNode& node = x0_treeMap[xcec_rootId];
    zeus::CQuaternion quat;
    zeus::CMatrix3f mtx;
    zeus::CVector3f vec;
    RecursivelyBuildNoScale(xcec_rootId, node, pose, quat, mtx, vec);
}

CHierarchyPoseBuilder::CHierarchyPoseBuilder(const CLayoutDescription& layout)
: xcf4_layoutDesc(layout)
{
    TLockedToken<CCharLayoutInfo> layoutInfoTok;
    if (layout.GetScaledLayoutDescription())
        layoutInfoTok = layout.GetScaledLayoutDescription()->GetCharLayoutInfo();
    else
        layoutInfoTok = layout.GetCharLayoutInfo();
    const CCharLayoutInfo& layoutInfo = *layoutInfoTok.GetObj();

    const CSegIdList& segIDs = layoutInfo.GetSegIdList();
    for (const CSegId& id : segIDs.GetList())
        BuildIntoHierarchy(layoutInfo, id, 2);
}

}
