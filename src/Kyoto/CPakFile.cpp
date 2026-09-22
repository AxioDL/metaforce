#include "Kyoto/CPakFile.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

#include <stdio.h>

static inline int round_up_32(int val) { return (val + 31) & ~31; }

static int kMinReserveBytes = 64;

CPakFile::SResInfo::SResInfo(uint id, uint fourCC, uint offset, uint size, uint flags) : x0_id(id) {
  uint typeIdx = CFactoryMgr::FourCCToTypeIdx(fourCC);
  x4_data[0] = static_cast< uchar >(typeIdx | (flags != 0 ? 0x80 : 0));
  x4_data[1] = static_cast< uchar >(offset >> 5);
  x4_data[2] = static_cast< uchar >(offset >> 13);
  x4_data[3] = static_cast< uchar >((offset >> 21) | ((size << 2) & 0x80));
  x4_data[4] = static_cast< uchar >(size >> 6);
  x4_data[5] = static_cast< uchar >(size >> 14);
}

uint CPakFile::SResInfo::GetType() const { return CFactoryMgr::TypeIdxToFourCC(x4_data[0] & 0x7f); }

uint CPakFile::SResInfo::GetOffset() const {
  return ((x4_data[1] | (x4_data[2] << 8) | (x4_data[3] << 16)) & 0x7FFFFF) << 5;
}

uint CPakFile::SResInfo::GetSize() const {
  return ((x4_data[3] >> 7) | (x4_data[4] << 1) | (x4_data[5] << 9)) << 5;
}

bool CPakFile::SResInfo::IsCompressed() const { return (x4_data[0] & ~0x7F) != 0; }

CPakFile::CPakFile(const rstl::string& filename, bool buildDepList, bool worldPak)
: x0_file(filename.data())
, x28_24_buildDepList(buildDepList)
, x28_25_aramFile(x0_file.IsARAMFile())
, x28_26_worldPak(worldPak)
, x28_27_stashedInARAM(false)
, x2c_asyncLoadPhase(kAP_Warmup)
, x48_resTableOffset(0)
, x4c_resTableCount(0)
, x50_aramBase(CARAMManager::GetInvalidAlloc())
, x84_currentSeek(-1) {}

CPakFile::~CPakFile() {
  while (x2c_asyncLoadPhase != kAP_Loaded) {
    AsyncIdle();
  }

  CMemory::OffsetFakeStatics(-static_cast< int >(GetFakeStaticSize()));
  CARAMManager::Free(x50_aramBase);
}

void CPakFile::AsyncIdle() {
  if (x2c_asyncLoadPhase == kAP_Loaded)
    return;
  if (!x0_file.IsARAMFileLoaded())
    return;
  if (!x30_dvdReq.null() && !x30_dvdReq->IsComplete())
    return;
  switch (x2c_asyncLoadPhase) {
  case kAP_Warmup:
    Warmup();
    break;
  case kAP_InitialHeaderLoad:
    InitialHeaderLoad();
    break;
  case kAP_DataLoad:
    DataLoad();
    break;
  default:
    break;
  }
}

void CPakFile::Warmup() {
  int length = rstl::min_val< int >(x0_file.Length(), 8192);
  x38_headerData.resize(length);
  x30_dvdReq = rstl::auto_ptr< CDvdRequest >(x0_file.SyncRead(&x38_headerData[0], length));
  x2c_asyncLoadPhase = kAP_InitialHeaderLoad;
}

void CPakFile::InitialHeaderLoad() {
  CMemoryInStream in(&x38_headerData[0], x38_headerData.size());
  x30_dvdReq = rstl::auto_ptr< CDvdRequest >();

  int version = in.ReadInt32();
  if (version != 0x00030005) {
    char buf[248];
#if NONMATCHING
    snprintf(buf, sizeof(buf), "%s: Incompatible pak file version -- Current version is %x, you're using %x",
            x0_file.GetFilename().data(), 0x00030005, version);
#else
    sprintf(buf, "%s: Incompatible pak file version -- Current version is %x, you're using %x",
            x0_file.GetFilename().data(), 0x00030005, version);
#endif
    return;
  }

  in.ReadInt32();
  int nameCount = in.ReadInt32();
  x54_nameList.reserve(nameCount);

  for (int i = 0; i < nameCount; ++i) {
    int type = in.ReadInt32();
    int id = in.ReadInt32();
    rstl::string name = CStringExtras::ReadString(in);
    x54_nameList.push_back(rstl::pair< rstl::string, SObjectTag >(name, SObjectTag(type, id)));
  }

  x4c_resTableCount = in.ReadInt32();
  x48_resTableOffset = in.GetReadPosition();
  x2c_asyncLoadPhase = kAP_DataLoad;

  int origSize = x38_headerData.size();
  uint resDataSize = x4c_resTableCount * 20;
  int newSize = (resDataSize + x48_resTableOffset + 31) & ~31;
  if (newSize > origSize) {
    x38_headerData.resize(newSize);
    x30_dvdReq = rstl::auto_ptr< CDvdRequest >(x0_file.AsyncSeekRead(
        &x38_headerData[0] + origSize, x38_headerData.size() - origSize, kSO_Begin, origSize));
  } else {
    DataLoad();
  }
}

void CPakFile::DataLoad() {
  x30_dvdReq = rstl::auto_ptr< CDvdRequest >();
  CMemoryInStream in(&x38_headerData[x48_resTableOffset],
                     x38_headerData.size() - x48_resTableOffset);
  LoadResourceTable(in);
  x2c_asyncLoadPhase = kAP_Loaded;
  if (x28_26_worldPak) {
    uint size = round_up_32(x4c_resTableCount * sizeof(SResInfo));
    x50_aramBase = CARAMManager::Alloc(size);
    uint handle =
        CARAMManager::DMAToARAM(x74_resList.data(), x50_aramBase, size, CARAMManager::kDMAPrio_One);
    CARAMManager::WaitForDMACompletion(handle);
  }
  x38_headerData = rstl::vector< uchar, rstl::aligned_allocator >();
  CMemory::OffsetFakeStatics(GetFakeStaticSize());
}

void CPakFile::LoadResourceTable(CMemoryInStream& in) {
  int reserveBytes = round_up_32(x4c_resTableCount * static_cast< int >(sizeof(SResInfo))) +
                     static_cast< int >(sizeof(SResInfo)) - 1;
  x74_resList.reserve(static_cast< uint >(rstl::max_val(reserveBytes, kMinReserveBytes)) /
                      sizeof(SResInfo));

  if (x28_24_buildDepList) {
    x64_depList.reserve(x4c_resTableCount);
  }

  for (int i = 0; i < static_cast< int >(x4c_resTableCount); ++i) {
    uint flags = in.Get< uint >();
    uint fourCC = in.Get< uint >();
    uint id = in.Get< uint >();
    uint size = in.Get< uint >();
    uint offset = in.Get< uint >();
    x74_resList.push_back(SResInfo(id, fourCC, offset, size, flags));
    if (x28_24_buildDepList) {
      x64_depList.push_back(id);
    }
  }

  static rstl::less< SResInfo > compare;
  rstl::sort(x74_resList.begin(), x74_resList.end(), compare);
}

uint CPakFile::GetFakeStaticSize() const {
  return x64_depList.size() * sizeof(CAssetId) + x74_resList.size() * sizeof(SResInfo);
}

const CPakFile::SResInfo* CPakFile::GetResInfo(uint id) const {
  if (!IsCompletelyLoaded())
    return nullptr;
  if (x28_27_stashedInARAM)
    return nullptr;
  SResInfo key(id, 'TXTR', 0, 0, 0);
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::const_iterator it =
      rstl::lower_bound(x74_resList.begin(), x74_resList.end(), key, compare);
  if (it == x74_resList.end() || it->GetId() != id) {
    return nullptr;
  }
  return &*it;
}

const CPakFile::SResInfo* CPakFile::GetResInfoForLoadDirectionless(uint id) {
  if (x28_27_stashedInARAM)
    return nullptr;
  SResInfo key(id, 'TXTR', 0, 0, 0);
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::iterator it =
      rstl::lower_bound(x74_resList.begin(), x74_resList.end(), key, compare);
  if (it == x74_resList.end() || it->GetId() != id) {
    return nullptr;
  }
  const SResInfo* bestInfo = &*it;
  int bestDelta = CMath::AbsI(static_cast< int >(it->GetOffset() - x84_currentSeek));
  for (AUTO(next, it + 1); next != x74_resList.end(); ++next) {
    if (next->GetId() != id)
      break;
    int thisDelta = CMath::AbsI(static_cast< int >(next->GetOffset() - x84_currentSeek));
    if (thisDelta < bestDelta) {
      bestInfo = &*next;
      bestDelta = thisDelta;
    }
  }
  x84_currentSeek = bestInfo->GetOffset() + bestInfo->GetSize();
  return bestInfo;
}

const CPakFile::SResInfo* CPakFile::GetResInfoForLoadPreferForward(uint id) {
  if (x28_27_stashedInARAM)
    return nullptr;
  SResInfo key(id, 'TXTR', 0, 0, 0);
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::iterator it =
      rstl::lower_bound(x74_resList.begin(), x74_resList.end(), key, compare);
  if (it == x74_resList.end() || it->GetId() != id)
    return nullptr;
  const SResInfo* bestInfo = &*it;
  int bestDelta = x84_currentSeek - static_cast< int >(bestInfo->GetOffset());
  ++it;
  while (it != x74_resList.end()) {
    if (it->GetId() != id)
      break;
    int thisDelta = x84_currentSeek - static_cast< int >(it->GetOffset());
    if ((bestDelta < 0 && (thisDelta > 0 || thisDelta > bestDelta)) ||
        (bestDelta >= 0 && thisDelta > 0 && thisDelta < bestDelta)) {
      bestInfo = &*it;
      bestDelta = thisDelta;
    }
    ++it;
  }
  x84_currentSeek = bestInfo->GetOffset() + bestInfo->GetSize();
  return bestInfo;
}

const SObjectTag* CPakFile::GetResIdByName(const char* name) const {
  if (!x28_27_stashedInARAM) {
    for (AUTO(it, x54_nameList.begin()); it != x54_nameList.end(); ++it) {
      int cmp = CStringExtras::CompareCaseInsensitive(it->first, rstl::string_l(name));
      if (cmp == 0) {
        return &it->second;
      }
    }
  }
  return nullptr;
}

const rstl::vector< CAssetId >* CPakFile::GetDepList() const {
  if (x64_depList.size() != 0)
    return &x64_depList;
  return nullptr;
}

void CPakFile::sub_8036742c() {
  if (x28_26_worldPak) {
    x28_27_stashedInARAM = true;
    x64_depList = rstl::vector< CAssetId >();
    x74_resList = rstl::vector< SResInfo >();
  }
}

void CPakFile::EnsureWorldPakReady() {
  if (x28_26_worldPak && x28_27_stashedInARAM) {
    int reserveBytes = round_up_32(x4c_resTableCount * static_cast< int >(sizeof(SResInfo))) +
                       static_cast< int >(sizeof(SResInfo)) - 1;
    x74_resList.reserve(static_cast< uint >(rstl::max_val(reserveBytes, kMinReserveBytes)) /
                        sizeof(SResInfo));

    for (int i = 0; i < static_cast< int >(x4c_resTableCount); ++i) {
      x74_resList.push_back(SResInfo(i, 'TXTR', 0, 0, 0));
    }

    uint dmaSize = round_up_32(x4c_resTableCount * sizeof(SResInfo));
    CARAMManager::WaitForDMACompletion(CARAMManager::DMAToMRAM(
        x50_aramBase, x74_resList.data(), dmaSize, CARAMManager::kDMAPrio_One));

    if (x28_24_buildDepList) {
      x64_depList.reserve(x4c_resTableCount);
      for (int i = 0; i < static_cast< int >(x4c_resTableCount); ++i) {
        x64_depList.push_back(x74_resList[i].GetId());
      }
    }

    x28_27_stashedInARAM = false;
  }
}
