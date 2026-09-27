#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CDependencyGroup::CDependencyGroup(CInputStream& in) { ReadFromStream(in); }

void CDependencyGroup::ReadFromStream(CInputStream& in) {
  int numTags = in.ReadInt32();
  mObjectTags.reserve(numTags);

  for (int i = 0; i < numTags; ++i) {
    FourCC type = in.ReadInt32();
    CAssetId id = in.ReadInt32();
    mObjectTags.push_back(SObjectTag(type, id));
  }
}

int CDependencyGroup::GetCountForResType(FourCC type) const {
  int ret = 0;
  for (rstl::vector< SObjectTag >::const_iterator it = mObjectTags.begin();
       it != mObjectTags.end(); ++it) {
    if (it->GetType() == type) {
      ++ret;
    }
  }

  return ret;
}

const CFactoryFnReturn FDependencyGroupFactory(const SObjectTag& tag, CInputStream& in,
                                         const CVParamTransfer& xfer) {
  return rs_new CDependencyGroup(in);
}
