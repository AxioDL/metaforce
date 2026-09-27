#include "Kyoto/CFactoryMgr.hpp"

#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Streams/CZipInputStream.hpp"
#include "ctype.h"

static const uint sTypeTable[] = {
    'CLSN', 'CMDL', 'CSKR', 'ANIM', 'CINF', 'TXTR', 'PLTT', 'FONT', 'ANCS', 'EVNT',
    'MADF', 'MLVL', 'MREA', 'MAPW', 'MAPA', 'SAVW', 'SAVA', 'PART', 'WPSC', 'SWHC',
    'DPSC', 'ELSC', 'CRSC', 'AFSM', 'DCLN', 'AGSC', 'ATBL', 'CSNG', 'STRG', 'SCAN',
    'PATH', 'DGRP', 'HMAP', 'CTWK', 'FRME', 'HINT', 'MAPU', 'DUMB',
};

CFactoryMgr::CFactoryMgr() {}

CFactoryMgr::~CFactoryMgr() {}

void CFactoryMgr::AddFactory(uint type, FFactoryFunc factory) {
  AUTO(it, mFactories.find(type));
  if (it != mFactories.end()) {
    return;
  }
  mFactories.insert(rstl::pair< int, FFactoryFunc >(type, factory));
}

void CFactoryMgr::AddFactory(uint type, FMemFactoryFunc factory) {
  AUTO(it, mMemFactories.find(type));
  if (it != mMemFactories.end()) {
    return;
  }
  mMemFactories.insert(rstl::pair< int, FMemFactoryFunc >(type, factory));
}

bool CFactoryMgr::CanMakeMemory(const SObjectTag& tag) const {
  return mMemFactories.find(tag.GetType()) != mMemFactories.end();
}

rstl::auto_ptr< IObj > CFactoryMgr::MakeObject(const SObjectTag& tag, CInputStream& in,
                                               const CVParamTransfer& params) {
  AUTO(it, mFactories.find(tag.GetType()));
  return it->second(tag, in, params).GetObjForTransfer();
}

rstl::auto_ptr< IObj > CFactoryMgr::MakeObjectFromMemory(const SObjectTag& tag, void* buffer,
                                                         int size, bool compressed,
                                                         const CVParamTransfer& params) {
  rstl::map< int, FMemFactoryFunc >::const_iterator memIt = mMemFactories.find(tag.GetType());
  if (memIt != mMemFactories.end()) {
    FMemFactoryFunc factory = memIt->second;
    if (compressed) {
      rstl::auto_ptr< CInputStream > in(
          rs_new CMemoryInStream(buffer, size, CMemoryInStream::kOS_Owned));
      const uint length = in->ReadLong();
      rstl::auto_ptr< uchar > data(rs_new uchar[length]);
      {
        CZipInputStream zip(in);
        zip.Get(data.get(), length);
      }
      return factory(tag, data, length, params).GetObjForTransfer();
    }
    rstl::auto_ptr< uchar > data(static_cast< uchar* >(buffer));
    return factory(tag, data, size, params).GetObjForTransfer();
  }

  FFactoryFunc factory = mFactories.find(tag.GetType())->second;
  if (compressed) {
    CInputStream* in = rs_new CMemoryInStream(buffer, size, CMemoryInStream::kOS_Owned);
    in->ReadLong();
    CZipInputStream zip(in);
    return factory(tag, zip, params).GetObjForTransfer();
  }
  CMemoryInStream in(buffer, size, CMemoryInStream::kOS_Owned);
  return factory(tag, in, params).GetObjForTransfer();
}

uint CFactoryMgr::TypeIdxToFourCC(uint idx) { return sTypeTable[idx]; }

uint CFactoryMgr::FourCCToTypeIdx(uint fcc) {
  char* type = reinterpret_cast< char* >(&fcc);
  type[0] = toupper(type[0]);
  type[1] = toupper(type[1]);
  type[2] = toupper(type[2]);
  type[3] = toupper(type[3]);
  for (uint i = 0; i < sizeof(sTypeTable) / sizeof(sTypeTable[0]); ++i) {
    if (fcc == sTypeTable[i]) {
      return i;
    }
  }
  return -1;
}
