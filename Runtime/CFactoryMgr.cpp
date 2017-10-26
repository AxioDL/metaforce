#include "CFactoryMgr.hpp"
#include "IObj.hpp"

namespace urde
{

CFactoryFnReturn CFactoryMgr::MakeObject(const SObjectTag& tag, urde::CInputStream& in,
                                         const CVParamTransfer& paramXfer, CObjectReference* selfRef)
{
    auto search = m_factories.find(tag.type);
    if (search == m_factories.end())
        return {};

    return search->second(tag, in, paramXfer, selfRef);
}

bool CFactoryMgr::CanMakeMemory(const urde::SObjectTag& tag) const
{
    auto search = m_memFactories.find(tag.type);
    return search != m_memFactories.cend();
}

CFactoryFnReturn CFactoryMgr::MakeObjectFromMemory(const SObjectTag& tag, std::unique_ptr<u8[]>&& buf, int size,
                                                   bool compressed, const CVParamTransfer& paramXfer,
                                                   CObjectReference* selfRef)
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
            return search->second(tag, std::move(decompBuf), decompLen, paramXfer, selfRef);
        }
        else
        {
            return search->second(tag, std::move(localBuf), size, paramXfer, selfRef);
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
            return search->second(tag, r, paramXfer, selfRef);
        }
        else
        {
            CMemoryInStream r(localBuf.get(), size);
            return search->second(tag, r, paramXfer, selfRef);
        }
    }
}

static const FourCC TypeTable[] =
{
    FOURCC('CLSN'), FOURCC('CMDL'), FOURCC('CSKR'), FOURCC('ANIM'), FOURCC('CINF'), FOURCC('TXTR'),
    FOURCC('PLTT'), FOURCC('FONT'), FOURCC('ANCS'), FOURCC('EVNT'), FOURCC('MADF'), FOURCC('MLVL'),
    FOURCC('MREA'), FOURCC('MAPW'), FOURCC('MAPA'), FOURCC('SAVW'), FOURCC('SAVA'), FOURCC('PART'),
    FOURCC('WPSC'), FOURCC('SWHC'), FOURCC('DPSC'), FOURCC('ELSC'), FOURCC('CRSC'), FOURCC('AFSM'),
    FOURCC('DCLN'), FOURCC('AGSC'), FOURCC('ATBL'), FOURCC('CSNG'), FOURCC('STRG'), FOURCC('SCAN'),
    FOURCC('PATH'), FOURCC('DGRP'), FOURCC('HMAP'), FOURCC('CTWK'), FOURCC('FRME'), FOURCC('HINT'),
    FOURCC('MAPU'), FOURCC('DUMB')
};

CFactoryMgr::ETypeTable CFactoryMgr::FourCCToTypeIdx(FourCC fcc)
{
    for (int i=0 ; i<4 ; ++i)
        fcc.getChars()[i] = char(std::toupper(fcc.getChars()[i]));
    auto search = std::find_if(std::begin(TypeTable), std::end(TypeTable),
                               [fcc](const FourCC& test) { return test == fcc; });
    if (search == std::end(TypeTable))
        return ETypeTable::Invalid;
    return ETypeTable(search - std::begin(TypeTable));
}

FourCC CFactoryMgr::TypeIdxToFourCC(ETypeTable fcc)
{
    return TypeTable[int(fcc)];
}

}
