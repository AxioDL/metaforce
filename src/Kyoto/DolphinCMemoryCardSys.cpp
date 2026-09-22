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
  rstl::vector< uchar >& saveData = x34_saveData;
  saveData = rstl::vector< uchar >();
  const uint size = x24_saveFileData.size();
  const void* data = x24_saveFileData.data();
#if NONMATCHING
  const uint crc = CBasics::SwapBytes(*static_cast< const uint* >(data));
#else
  const uint crc = *static_cast< const uint* >(data);
#endif
  if (crc == CCRC32::Calculate(static_cast< const uchar* >(data) + 4, size - 4)) {
    uint offset;
    ECardResult result = GetSaveDataOffset(offset);
    if (result != kCR_READY) {
      x24_saveFileData = rstl::vector< uchar, rstl::aligned_allocator >();
      return result;
    }
    const uint saveSize = size - offset;
    saveData.assign(saveSize);
    (memcpy)(saveData.data(), static_cast< const uchar* >(data) + offset, saveSize);
    x24_saveFileData = rstl::vector< uchar, rstl::aligned_allocator >();
    return kCR_READY;
  } else {
    x24_saveFileData = rstl::vector< uchar, rstl::aligned_allocator >();
    return kCR_CRC_MISMATCH;
  }
}

CMemoryCardSys::CCardFileInfo::Icon::Icon(CAssetId id, int speed, CSimplePool& pool)
: x0_id(id)
, x4_speed(speed)
, x8_tex(pool.GetObj(SObjectTag('TXTR', id))) {}

CMemoryCardSys::EMemoryCardPort SMemoryCardFileInfo::GetFileCardPort() {
  return static_cast< CMemoryCardSys::EMemoryCardPort >(x0_fileInfo.chan);
}

int SMemoryCardFileInfo::GetFileNo() const { return x0_fileInfo.fileNo; }

CMemoryCardSys::EMemoryCardPort CMemoryCardSys::CCardFileInfo::GetCardPort() {
  return static_cast< EMemoryCardPort >(x4_fileInfo.chan);
}

int CMemoryCardSys::CCardFileInfo::GetFileNo() { return x4_fileInfo.fileNo; }

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
  if (x3c_bannerTex != kInvalidAssetId) {
    const CTexture& texture = ***x40_bannerTok;
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
  for (int i = 0; i < x50_iconToks.size(); ++i) {
    const CTexture& texture = **x50_iconToks[i].x8_tex;
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
  const uint totalSize = (bannerSize + xf4_saveBuffer.size() + 8191) & ~8191;
  x104_cardBuffer.assign(totalSize);
  void* data = x104_cardBuffer.data();
  {
    CMemoryStreamOut out(data, totalSize);
    out.WriteInt32(0);
    char comment[64];
    strncpy(comment, x28_comment.data(), sizeof(comment));
    out.Put(comment, sizeof(comment));
    WriteBannerData(out);
    WriteIconData(out);
  }
  (memcpy)(x104_cardBuffer.data() + bannerSize, xf4_saveBuffer.data(), xf4_saveBuffer.size());
  *static_cast< uint* >(data) =
      CCRC32::Calculate(static_cast< const uchar* >(data) + 4, totalSize - 4);
#if NONMATCHING
  *static_cast< uint* >(data) = CBasics::SwapBytes(*static_cast< uint* >(data));
#endif
  xf4_saveBuffer = rstl::vector< uchar >();
}

uint CMemoryCardSys::CCardFileInfo::CalculateTotalDataSize() {
  const uint saveSize = xf4_saveBuffer.size();
  uint totalSize = CalculateBannerDataSize();
  totalSize += saveSize;
  return (totalSize + 8191) & ~8191;
}

uint CMemoryCardSys::CCardFileInfo::CalculateBannerDataSize() {
  uint size = 68;
  if (x3c_bannerTex != kInvalidAssetId) {
    if ((*x40_bannerTok)->GetTexelFormat() == kTF_RGB5A3) {
      size = 6212;
    } else {
      size = 3652;
    }
  }
  bool palette = false;
  for (int i = 0; i < x50_iconToks.size(); ++i) {
    if (x50_iconToks[i].x8_tex->GetTexelFormat() == kTF_RGB5A3) {
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

int CardStat::GetFileLength() { return x0_stat.length; }

int CardStat::GetTime() const { return x0_stat.time; }

int CardStat::GetBannerFormat() { return CARDGetBannerFormat(&x0_stat); }

int CardStat::GetIconFormat(int idx) { return CARDGetIconFormat(&x0_stat, idx); }

int CardStat::GetCommentAddr() const { return x0_stat.commentAddr; }

void CardStat::SetBannerFormat(int format) { CARDSetBannerFormat(&x0_stat, format); }

void CardStat::SetIconFormat(int format, int idx) { CARDSetIconFormat(&x0_stat, idx, format); }

void CardStat::SetIconSpeed(int speed, int idx) { CARDSetIconSpeed(&x0_stat, idx, speed); }

void CardStat::SetIconAddr(int addr) { CARDSetIconAddress(&x0_stat, addr); }

void CardStat::SetCommentAddr(int addr) { CARDSetCommentAddress(&x0_stat, addr); }

void CMemoryCardSys::CCardFileInfo::SetComment(const rstl::string& comment) {
  x28_comment = comment;
}

CMemoryCardSys::CMemoryCardSys() {
  Initialize();
  mIsCardSysExists = true;
}

#if !TARGET_PC
void CMemoryCardSys::Initialize() {
  if (!mIsInitialized) {
    CARDInit();
    mIsInitialized = true;
  }
}
#endif

CMemoryCardSys::~CMemoryCardSys() {
  mIsCardSysExists = false;
  FreeCardWorkArea(kCS_SlotA);
  FreeCardWorkArea(kCS_SlotB);
}

ProbeResults CMemoryCardSys::IsMemoryCardInserted(EMemoryCardPort port) {
  ProbeResults result;
  result.x0_error =
      static_cast< ECardResult >(CARDProbeEx(port, &result.x4_cardSize, &result.x8_sectorSize));
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
  return static_cast< ECardResult >(CARDFormatAsync(port, nullptr));
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

SMemoryCardFileInfo::SMemoryCardFileInfo(int cardPort, const rstl::string& name) : x14_name(name) {
  x0_fileInfo.chan = cardPort;
  x0_fileInfo.fileNo = -1;
}

CMemoryCardSys::CCardFileInfo::CCardFileInfo(EMemoryCardPort port, const rstl::string& name)
: x0_status(kS_Standby)
, x18_fileName(name)
, x38_(0)
, x3c_bannerTex(kInvalidAssetId) {
  x4_fileInfo.chan = port;
  x4_fileInfo.fileNo = -1;
}

void CMemoryCardSys::CCardFileInfo::LockBannerToken(CAssetId bannerTxtr, CSimplePool& pool) {
  x3c_bannerTex = bannerTxtr;
  x40_bannerTok = TLockedToken< CTexture >(pool.GetObj(SObjectTag('TXTR', x3c_bannerTex)));
}

void CMemoryCardSys::CCardFileInfo::LockIconToken(CAssetId iconTxtr, int speed, CSimplePool& pool) {
  x50_iconToks.push_back(Icon(iconTxtr, speed, pool));
}

ECardResult SMemoryCardFileInfo::Open() {
  return static_cast< ECardResult >(CARDOpen(GetFileCardPort(), x14_name.data(), &x0_fileInfo));
}

ECardResult CMemoryCardSys::CCardFileInfo::CreateFile() {
  const uint size = CalculateTotalDataSize();
  return static_cast< ECardResult >(
      CARDCreateAsync(GetCardPort(), x18_fileName.data(), size, &x4_fileInfo, nullptr));
}

ECardResult CMemoryCardSys::DeleteFile(EMemoryCardPort port, const rstl::string& name) {
  return static_cast< ECardResult >(CARDDeleteAsync(port, name.data(), nullptr));
}

ECardResult CMemoryCardSys::FastDeleteFile(EMemoryCardPort port, int fileNo) {
  return static_cast< ECardResult >(CARDFastDeleteAsync(port, fileNo, nullptr));
}

ECardResult SMemoryCardFileInfo::Close() {
  const CMemoryCardSys::EMemoryCardPort port = GetFileCardPort();
  ECardResult result = static_cast< ECardResult >(CARDClose(&x0_fileInfo));
  x0_fileInfo.chan = port;
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::CloseFile() {
  const EMemoryCardPort port = GetCardPort();
  ECardResult result = static_cast< ECardResult >(CARDClose(&x4_fileInfo));
  x4_fileInfo.chan = port;
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
  void* data = x104_cardBuffer.data();
  const int size = x104_cardBuffer.size();
  DCStoreRange(data, size);
  ECardResult result =
      static_cast< ECardResult >(CARDWriteAsync(&x4_fileInfo, data, size, 0, nullptr));
  x0_status = kS_Transferring;
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::PumpCardTransfer() {
  if (x0_status == kS_Standby) {
    return kCR_READY;
  } else if (x0_status == kS_Transferring) {
    ECardResult result = GetResultCode(GetCardPort());
    if (result != kCR_BUSY) {
      x104_cardBuffer = rstl::vector< uchar, rstl::aligned_allocator >();
    }
    if (result != kCR_READY) {
      return result;
    }
    x0_status = kS_Done;
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
    x0_status = kS_Standby;
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
  if (x3c_bannerTex == kInvalidAssetId) {
    bannerFormat = CARD_STAT_BANNER_NONE;
  } else if ((*x40_bannerTok)->GetTexelFormat() == kTF_RGB5A3) {
    bannerFormat = CARD_STAT_BANNER_RGB5A3;
  } else {
    bannerFormat = CARD_STAT_BANNER_C8;
  }
  stat.SetBannerFormat(bannerFormat);
  int i = 0;
  for (; i < x50_iconToks.size(); ++i) {
    int format = CARD_STAT_ICON_C8;
    if (x50_iconToks[i].x8_tex->GetTexelFormat() == kTF_RGB5A3) {
      format = CARD_STAT_ICON_RGB5A3;
    }
    stat.SetIconFormat(format, i);
    stat.SetIconSpeed(x50_iconToks[i].x4_speed, i);
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
  x34_saveData = rstl::vector< uchar >();
  x24_saveFileData.assign(size);
  return static_cast< ECardResult >(
      CARDReadAsync(&x0_fileInfo, x24_saveFileData.data(), size, 0, nullptr));
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
  memcpy(&statOut.x0_stat, &stat, sizeof(stat));
  return result;
}

ECardResult CMemoryCardSys::SetStatus(EMemoryCardPort port, int fileNo, const CardStat& stat) {
  return static_cast< ECardResult >(
      CARDSetStatusAsync(port, fileNo, const_cast< CARDStat* >(&stat.x0_stat), nullptr));
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
