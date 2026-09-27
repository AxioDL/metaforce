#ifndef _TTOKEN
#define _TTOKEN

#include "types.h"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/IObj.hpp"

#include "rstl/auto_ptr.hpp"

template < typename T >
class TToken : public CToken {
public:
  TToken() {}
  TToken(const CToken& token) : CToken(token) {}
  TToken(T* obj) : CToken(TObjOwnerDerivedFromIObj< T >::GetNewDerivedObject(obj).release()) {}
  TToken(const rstl::auto_ptr< T >& obj) : CToken(GetIObjObjectFor(obj).release()) {}

  // TToken& operator=(const TToken& other) {
    
  // }

  T* GetT() { return reinterpret_cast< T* >(CToken::GetObj()->GetContents()); }
  T* operator*() { return GetT(); }
  T* operator->() { return GetT(); }

#if VERSION >= VERSION_R3IJ_00
  typedef rstl::auto_ptr< IObj > TOwnerPtr;
#else
  typedef rstl::auto_ptr< TObjOwnerDerivedFromIObj< T > > TOwnerPtr;
#endif

  static TOwnerPtr GetIObjObjectFor(const rstl::auto_ptr< T >& obj) {
    return TObjOwnerDerivedFromIObj< T >::GetNewDerivedObject(obj);
  }

  TToken< T > NonConstCopy() const { return *this; }
};

template < typename T >
class TCachedToken : public TToken< T > {
public:
  TCachedToken() {}
  TCachedToken(const CToken& token) : TToken< T >(token), mItem(nullptr) {}
  // __ct__23TCachedToken<8CTexture>FRC23TCachedToken<8CTexture>
  // __as__21TCachedToken<6CModel>FRC21TCachedToken<6CModel>

  // T* operator*() { return x8_item; }
  T* GetObject() const { return mItem; }

  bool TryCache() {
    if (mItem != nullptr) {
      return true;
    }
    if (CToken::IsLocked() && CToken::IsLoaded()) {
      mItem = TToken< T >::GetT();
      return true;
    }
    return false;
  }

  void Lock() { TToken< T >::Lock(); }

  const CToken& GetToken() const { return *this; }

  void Unlock() {
    mItem = nullptr;
    TToken< T >::Unlock();
  }

  bool IsLoaded() const { return mItem != nullptr || CToken::IsLoaded(); }

  void ForceCache() {
    if (mItem == nullptr) {
      mItem = TToken< T >::GetT();
    }
  }

private:
  T* mItem;
};

template < typename T >
class TLockedToken : public TToken< T > {
public:
  TLockedToken() {}
  TLockedToken(T* item) : TToken< T >(item), mItem(item) { CToken::Lock(); }
  TLockedToken(const CToken& token) : TToken< T >(token), mItem(TToken< T >::GetT()) {}
  TLockedToken(const TLockedToken< T >& token) : TToken< T >(token), mItem(*token) {
    CToken::Lock();
  }

  // TLockedToken& operator=(const TLockedToken< T >& token) {
  //   TToken< T >::operator=(token);
  //   x8_item = *token;
  //   return *this;
  // }

  T* operator*() const { return mItem; }
  T* operator->() const { return mItem; }

private:
  T* mItem;
};

#endif // _TTOKEN
