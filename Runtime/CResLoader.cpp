#include "CResLoader.hpp"
#include "CPakFile.hpp"

namespace urde {
static logvisor::Module Log("CResLoader");

CResLoader::CResLoader() { x48_curPak = x18_pakLoadedList.end(); }

const std::vector<CAssetId>* CResLoader::GetTagListForFile(std::string_view name) const {
  std::string namePak = std::string(name) + ".upak";
  for (const std::unique_ptr<CPakFile>& pak : x18_pakLoadedList)
    if (!CStringExtras::CompareCaseInsensitive(namePak, pak->x18_path))
      return &pak->GetDepList();
  return nullptr;
}

void CResLoader::AddPakFileAsync(std::string_view name, bool buildDepList, bool worldPak) {
  std::string namePak = std::string(name) + ".upak";
  if (CDvdFile::FileExists(namePak.c_str())) {
    x30_pakLoadingList.emplace_back(new CPakFile(namePak, buildDepList, worldPak));
    ++x44_pakLoadingCount;
  }
}

void CResLoader::AddPakFile(std::string_view name, bool samusPak, bool worldPak) {
  AddPakFileAsync(name, samusPak, worldPak);
  WaitForPakFileLoadingComplete();
}

void CResLoader::WaitForPakFileLoadingComplete() {
  while (x44_pakLoadingCount)
    AsyncIdlePakLoading();
}

std::unique_ptr<CInputStream> CResLoader::LoadNewResourcePartSync(const SObjectTag& tag, u32 length, u32 offset,
                                                                  void* extBuf) {
  void* buf = extBuf;
  CPakFile* file = FindResourceForLoad(tag);
  if (!buf)
    buf = new u8[length];
  file->SyncSeekRead(buf, length, ESeekOrigin::Begin, x50_cachedResInfo->GetOffset() + offset);
  return std::unique_ptr<CInputStream>(new athena::io::MemoryReader((atUint8*)buf, length, !extBuf));
}

void CResLoader::LoadMemResourceSync(const SObjectTag& tag, std::unique_ptr<u8[]>& bufOut, int* sizeOut) {
  if (CPakFile* file = FindResourceForLoad(tag)) {
    bufOut = std::unique_ptr<u8[]>(new u8[x50_cachedResInfo->GetSize()]);
    file->SyncSeekRead(bufOut.get(), x50_cachedResInfo->GetSize(), ESeekOrigin::Begin, x50_cachedResInfo->GetOffset());
    *sizeOut = x50_cachedResInfo->GetSize();
  }
}

std::unique_ptr<CInputStream> CResLoader::LoadResourceFromMemorySync(const SObjectTag& tag, const void* buf) {
  FindResourceForLoad(tag);
  CInputStream* newStrm = new athena::io::MemoryReader((atUint8*)buf, x50_cachedResInfo->GetSize());
  if (x50_cachedResInfo->IsCompressed()) {
    newStrm->readUint32Big();
    newStrm = new CZipInputStream(std::unique_ptr<CInputStream>(newStrm));
  }
  return std::unique_ptr<CInputStream>(newStrm);
}

std::unique_ptr<CInputStream> CResLoader::LoadNewResourceSync(const SObjectTag& tag, void* extBuf) {
  void* buf = extBuf;
  if (CPakFile* file = FindResourceForLoad(tag)) {
    size_t resSz = ROUND_UP_32(x50_cachedResInfo->GetSize());
    if (!buf)
      buf = new u8[resSz];
    file->SyncSeekRead(buf, resSz, ESeekOrigin::Begin, x50_cachedResInfo->GetOffset());
    CInputStream* newStrm = new athena::io::MemoryReader((atUint8*)buf, resSz, !extBuf);
    if (x50_cachedResInfo->IsCompressed()) {
      newStrm->readUint32Big();
      newStrm = new CZipInputStream(std::unique_ptr<CInputStream>(newStrm));
    }
    return std::unique_ptr<CInputStream>(newStrm);
  }
  return {};
}

std::shared_ptr<IDvdRequest> CResLoader::LoadResourcePartAsync(const SObjectTag& tag, u32 off, u32 size, void* buf) {
  CPakFile* file = FindResourceForLoad(tag.id);
  return file->AsyncSeekRead(buf, size, ESeekOrigin::Begin, x50_cachedResInfo->GetOffset() + off);
}

std::shared_ptr<IDvdRequest> CResLoader::LoadResourceAsync(const SObjectTag& tag, void* buf) {
  CPakFile* file = FindResourceForLoad(tag.id);
  return file->AsyncSeekRead(buf, ROUND_UP_32(x50_cachedResInfo->GetSize()), ESeekOrigin::Begin,
                             x50_cachedResInfo->GetOffset());
}

std::unique_ptr<u8[]> CResLoader::LoadResourceSync(const urde::SObjectTag& tag) {
  CPakFile* file = FindResourceForLoad(tag.id);
  u32 size = ROUND_UP_32(x50_cachedResInfo->GetSize());
  std::unique_ptr<u8[]> ret(new u8[size]);
  file->SyncSeekRead(ret.get(), size, ESeekOrigin::Begin, x50_cachedResInfo->GetOffset());
  return ret;
}

std::unique_ptr<u8[]> CResLoader::LoadNewResourcePartSync(const urde::SObjectTag& tag, u32 off, u32 size) {
  CPakFile* file = FindResourceForLoad(tag.id);
  std::unique_ptr<u8[]> ret(new u8[size]);
  file->SyncSeekRead(ret.get(), size, ESeekOrigin::Begin, x50_cachedResInfo->GetOffset() + off);
  return ret;
}

void CResLoader::GetTagListForFile(const char* pakName, std::vector<SObjectTag>& out) const {
  std::string path = std::string(pakName) + ".upak";
  for (const std::unique_ptr<CPakFile>& file : x18_pakLoadedList) {
    if (!CStringExtras::CompareCaseInsensitive(file->GetPath(), path)) {
      auto& depList = file->GetDepList();
      out.reserve(depList.size());
      for (const auto& dep : depList) {
        auto resInfo = file->GetResInfo(dep);
        out.emplace_back(resInfo->GetType(), dep);
      }
      return;
    }
  }
}

bool CResLoader::GetResourceCompression(const SObjectTag& tag) {
  if (FindResource(tag.id))
    return x50_cachedResInfo->IsCompressed();
  return false;
}

u32 CResLoader::ResourceSize(const SObjectTag& tag) {
  if (FindResource(tag.id))
    return x50_cachedResInfo->GetSize();
  return 0;
}

bool CResLoader::ResourceExists(const SObjectTag& tag) { return FindResource(tag.id); }

FourCC CResLoader::GetResourceTypeById(CAssetId id) const {
  if (id.IsValid() && FindResource(id))
    return x50_cachedResInfo->GetType();
  return {};
}

const SObjectTag* CResLoader::GetResourceIdByName(std::string_view name) const {
  for (const std::unique_ptr<CPakFile>& file : x18_pakLoadedList)
    if (const SObjectTag* id = file->GetResIdByName(name))
      return id;
  return nullptr;
}

bool CResLoader::AreAllPaksLoaded() const { return x44_pakLoadingCount == 0; }

void CResLoader::AsyncIdlePakLoading() {
  for (auto it = x30_pakLoadingList.begin(); it != x30_pakLoadingList.end();) {
    (*it)->AsyncIdle();
    if ((*it)->x2c_asyncLoadPhase == CPakFile::EAsyncPhase::Loaded) {
      MoveToCorrectLoadedList(std::move(*it));
      it = x30_pakLoadingList.erase(it);
      --x44_pakLoadingCount;
      continue;
    }
    ++it;
  }
}

bool CResLoader::FindResource(CAssetId id) const {
  if (x4c_cachedResId == id)
    return true;

  if (x48_curPak != x18_pakLoadedList.end())
    if (CacheFromPak(**x48_curPak, id))
      return true;

  for (auto it = x18_pakLoadedList.begin(); it != x18_pakLoadedList.end(); ++it) {
    if (it == x48_curPak)
      continue;
    if (CacheFromPak(**it, id))
      return true;
  }

  Log.report(logvisor::Warning, fmt("Unable to find asset {}"), id);
  return false;
}

CPakFile* CResLoader::FindResourceForLoad(CAssetId id) {
  if (x48_curPak != x18_pakLoadedList.end())
    if (CacheFromPakForLoad(**x48_curPak, id))
      return &**x48_curPak;

  for (auto it = x18_pakLoadedList.begin(); it != x18_pakLoadedList.end(); ++it) {
    if (it == x48_curPak)
      continue;
    if (CacheFromPakForLoad(**it, id)) {
      x48_curPak = it;
      return &**it;
    }
  }

  Log.report(logvisor::Error, fmt("Unable to find asset {}"), id);
  return nullptr;
}

CPakFile* CResLoader::FindResourceForLoad(const SObjectTag& tag) { return FindResourceForLoad(tag.id); }

bool CResLoader::CacheFromPakForLoad(CPakFile& file, CAssetId id) {
  const CPakFile::SResInfo* info;
  if (x54_forwardSeek) {
    info = file.GetResInfoForLoadPreferForward(id);
    x54_forwardSeek = false;
  } else {
    info = file.GetResInfoForLoadDirectionless(id);
  }
  if (info) {
    x4c_cachedResId = id;
    x50_cachedResInfo = info;
    return true;
  }
  return false;
}

bool CResLoader::CacheFromPak(const CPakFile& file, CAssetId id) const {
  const CPakFile::SResInfo* info = file.GetResInfo(id);
  if (info) {
    x4c_cachedResId = id;
    x50_cachedResInfo = info;
    return true;
  }
  return false;
}

void CResLoader::MoveToCorrectLoadedList(std::unique_ptr<CPakFile>&& file) {
  x18_pakLoadedList.push_back(std::move(file));
}

std::vector<std::pair<std::string, SObjectTag>> CResLoader::GetResourceIdToNameList() const {
  std::vector<std::pair<std::string, SObjectTag>> ret;
  for (auto it = x18_pakLoadedList.begin(); it != x18_pakLoadedList.end(); ++it)
    for (const auto& name : (*it)->GetNameList())
      ret.push_back(name);
  return ret;
}

void CResLoader::EnumerateResources(const std::function<bool(const SObjectTag&)>& lambda) const {
  for (auto it = x18_pakLoadedList.begin(); it != x18_pakLoadedList.end(); ++it) {
    for (const CAssetId& id : (*it)->GetDepList()) {
      SObjectTag fcc(GetResourceTypeById(id), id);
      if (!lambda(fcc))
        return;
    }
  }
}

void CResLoader::EnumerateNamedResources(const std::function<bool(std::string_view, const SObjectTag&)>& lambda) const {
  for (auto it = x18_pakLoadedList.begin(); it != x18_pakLoadedList.end(); ++it)
    for (const auto& name : (*it)->GetNameList())
      if (!lambda(name.first, name.second))
        return;
}

} // namespace urde
