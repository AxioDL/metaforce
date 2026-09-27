#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/IFactory.hpp"

CSimplePool::CSimplePool(IFactory& factory)
: mFactory(factory), mParamXfr(CVParamTransfer::Null()) {
  mParamXfr = CVParamTransfer(rs_new TObjOwnerParam< IObjectStore* >(this));
}

CSimplePool::~CSimplePool() {
  Flush();
  if (mResources.size() > 0) {
    DebugDumpPool();
  }
}

void CSimplePool::ObjectUnreferenced(const SObjectTag& tag) {
  mResources.erase(mResources.find(tag));
}

CToken CSimplePool::GetObj(const SObjectTag& tag, const CVParamTransfer& xfer) {
  AUTO(it, mResources.find(tag));
  if (it != mResources.end()) {
    return CToken(it->second);
  }

  CObjectReference* ref =
      rs_new CObjectReference(*this, rstl::auto_ptr< IObj >(nullptr), tag, xfer);
  const ResourceMap::value_type item(tag, ref);
  mResources.insert(item);
  return CToken(ref);
}

CToken CSimplePool::GetObj(const SObjectTag& tag) { return CSimplePool::GetObj(tag, mParamXfr); }

CToken CSimplePool::GetObj(const char* name) { return CSimplePool::GetObj(name, mParamXfr); }

CToken CSimplePool::GetObj(const char* name, const CVParamTransfer& xfer) {
  const SObjectTag* tag = CSimplePool::GetFactory().GetResourceIdByName(name);
  return CSimplePool::GetObj(*tag, xfer);
}

bool CSimplePool::HasObject(const SObjectTag& tag) const {
  AUTO(it, mResources.find(tag));
  bool result = true;
  if (!(it != mResources.end())) {
#if NONMATCHING
    const bool canBuild = mFactory.CanBuild(tag);
#else
    const bool canBuild = &mFactory != nullptr && mFactory.CanBuild(tag);
#endif
    if (!canBuild) {
      result = false;
    }
  }
  return result;
}

bool CSimplePool::ObjectIsLive(const SObjectTag& tag) const {
  AUTO(it, mResources.find(tag));
  if (it == mResources.end()) {
    return false;
  }
  return it->second->IsLoaded();
}

void CSimplePool::Flush() {}

void CSimplePool::DebugDumpPool() const {
  AUTO(it, mResources.begin());
  for (; it != mResources.end(); ++it) {
    SObjectTag::Type2Text(it->first.GetType());
  }
}

rstl::vector< SObjectTag > CSimplePool::GetReferencedTags() {
  rstl::vector< SObjectTag > tags;
  tags.reserve(mResources.size());
  AUTO(it, mResources.begin());
  AUTO(end, mResources.end());
  for (; it != end; ++it) {
    tags.push_back(it->first);
  }
  return tags;
}
