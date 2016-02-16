#include "CFactoryMgr.hpp"
#include "IObj.hpp"

namespace pshag
{

CFactoryFnReturn CFactoryMgr::MakeObject(const SObjectTag& tag, pshag::CInputStream& in,
                                         const CVParamTransfer& paramXfer)
{
    auto search = m_factories.find(tag.type);
    if (search == m_factories.end())
        return {};

    return search->second(tag, in, paramXfer);
}

CFactoryFnReturn CFactoryMgr::MakeObjectFromMemory(const SObjectTag& tag, void* buf, int size,
                                                   bool compressed, const CVParamTransfer& paramXfer)
{
    auto search = m_factories.find(tag.type);
    if (search == m_factories.end())
        return {};

    if (compressed)
    {
        CZipInputStream r(std::make_unique<Athena::io::MemoryReader>(buf, size));
        return search->second(tag, r, paramXfer);
    }
    else
    {
        Athena::io::MemoryReader r(buf, size);
        return search->second(tag, r, paramXfer);
    }
}

}
