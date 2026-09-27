#ifndef _CTOKEN
#define _CTOKEN

#include "types.h"

#include "Kyoto/CObjectReference.hpp"
#include "Kyoto/IObj.hpp"

class CObjectReference;

class CToken {
public:
  CToken() : mObjRef(NULL), mLockHeld_init(0) {}
  CToken(CObjectReference* ref);
  CToken(IObj* obj); // : x0_objRef(new CObjectReference(obj)), x4_lockHeld(false) {}
  CToken(const CToken& other);
  ~CToken();

  CObjOwnerDerivedFromIObjUntyped* GetObj();
  void Lock();
  void Unlock();
  bool IsLocked() const { return mLockHeld; }
  bool IsLoaded() const { return mObjRef->IsLoaded(); }
  void RemoveRef();
  CToken& operator=(const CToken&);
  bool HasReference() const { return mObjRef != nullptr; }
  const SObjectTag& GetTag() const { return mObjRef->GetTag(); }
  FourCC GetReferenceType() const { return GetTag().GetType(); }

  const CObjectReference* GetRef() const { return mObjRef; }

private:
  CObjectReference* mObjRef;
  union {
    bool mLockHeld;
    int mLockHeld_init;
  };
};

#endif // _CTOKEN
