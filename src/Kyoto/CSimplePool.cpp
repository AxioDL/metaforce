#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/IFactory.hpp"

CSimplePool::CSimplePool(IFactory& factory)
: x18_factory(factory), x1c_paramXfr(CVParamTransfer::Null()) {
  x1c_paramXfr = CVParamTransfer(rs_new TObjOwnerParam< IObjectStore* >(this));
}

CSimplePool::~CSimplePool() {
  Flush();
  if (x4_resources.size() > 0) {
    DebugDumpPool();
  }
}

void CSimplePool::ObjectUnreferenced(const SObjectTag& tag) {
  x4_resources.erase(x4_resources.find(tag));
}

CToken CSimplePool::GetObj(const SObjectTag& tag, const CVParamTransfer& xfer) {
  AUTO(it, x4_resources.find(tag));
  if (it != x4_resources.end()) {
    return CToken(it->second);
  }

  CObjectReference* ref =
      rs_new CObjectReference(*this, rstl::auto_ptr< IObj >(nullptr), tag, xfer);
  const ResourceMap::value_type item(tag, ref);
  x4_resources.insert(item);
  return CToken(ref);
}

CToken CSimplePool::GetObj(const SObjectTag& tag) { return CSimplePool::GetObj(tag, x1c_paramXfr); }

CToken CSimplePool::GetObj(const char* name) { return CSimplePool::GetObj(name, x1c_paramXfr); }

CToken CSimplePool::GetObj(const char* name, const CVParamTransfer& xfer) {
  const SObjectTag* tag = CSimplePool::GetFactory().GetResourceIdByName(name);
  return CSimplePool::GetObj(*tag, xfer);
}

bool CSimplePool::HasObject(const SObjectTag& tag) const {
  AUTO(it, x4_resources.find(tag));
  bool result = true;
  if (!(it != x4_resources.end())) {
#if NONMATCHING
    const bool canBuild = x18_factory.CanBuild(tag);
#else
    const bool canBuild = &x18_factory != nullptr && x18_factory.CanBuild(tag);
#endif
    if (!canBuild) {
      result = false;
    }
  }
  return result;
}

bool CSimplePool::ObjectIsLive(const SObjectTag& tag) const {
  AUTO(it, x4_resources.find(tag));
  if (it == x4_resources.end()) {
    return false;
  }
  return it->second->IsLoaded();
}

void CSimplePool::Flush() {}

void CSimplePool::DebugDumpPool() const {
  AUTO(it, x4_resources.begin());
  for (; it != x4_resources.end(); ++it) {
    SObjectTag::Type2Text(it->first.GetType());
  }
}

rstl::vector< SObjectTag > CSimplePool::GetReferencedTags() {
  rstl::vector< SObjectTag > tags;
  tags.reserve(x4_resources.size());
  AUTO(it, x4_resources.begin());
  AUTO(end, x4_resources.end());
  for (; it != end; ++it) {
    tags.push_back(it->first);
  }
  return tags;
}
