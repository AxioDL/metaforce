#include "CFactoryMgr.hpp"
#include "IObj.hpp"

namespace urde
{

CFactoryFnReturn CFactoryMgr::MakeObject(const SObjectTag& tag, urde::CInputStream& in,
                                         const CVParamTransfer& paramXfer)
{
    auto search = m_factories.find(tag.type);
    if (search == m_factories.end())
        return {};

    return search->second(tag, in, paramXfer);
}

bool CFactoryMgr::CanMakeMemory(const urde::SObjectTag& tag) const
{
    auto search = m_memFactories.find(tag.type);
    return search != m_memFactories.cend();
}

CFactoryFnReturn CFactoryMgr::MakeObjectFromMemory(const SObjectTag& tag, std::unique_ptr<u8[]>&& buf, int size,
                                                   bool compressed, const CVParamTransfer& paramXfer)
{
    std::unique_ptr<u8[]> localBuf = std::move(buf);

    auto search = m_memFactories.find(tag.type);
    if (search != m_memFactories.cend())
    {
        if (compressed)
        {
            std::unique_ptr<CInputStream> compRead =
                std::make_unique<athena::io::MemoryReader>(localBuf.get(), size);
            u32 decompLen = compRead->readUint32Big();
            CZipInputStream r(std::move(compRead));
            std::unique_ptr<u8[]> decompBuf = r.readUBytes(decompLen);
            return search->second(tag, std::move(decompBuf), decompLen, paramXfer);
        }
        else
        {
            return search->second(tag, std::move(localBuf), size, paramXfer);
        }
    }
    else
    {
        auto search = m_factories.find(tag.type);
        if (search == m_factories.end())
            return {};

        if (compressed)
        {
            std::unique_ptr<CInputStream> compRead =
                std::make_unique<athena::io::MemoryReader>(localBuf.get(), size);
            u32 decompLen = compRead->readUint32Big();
            CZipInputStream r(std::move(compRead));
            return search->second(tag, r, paramXfer);
        }
        else
        {
            CMemoryInStream r(localBuf.get(), size);
            return search->second(tag, r, paramXfer);
        }
    }
}

}
