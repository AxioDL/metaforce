#include "Kyoto/Animation/CHierarchyPoseBuilder.hpp"

#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CPoseAsTransforms.hpp"
#include "rstl/reserved_vector.hpp"

CHierarchyPoseBuilder::CHierarchyPoseBuilder(const CLayoutDescription& layout)
: x0_layoutDesc(layout), x38_treeMap(layout.GetNumSegments()) {
  TToken< CCharLayoutInfo > layoutToken = layout.ScaledLayout();
  const CCharLayoutInfo& layoutInfo = **layoutToken;
  const CSegIdList& segments = layoutInfo.GetBodyPartSegIds();
  for (CSegIdList::const_iterator it = segments.begin(), end = segments.end(); it != end; ++it) {
    CSegId seg = *it;
    BuildIntoHeirarchy(layoutInfo, seg, CSegId::Character());
  }
}

void CHierarchyPoseBuilder::BuildIntoHeirarchy(const CCharLayoutInfo& layout, const CSegId& seg,
                                               const CSegId& root) {
  const CSegId id = seg;
  if (!x38_treeMap.ContainsDataFor(id)) {
    CSegId parent = layout.GetOriginalParent(seg);
    if (parent == root) {
      x30_rootId.build(seg);
      x38_treeMap.insert(seg, CTreeNode(CSegId::Null(), CSegId::Null(),
                                     layout.GetFromParentUnrotated(seg)));
    } else {
      BuildIntoHeirarchy(layout, parent, root);
      x38_treeMap.insert(seg, x38_treeMap[parent].NodeForNextChildInserted(
                                  seg, CSegId::Null(), layout.GetFromParentUnrotated(seg)));
    }
  }
}

void CHierarchyPoseBuilder::BuildNoScale(CPoseAsTransforms& pose) {
  CSegId root = *x30_rootId;
  pose.Clear();
  RecursivelyBuildNoScale(root, x38_treeMap[root], pose, CQuaternion::NoRotation(),
                          CMatrix3f::Identity(), CVector3f::Zero());
}

void CHierarchyPoseBuilder::RecursivelyBuild(const CSegId& seg, const CTreeNode& node,
                                             CPoseAsTransforms& pose, const CQuaternion& rotation,
                                             const CMatrix3f& matrix,
                                             const CVector3f& offset) const {
  const CQuaternion& nodeRotation = node.GetRotation();
  CQuaternion childRotation = rotation * nodeRotation;
  float scale = x0_layoutDesc.GlobalScale();
  CMatrix3f childMatrix = scale == 1.f
                              ? childRotation.BuildTransform()
                              : matrix * (nodeRotation.BuildTransform() * CMatrix3f::Scale(scale));
  CVector3f childOffset = offset + matrix * node.GetOffset();
  pose.Insert(seg, childMatrix, childOffset);

  CSegId child = node.GetFirstChildSegment();
  while (child != CSegId::Null()) {
    const CSegId childId = child;
    const CTreeNode& childNode = x38_treeMap[childId];
    RecursivelyBuild(childId, childNode, pose, childRotation, childRotation.BuildTransform(),
                     childOffset);
    child = childNode.GetNextSiblingSegment();
  }
}

void CHierarchyPoseBuilder::RecursivelyBuildNoScale(const CSegId& seg, const CTreeNode& node,
                                                    CPoseAsTransforms& pose,
                                                    const CQuaternion& rotation,
                                                    const CMatrix3f& matrix,
                                                    const CVector3f& offset) const {
  CQuaternion childRotation = rotation * node.GetRotation();
  CMatrix3f childMatrix = childRotation.BuildTransform();
  CVector3f childOffset = offset + matrix * node.GetOffset();
  pose.Insert(seg, childMatrix, childOffset);

  CSegId child = node.GetFirstChildSegment();
  while (child != CSegId::Null()) {
    const CSegId childId = child;
    const CTreeNode& childNode = x38_treeMap[childId];
    RecursivelyBuild(childId, childNode, pose, childRotation, childMatrix, childOffset);
    child = childNode.GetNextSiblingSegment();
  }
}

void CHierarchyPoseBuilder::BuildTransform(const CSegId& seg, CTransform4f& transform) const {
  rstl::reserved_vector< CSegId, 100 > segments;
  CSegId current = seg;
  const CCharLayoutInfo& layout = **CharLayoutInfo();
  float scale = x0_layoutDesc.GlobalScale();
  while (current != CSegId::Character()) {
    segments.push_back(current);
    current = layout.GetOriginalParent(current);
  }

  CQuaternion rotation = CQuaternion::NoRotation();
  CVector3f offset(0.f, 0.f, 0.f);
  CMatrix3f matrix = CMatrix3f::Identity();
  for (AUTO(it, segments.end()); it != segments.begin(); --it) {
    const CTreeNode& node = x38_treeMap[*(it - 1)];
    const CQuaternion& nodeRotation = node.GetRotation();
    rotation = rotation * nodeRotation;
    offset += matrix * node.GetOffset();
    matrix = scale == 1.f ? rotation.BuildTransform()
                          : matrix * (nodeRotation.BuildTransform() * CMatrix3f::Scale(scale));
  }
  transform.SetRotation(matrix);
  transform.SetTranslation(offset);
}

uchar CLayoutDescription::GetNumSegments() const {
  return x0_layoutToken->GetBodyPartSegIds().size();
}
