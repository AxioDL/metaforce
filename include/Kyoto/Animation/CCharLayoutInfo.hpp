#ifndef _CCHARLAYOUTINFO
#define _CCHARLAYOUTINFO

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/TSegIdMap.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/object_owner.hpp"
#include <rstl/auto_ptr.hpp>
#include <rstl/map.hpp>
#include <rstl/string.hpp>

class CCharLayoutNode {
public:
  CCharLayoutNode(CInputStream& in);
  ~CCharLayoutNode() {}
  CSegId GetParent() const { return mParent; }
  const CVector3f& GetReferenceStanceOffset() const { return mReferenceStanceOffset; }
  int GetNumConnectedParts() const { return mConnectedParts.size(); }

private:
  CSegId mParent;
  CVector3f mReferenceStanceOffset;
  rstl::vector< CSegId > mConnectedParts;
};
#if VERSION < VERSION_R3IJ_00
CHECK_SIZEOF(CCharLayoutNode, 0x20)
#endif

class CCharLayoutInfo {
public:
  CCharLayoutInfo(CInputStream& in);
  CSegId GetSegIdFromString(const rstl::string& bone) const;

  const CSegIdList& GetBodyPartSegIds() const { return mSegIdList; }
  const CVector3f& GetReferenceStanceOffset(const CSegId& id) const {
    return id != CSegId::Root() ? GetSegmentData(id).GetReferenceStanceOffset() : CVector3f::Zero();
  }

  CVector3f GetFromParentUnrotated(const CSegId& id) const {
    const CCharLayoutNode& node = GetSegmentData(id);
    return !mNodes->ContainsDataFor(node.GetParent())
               ? node.GetReferenceStanceOffset()
               : node.GetReferenceStanceOffset() -
                     GetSegmentData(node.GetParent()).GetReferenceStanceOffset();
  }
  const CCharLayoutNode& GetSegmentData(const CSegId& seg) const { return (*mNodes)[seg]; }
  CSegId GetOriginalParent(const CSegId& seg) const { return GetSegmentData(seg).GetParent(); }

private:
  rstl::object_owner< TSegIdMap< CCharLayoutNode > > mNodes;
  CSegIdList mSegIdList;
  rstl::map< rstl::string, CSegId > mNameMap;
};
#if VERSION < VERSION_R3IJ_00
CHECK_SIZEOF(CCharLayoutInfo, 0x2c)
#endif

#endif // _CCHARLAYOUTINFO
