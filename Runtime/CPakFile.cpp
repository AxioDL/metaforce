#include "Runtime/CPakFile.hpp"

namespace metaforce {
static logvisor::Module Log("metaforce::CPakFile");

CPakFile::CPakFile(std::string_view filename, bool buildDepList, bool worldPak, bool override) : CDvdFile(filename) {
  if (!CDvdFile::operator bool())
    Log.report(logvisor::Fatal, FMT_STRING("{}: Unable to open"), GetPath());
  x28_24_buildDepList = buildDepList;
  // x28_24_buildDepList = true; // Always do this so metaforce can rapidly pre-warm shaders
  x28_26_worldPak = worldPak;
  m_override = override;
}

CPakFile::~CPakFile() {
  if (x30_dvdReq)
    x30_dvdReq->PostCancelRequest();
}

const SObjectTag* CPakFile::GetResIdByName(std::string_view name) const {
  for (const std::pair<std::string, SObjectTag>& p : x54_nameList) {
    if (CStringExtras::CompareCaseInsensitive(p.first, name)) {
      return &p.second;
    }
  }
  return nullptr;
}

void CPakFile::LoadResourceTable(CInputStream& r) {
  x74_resList.reserve(
      std::max(size_t(64), size_t(ROUND_UP_32(x4c_resTableCount * sizeof(SResInfo)) + sizeof(SResInfo) - 1)) /
      sizeof(SResInfo));
  if (x28_24_buildDepList)
    x64_depList.reserve(x4c_resTableCount);
  for (u32 i = 0; i < x4c_resTableCount; ++i) {
    u32 flags = r.ReadLong();
    FourCC fcc;
    r.ReadBytes(reinterpret_cast<u8*>(&fcc), 4);
    CAssetId id = r.ReadLong();
    u32 size = r.ReadLong();
    u32 offset = r.ReadLong();
    if (fcc == FOURCC('MLVL'))
      m_mlvlId = id;
    x74_resList.emplace_back(id, fcc, offset, size, flags);
    if (x28_24_buildDepList)
      x64_depList.push_back(id);
  }
  std::sort(x74_resList.begin(), x74_resList.end(), [](const auto& a, const auto& b) { return a.x0_id < b.x0_id; });
}

void CPakFile::DataLoad() {
  x30_dvdReq.reset();
  CMemoryInStream r(x38_headerData.data() + x48_resTableOffset, x38_headerData.size() - x48_resTableOffset,
                    CMemoryInStream::EOwnerShip::NotOwned);
  LoadResourceTable(r);
  x2c_asyncLoadPhase = EAsyncPhase::Loaded;
  if (x28_26_worldPak) {
    // Allocate ARAM space DMA x74_resList to ARAM
  }
  x38_headerData.clear();
}

void CPakFile::InitialHeaderLoad() {
  CMemoryInStream r(x38_headerData.data(), x38_headerData.size(), CMemoryInStream::EOwnerShip::NotOwned);
  x30_dvdReq.reset();
  u32 version = r.ReadLong();
  if (version != 0x00030005) {
    Log.report(logvisor::Fatal,
               FMT_STRING("{}: Incompatible pak file version -- Current version is {:08X}, you're using {:08X}"),
               GetPath(), 0x00030005, version);
    return;
  }

  r.ReadLong();
  u32 nameCount = r.ReadLong();
  x54_nameList.reserve(nameCount);
  for (u32 i = 0; i < nameCount; ++i) {
    SObjectTag tag(r);
    auto name = CStringExtras::ReadString(r);
    x54_nameList.emplace_back(name, tag);
  }

  x4c_resTableCount = r.ReadLong();
  x48_resTableOffset = u32(r.GetReadPosition());
  x2c_asyncLoadPhase = EAsyncPhase::DataLoad;
  u32 newSize = ROUND_UP_32(x4c_resTableCount * 20 + x48_resTableOffset);
  u32 origSize = u32(x38_headerData.size());
  if (newSize > origSize) {
    x38_headerData.resize(newSize);
    x30_dvdReq = AsyncSeekRead(x38_headerData.data() + origSize, u32(x38_headerData.size() - origSize),
                               ESeekOrigin::Begin, origSize);
  } else {
    DataLoad();
  }
};

void CPakFile::Warmup() {
  u32 length = std::min(u32(Length()), u32(8192));
  x38_headerData.resize(length);
  x30_dvdReq = AsyncSeekRead(x38_headerData.data(), length, ESeekOrigin::Cur, 0);
  x2c_asyncLoadPhase = EAsyncPhase::InitialHeader;
}

const CPakFile::SResInfo* CPakFile::GetResInfoForLoadPreferForward(CAssetId id) const {
  if (x28_27_stashedInARAM)
    return nullptr;
  auto search =
      rstl::binary_find(x74_resList.begin(), x74_resList.end(), id, [](const SResInfo& test) { return test.x0_id; });
  if (search == x74_resList.end())
    return nullptr;
  const SResInfo* bestInfo = &*search;
  s32 bestDelta = x84_currentSeek - bestInfo->GetOffset();
  while (++search != x74_resList.end()) {
    const SResInfo* thisInfo = &*search;
    if (thisInfo->x0_id != id)
      break;
    s32 thisDelta = x84_currentSeek - bestInfo->GetOffset();
    if ((bestDelta < 0 && (thisDelta > 0 || thisDelta > bestDelta)) ||
        (bestDelta >= 0 && thisDelta > 0 && thisDelta < bestDelta)) {
      bestDelta = thisDelta;
      bestInfo = thisInfo;
    }
  }
  x84_currentSeek = bestInfo->GetOffset() + bestInfo->GetSize();
  return bestInfo;
}

const CPakFile::SResInfo* CPakFile::GetResInfoForLoadDirectionless(CAssetId id) const {
  if (x28_27_stashedInARAM)
    return nullptr;
  auto search =
      rstl::binary_find(x74_resList.begin(), x74_resList.end(), id, [](const SResInfo& test) { return test.x0_id; });
  if (search == x74_resList.end())
    return nullptr;
  const SResInfo* bestInfo = &*search;
  s32 bestDelta = std::abs(s32(x84_currentSeek - bestInfo->GetOffset()));
  while (++search != x74_resList.end()) {
    const SResInfo* thisInfo = &*search;
    if (thisInfo->x0_id != id)
      break;
    s32 thisDelta = std::abs(s32(x84_currentSeek - bestInfo->GetOffset()));
    if (thisDelta < bestDelta) {
      bestDelta = thisDelta;
      bestInfo = thisInfo;
    }
  }
  x84_currentSeek = bestInfo->GetOffset() + bestInfo->GetSize();
  return bestInfo;
}

const CPakFile::SResInfo* CPakFile::GetResInfo(CAssetId id) const {
  if (x2c_asyncLoadPhase != EAsyncPhase::Loaded)
    return nullptr;
  if (x28_27_stashedInARAM)
    return nullptr;
  auto search =
      rstl::binary_find(x74_resList.begin(), x74_resList.end(), id, [](const SResInfo& i) { return i.x0_id; });
  if (search == x74_resList.end())
    return nullptr;
  return &*search;
}

void CPakFile::AsyncIdle() {
  if (x2c_asyncLoadPhase == EAsyncPhase::Loaded)
    return;
  if (x30_dvdReq && !x30_dvdReq->IsComplete())
    return;
  switch (x2c_asyncLoadPhase) {
  case EAsyncPhase::Warmup:
    Warmup();
    break;
  case EAsyncPhase::InitialHeader:
    InitialHeaderLoad();
    break;
  case EAsyncPhase::DataLoad:
    DataLoad();
    break;
  default:
    break;
  }
}

} // namespace metaforce
