#include "CHierarchyPoseBuilder.hpp"
#include "CAnimData.hpp"
#include "CCharLayoutInfo.hpp"
#include "zeus/CEulerAngles.hpp"

namespace urde {

void CHierarchyPoseBuilder::BuildIntoHierarchy(const CCharLayoutInfo& layout, const CSegId& boneId,
                                               const CSegId& nullId) {
  if (!x38_treeMap.HasElement(boneId)) {
    const CCharLayoutNode::Bone& bone = layout.GetRootNode()->GetBoneMap()[boneId];
    if (bone.x0_parentId == nullId) {
      xcec_rootId = boneId;
      xcf0_hasRoot = true;
      zeus::CVector3f origin = layout.GetFromParentUnrotated(boneId);
      CTreeNode& node = x38_treeMap[boneId];
      node.x14_offset = origin;
    } else {
      BuildIntoHierarchy(layout, bone.x0_parentId, nullId);
      zeus::CVector3f origin = layout.GetFromParentUnrotated(boneId);
      CTreeNode& pNode = x38_treeMap[bone.x0_parentId];
      CTreeNode& node = x38_treeMap[boneId];
      node.x14_offset = origin;
      node.x1_sibling = pNode.x0_child;
      pNode.x0_child = boneId;
    }
  }
}

void CHierarchyPoseBuilder::RecursivelyBuildNoScale(const CSegId& boneId, const CTreeNode& node,
                                                    CPoseAsTransforms& pose, const zeus::CQuaternion& parentRot,
                                                    const zeus::CMatrix3f& parentXf,
                                                    const zeus::CVector3f& parentOffset) const {
  zeus::CVector3f bindOffset;
  if (x0_layoutDesc.GetScaledLayoutDescription()) {
    const CLayoutDescription::CScaledLayoutDescription& desc = *x0_layoutDesc.GetScaledLayoutDescription();
    bindOffset = desc.ScaledLayout()->GetFromRootUnrotated(boneId);
  } else
    bindOffset = x0_layoutDesc.GetCharLayoutInfo()->GetFromRootUnrotated(boneId);

  zeus::CQuaternion quat = parentRot * node.x4_rotation;
  zeus::CMatrix3f xf = quat;
  zeus::CVector3f xfOffset = parentXf * node.x14_offset + parentOffset;
  pose.Insert(boneId, quat, xfOffset, bindOffset);

  CSegId curBone = node.x0_child;
  while (curBone != 0) {
    const CTreeNode& node = x38_treeMap[curBone];
    RecursivelyBuild(curBone, node, pose, quat, xf, xfOffset);
    curBone = node.x1_sibling;
  }
}

void CHierarchyPoseBuilder::RecursivelyBuild(const CSegId& boneId, const CTreeNode& node, CPoseAsTransforms& pose,
                                             const zeus::CQuaternion& parentRot, const zeus::CMatrix3f& parentXf,
                                             const zeus::CVector3f& parentOffset) const {
  zeus::CQuaternion quat = parentRot * node.x4_rotation;
  if (boneId == 4 && x0_layoutDesc.GetCharLayoutInfo().GetObjectTag()->id == 1683497588) {
    zeus::CQuaternion rootq(CGraphics::g_GXModelMatrix.basis);
    printf("WTF3 %f\n", zeus::radToDeg(zeus::CEulerAngles(rootq).z()));
    printf("WTF4 %f\n", zeus::radToDeg(zeus::CEulerAngles(node.x4_rotation).z()));
  }
  if (boneId == 6 && x0_layoutDesc.GetCharLayoutInfo().GetObjectTag()->id == 1683497588)
    printf("WTF6 %f %f %f %f\n",
           float(node.x4_rotation.w()), float(node.x4_rotation.x()),
           float(node.x4_rotation.y()), float(node.x4_rotation.z()));

  float scale;
  zeus::CVector3f bindOffset;
  if (x0_layoutDesc.GetScaledLayoutDescription()) {
    const CLayoutDescription::CScaledLayoutDescription& desc = *x0_layoutDesc.GetScaledLayoutDescription();
    scale = desc.GlobalScale();
    bindOffset = desc.ScaledLayout()->GetFromRootUnrotated(boneId);
  } else {
    scale = 1.f;
    bindOffset = x0_layoutDesc.GetCharLayoutInfo()->GetFromRootUnrotated(boneId);
  }

  zeus::CMatrix3f mtxXf;
  if (scale == 1.f)
    mtxXf = quat;
  else
    mtxXf = parentXf * zeus::CMatrix3f(scale);

  zeus::CVector3f xfOffset = parentXf * node.x14_offset + parentOffset;
  pose.Insert(boneId, mtxXf, xfOffset, bindOffset);

  CSegId curBone = node.x0_child;
  while (curBone != 0) {
    const CTreeNode& node = x38_treeMap[curBone];
    RecursivelyBuild(curBone, node, pose, quat, quat, xfOffset);
    curBone = node.x1_sibling;
  }
}

void CHierarchyPoseBuilder::BuildTransform(const CSegId& boneId, zeus::CTransform& xfOut) const {
  TLockedToken<CCharLayoutInfo> layoutInfoTok;
  float scale;
  if (x0_layoutDesc.GetScaledLayoutDescription()) {
    layoutInfoTok = x0_layoutDesc.GetScaledLayoutDescription()->ScaledLayout();
    scale = x0_layoutDesc.GetScaledLayoutDescription()->GlobalScale();
  } else {
    layoutInfoTok = x0_layoutDesc.GetCharLayoutInfo();
    scale = 1.f;
  }
  const CCharLayoutInfo& layoutInfo = *layoutInfoTok.GetObj();

  u32 idCount = 0;
  CSegId buildIDs[100];
  {
    CSegId curId = boneId;
    while (curId != 2) {
      buildIDs[idCount++] = curId;
      curId = layoutInfo.GetRootNode()->GetBoneMap()[curId].x0_parentId;
    }
  }

  zeus::CQuaternion accumRot;
  zeus::CMatrix3f accumXF;
  zeus::CVector3f accumPos;
  for (CSegId* id = &buildIDs[idCount]; id != buildIDs; --id) {
    CSegId& thisId = id[-1];
    const CTreeNode& node = x38_treeMap[thisId];
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

void CHierarchyPoseBuilder::BuildNoScale(CPoseAsTransforms& pose) {
  pose.Clear();
  const CTreeNode& node = x38_treeMap[xcec_rootId];
  zeus::CQuaternion quat;
  zeus::CMatrix3f mtx;
  zeus::CVector3f vec;
  RecursivelyBuildNoScale(xcec_rootId, node, pose, quat, mtx, vec);
}

void CHierarchyPoseBuilder::Insert(const CSegId& boneId, const zeus::CQuaternion& quat) {
  CTreeNode& node = x38_treeMap[boneId];
  node.x4_rotation = quat;
}

void CHierarchyPoseBuilder::Insert(const CSegId& boneId, const zeus::CQuaternion& quat, const zeus::CVector3f& offset) {
  CTreeNode& node = x38_treeMap[boneId];
  node.x4_rotation = quat;
  node.x14_offset = offset;
}

CHierarchyPoseBuilder::CHierarchyPoseBuilder(const CLayoutDescription& layout)
: x0_layoutDesc(layout), x38_treeMap(layout.GetCharLayoutInfo()->GetSegIdList().GetList().size()) {
  TLockedToken<CCharLayoutInfo> layoutInfoTok;
  if (layout.GetScaledLayoutDescription())
    layoutInfoTok = layout.GetScaledLayoutDescription()->ScaledLayout();
  else
    layoutInfoTok = layout.GetCharLayoutInfo();
  const CCharLayoutInfo& layoutInfo = *layoutInfoTok.GetObj();

  const CSegIdList& segIDs = layoutInfo.GetSegIdList();
  for (const CSegId& id : segIDs.GetList())
    BuildIntoHierarchy(layoutInfo, id, 2);
}

} // namespace urde
