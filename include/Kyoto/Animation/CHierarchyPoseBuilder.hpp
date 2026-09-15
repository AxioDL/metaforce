#ifndef _CHIERARCHYPOSEBUILDER
#define _CHIERARCHYPOSEBUILDER

#include "types.h"

#include "Kyoto/Animation/CLayoutDescription.hpp"
#include "Kyoto/Animation/CSegId.hpp"

#include "Kyoto/Animation/TSegIdMap.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/construction_deferred.hpp"

class CCharLayoutInfo;
class CPoseAsTransforms;
class CTransform4f;

class CHierarchyPoseBuilder {
public:
  CHierarchyPoseBuilder(const CLayoutDescription& layout);
  ~CHierarchyPoseBuilder() {}

  void BuildNoScale(CPoseAsTransforms& pose);
  void BuildTransform(const CSegId& seg, CTransform4f& transform) const;

  class CTreeNode {
  public:
    CTreeNode(const CSegId& sibling, const CSegId& child, const CVector3f& offset)
    : x0_child(child)
    , x1_sibling(sibling)
    , x4_rotation(CQuaternion::NoRotation())
    , x14_offset(offset) {}

    const CQuaternion& GetRotation() const { return x4_rotation; }
    const CVector3f& GetOffset() const { return x14_offset; }
    CSegId GetFirstChildSegment() const { return x0_child; }
    CSegId GetNextSiblingSegment() const { return x1_sibling; }
    void SetRotation(const CQuaternion& rot) { x4_rotation = rot; }
    void SetOffset(const CVector3f& off) { x14_offset = off; }
    CTreeNode NodeForNextChildInserted(const CSegId& child, const CSegId& nullId,
                                       const CVector3f& offset) {
      CSegId sibling = x0_child;
      x0_child = child;
      return CTreeNode(sibling, nullId, offset);
    }

  private:
    CSegId x0_child;
    CSegId x1_sibling;
    CQuaternion x4_rotation;
    CVector3f x14_offset;
  };

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  void Insert(const CSegId& id, const CQuaternion& rot);
  void Insert(const CSegId& id, const CVector3f& off);
#else
  void Insert(const CSegId& id, const CQuaternion& rot) { x38_treeMap[id].SetRotation(rot); }
  void Insert(const CSegId& id, const CVector3f& off) { x38_treeMap[id].SetOffset(off); }
#endif
  CQuaternion GetSegRotation(const CSegId& id) const { return x38_treeMap[id].GetRotation(); }
  const TLockedToken< CCharLayoutInfo >& CharLayoutInfo() const {
    return x0_layoutDesc.ScaledLayout();
  }

private:
  void BuildIntoHeirarchy(const CCharLayoutInfo& layout, const CSegId& seg, const CSegId& root);
  void RecursivelyBuildNoScale(const CSegId& seg, const CTreeNode& node, CPoseAsTransforms& pose,
                               const CQuaternion& rotation, const CMatrix3f& matrix,
                               const CVector3f& offset) const;
  void RecursivelyBuild(const CSegId& seg, const CTreeNode& node, CPoseAsTransforms& pose,
                        const CQuaternion& rotation, const CMatrix3f& matrix,
                        const CVector3f& offset) const;

  CLayoutDescription x0_layoutDesc;
  rstl::construction_deferred< CSegId > x30_rootId;
  TSegIdMap< CTreeNode > x38_treeMap;
};
CHECK_SIZEOF(CHierarchyPoseBuilder, 0x110)
NESTED_CHECK_SIZEOF(CHierarchyPoseBuilder, CTreeNode, 0x20)

#endif // _CHIERARCHYPOSEBUILDER
