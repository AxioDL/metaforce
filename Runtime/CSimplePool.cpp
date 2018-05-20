#include "CSimplePool.hpp"
#include "IVParamObj.hpp"
#include "CToken.hpp"

namespace urde
{

CSimplePool::CSimplePool(IFactory& factory)
: x18_factory(factory), x1c_paramXfer(new TObjOwnerParam<IObjectStore*>(this))
{}

CSimplePool::~CSimplePool()
{
    assert(x8_resources.empty() && "Dangling CSimplePool resources detected");
}

CToken CSimplePool::GetObj(const SObjectTag& tag, const CVParamTransfer& paramXfer)
{
    if (!tag)
        return {};

    auto iter = x8_resources.find(tag);
    if (iter != x8_resources.end())
        return CToken(iter->second);

    CObjectReference* ret = new CObjectReference(*this, std::unique_ptr<IObj>(), tag, paramXfer);
    x8_resources.emplace(std::make_pair<SObjectTag, CObjectReference*>((SObjectTag)tag, std::move(ret)));
    return CToken(ret);
}

CToken CSimplePool::GetObj(const SObjectTag& tag)
{
    return GetObj(tag, x1c_paramXfer);
}

CToken CSimplePool::GetObj(std::string_view resourceName)
{
    return GetObj(resourceName, x1c_paramXfer);
}

CToken CSimplePool::GetObj(std::string_view resourceName, const CVParamTransfer& paramXfer)
{
    const SObjectTag* tag = x18_factory.GetResourceIdByName(resourceName);
    if (!tag)
        return {};
    return GetObj(*tag, paramXfer);
}

bool CSimplePool::HasObject(const SObjectTag& tag) const
{
    auto iter = x8_resources.find(tag);
    if (iter != x8_resources.cend())
        return true;
    return x18_factory.CanBuild(tag);
}

bool CSimplePool::ObjectIsLive(const SObjectTag& tag) const
{
    auto iter = x8_resources.find(tag);
    if (iter == x8_resources.cend())
        return false;
    return iter->second->IsLoaded();
}

void CSimplePool::Flush()
{
}

void CSimplePool::ObjectUnreferenced(const SObjectTag& tag)
{
    auto iter = x8_resources.find(tag);
    if (iter != x8_resources.end())
        x8_resources.erase(iter);
}

std::vector<SObjectTag> CSimplePool::GetReferencedTags() const
{
    std::vector<SObjectTag> ret;
    ret.reserve(x8_resources.size());
    for (const auto& obj : x8_resources)
        ret.push_back(obj.first);
    return ret;
}

}
