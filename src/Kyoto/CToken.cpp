#include "Kyoto/CToken.hpp"
#include "Kyoto/Alloc/CMemory.hpp"

CToken::CToken(CObjectReference* ref) : mObjRef(ref), mLockHeld(false) {
  mObjRef->AddReference();
}

CToken::CToken(IObj* obj)
: mObjRef(rs_new CObjectReference(obj)), mLockHeld(false) {
  mObjRef->AddReference();
  Lock();
}

CToken::CToken(const CToken& other) : mObjRef(other.mObjRef), mLockHeld(false) {
  mObjRef->AddReference();
  if (other.mLockHeld) {
    Lock();
  }
}

CToken::~CToken() {
  if (mLockHeld) {
    mObjRef->Unlock();
  }
  RemoveRef();
}

CObjOwnerDerivedFromIObjUntyped* CToken::GetObj() {
  Lock();

  return reinterpret_cast< CObjOwnerDerivedFromIObjUntyped* >(mObjRef->GetObject());
}

void CToken::RemoveRef() {
  if (mObjRef->RemoveReference() == 0) {
    delete mObjRef;
  }
}

void CToken::Lock() {
  if (!mLockHeld) {
    mObjRef->Lock();
    mLockHeld = true;
  }
}

void CToken::Unlock() {
  if (mLockHeld == 0) {
    return;
  }

  mObjRef->Unlock();
  mLockHeld = false;
}

CToken& CToken::operator=(const CToken& other) {
  if (&other == this) {
    return *this;
  }

  Unlock();
  RemoveRef();
  mObjRef = other.mObjRef;
  mObjRef->AddReference();
  if (other.mLockHeld) {
    Lock();
  }

  return *this;
}
