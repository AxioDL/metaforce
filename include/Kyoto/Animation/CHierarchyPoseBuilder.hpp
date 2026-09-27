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
    : mChild(child)
    , mSibling(sibling)
    , mRotation(CQuaternion::NoRotation())
    , mOffset(offset) {}

    const CQuaternion& GetRotation() const { return mRotation; }
    const CVector3f& GetOffset() const { return mOffset; }
    CSegId GetFirstChildSegment() const { return mChild; }
    CSegId GetNextSiblingSegment() const { return mSibling; }
    void SetRotation(const CQuaternion& rot) { mRotation = rot; }
    void SetOffset(const CVector3f& off) { mOffset = off; }
    CTreeNode NodeForNextChildInserted(const CSegId& child, const CSegId& nullId,
                                       const CVector3f& offset) {
      CSegId sibling = mChild;
      mChild = child;
      return CTreeNode(sibling, nullId, offset);
    }

  private:
    CSegId mChild;
    CSegId mSibling;
    CQuaternion mRotation;
    CVector3f mOffset;
  };

#if VERSION >= VERSION_GM8P_00
  void Insert(const CSegId& id, const CQuaternion& rot);
  void Insert(const CSegId& id, const CVector3f& off);
#else
  void Insert(const CSegId& id, const CQuaternion& rot) { mTreeMap[id].SetRotation(rot); }
  void Insert(const CSegId& id, const CVector3f& off) { mTreeMap[id].SetOffset(off); }
#endif
  CQuaternion GetSegRotation(const CSegId& id) const { return mTreeMap[id].GetRotation(); }
  const TLockedToken< CCharLayoutInfo >& CharLayoutInfo() const {
    return mLayoutDesc.ScaledLayout();
  }

private:
  void BuildIntoHeirarchy(const CCharLayoutInfo& layout, const CSegId& seg, const CSegId& root);
  void RecursivelyBuildNoScale(const CSegId& seg, const CTreeNode& node, CPoseAsTransforms& pose,
                               const CQuaternion& rotation, const CMatrix3f& matrix,
                               const CVector3f& offset) const;
  void RecursivelyBuild(const CSegId& seg, const CTreeNode& node, CPoseAsTransforms& pose,
                        const CQuaternion& rotation, const CMatrix3f& matrix,
                        const CVector3f& offset) const;

  CLayoutDescription mLayoutDesc;
  rstl::construction_deferred< CSegId > mRootId;
  TSegIdMap< CTreeNode > mTreeMap;
};
CHECK_SIZEOF(CHierarchyPoseBuilder, 0x110)
NESTED_CHECK_SIZEOF(CHierarchyPoseBuilder, CTreeNode, 0x20)

#endif // _CHIERARCHYPOSEBUILDER
