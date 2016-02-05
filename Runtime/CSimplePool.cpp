#include "CSimplePool.hpp"
#include "IVParamObj.hpp"
#include "CToken.hpp"

namespace Retro
{

CSimplePool::CSimplePool(IFactory& factory)
: x30_factory(factory), x34_paramXfer(new TObjOwnerParam<IObjectStore*>(this))
{}

CToken CSimplePool::GetObj(const SObjectTag&, const CVParamTransfer&)
{
}

CToken CSimplePool::GetObj(const SObjectTag&)
{
}

CToken CSimplePool::GetObj(char const*)
{
}

CToken CSimplePool::GetObj(char const*, const CVParamTransfer&)
{
}

void CSimplePool::HasObject(const SObjectTag&) const
{
}

void CSimplePool::ObjectIsLive(const SObjectTag&) const
{
}

void CSimplePool::Flush()
{
}

void CSimplePool::ObjectUnreferenced(const SObjectTag&)
{
}

}
