#ifndef _CDEPENDENCYGROUP
#define _CDEPENDENCYGROUP

#include "Kyoto/SObjectTag.hpp"

#include "rstl/vector.hpp"

class CInputStream;

class CDependencyGroup {
public:
  explicit CDependencyGroup(CInputStream& in);
  ~CDependencyGroup() {}
  void ReadFromStream(CInputStream& in);
  int GetCountForResType(FourCC type) const;

  const rstl::vector< SObjectTag >& GetObjectTagVector() const { return mObjectTags; }

private:
  rstl::vector< SObjectTag > mObjectTags;
};

#endif // _CDEPENDENCYGROUP
