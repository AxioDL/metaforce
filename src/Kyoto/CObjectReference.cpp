#include "Kyoto/CObjectReference.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"

CObjectReference::CObjectReference(IObjectStore& store, const rstl::auto_ptr< IObj >& obj,
                                   const SObjectTag& tag, CVParamTransfer xfer)
: mRefCount(0)
, mLockCount(0)
, mLoading(false)
, mObjTag(tag)
, mObjectStore(&store)
, mObject(obj.release())
, mParams(xfer) {}

CObjectReference::CObjectReference(const rstl::auto_ptr< IObj >& obj)
: mRefCount(0)
, mLockCount(0)
, mLoading(false)
, mObjectStore(nullptr)
, mObject(obj.release())
, mParams(CVParamTransfer::Null()) {}

CObjectReference::~CObjectReference() {
  if (mObject) {
    delete mObject;
  } else if (mLoading) {
    mObjectStore->GetFactory().CancelBuild(mObjTag);
  }
}

void CObjectReference::Lock() {
  ++mLockCount;

  if (mObject == nullptr && !mLoading) {
    mObjectStore->GetFactory().BuildAsync(mObjTag, mParams, &mObject);
    mLoading = true;
  }
}

IObj* CObjectReference::GetObject() {
  if (mObject == nullptr) {
    mObject = mObjectStore->GetFactory().Build(mObjTag, mParams).release();
  }
  mLoading = false;
  return mObject;
}

void CObjectReference::Unload() {
  delete mObject;
  mObject = nullptr;
  mLoading = false;
}

void CObjectReference::CancelLoad() {
  if (mObjectStore == nullptr) {
    return;
  }

  if (!IsLoading()) {
    return;
  }
  mObjectStore->GetFactory().CancelBuild(mObjTag);
  mLoading = false;
}

int CObjectReference::RemoveReference() {
  --mRefCount;
  if (mRefCount == 0) {
    if (mObject != nullptr) {
      Unload();
    } else if (IsLoading()) {
      CancelLoad();
    }
    if (mObjectStore != nullptr) {
      mObjectStore->ObjectUnreferenced(mObjTag);
    }
  }

  return mRefCount;
}

void CObjectReference::Unlock() {
  --mLockCount;

  if (mLockCount != 0) {
    return;
  }

  if (mObject != nullptr && mObjectStore != nullptr) {
    Unload();
  } else if (IsLoading()) {
    CancelLoad();
  }
}

bool CObjectReference::IsLoading() const { return mLoading && mObject == nullptr; }
