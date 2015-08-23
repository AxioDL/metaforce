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
    u16 x0_refCount = 0;
    bool x3_loading = false;
    SObjectTag x4_objTag;
    IObjectStore* xC_objectStore = nullptr;
    std::unique_ptr<IObj> x10_object;
    CVParamTransfer x14_params;
public:
    CObjectReference(IObjectStore& objStore, std::unique_ptr<IObj>&& obj,
                     const SObjectTag& objTag, CVParamTransfer buildParams)
        : x4_objTag(objTag), xC_objectStore(&objStore),
          x10_object(std::move(obj)), x14_params(buildParams) {}
    CObjectReference(std::unique_ptr<IObj>&& obj);

    bool IsLoading() const {return x3_loading;}
    void Unlock() {}
    void Lock() {}
    u32 RemoveReference()
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
    }
    void CancelLoad() {}
    void Unload()
    {
        x10_object.reset(nullptr);
        x3_loading = false;
    }
    IObj& GetObject()
    {
        IFactory& factory = xC_objectStore->GetFactory();
        factory.Build(x4_objTag, x14_params);
    }

};

class CToken
{
    CObjectReference& x0_objRef;
    bool x4_lockHeld = false;
public:
    void Unlock()
    {
        if (x4_lockHeld)
        {
            x0_objRef.Unlock();
            x4_lockHeld = false;
        }
    }
    void Lock()
    {
        if (!x4_lockHeld)
        {
            x0_objRef.Lock();
            x4_lockHeld = true;
        }
    }
    void RemoveRef()
    {

    }
    IObj& GetObj()
    {
        Lock();
        return x0_objRef.GetObject();
    }
    CToken& operator=(CToken&& other)
    {

    }
    ~CToken()
    {
        if (x0_objRef && x4_lockHeld)
            x0_objRef->Unlock();
    }
};

template<class T>
class TLockedToken
{
};

}

#endif // __RETRO_CTOKEN_HPP__
