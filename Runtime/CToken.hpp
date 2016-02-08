#ifndef __RETRO_CTOKEN_HPP__
#define __RETRO_CTOKEN_HPP__

#include <memory>
#include "IObj.hpp"
#include "RetroTypes.hpp"
#include "IVParamObj.hpp"
#include "IObjectStore.hpp"
#include "IFactory.hpp"

namespace Retro
{
class IObjectStore;
class IObj;

class CObjectReference
{
    friend class CToken;
    u16 x0_refCount = 0;
    u16 x2_lockCount = 0;
    bool x3_loading = false; /* Rightmost bit of lockCount */
    SObjectTag x4_objTag;
    IObjectStore* xC_objectStore = nullptr;
    IObj* x10_object = nullptr;
    CVParamTransfer x14_params;
public:
    CObjectReference(IObjectStore& objStore, std::unique_ptr<IObj>&& obj,
                     const SObjectTag& objTag, CVParamTransfer buildParams)
    : x4_objTag(objTag), xC_objectStore(&objStore),
      x10_object(obj.release()), x14_params(buildParams) {}
    CObjectReference(std::unique_ptr<IObj>&& obj)
    : x10_object(obj.release()) {}

    bool IsLoading() const {return x3_loading;}
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
    void CancelLoad()
    {
        if (xC_objectStore && IsLoading())
        {
            xC_objectStore->GetFactory().CancelBuild(x4_objTag);
            x3_loading = false;
        }
    }
    void Unload()
    {
        delete x10_object;
        x10_object = nullptr;
        x3_loading = false;
    }
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
    ~CObjectReference()
    {
        if (x10_object)
            delete x10_object;
        else if (x3_loading)
            xC_objectStore->GetFactory().CancelBuild(x4_objTag);
    }
};

class CToken
{
    CObjectReference* x0_objRef = nullptr;
    bool x4_lockHeld = false;
public:
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
    void RemoveRef()
    {
        if (x0_objRef && x0_objRef->RemoveReference() == 0)
        {
            delete x0_objRef;
            x0_objRef = nullptr;
        }
    }
    IObj* GetObj()
    {
        if (!x0_objRef)
            return nullptr;
        Lock();
        return x0_objRef->GetObject();
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
    CToken(CObjectReference* obj)
    {
        x0_objRef = obj;
        ++x0_objRef->x0_refCount;
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
    TToken(T* obj)
    : CToken(GetIObjObjectFor(std::unique_ptr<T>(obj))) {}
    TToken& operator=(T* obj) {*this = CToken(GetIObjObjectFor(obj)); return this;}
    T* GetObj() {return static_cast<TObjOwnerDerivedFromIObj<T>*>(CToken::GetObj())->GetObj();}
};

template <class T>
class TLockedToken : public TToken<T>
{
    T* m_obj;
public:
    TLockedToken(const CToken& other) : TToken<T>(other) {m_obj = TToken<T>::GetObj();}
    TLockedToken(CToken&& other) : TToken<T>(std::move(other)) {m_obj = TToken<T>::GetObj();}
    T* GetObj() {return m_obj;}
};

}

#endif // __RETRO_CTOKEN_HPP__
