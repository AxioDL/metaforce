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

CPakFile::SResInfo::SResInfo(uint id, uint fourCC, uint offset, uint size, uint flags) : mId(id) {
  uint typeIdx = CFactoryMgr::FourCCToTypeIdx(fourCC);
  mData[0] = static_cast< uchar >(typeIdx | (flags != 0 ? 0x80 : 0));
  mData[1] = static_cast< uchar >(offset >> 5);
  mData[2] = static_cast< uchar >(offset >> 13);
  mData[3] = static_cast< uchar >((offset >> 21) | ((size << 2) & 0x80));
  mData[4] = static_cast< uchar >(size >> 6);
  mData[5] = static_cast< uchar >(size >> 14);
}

uint CPakFile::SResInfo::GetType() const { return CFactoryMgr::TypeIdxToFourCC(mData[0] & 0x7f); }

uint CPakFile::SResInfo::GetOffset() const {
  return ((mData[1] | (mData[2] << 8) | (mData[3] << 16)) & 0x7FFFFF) << 5;
}

uint CPakFile::SResInfo::GetSize() const {
  return ((mData[3] >> 7) | (mData[4] << 1) | (mData[5] << 9)) << 5;
}

bool CPakFile::SResInfo::IsCompressed() const { return (mData[0] & ~0x7F) != 0; }

CPakFile::CPakFile(const rstl::string& filename, bool buildDepList, bool worldPak)
: mFile(filename.data())
, mBuildDepList(buildDepList)
, mAramFile(mFile.IsARAMFile())
, mWorldPak(worldPak)
, mStashedInARAM(false)
, mAsyncLoadPhase(kAP_Warmup)
, mResTableOffset(0)
, mResTableCount(0)
#if !defined(TARGET_PC)
, mAramBase(CARAMManager::GetInvalidAlloc())
#endif
, mCurrentSeek(-1) {}

CPakFile::~CPakFile() {
  while (mAsyncLoadPhase != kAP_Loaded) {
    AsyncIdle();
  }

  CMemory::OffsetFakeStatics(-static_cast< int >(GetFakeStaticSize()));
#if !defined(TARGET_PC)
  CARAMManager::Free(mAramBase);
#endif
}

void CPakFile::AsyncIdle() {
  if (mAsyncLoadPhase == kAP_Loaded)
    return;
  if (!mFile.IsARAMFileLoaded())
    return;
  if (!mDvdReq.null() && !mDvdReq->IsComplete())
    return;
  switch (mAsyncLoadPhase) {
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
  int length = rstl::min_val< int >(mFile.Length(), 8192);
  mHeaderData.resize(length);
  mDvdReq = rstl::auto_ptr< CDvdRequest >(mFile.SyncRead(&mHeaderData[0], length));
  mAsyncLoadPhase = kAP_InitialHeaderLoad;
}

void CPakFile::InitialHeaderLoad() {
  CMemoryInStream in(&mHeaderData[0], mHeaderData.size());
  mDvdReq = rstl::auto_ptr< CDvdRequest >();

  int version = in.ReadInt32();
  if (version != 0x00030005) {
    char buf[248];
#if NONMATCHING
    snprintf(buf, sizeof(buf), "%s: Incompatible pak file version -- Current version is %x, you're using %x",
            mFile.GetFilename().data(), 0x00030005, version);
#else
    sprintf(buf, "%s: Incompatible pak file version -- Current version is %x, you're using %x",
            mFile.GetFilename().data(), 0x00030005, version);
#endif
    return;
  }

  in.ReadInt32();
  int nameCount = in.ReadInt32();
  mNameList.reserve(nameCount);

  for (int i = 0; i < nameCount; ++i) {
    int type = in.ReadInt32();
    int id = in.ReadInt32();
    rstl::string name = CStringExtras::ReadString(in);
    mNameList.push_back(rstl::pair< rstl::string, SObjectTag >(name, SObjectTag(type, id)));
  }

  mResTableCount = in.ReadInt32();
  mResTableOffset = in.GetReadPosition();
  mAsyncLoadPhase = kAP_DataLoad;

  int origSize = mHeaderData.size();
  uint resDataSize = mResTableCount * 20;
  int newSize = (resDataSize + mResTableOffset + 31) & ~31;
  if (newSize > origSize) {
    mHeaderData.resize(newSize);
    mDvdReq = rstl::auto_ptr< CDvdRequest >(mFile.AsyncSeekRead(
        &mHeaderData[0] + origSize, mHeaderData.size() - origSize, kSO_Begin, origSize));
  } else {
    DataLoad();
  }
}

void CPakFile::DataLoad() {
  mDvdReq = rstl::auto_ptr< CDvdRequest >();
  CMemoryInStream in(&mHeaderData[mResTableOffset],
                     mHeaderData.size() - mResTableOffset);
  LoadResourceTable(in);
  mAsyncLoadPhase = kAP_Loaded;
#if !defined(TARGET_PC)
  if (mWorldPak) {
    uint size = round_up_32(mResTableCount * sizeof(SResInfo));
    mAramBase = CARAMManager::Alloc(size);
    uint handle =
        CARAMManager::DMAToARAM(mResList.data(), mAramBase, size, CARAMManager::kDMAPrio_One);
    CARAMManager::WaitForDMACompletion(handle);
  }
#endif
  mHeaderData = rstl::vector< uchar, rstl::aligned_allocator >();
  CMemory::OffsetFakeStatics(GetFakeStaticSize());
}

void CPakFile::LoadResourceTable(CMemoryInStream& in) {
  int reserveBytes = round_up_32(mResTableCount * static_cast< int >(sizeof(SResInfo))) +
                     static_cast< int >(sizeof(SResInfo)) - 1;
  mResList.reserve(static_cast< uint >(rstl::max_val(reserveBytes, kMinReserveBytes)) /
                      sizeof(SResInfo));

  if (mBuildDepList) {
    mDepList.reserve(mResTableCount);
  }

  for (int i = 0; i < static_cast< int >(mResTableCount); ++i) {
    uint flags = in.Get< uint >();
    uint fourCC = in.Get< uint >();
    uint id = in.Get< uint >();
    uint size = in.Get< uint >();
    uint offset = in.Get< uint >();
    mResList.push_back(SResInfo(id, fourCC, offset, size, flags));
    if (mBuildDepList) {
      mDepList.push_back(id);
    }
  }

  static rstl::less< SResInfo > compare;
  rstl::sort(mResList.begin(), mResList.end(), compare);
}

uint CPakFile::GetFakeStaticSize() const {
  return mDepList.size() * sizeof(CAssetId) + mResList.size() * sizeof(SResInfo);
}

const CPakFile::SResInfo* CPakFile::GetResInfo(uint id) const {
  if (!IsCompletelyLoaded())
    return nullptr;
  if (mStashedInARAM)
    return nullptr;
  SResInfo key(id, 'TXTR', 0, 0, 0);
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::const_iterator it =
      rstl::lower_bound(mResList.begin(), mResList.end(), key, compare);
  if (it == mResList.end() || it->GetId() != id) {
    return nullptr;
  }
  return &*it;
}

const CPakFile::SResInfo* CPakFile::GetResInfoForLoadDirectionless(uint id) {
  if (mStashedInARAM)
    return nullptr;
  SResInfo key(id, 'TXTR', 0, 0, 0);
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::iterator it =
      rstl::lower_bound(mResList.begin(), mResList.end(), key, compare);
  if (it == mResList.end() || it->GetId() != id) {
    return nullptr;
  }
  const SResInfo* bestInfo = &*it;
  int bestDelta = CMath::AbsI(static_cast< int >(it->GetOffset() - mCurrentSeek));
  for (AUTO(next, it + 1); next != mResList.end(); ++next) {
    if (next->GetId() != id)
      break;
    int thisDelta = CMath::AbsI(static_cast< int >(next->GetOffset() - mCurrentSeek));
    if (thisDelta < bestDelta) {
      bestInfo = &*next;
      bestDelta = thisDelta;
    }
  }
  mCurrentSeek = bestInfo->GetOffset() + bestInfo->GetSize();
  return bestInfo;
}

const CPakFile::SResInfo* CPakFile::GetResInfoForLoadPreferForward(uint id) {
  if (mStashedInARAM)
    return nullptr;
  SResInfo key(id, 'TXTR', 0, 0, 0);
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::iterator it =
      rstl::lower_bound(mResList.begin(), mResList.end(), key, compare);
  if (it == mResList.end() || it->GetId() != id)
    return nullptr;
  const SResInfo* bestInfo = &*it;
  int bestDelta = mCurrentSeek - static_cast< int >(bestInfo->GetOffset());
  ++it;
  while (it != mResList.end()) {
    if (it->GetId() != id)
      break;
    int thisDelta = mCurrentSeek - static_cast< int >(it->GetOffset());
    if ((bestDelta < 0 && (thisDelta > 0 || thisDelta > bestDelta)) ||
        (bestDelta >= 0 && thisDelta > 0 && thisDelta < bestDelta)) {
      bestInfo = &*it;
      bestDelta = thisDelta;
    }
    ++it;
  }
  mCurrentSeek = bestInfo->GetOffset() + bestInfo->GetSize();
  return bestInfo;
}

const SObjectTag* CPakFile::GetResIdByName(const char* name) const {
  if (!mStashedInARAM) {
    for (AUTO(it, mNameList.begin()); it != mNameList.end(); ++it) {
      int cmp = CStringExtras::CompareCaseInsensitive(it->first, rstl::string_l(name));
      if (cmp == 0) {
        return &it->second;
      }
    }
  }
  return nullptr;
}

const rstl::vector< CAssetId >* CPakFile::GetDepList() const {
  if (mDepList.size() != 0)
    return &mDepList;
  return nullptr;
}

void CPakFile::sub_8036742c() {
#if !defined(TARGET_PC)
  if (mWorldPak) {
    mStashedInARAM = true;
    mDepList = rstl::vector< CAssetId >();
    mResList = rstl::vector< SResInfo >();
  }
#endif
}

void CPakFile::EnsureWorldPakReady() {
#if !defined(TARGET_PC)
  if (mWorldPak && mStashedInARAM) {
    int reserveBytes = round_up_32(mResTableCount * static_cast< int >(sizeof(SResInfo))) +
                       static_cast< int >(sizeof(SResInfo)) - 1;
    mResList.reserve(static_cast< uint >(rstl::max_val(reserveBytes, kMinReserveBytes)) /
                        sizeof(SResInfo));

    for (int i = 0; i < static_cast< int >(mResTableCount); ++i) {
      mResList.push_back(SResInfo(i, 'TXTR', 0, 0, 0));
    }

    uint dmaSize = round_up_32(mResTableCount * sizeof(SResInfo));
    CARAMManager::WaitForDMACompletion(CARAMManager::DMAToMRAM(
        mAramBase, mResList.data(), dmaSize, CARAMManager::kDMAPrio_One));

    if (mBuildDepList) {
      mDepList.reserve(mResTableCount);
      for (int i = 0; i < static_cast< int >(mResTableCount); ++i) {
        mDepList.push_back(mResList[i].GetId());
      }
    }

    mStashedInARAM = false;
  }
#endif
}
