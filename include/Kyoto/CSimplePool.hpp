#ifndef _CSIMPLEPOOL
#define _CSIMPLEPOOL

#include "Kyoto/CVParamTransfer.hpp"
#include "types.h"

#include "rstl/map.hpp"
#include "rstl/rc_ptr.hpp"

#include "Kyoto/CToken.hpp"
#include "Kyoto/IObjectStore.hpp"

class IFactory;
class CSimplePool : public IObjectStore {
  struct TagIdLess {
    bool operator()(const SObjectTag& a, const SObjectTag& b) const {
      return a.GetId() < b.GetId();
    }
  };
  typedef rstl::map< SObjectTag, CObjectReference*, TagIdLess > ResourceMap;

public:
  CSimplePool(IFactory& factory);
  ~CSimplePool();
  void DebugDumpPool() const;
  CToken GetObj(const SObjectTag& tag, const CVParamTransfer& xfer) override;
  CToken GetObj(const SObjectTag& tag) override;
  CToken GetObj(const char* name) override;
  CToken GetObj(const char* name, const CVParamTransfer& xfer) override;
  bool HasObject(const SObjectTag& tag) const override;
  bool ObjectIsLive(const SObjectTag& tag) const override;
  IFactory& GetFactory() const override { return mFactory; }
  void Flush() override;
  void ObjectUnreferenced(const SObjectTag& tag) override;
  rstl::vector< SObjectTag > GetReferencedTags();

private:
  ResourceMap mResources;
  IFactory& mFactory;
  CVParamTransfer mParamXfr;
};
CHECK_SIZEOF(CSimplePool, (VERSION >= VERSION_R3IJ_00 ? 0x24 : 0x20))

extern CSimplePool* gpSimplePool;

#endif // _CSIMPLEPOOL
