#include "CToken.hpp"

namespace urde {
u16 CObjectReference::RemoveReference() {
  --x0_refCount;
  if (x0_refCount == 0) {
    if (x10_object)
      Unload();
    if (IsLoading())
      CancelLoad();
    if (xC_objectStore)
      xC_objectStore->ObjectUnreferenced(x4_objTag);
  }
  return x0_refCount;
}

CObjectReference::CObjectReference(IObjectStore& objStore, std::unique_ptr<IObj>&& obj, const SObjectTag& objTag,
                                   CVParamTransfer buildParams)
: x4_objTag(objTag), xC_objectStore(&objStore), x10_object(std::move(obj)), x14_params(std::move(buildParams)) {}
CObjectReference::CObjectReference(std::unique_ptr<IObj>&& obj) : x10_object(std::move(obj)) {}

void CObjectReference::Unlock() {
  --x2_lockCount;
  if (x2_lockCount)
    return;
  if (x10_object && xC_objectStore)
    Unload();
  else if (IsLoading())
    CancelLoad();
}

void CObjectReference::Lock() {
  ++x2_lockCount;
  if (!x10_object && !x3_loading) {
    IFactory& fac = xC_objectStore->GetFactory();
    fac.BuildAsync(x4_objTag, x14_params, &x10_object, this);
    x3_loading = !x10_object.operator bool();
  }
}

void CObjectReference::CancelLoad() {
  if (xC_objectStore && IsLoading()) {
    xC_objectStore->GetFactory().CancelBuild(x4_objTag);
    x3_loading = false;
  }
}

void CObjectReference::Unload() {
  x10_object.reset();
  x3_loading = false;
}

IObj* CObjectReference::GetObject() {
  if (!x10_object) {
    IFactory& factory = xC_objectStore->GetFactory();
    x10_object = factory.Build(x4_objTag, x14_params, this);
  }
  x3_loading = false;
  return x10_object.get();
}

CObjectReference::~CObjectReference() {
  if (x10_object)
    x10_object.reset();
  else if (x3_loading)
    xC_objectStore->GetFactory().CancelBuild(x4_objTag);
}

void CToken::RemoveRef() {
  if (x0_objRef && x0_objRef->RemoveReference() == 0) {
    std::default_delete<CObjectReference>()(x0_objRef);
    x0_objRef = nullptr;
  }
}

CToken::CToken(CObjectReference* obj) {
  x0_objRef = obj;
  ++x0_objRef->x0_refCount;
}

void CToken::Unlock() {
  if (x0_objRef && x4_lockHeld) {
    x0_objRef->Unlock();
    x4_lockHeld = false;
  }
}
void CToken::Lock() {
  if (x0_objRef && !x4_lockHeld) {
    x0_objRef->Lock();
    x4_lockHeld = true;
  }
}
bool CToken::IsLoaded() const {
  if (!x0_objRef || !x4_lockHeld)
    return false;
  return x0_objRef->IsLoaded();
}
IObj* CToken::GetObj() {
  if (!x0_objRef)
    return nullptr;
  Lock();
  return x0_objRef->GetObject();
}
CToken& CToken::operator=(const CToken& other) {
  Unlock();
  RemoveRef();
  x0_objRef = other.x0_objRef;
  if (x0_objRef) {
    ++x0_objRef->x0_refCount;
    if (other.x4_lockHeld)
      Lock();
  }
  return *this;
}
CToken& CToken::operator=(CToken&& other) {
  Unlock();
  RemoveRef();
  x0_objRef = other.x0_objRef;
  other.x0_objRef = nullptr;
  x4_lockHeld = other.x4_lockHeld;
  other.x4_lockHeld = false;
  return *this;
}
CToken::CToken(const CToken& other) : x0_objRef(other.x0_objRef) {
  if (x0_objRef) {
    ++x0_objRef->x0_refCount;
    if (other.x4_lockHeld)
      Lock();
  }
}
CToken::CToken(CToken&& other) noexcept : x0_objRef(other.x0_objRef), x4_lockHeld(other.x4_lockHeld) {
  other.x0_objRef = nullptr;
  other.x4_lockHeld = false;
}
CToken::CToken(IObj* obj) {
  x0_objRef = new CObjectReference(std::unique_ptr<IObj>(obj));
  ++x0_objRef->x0_refCount;
  Lock();
}
CToken::CToken(std::unique_ptr<IObj>&& obj) {
  x0_objRef = new CObjectReference(std::move(obj));
  ++x0_objRef->x0_refCount;
  Lock();
}
const SObjectTag* CToken::GetObjectTag() const {
  if (!x0_objRef)
    return nullptr;
  return &x0_objRef->GetObjectTag();
}
CToken::~CToken() {
  if (x0_objRef) {
    if (x4_lockHeld)
      x0_objRef->Unlock();
    RemoveRef();
  }
}

} // namespace urde
