#ifndef __RETRO_IOBJECTSTORE_HPP__
#define __RETRO_IOBJECTSTORE_HPP__

namespace Retro
{
struct SObjectTag;
class CVParamTransfer;
class IFactory;
class CToken;

class IObjectStore
{
public:
    virtual CToken GetObj(const SObjectTag&, const CVParamTransfer&)=0;
    virtual CToken GetObj(const SObjectTag&)=0;
    virtual CToken GetObj(char const*)=0;
    virtual CToken GetObj(char const*, const CVParamTransfer&)=0;
    virtual void HasObject(const SObjectTag&) const=0;
    virtual void ObjectIsLive(const SObjectTag&) const=0;
    virtual IFactory& GetFactory() const=0;
    virtual void Flush()=0;
    virtual void ObjectUnreferenced(const SObjectTag&)=0;
};

}

#endif // __RETRO_IOBJECTSTORE_HPP__
