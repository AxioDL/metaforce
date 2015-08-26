#ifndef __RETRO_IVPARAMOBJ_HPP__
#define __RETRO_IVPARAMOBJ_HPP__

#include <memory>
#include "IObj.hpp"

namespace Retro
{

class IVParamObj : public IObj
{
public:
    virtual ~IVParamObj() {}
};

class CVParamTransfer
{
    rstl::rc_ptr<IVParamObj> m_ref;
public:
    CVParamTransfer();
    CVParamTransfer(IVParamObj* obj) : m_ref(obj) {}
    CVParamTransfer(const CVParamTransfer& other) : m_ref(other.m_ref) {}
    IVParamObj* GetObj() const {return m_ref.get();}
    CVParamTransfer ShareTransferRef() {return CVParamTransfer(*this);}

    static CVParamTransfer Null() {return CVParamTransfer();}
};

template<class T>
class TObjOwnerParam : public IVParamObj
{
    T m_param;
protected:
    ~TObjOwnerParam() {}
public:
    TObjOwnerParam(T&& obj) : m_param(std::move(obj)) {}
};

}

#endif // __RETRO_IVPARAMOBJ_HPP__
