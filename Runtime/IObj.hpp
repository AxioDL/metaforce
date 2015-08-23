#ifndef __RETRO_IOBJ_HPP__
#define __RETRO_IOBJ_HPP__

#include "RetroTypes.hpp"

namespace Retro
{

class IObj
{
public:
    virtual ~IObj() {}
};

class TObjOwnerDerivedFromIObjUntyped : public IObj
{
protected:
    IObj* m_objPtr;
    TObjOwnerDerivedFromIObjUntyped(IObj* objPtr) : m_objPtr(objPtr) {}
};

template<class T>
class TObjOwnerDerivedFromIObj : public TObjOwnerDerivedFromIObjUntyped
{
    TObjOwnerDerivedFromIObj(T* objPtr) : TObjOwnerDerivedFromIObjUntyped(objPtr) {}
public:
    static std::unique_ptr<TObjOwnerDerivedFromIObj<T>> GetNewDerivedObject(std::unique_ptr<T>&& obj)
    {
        std::unique_ptr<T> movedObjPtr = std::move(obj);
        return std::unique_ptr<TObjOwnerDerivedFromIObj<T>>
        (new TObjOwnerDerivedFromIObj<T>(movedObjPtr.release()));
    }
    ~TObjOwnerDerivedFromIObj() {delete static_cast<T*>(m_objPtr);}
};

template<class T>
class TToken
{
public:
    static std::unique_ptr<TObjOwnerDerivedFromIObj<T>> GetIObjObjectFor(std::unique_ptr<T>&& obj)
    {
        return TObjOwnerDerivedFromIObj<T>::GetNewDerivedObject(std::move(obj));
    }
};

}

#endif // __RETRO_IOBJ_HPP__
