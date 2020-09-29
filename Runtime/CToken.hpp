#pragma once

#include <memory>

#include "Runtime/IFactory.hpp"
#include "Runtime/IObj.hpp"
#include "Runtime/IObjectStore.hpp"
#include "Runtime/IVParamObj.hpp"
#include "Runtime/RetroTypes.hpp"

namespace urde {
class IObjectStore;

/** Shared data-structure for CToken references, analogous to std::shared_ptr */
class CObjectReference {
  friend class CSimplePool;
  friend class CToken;

  u16 x0_refCount = 0;
  u16 x2_lockCount = 0;
  bool x3_loading = false; /* Rightmost bit of lockCount */
  SObjectTag x4_objTag;
  IObjectStore* xC_objectStore = nullptr;
  std::unique_ptr<IObj> x10_object;
  CVParamTransfer x14_params;

  /** Mechanism by which CToken decrements 1st ref-count, indicating CToken invalidation or reset.
   *  Reaching 0 indicates the CToken should delete the CObjectReference */
  u16 RemoveReference();

  CObjectReference(IObjectStore& objStore, std::unique_ptr<IObj>&& obj, const SObjectTag& objTag,
                   CVParamTransfer buildParams);
  CObjectReference(std::unique_ptr<IObj>&& obj);

  /** Indicates an asynchronous load transaction has been submitted and is not yet finished */
  bool IsLoading() const { return x3_loading; }

  /** Indicates an asynchronous load transaction has finished and object is completely loaded */
  bool IsLoaded() const { return x10_object.operator bool(); }

  /** Decrements 2nd ref-count, performing unload or async-load-cancel if 0 reached */
  void Unlock();

  /** Increments 2nd ref-count, performing async-factory-load if needed */
  void Lock();

  void CancelLoad();

  /** Pointer-synchronized object-destructor, another building Lock cycle may be performed after */
  void Unload();

  /** Synchronous object-fetch, guaranteed to return complete object on-demand, blocking build if not ready */
  IObj* GetObject();

public:
  const SObjectTag& GetObjectTag() const { return x4_objTag; }

  ~CObjectReference();
};

/** Counted meta-object, reference-counting against a shared CObjectReference
 *  This class is analogous to std::shared_ptr and C++11 rvalues have been implemented accordingly
 *  (default/empty constructor, move constructor/assign) */
class CToken {
  friend class CModel;
  friend class CSimplePool;

  CObjectReference* x0_objRef = nullptr;
  bool x4_lockHeld = false;

  void RemoveRef();

  CToken(CObjectReference* obj);

public:
  /* Added to test for non-null state */
  explicit operator bool() const { return HasReference(); }
  bool HasReference() const { return x0_objRef != nullptr; }

  void Unlock();
  void Lock();
  bool IsLocked() const { return x4_lockHeld; }
  bool IsLoaded() const;
  IObj* GetObj();
  const IObj* GetObj() const { return const_cast<CToken*>(this)->GetObj(); }
  CToken& operator=(const CToken& other);
  CToken& operator=(CToken&& other) noexcept;
  CToken() = default;
  CToken(const CToken& other);
  CToken(CToken&& other) noexcept;
  CToken(IObj* obj);
  CToken(std::unique_ptr<IObj>&& obj);
  const SObjectTag* GetObjectTag() const;
  ~CToken();
};

template <class T>
class TToken : public CToken {
public:
  static std::unique_ptr<TObjOwnerDerivedFromIObj<T>> GetIObjObjectFor(std::unique_ptr<T>&& obj) {
    return TObjOwnerDerivedFromIObj<T>::GetNewDerivedObject(std::move(obj));
  }
  TToken() = default;
  virtual ~TToken() = default;
  TToken(const CToken& other) : CToken(other) {}
  TToken(CToken&& other) : CToken(std::move(other)) {}
  TToken(std::unique_ptr<T>&& obj) : CToken(GetIObjObjectFor(std::move(obj))) {}
  TToken& operator=(std::unique_ptr<T>&& obj) {
    *this = CToken(GetIObjObjectFor(std::move(obj)));
    return *this;
  }
  virtual void Unlock() { CToken::Unlock(); }
  virtual void Lock() { CToken::Lock(); }
  virtual T* GetObj() {
    TObjOwnerDerivedFromIObj<T>* owner = static_cast<TObjOwnerDerivedFromIObj<T>*>(CToken::GetObj());
    if (owner)
      return owner->GetObj();
    return nullptr;
  }
  virtual const T* GetObj() const { return const_cast<TToken<T>*>(this)->GetObj(); }
  virtual TToken& operator=(const CToken& other) {
    CToken::operator=(other);
    return *this;
  }
  T* operator->() { return GetObj(); }
  const T* operator->() const { return GetObj(); }
  T& operator*() { return *GetObj(); }
  const T& operator*() const { return *GetObj(); }
};

template <class T>
class TCachedToken : public TToken<T> {
protected:
  T* m_obj = nullptr;

public:
  TCachedToken() = default;
  TCachedToken(const CToken& other) : TToken<T>(other) {}
  TCachedToken(CToken&& other) : TToken<T>(std::move(other)) {}
  T* GetObj() override {
    if (!m_obj)
      m_obj = TToken<T>::GetObj();
    return m_obj;
  }
  const T* GetObj() const override { return const_cast<TCachedToken<T>*>(this)->GetObj(); }
  void Unlock() override {
    TToken<T>::Unlock();
    m_obj = nullptr;
  }

  TCachedToken& operator=(const TCachedToken& other) {
    TToken<T>::operator=(other);
    m_obj = nullptr;
    return *this;
  }
  TCachedToken& operator=(const CToken& other) override {
    TToken<T>::operator=(other);
    m_obj = nullptr;
    return *this;
  }
};

template <class T>
class TLockedToken : public TCachedToken<T> {
public:
  TLockedToken() = default;
  TLockedToken(const TLockedToken& other) : TCachedToken<T>(other) { CToken::Lock(); }
  TLockedToken& operator=(const TLockedToken& other) {
    CToken oldTok = std::move(*this);
    TCachedToken<T>::operator=(other);
    CToken::Lock();
    return *this;
  }
  TLockedToken(const CToken& other) : TCachedToken<T>(other) { CToken::Lock(); }
  TLockedToken& operator=(const CToken& other) override {
    CToken oldTok = std::move(*this);
    TCachedToken<T>::operator=(other);
    CToken::Lock();
    return *this;
  }
  TLockedToken(CToken&& other) {
    CToken oldTok = std::move(*this);
    *this = TCachedToken<T>(std::move(other));
    CToken::Lock();
  }
};

} // namespace urde
