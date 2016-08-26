#ifndef __URDE_CTOKEN_HPP__
#define __URDE_CTOKEN_HPP__

#include <memory>
#include "IObj.hpp"
#include "RetroTypes.hpp"
#include "IVParamObj.hpp"
#include "IObjectStore.hpp"
#include "IFactory.hpp"

namespace urde
{
class IObjectStore;

/** Shared data-structure for CToken references, analogous to std::shared_ptr */
class CObjectReference
{
    friend class CToken;
    friend class CSimplePool;
    u16 x0_refCount = 0;
    u16 x2_lockCount = 0;
    bool x3_loading = false; /* Rightmost bit of lockCount */
    SObjectTag x4_objTag;
    IObjectStore* xC_objectStore = nullptr;
    IObj* x10_object = nullptr;
    CVParamTransfer x14_params;

    /** Mechanism by which CToken decrements 1st ref-count, indicating CToken invalidation or reset.
     *  Reaching 0 indicates the CToken should delete the CObjectReference */
    u16 RemoveReference()
    {
        --x0_refCount;
        if (x0_refCount == 0)
        {
            if (x10_object)
                Unload();
            if (IsLoading())
                CancelLoad();
            xC_objectStore->ObjectUnreferenced(x4_objTag);
        }
        return x0_refCount;
    }

    CObjectReference(IObjectStore& objStore, std::unique_ptr<IObj>&& obj,
                     const SObjectTag& objTag, CVParamTransfer buildParams)
    : x4_objTag(objTag), xC_objectStore(&objStore),
      x10_object(obj.release()), x14_params(buildParams) {}
    CObjectReference(std::unique_ptr<IObj>&& obj)
    : x10_object(obj.release()) {}

    /** Indicates an asynchronous load transaction has been submitted and is not yet finished */
    bool IsLoading() const {return x3_loading;}

    /** Indicates an asynchronous load transaction has finished and object is completely loaded */
    bool IsLoaded() const {return x10_object != nullptr;}

    /** Decrements 2nd ref-count, performing unload or async-load-cancel if 0 reached */
    void Unlock()
    {
        --x2_lockCount;
        if (x2_lockCount)
            return;
        if (x10_object && xC_objectStore)
            Unload();
        else if (IsLoading())
            CancelLoad();
    }

    /** Increments 2nd ref-count, performing async-factory-load if needed */
    void Lock()
    {
        ++x2_lockCount;
        if (!x10_object && !x3_loading)
        {
            IFactory& fac = xC_objectStore->GetFactory();
            fac.BuildAsync(x4_objTag, x14_params, &x10_object);
            x3_loading = true;
        }
    }

    void CancelLoad()
    {
        if (xC_objectStore && IsLoading())
        {
            xC_objectStore->GetFactory().CancelBuild(x4_objTag);
            x3_loading = false;
        }
    }

    /** Pointer-synchronized object-destructor, another building Lock cycle may be performed after */
    void Unload()
    {
        std::default_delete<IObj>()(x10_object);
        x10_object = nullptr;
        x3_loading = false;
    }

    /** Synchronous object-fetch, guaranteed to return complete object on-demand, blocking build if not ready */
    IObj* GetObject()
    {
        if (!x10_object)
        {
            IFactory& factory = xC_objectStore->GetFactory();
            x10_object = factory.Build(x4_objTag, x14_params).release();
        }
        x3_loading = false;
        return x10_object;
    }
    const SObjectTag& GetObjectTag() const
    {
        return x4_objTag;
    }

public:
    ~CObjectReference()
    {
        if (x10_object)
            std::default_delete<IObj>()(x10_object);
        else if (x3_loading)
            xC_objectStore->GetFactory().CancelBuild(x4_objTag);
    }
};

/** Counted meta-object, reference-counting against a shared CObjectReference
 *  This class is analogous to std::shared_ptr and C++11 rvalues have been implemented accordingly
 *  (default/empty constructor, move constructor/assign) */
class CToken
{
    friend class CSimplePool;
    CObjectReference* x0_objRef = nullptr;
    bool x4_lockHeld = false;

    void RemoveRef()
    {
        if (x0_objRef && x0_objRef->RemoveReference() == 0)
        {
            std::default_delete<CObjectReference>()(x0_objRef);
            x0_objRef = nullptr;
        }
    }

    CToken(CObjectReference* obj)
    {
        x0_objRef = obj;
        ++x0_objRef->x0_refCount;
    }

public:
    /* Added to test for non-null state */
    operator bool() const {return x0_objRef != nullptr;}
    void Unlock()
    {
        if (x0_objRef && x4_lockHeld)
        {
            x0_objRef->Unlock();
            x4_lockHeld = false;
        }
    }
    void Lock()
    {
        if (x0_objRef && !x4_lockHeld)
        {
            x0_objRef->Lock();
            x4_lockHeld = true;
        }
    }
    bool IsLocked() const {return x4_lockHeld;}
    bool IsLoaded() const
    {
        if (!x0_objRef)
            return false;
        return x0_objRef->IsLoaded();
    }
    IObj* GetObj()
    {
        if (!x0_objRef)
            return nullptr;
        Lock();
        return x0_objRef->GetObject();
    }
    const IObj* GetObj() const
    {
        return ((CToken*)this)->GetObj();
    }
    CToken& operator=(const CToken& other)
    {
        Unlock();
        RemoveRef();
        x0_objRef = other.x0_objRef;
        if (x0_objRef)
        {
            ++x0_objRef->x0_refCount;
            if (other.x4_lockHeld)
                Lock();
        }
        return *this;
    }
    CToken& operator=(CToken&& other)
    {
        Unlock();
        RemoveRef();
        x0_objRef = other.x0_objRef;
        other.x0_objRef = nullptr;
        x4_lockHeld = other.x4_lockHeld;
        other.x4_lockHeld = false;
        return *this;
    }
    CToken() = default;
    CToken(const CToken& other)
    : x0_objRef(other.x0_objRef)
    {
        if (x0_objRef)
            ++x0_objRef->x0_refCount;
    }
    CToken(CToken&& other)
    : x0_objRef(other.x0_objRef), x4_lockHeld(other.x4_lockHeld)
    {
        other.x0_objRef = nullptr;
        other.x4_lockHeld = false;
    }
    CToken(IObj* obj)
    {
        x0_objRef = new CObjectReference(std::unique_ptr<IObj>(obj));
        ++x0_objRef->x0_refCount;
        Lock();
    }
    CToken(std::unique_ptr<IObj>&& obj)
    {
        x0_objRef = new CObjectReference(std::move(obj));
        ++x0_objRef->x0_refCount;
        Lock();
    }
    const SObjectTag* GetObjectTag() const
    {
        if (!x0_objRef)
            return nullptr;
        return &x0_objRef->GetObjectTag();
    }
    ~CToken()
    {
        if (x0_objRef)
        {
            if (x4_lockHeld)
                x0_objRef->Unlock();
            RemoveRef();
        }
    }
};

template <class T>
class TToken : public CToken
{
public:
    static std::unique_ptr<TObjOwnerDerivedFromIObj<T>> GetIObjObjectFor(std::unique_ptr<T>&& obj)
    {
        return TObjOwnerDerivedFromIObj<T>::GetNewDerivedObject(std::move(obj));
    }
    TToken() = default;
    TToken(const CToken& other) : CToken(other) {}
    TToken(CToken&& other) : CToken(std::move(other)) {}
    TToken(std::unique_ptr<T>&& obj) : CToken(GetIObjObjectFor(std::move(obj))) {}
    TToken& operator=(std::unique_ptr<T>&& obj)
    {
        *this = CToken(GetIObjObjectFor(std::move(obj)));
        return this;
    }
    T* GetObj()
    {
        TObjOwnerDerivedFromIObj<T>* owner = static_cast<TObjOwnerDerivedFromIObj<T>*>(CToken::GetObj());
        if (owner)
            return owner->GetObj();
        return nullptr;
    }
    const T* GetObj() const
    {
        return ((TToken<T>*)this)->GetObj();
    }
    T* operator->() {return GetObj();}
    const T* operator->() const {return GetObj();}
    T& operator*() {return *GetObj();}
    const T& operator*() const {return *GetObj();}
};

template <class T>
class TCachedToken : public TToken<T>
{
protected:
    T* m_obj = nullptr;
public:
    TCachedToken() = default;
    TCachedToken(const CToken& other) : TToken<T>(other) {}
    TCachedToken(CToken&& other) : TToken<T>(std::move(other)) {}
    T* GetObj()
    {
        if (!m_obj)
            m_obj = TToken<T>::GetObj();
        return m_obj;
    }
    const T* GetObj() const
    {
        return ((TCachedToken<T>*)this)->GetObj();
    }
    T* operator->() {return GetObj();}
    const T* operator->() const {return GetObj();}
    void Unlock() {TToken<T>::Unlock(); m_obj = nullptr;}

    TCachedToken& operator=(const TCachedToken& other) { CToken::operator=(other); m_obj = nullptr; return *this; }
    TCachedToken& operator=(const CToken& other) { CToken::operator=(other); m_obj = nullptr; return *this; }
};

template <class T>
class TLockedToken : public TCachedToken<T>
{
public:
    TLockedToken() = default;
    TLockedToken(const TLockedToken& other) : TCachedToken<T>(other) { CToken::Lock(); }
    TLockedToken& operator=(const TLockedToken& other) { TCachedToken<T>::operator=(other); CToken::Lock(); return *this; }
    TLockedToken(const CToken& other) : TCachedToken<T>(other) { CToken::Lock(); }
    TLockedToken& operator=(const CToken& other) { TCachedToken<T>::operator=(other); CToken::Lock(); return *this; }
    TLockedToken(CToken&& other) : TCachedToken<T>(std::move(other)) { CToken::Lock(); }
};

}

#endif // __URDE_CTOKEN_HPP__
