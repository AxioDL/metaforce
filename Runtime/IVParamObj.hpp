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
    rstl::CRefData* m_refData;
public:
    CVParamTransfer(rstl::CRefData* rd) : m_refData(rd) {m_refData->AddRef();}
    ~CVParamTransfer()
    {
        if (m_refData->DelRef() <= 0)
        {
            delete static_cast<IVParamObj*>(m_refData->GetPtr());
            delete m_refData;
        }
    }
    IVParamObj* GetObj() const {return static_cast<IVParamObj*>(m_refData->GetPtr());}
    CVParamTransfer ShareTransferRef() {return CVParamTransfer(m_refData);}

    static CVParamTransfer Null() {return CVParamTransfer(&rstl::CRefData::sNull);}
};

template<class T>
class TObjOwnerParam : public IVParamObj
{
    T m_param;
protected:
    ~TObjOwnerParam() {}
public:
    TObjOwnerParam(T&& obj) : m_param(std::move(obj)) {}
    CVParamTransfer NewTransferRef() {return CVParamTransfer(new rstl::CRefData(this));}
};

}

#endif // __RETRO_IVPARAMOBJ_HPP__
