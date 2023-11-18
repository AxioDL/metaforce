#include "Runtime/CFactoryMgr.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <iterator>
#include "optick.h"

#include "Runtime/CStopwatch.hpp"
#include "Runtime/IObj.hpp"

namespace metaforce {
constexpr std::array TypeTable{
    FOURCC('CLSN'), FOURCC('CMDL'), FOURCC('CSKR'), FOURCC('ANIM'), FOURCC('CINF'), FOURCC('TXTR'), FOURCC('PLTT'),
    FOURCC('FONT'), FOURCC('ANCS'), FOURCC('EVNT'), FOURCC('MADF'), FOURCC('MLVL'), FOURCC('MREA'), FOURCC('MAPW'),
    FOURCC('MAPA'), FOURCC('SAVW'), FOURCC('SAVA'), FOURCC('PART'), FOURCC('WPSC'), FOURCC('SWHC'), FOURCC('DPSC'),
    FOURCC('ELSC'), FOURCC('CRSC'), FOURCC('AFSM'), FOURCC('DCLN'), FOURCC('AGSC'), FOURCC('ATBL'), FOURCC('CSNG'),
    FOURCC('STRG'), FOURCC('SCAN'), FOURCC('PATH'), FOURCC('DGRP'), FOURCC('HMAP'), FOURCC('CTWK'), FOURCC('FRME'),
    FOURCC('HINT'), FOURCC('MAPU'), FOURCC('DUMB'), FOURCC('OIDS'),
};

CFactoryFnReturn CFactoryMgr::MakeObject(const SObjectTag& tag, metaforce::CInputStream& in,
                                         const CVParamTransfer& paramXfer, CObjectReference* selfRef) {
  auto search = x10_factories.find(tag.type);
  if (search == x10_factories.end())
    return {};

  return search->second(tag, in, paramXfer, selfRef);
}

bool CFactoryMgr::CanMakeMemory(const metaforce::SObjectTag& tag) const {
  auto search = x24_memFactories.find(tag.type);
  return search != x24_memFactories.cend();
}

CFactoryFnReturn CFactoryMgr::MakeObjectFromMemory(const SObjectTag& tag, std::unique_ptr<u8[]>&& buf, int size,
                                                   bool compressed, const CVParamTransfer& paramXfer,
                                                   CObjectReference* selfRef) {
  OPTICK_EVENT();
  std::unique_ptr<u8[]> localBuf = std::move(buf);

  const auto memFactoryIter = x24_memFactories.find(tag.type);
  if (memFactoryIter != x24_memFactories.cend()) {
    if (compressed) {
      std::unique_ptr<CInputStream> compRead =
          std::make_unique<CMemoryInStream>(localBuf.get(), size, CMemoryInStream::EOwnerShip::NotOwned);
      const u32 decompLen = compRead->ReadLong();
      CZipInputStream r(std::move(compRead));
      std::unique_ptr<u8[]> decompBuf(new u8[decompLen]);
      r.Get(decompBuf.get(), decompLen);
      return memFactoryIter->second(tag, std::move(decompBuf), decompLen, paramXfer, selfRef);
    } else {
      return memFactoryIter->second(tag, std::move(localBuf), size, paramXfer, selfRef);
    }
  } else {
    const auto factoryIter = x10_factories.find(tag.type);
    if (factoryIter == x10_factories.end()) {
      return {};
    }

    if (compressed) {
      std::unique_ptr<CInputStream> compRead =
          std::make_unique<CMemoryInStream>(localBuf.get(), size, CMemoryInStream::EOwnerShip::NotOwned);

      compRead->ReadLong();
      CZipInputStream r(std::move(compRead));
      return factoryIter->second(tag, r, paramXfer, selfRef);
    } else {
      CMemoryInStream r(localBuf.get(), size, CMemoryInStream::EOwnerShip::NotOwned);
      return factoryIter->second(tag, r, paramXfer, selfRef);
    }
  }
}

CFactoryMgr::ETypeTable CFactoryMgr::FourCCToTypeIdx(FourCC fcc) {
  for (size_t i = 0; i < 4; ++i) {
    fcc.getChars()[i] = char(std::toupper(fcc.getChars()[i]));
  }

  const auto search =
      std::find_if(TypeTable.cbegin(), TypeTable.cend(), [fcc](const FourCC& test) { return test == fcc; });
  if (search == TypeTable.cend()) {
    return ETypeTable::Invalid;
  }
  return ETypeTable(std::distance(TypeTable.cbegin(), search));
}

FourCC CFactoryMgr::TypeIdxToFourCC(ETypeTable fcc) { return TypeTable[size_t(fcc)]; }

} // namespace metaforce
