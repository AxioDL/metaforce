#pragma once

#include <string_view>

namespace urde
{
struct SObjectTag;
class CVParamTransfer;
class IFactory;
class CToken;

class IObjectStore
{
public:
    virtual ~IObjectStore() = default;
    virtual CToken GetObj(const SObjectTag&, const CVParamTransfer&)=0;
    virtual CToken GetObj(const SObjectTag&)=0;
    virtual CToken GetObj(std::string_view)=0;
    virtual CToken GetObj(std::string_view, const CVParamTransfer&)=0;
    virtual bool HasObject(const SObjectTag&) const=0;
    virtual bool ObjectIsLive(const SObjectTag&) const=0;
    virtual IFactory& GetFactory() const=0;
    virtual void Flush()=0;
    virtual void ObjectUnreferenced(const SObjectTag&)=0;
};

}

