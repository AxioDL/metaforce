#pragma once

#include "Runtime/Character/CLayoutDescription.hpp"
#include "Runtime/Character/CSegId.hpp"
#include "Runtime/Character/TSegIdMap.hpp"

#include <zeus/CQuaternion.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CCharLayoutInfo;
class CLayoutDescription;
class CPoseAsTransforms;

class CHierarchyPoseBuilder {
public:
  struct CTreeNode {
    CSegId x0_child = 0;
    CSegId x1_sibling = 0;
    zeus::CQuaternion x4_rotation;
    zeus::CVector3f x14_offset;
  };

private:
  CLayoutDescription x0_layoutDesc;
  CSegId x30_rootId;
  bool x34_hasRoot = false;
  TSegIdMap<CTreeNode> x38_treeMap;

  void BuildIntoHierarchy(const CCharLayoutInfo& layout, const CSegId& boneId, const CSegId& nullId);
  void RecursivelyBuildNoScale(const CSegId& boneId, const CTreeNode& node, CPoseAsTransforms& pose,
                               const zeus::CQuaternion& rot, const zeus::CMatrix3f& scale,
                               const zeus::CVector3f& offset) const;
  void RecursivelyBuild(const CSegId& boneId, const CTreeNode& node, CPoseAsTransforms& pose,
                        const zeus::CQuaternion& rot, const zeus::CMatrix3f& scale,
                        const zeus::CVector3f& offset) const;

public:
  explicit CHierarchyPoseBuilder(const CLayoutDescription& layout);

  const TLockedToken<CCharLayoutInfo>& CharLayoutInfo() const { return x0_layoutDesc.ScaledLayout(); }
  bool HasRoot() const { return x34_hasRoot; }
  void BuildTransform(const CSegId& boneId, zeus::CTransform& xfOut) const;
  void BuildNoScale(CPoseAsTransforms& pose);
  void Insert(const CSegId& boneId, const zeus::CQuaternion& quat);
  void Insert(const CSegId& boneId, const zeus::CQuaternion& quat, const zeus::CVector3f& offset);
  TSegIdMap<CTreeNode>& GetTreeMap() { return x38_treeMap; }
};

} // namespace metaforce
