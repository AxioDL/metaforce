#include "Kyoto/CCrc32.hpp"
#include "Kyoto/CMemoryCardSys.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "dolphin/os/OSCache.h"

#include "Kyoto/MemoryCopy.hpp"

bool CMemoryCardSys::mIsInitialized;
bool CMemoryCardSys::mIsCardSysExists;
rstl::vector< char, rstl::aligned_allocator > CMemoryCardSys::mWorkAreaA;
rstl::vector< char, rstl::aligned_allocator > CMemoryCardSys::mWorkAreaB;

ECardResult SMemoryCardFileInfo::FileRead() {
  rstl::vector< uchar >& saveData = mSaveData;
  saveData = rstl::vector< uchar >();
  const uint size = mSaveFileData.size();
  const void* data = mSaveFileData.data();
#if NONMATCHING
  const uint crc = CBasics::SwapBytes(*static_cast< const uint* >(data));
#else
  const uint crc = *static_cast< const uint* >(data);
#endif
  if (crc == CCRC32::Calculate(static_cast< const uchar* >(data) + 4, size - 4)) {
    uint offset;
    ECardResult result = GetSaveDataOffset(offset);
    if (result != kCR_READY) {
      mSaveFileData = rstl::vector< uchar, rstl::aligned_allocator >();
      return result;
    }
    const uint saveSize = size - offset;
    saveData.assign(saveSize);
    (memcpy)(saveData.data(), static_cast< const uchar* >(data) + offset, saveSize);
    mSaveFileData = rstl::vector< uchar, rstl::aligned_allocator >();
    return kCR_READY;
  } else {
    mSaveFileData = rstl::vector< uchar, rstl::aligned_allocator >();
    return kCR_CRC_MISMATCH;
  }
}

CMemoryCardSys::CCardFileInfo::Icon::Icon(CAssetId id, int speed, CSimplePool& pool)
: mId(id), mSpeed(speed), mTex(pool.GetObj(SObjectTag('TXTR', id))) {}

CMemoryCardSys::EMemoryCardPort SMemoryCardFileInfo::GetFileCardPort() {
  return static_cast< CMemoryCardSys::EMemoryCardPort >(mFileInfo.chan);
}

int SMemoryCardFileInfo::GetFileNo() const { return mFileInfo.fileNo; }

CMemoryCardSys::EMemoryCardPort CMemoryCardSys::CCardFileInfo::GetCardPort() {
  return static_cast< EMemoryCardPort >(mFileInfo.chan);
}

int CMemoryCardSys::CCardFileInfo::GetFileNo() { return mFileInfo.fileNo; }

ECardResult SMemoryCardFileInfo::GetSaveDataOffset(uint& offOut) {
  CardStat stat;
  ECardResult result = CMemoryCardSys::GetStatus(GetFileCardPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    offOut = -1;
    return result;
  }
  offOut = 4;
  offOut += 64;
  const int bannerFormat = stat.GetBannerFormat();
  if (bannerFormat == CARD_STAT_BANNER_C8) {
    offOut += 3584;
  } else if (bannerFormat == CARD_STAT_BANNER_RGB5A3) {
    offOut += 6144;
  }

  bool palette = false;
  int idx = 0;
  int format = stat.GetIconFormat(idx);
  while (format != CARD_STAT_ICON_NONE) {
    if (format == CARD_STAT_ICON_C8) {
      palette = true;
      offOut += 1024;
    } else {
      offOut += 2048;
    }
    ++idx;
    format = stat.GetIconFormat(idx);
  }
  if (palette) {
    offOut += 512;
  }
  return kCR_READY;
}

void CMemoryCardSys::CCardFileInfo::WriteBannerData(COutputStream& out) {
  if (mBannerTex != kInvalidAssetId) {
    const CTexture& texture = ***mBannerTok;
    const ETexelFormat format = texture.GetTexelFormat();
    const void* data = texture.GetConstBitMapData(0);
    const uint size = format == kTF_RGB5A3 ? 6144 : 3072;
    out.Put(data, size);
    if (format == kTF_C8) {
      out.Put(texture.GetPalette()->GetPaletteData(), 512);
    }
  }
}

void CMemoryCardSys::CCardFileInfo::WriteIconData(COutputStream& out) {
  const void* palette = nullptr;
  for (int i = 0; i < mIconToks.size(); ++i) {
    const CTexture& texture = **mIconToks[i].mTex;
    const ETexelFormat format = texture.GetTexelFormat();
    const void* data = texture.GetConstBitMapData(0);
    const uint size = format == kTF_RGB5A3 ? 2048 : 1024;
    out.Put(data, size);
    if (format == kTF_C8) {
      palette = texture.GetPalette()->GetPaletteData();
    }
  }
  if (palette != nullptr) {
    out.Put(palette, 512);
  }
}

void CMemoryCardSys::CCardFileInfo::BuildCardBuffer() {
  const uint bannerSize = CalculateBannerDataSize();
  const uint totalSize = (bannerSize + mSaveBuffer.size() + 8191) & ~8191;
  mCardBuffer.assign(totalSize);
  void* data = mCardBuffer.data();
  {
    CMemoryStreamOut out(data, totalSize);
    out.WriteInt32(0);
    char comment[64];
    strncpy(comment, mComment.data(), sizeof(comment));
    out.Put(comment, sizeof(comment));
    WriteBannerData(out);
    WriteIconData(out);
  }
  (memcpy)(mCardBuffer.data() + bannerSize, mSaveBuffer.data(), mSaveBuffer.size());
  *static_cast< uint* >(data) =
      CCRC32::Calculate(static_cast< const uchar* >(data) + 4, totalSize - 4);
#if NONMATCHING
  *static_cast< uint* >(data) = CBasics::SwapBytes(*static_cast< uint* >(data));
#endif
  mSaveBuffer = rstl::vector< uchar >();
}

uint CMemoryCardSys::CCardFileInfo::CalculateTotalDataSize() {
  const uint saveSize = mSaveBuffer.size();
  uint totalSize = CalculateBannerDataSize();
  totalSize += saveSize;
  return (totalSize + 8191) & ~8191;
}

uint CMemoryCardSys::CCardFileInfo::CalculateBannerDataSize() {
  uint size = 68;
  if (mBannerTex != kInvalidAssetId) {
    if ((*mBannerTok)->GetTexelFormat() == kTF_RGB5A3) {
      size = 6212;
    } else {
      size = 3652;
    }
  }
  bool palette = false;
  for (int i = 0; i < mIconToks.size(); ++i) {
    if (mIconToks[i].mTex->GetTexelFormat() == kTF_RGB5A3) {
      size += 2048;
    } else {
      size += 1024;
      palette = true;
    }
  }
  if (palette) {
    size += 512;
  }
  return size;
}

int CardStat::GetFileLength() { return mStat.length; }

int CardStat::GetTime() const { return mStat.time; }

int CardStat::GetBannerFormat() { return CARDGetBannerFormat(&mStat); }

int CardStat::GetIconFormat(int idx) { return CARDGetIconFormat(&mStat, idx); }

int CardStat::GetCommentAddr() const { return mStat.commentAddr; }

void CardStat::SetBannerFormat(int format) { CARDSetBannerFormat(&mStat, format); }

void CardStat::SetIconFormat(int format, int idx) { CARDSetIconFormat(&mStat, idx, format); }

void CardStat::SetIconSpeed(int speed, int idx) { CARDSetIconSpeed(&mStat, idx, speed); }

void CardStat::SetIconAddr(int addr) { CARDSetIconAddress(&mStat, addr); }

void CardStat::SetCommentAddr(int addr) { CARDSetCommentAddress(&mStat, addr); }

void CMemoryCardSys::CCardFileInfo::SetComment(const rstl::string& comment) {
  mComment = comment;
}

CMemoryCardSys::CMemoryCardSys() {
  Initialize();
  mIsCardSysExists = true;
}

void CMemoryCardSys::Initialize() {
  if (!mIsInitialized) {
#if TARGET_PC
    CARDSetLoadType(CARD_RAWIMAGE);
    // CARDDetectDolphin(-1);
    CARDInit("GM8E", "01");
#else
    CARDInit();
#endif
    mIsInitialized = true;
  }
}

CMemoryCardSys::~CMemoryCardSys() {
  mIsCardSysExists = false;
  FreeCardWorkArea(kCS_SlotA);
  FreeCardWorkArea(kCS_SlotB);
}

ProbeResults CMemoryCardSys::IsMemoryCardInserted(EMemoryCardPort port) {
  ProbeResults result;
  result.mError =
      static_cast< ECardResult >(CARDProbeEx(port, &result.mCardSize, &result.mSectorSize));
  return result;
}

ECardResult CMemoryCardSys::GetResultCode(int port) {
  return static_cast< ECardResult >(CARDGetResultCode(port));
}

ECardResult CMemoryCardSys::MountCard(EMemoryCardPort port) {
  return static_cast< ECardResult >(
      CARDMountAsync(port, AllocCardWorkArea(port), nullptr, nullptr));
}

ECardResult CMemoryCardSys::UnmountCard(EMemoryCardPort port) {
  ECardResult result = static_cast< ECardResult >(CARDUnmount(port));
  FreeCardWorkArea(port);
  return result;
}

ECardResult CMemoryCardSys::FormatCard(EMemoryCardPort port) {
#if defined(TARGET_PC)
  return static_cast< ECardResult >(CARDFormat(port));
#else
  return static_cast< ECardResult >(CARDFormatAsync(port, nullptr));
#endif
}

ECardResult CMemoryCardSys::GetNumFreeBytes(EMemoryCardPort port, uint& freeBytes,
                                            uint& freeFiles) {
  s32 bytes;
  s32 files;
  ECardResult result = static_cast< ECardResult >(CARDFreeBlocks(port, &bytes, &files));
  freeBytes = bytes;
  freeFiles = files;
  return result;
}

SMemoryCardFileInfo::SMemoryCardFileInfo(int cardPort, const rstl::string& name) : mName(name) {
  mFileInfo.chan = cardPort;
  mFileInfo.fileNo = -1;
}

CMemoryCardSys::CCardFileInfo::CCardFileInfo(EMemoryCardPort port, const rstl::string& name)
: mStatus(kS_Standby), mFileName(name), x38_(0), mBannerTex(kInvalidAssetId) {
  mFileInfo.chan = port;
  mFileInfo.fileNo = -1;
}

void CMemoryCardSys::CCardFileInfo::LockBannerToken(CAssetId bannerTxtr, CSimplePool& pool) {
  mBannerTex = bannerTxtr;
  mBannerTok = TLockedToken< CTexture >(pool.GetObj(SObjectTag('TXTR', mBannerTex)));
}

void CMemoryCardSys::CCardFileInfo::LockIconToken(CAssetId iconTxtr, int speed, CSimplePool& pool) {
  mIconToks.push_back(Icon(iconTxtr, speed, pool));
}

ECardResult SMemoryCardFileInfo::Open() {
  return static_cast< ECardResult >(CARDOpen(GetFileCardPort(), mName.data(), &mFileInfo));
}

ECardResult CMemoryCardSys::CCardFileInfo::CreateFile() {
  const uint size = CalculateTotalDataSize();
  return static_cast< ECardResult >(
      CARDCreateAsync(GetCardPort(), mFileName.data(), size, &mFileInfo, nullptr));
}

ECardResult CMemoryCardSys::DeleteFile(EMemoryCardPort port, const rstl::string& name) {
  return static_cast< ECardResult >(CARDDeleteAsync(port, name.data(), nullptr));
}

ECardResult CMemoryCardSys::FastDeleteFile(EMemoryCardPort port, int fileNo) {
  return static_cast< ECardResult >(CARDFastDeleteAsync(port, fileNo, nullptr));
}

ECardResult SMemoryCardFileInfo::Close() {
  const CMemoryCardSys::EMemoryCardPort port = GetFileCardPort();
  ECardResult result = static_cast< ECardResult >(CARDClose(&mFileInfo));
  mFileInfo.chan = port;
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::CloseFile() {
  const EMemoryCardPort port = GetCardPort();
  ECardResult result = static_cast< ECardResult >(CARDClose(&mFileInfo));
  mFileInfo.chan = port;
  return result;
}

ECardResult CMemoryCardSys::Rename(EMemoryCardPort port, const rstl::string& oldName,
                                   const rstl::string& newName) {
  return static_cast< ECardResult >(CARDRenameAsync(port, oldName.data(), newName.data(), nullptr));
}

ECardResult CMemoryCardSys::CheckCard(EMemoryCardPort port) {
  return static_cast< ECardResult >(CARDCheckAsync(port, nullptr));
}

ECardResult CMemoryCardSys::CCardFileInfo::WriteFile() {
  BuildCardBuffer();
  void* data = mCardBuffer.data();
  const int size = mCardBuffer.size();
  DCStoreRange(data, size);
  ECardResult result =
      static_cast< ECardResult >(CARDWriteAsync(&mFileInfo, data, size, 0, nullptr));
  mStatus = kS_Transferring;
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::PumpCardTransfer() {
  if (mStatus == kS_Standby) {
    return kCR_READY;
  } else if (mStatus == kS_Transferring) {
    ECardResult result = GetResultCode(GetCardPort());
    if (result != kCR_BUSY) {
      mCardBuffer = rstl::vector< uchar, rstl::aligned_allocator >();
    }
    if (result != kCR_READY) {
      return result;
    }
    mStatus = kS_Done;
    CardStat stat;
    result = GetStatus(stat);
    if (result != kCR_READY) {
      return result;
    }
    result = SetStatus(GetCardPort(), GetFileNo(), stat);
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else {
    ECardResult result = GetResultCode(GetCardPort());
    if (result != kCR_READY) {
      return result;
    }
    mStatus = kS_Standby;
    return kCR_READY;
  }
}

ECardResult CMemoryCardSys::CCardFileInfo::GetStatus(CardStat& stat) {
  ECardResult result = CMemoryCardSys::GetStatus(GetCardPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  stat.SetCommentAddr(4);
  stat.SetIconAddr(68);
  int bannerFormat;
  if (mBannerTex == kInvalidAssetId) {
    bannerFormat = CARD_STAT_BANNER_NONE;
  } else if ((*mBannerTok)->GetTexelFormat() == kTF_RGB5A3) {
    bannerFormat = CARD_STAT_BANNER_RGB5A3;
  } else {
    bannerFormat = CARD_STAT_BANNER_C8;
  }
  stat.SetBannerFormat(bannerFormat);
  int i = 0;
  for (; i < mIconToks.size(); ++i) {
    int format = CARD_STAT_ICON_C8;
    if (mIconToks[i].mTex->GetTexelFormat() == kTF_RGB5A3) {
      format = CARD_STAT_ICON_RGB5A3;
    }
    stat.SetIconFormat(format, i);
    stat.SetIconSpeed(mIconToks[i].mSpeed, i);
  }
  if (i < 8) {
    stat.SetIconFormat(CARD_STAT_ICON_NONE, i);
    stat.SetIconSpeed(CARD_STAT_SPEED_END, i);
  }
  return kCR_READY;
}

ECardResult SMemoryCardFileInfo::StartRead() {
  CardStat stat;
  ECardResult result = CMemoryCardSys::GetStatus(GetFileCardPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  const uint size = stat.GetFileLength();
  mSaveData = rstl::vector< uchar >();
  mSaveFileData.assign(size);
  return static_cast< ECardResult >(
      CARDReadAsync(&mFileInfo, mSaveFileData.data(), size, 0, nullptr));
}

ECardResult SMemoryCardFileInfo::TryFileRead() {
  ECardResult result = CMemoryCardSys::GetResultCode(GetFileCardPort());
  if (result != kCR_READY) {
    return result;
  }
  return FileRead();
}

ECardResult CMemoryCardSys::GetSerialNo(EMemoryCardPort port, long long& serialOut) {
  return static_cast< ECardResult >(CARDGetSerialNo(port, reinterpret_cast< u64* >(&serialOut)));
}

ECardResult CMemoryCardSys::GetStatus(EMemoryCardPort port, int fileNo, CardStat& statOut) {
  CARDStat stat;
  ECardResult result = static_cast< ECardResult >(CARDGetStatus(port, fileNo, &stat));
  memcpy(&statOut.mStat, &stat, sizeof(stat));
  return result;
}

ECardResult CMemoryCardSys::SetStatus(EMemoryCardPort port, int fileNo, const CardStat& stat) {
  return static_cast< ECardResult >(
      CARDSetStatusAsync(port, fileNo, const_cast< CARDStat* >(&stat.mStat), nullptr));
}

rstl::vector< char, rstl::aligned_allocator >&
CMemoryCardSys::WorkAreaVector(EMemoryCardPort port) {
  switch (port) {
  case kCS_SlotA:
    return mWorkAreaA;
  case kCS_SlotB:
    return mWorkAreaB;
  default:
    return mWorkAreaA;
  }
}

char* CMemoryCardSys::AllocCardWorkArea(EMemoryCardPort port) {
  rstl::vector< char, rstl::aligned_allocator >& area = WorkAreaVector(port);
  area.resize(0xa000);
  char* data = area.data();
  DCInvalidateRange(data, area.size());
  return data;
}

void CMemoryCardSys::FreeCardWorkArea(EMemoryCardPort port) {
  rstl::vector< char, rstl::aligned_allocator >& area = WorkAreaVector(port);
  area = rstl::vector< char, rstl::aligned_allocator >();
}
