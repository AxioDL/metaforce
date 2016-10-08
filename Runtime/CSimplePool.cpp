#include "CSimplePool.hpp"
#include "IVParamObj.hpp"
#include "CToken.hpp"

namespace urde
{

CSimplePool::CSimplePool(IFactory& factory)
: x30_factory(factory), x34_paramXfer(new TObjOwnerParam<IObjectStore*>(this))
{}

CToken CSimplePool::GetObj(const SObjectTag& tag, const CVParamTransfer& paramXfer)
{
    if (!tag)
        return {};

    auto iter = x4_resources.find(tag);
    if (iter != x4_resources.end())
        return CToken(iter->second);

    CObjectReference* ret = new CObjectReference(*this, std::unique_ptr<IObj>(), tag, paramXfer);
    x4_resources.emplace(std::make_pair<SObjectTag, CObjectReference*>((SObjectTag)tag, std::move(ret)));
    return CToken(ret);
}

CToken CSimplePool::GetObj(const SObjectTag& tag)
{
    return GetObj(tag, x34_paramXfer);
}

CToken CSimplePool::GetObj(const char* resourceName)
{
    return GetObj(resourceName, x34_paramXfer);
}

CToken CSimplePool::GetObj(const char* resourceName, const CVParamTransfer& paramXfer)
{
    const SObjectTag* tag = x30_factory.GetResourceIdByName(resourceName);
    if (!tag)
        return {};
    return GetObj(*tag, paramXfer);
}

bool CSimplePool::HasObject(const SObjectTag& tag) const
{
    auto iter = x4_resources.find(tag);
    if (iter != x4_resources.cend())
        return true;
    return x30_factory.CanBuild(tag);
}

bool CSimplePool::ObjectIsLive(const SObjectTag& tag) const
{
    auto iter = x4_resources.find(tag);
    if (iter == x4_resources.cend())
        return false;
    return iter->second->IsLoaded();
}

void CSimplePool::Flush()
{
}

void CSimplePool::ObjectUnreferenced(const SObjectTag& tag)
{
    auto iter = x4_resources.find(tag);
    if (iter != x4_resources.end())
        x4_resources.erase(iter);
}

}
