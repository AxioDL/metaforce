#include "Kyoto/CResLoader.hpp"
#include "Kyoto/CPakFile.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Streams/CZipInputStream.hpp"
#include "rstl/StringExtras.hpp"

static inline int align_size(const int size) { return (size + 31) & ~31; }

CResLoader::CResLoader()
: x48_curPak(x18_pakLoadedList.end())
, x4c_cachedResId(kInvalidAssetId)
, x50_cachedResInfo(nullptr)
, x54_forwardSeek(false) {}

CResLoader::~CResLoader() {
  for (AUTO(it, x30_pakLoadingList.begin()); it != x30_pakLoadingList.end(); ++it) {
    CPakFile* pak = it->get();
    while (!pak->IsCompletelyLoaded()) {
      pak->AsyncIdle();
    }
  }
}

void CResLoader::MoveToCorrectLoadedList(const rstl::auto_ptr< CPakFile >& pak) {
  if (pak->IsARAMPak()) {
    x0_aramList.push_back(pak);
  } else {
    x18_pakLoadedList.push_back(pak);
  }
}

bool CResLoader::CacheFromPak(const CPakFile& pak, const CAssetId asset) const {
  const CPakFile::SResInfo* resInfo = pak.GetResInfo(asset);
  if (!resInfo) {
    return false;
  }

  x4c_cachedResId = asset;
  x50_cachedResInfo = resInfo;

  return true;
}

bool CResLoader::CacheFromPakForLoad(CPakFile& pak, const CAssetId asset) {
  const CPakFile::SResInfo* resInfo = nullptr;
  if (x54_forwardSeek) {
    resInfo = pak.GetResInfoForLoadPreferForward(asset);
    x54_forwardSeek = false;
  } else {
    resInfo = pak.GetResInfoForLoadDirectionless(asset);
  }

  if (resInfo == nullptr) {
    return false;
  }

  x4c_cachedResId = asset;
  x50_cachedResInfo = resInfo;

  return true;
}

CPakFile* CResLoader::FindResourceForLoad(const SObjectTag& tag) {
  return FindResourceForLoad(tag.GetId());
}

CPakFile* CResLoader::FindResourceForLoad(const CAssetId asset) {
  rstl::list< rstl::auto_ptr< CPakFile > >::iterator it;
  for (it = x0_aramList.begin(); it != x0_aramList.end(); ++it) {
    CPakFile* pak = it->get();
    if (CacheFromPak(*pak, asset)) {
      return pak;
    }
  }

  if (x48_curPak != x18_pakLoadedList.end()) {
    CPakFile* pak = x48_curPak->get();
    if (CacheFromPakForLoad(*pak, asset)) {
      return pak;
    }
  }

  for (it = x18_pakLoadedList.begin(); it != x18_pakLoadedList.end(); ++it) {
    CPakFile* pak = it->get();
    if (x48_curPak != it && CacheFromPakForLoad(*pak, asset)) {
      x48_curPak = it;
      return pak;
    }
  }

  return nullptr;
}

CPakFile* CResLoader::FindResource(const SObjectTag& tag) {
  x54_forwardSeek = false;
  CPakFile* ret = FindResourceForLoad(tag);
  x54_forwardSeek = true;
  return ret;
}

bool CResLoader::ResourceExists(CAssetId asset) {
  if (x4c_cachedResId == asset) {
    return true;
  }

  for (AUTO(it, x0_aramList.begin()); it != x0_aramList.end(); ++it) {
    if (CacheFromPak(**it, asset)) {
      return true;
    }
  }

  if (x48_curPak != x18_pakLoadedList.end()) {
    if (CacheFromPak(**x48_curPak, asset)) {
      return true;
    }
  }

  for (AUTO(it, x18_pakLoadedList.begin()); it != x18_pakLoadedList.end(); ++it) {
    if (x48_curPak != it && CacheFromPak(**it, asset)) {
      return true;
    }
  }

  return false;
}

void CResLoader::ClearCache() {
  x48_curPak = x18_pakLoadedList.end();
  x4c_cachedResId = kInvalidAssetId;
  x50_cachedResInfo = nullptr;
}

void CResLoader::AsyncIdlePakLoading() {
  bool skipIdle = false;
  for (AUTO(it, x30_pakLoadingList.begin()); it != x30_pakLoadingList.end();) {
    CPakFile* pak = it->get();
    const bool aramPak = pak->IsARAMPak();
    if (aramPak || !skipIdle) {
      pak->AsyncIdle();
    }

    if (pak->IsCompletelyLoaded()) {
      MoveToCorrectLoadedList((*it));
      it = x30_pakLoadingList.erase(it);
    } else {
      if (!aramPak) {
        skipIdle = true;
      }
      ++it;
    }
  }
}

bool CResLoader::AreAllPaksLoaded() const { return x30_pakLoadingList.empty(); }

const SObjectTag* CResLoader::GetResourceIdByName(const char* name) const {
  for (AUTO(it, x0_aramList.begin()); it != x0_aramList.end(); ++it) {
    const SObjectTag* id = (*it)->GetResIdByName(name);
    if (id != nullptr) {
      return id;
    }
  }

  for (AUTO(it, x18_pakLoadedList.begin()); it != x18_pakLoadedList.end(); ++it) {
    const SObjectTag* id = (*it)->GetResIdByName(name);
    if (id != nullptr) {
      return id;
    }
  }

  return nullptr;
}

FourCC CResLoader::GetResourceTypeById(const CAssetId asset) const {
  if (const_cast< CResLoader& >(*this).ResourceExists(asset)) {
    return x50_cachedResInfo->GetType();
  }

  return 0;
}

bool CResLoader::ResourceExists(const SObjectTag& tag) const {
#if NONMATCHING
  return const_cast< CResLoader* >(this)->ResourceExists(tag.GetId());
#else
  return const_cast< CResLoader* >(this)->ResourceExists(tag.GetId()) != nullptr;
#endif
}

uint CResLoader::ResourceSize(const SObjectTag& tag) const {
  if (const_cast< CResLoader& >(*this).ResourceExists(tag.GetId())) {
    return x50_cachedResInfo->GetSize();
  }

  return 0;
}

CResLoader::ECompressionType CResLoader::GetResourceCompression(const SObjectTag& tag) const {
  if (const_cast< CResLoader& >(*this).ResourceExists(tag.GetId())) {
    return x50_cachedResInfo->IsCompressed() ? kCompressionType_Compressed
                                             : kCompressionType_Uncompressed;
  }

  return kCompressionType_Uncompressed;
}

CDvdRequest* CResLoader::LoadResourceAsync(const SObjectTag& tag, char* extBuf) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::SResInfo* info = x50_cachedResInfo;
  return curPak->DvdFile().AsyncSeekRead(extBuf, align_size(info->GetSize()), kSO_Begin, info->GetOffset());
}

CDvdRequest* CResLoader::LoadResourcePartAsync(const SObjectTag& tag, const int offset,
                                               const int length, char* extBuf) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::SResInfo* info = x50_cachedResInfo;
  return curPak->DvdFile().AsyncSeekRead(extBuf, length, kSO_Begin, info->GetOffset() + offset);
}
CInputStream* CResLoader::LoadNewResourceSync(const SObjectTag& tag, char* extBuf) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::SResInfo* info = x50_cachedResInfo;
  uint len = align_size(info->GetSize());
  void* dest = extBuf ? extBuf : CMemory::Alloc(len, IAllocator::kHI_RoundUpLen);

  curPak->DvdFile().SyncSeekRead(dest, len, kSO_Begin, info->GetOffset());
  CInputStream* input = rs_new CMemoryInStream(dest, info->GetSize(),
                                               extBuf == nullptr ? CMemoryInStream::kOS_Owned
                                                                 : CMemoryInStream::kOS_NotOwned);

  if (info->IsCompressed()) {
    input->Get< uint >();
    return rs_new CZipInputStream(input);
  }

  return input;
}

CInputStream* CResLoader::LoadResourceFromMemorySync(const SObjectTag& tag, const void* extBuf) {
  FindResourceForLoad(tag);
  const CPakFile::SResInfo* info = x50_cachedResInfo;
  CInputStream* input = rs_new CMemoryInStream(extBuf, info->GetSize());

  if (info->IsCompressed()) {
    input->Get< uint >();
    return rs_new CZipInputStream(input);
  }
  return input;
}

void CResLoader::LoadMemResourceSync(const SObjectTag& tag, char** bufOut, int* lenOut) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::SResInfo* info = x50_cachedResInfo;
  uint len = align_size(info->GetSize());
  char* buf = static_cast< char* >(CMemory::Alloc(len, IAllocator::kHI_RoundUpLen));
  curPak->DvdFile().SyncSeekRead(buf, len, kSO_Begin, info->GetOffset());
  *bufOut = buf;
  *lenOut = info->GetSize();
}

CInputStream* CResLoader::LoadNewResourcePartSync(const SObjectTag& tag, int offset, int length,
                                                  char* extBuf) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::SResInfo* info = x50_cachedResInfo;

  void* dest = extBuf ? extBuf : CMemory::Alloc(length, IAllocator::kHI_RoundUpLen);
  curPak->DvdFile().SyncSeekRead(dest, length, kSO_Begin, info->GetOffset() + offset);

  CInputStream* input = rs_new CMemoryInStream(
      dest, length, extBuf == nullptr ? CMemoryInStream::kOS_Owned : CMemoryInStream::kOS_NotOwned);
  return input;
}

void CResLoader::AddPakFileAsync(const rstl::string& filePath, const bool a, const bool b) {
  const rstl::string pathWithExt(filePath + ".pak");

  if (CDvdFile::FileExists(pathWithExt.data())) {
    x30_pakLoadingList.push_back(rs_new CPakFile(pathWithExt, a, b));
  }
}

void CResLoader::RemovePakFile(const rstl::string& filePath) {
  rstl::string pathWithExt(filePath + ".pak");
  ClearCache();
  rstl::list< rstl::auto_ptr< CPakFile > >* lists[] = {&x0_aramList, &x18_pakLoadedList};

  for (int i = 0; i < ARRAY_SIZE(lists); ++i) {
    rstl::list< rstl::auto_ptr< CPakFile > >& list = *lists[i];
    for (AUTO(it, list.begin()); it != list.end(); ++it) {
      const CPakFile* pak = it->get();
      if (CStringExtras::CompareCaseInsensitive(pak->GetDvdFile().GetFilename(), pathWithExt) == 0) {
        list.erase(it);
        return;
      }
    }
  }

  for (AUTO(it, x30_pakLoadingList.begin()); it != x30_pakLoadingList.end(); ++it) {
    const CPakFile* pak = it->get();
    if (CStringExtras::CompareCaseInsensitive(pak->GetDvdFile().GetFilename(), pathWithExt) == 0) {
      while (!pak->IsCompletelyLoaded()) {
        AsyncIdlePakLoading();
      }
      x30_pakLoadingList.erase(it);
      return;
    }
  }
}

const rstl::vector< CAssetId >* CResLoader::GetTagListForFile(const rstl::string& filePath) const {
  rstl::string pathWithExt(filePath + ".pak");

  const rstl::list< rstl::auto_ptr< CPakFile > >* lists[] = {&x0_aramList, &x18_pakLoadedList};

  for (int i = 0; i < ARRAY_SIZE(lists); ++i) {
    const rstl::list< rstl::auto_ptr< CPakFile > >& list = *lists[i];
    for (AUTO(it, list.begin()); it != list.end(); ++it) {
      const CPakFile* pak = it->get();
      if (CStringExtras::CompareCaseInsensitive(pak->GetDvdFile().GetFilename(), pathWithExt) == 0) {
        return pak->GetDepList();
      }
    }
  }

  return nullptr;
}

rstl::vector< rstl::pair< rstl::string, SObjectTag > > CResLoader::GetResourceIdToNameList() const {
  const rstl::list< rstl::auto_ptr< CPakFile > >* lists[] = {&x0_aramList, &x18_pakLoadedList};
  int nameCount = 0;
  for (int i = 0; i < ARRAY_SIZE(lists); ++i) {
    const rstl::list< rstl::auto_ptr< CPakFile > >& list = *lists[i];
    for (AUTO(it, list.begin()); it != list.end(); ++it) {
      const CPakFile* pak = it->get();
      if (!pak->IsStashedInARAM()) {
        nameCount += pak->GetStringToObjectList().size();
      }
    }
  }

  rstl::vector< rstl::pair< rstl::string, SObjectTag > > ret(nameCount);

  for (int i = 0; i < ARRAY_SIZE(lists); ++i) {
    const rstl::list< rstl::auto_ptr< CPakFile > >& list = *lists[i];
    for (rstl::list< rstl::auto_ptr< CPakFile > >::const_iterator it = list.begin();
         it != list.end(); ++it) {
      const CPakFile* pak = it->get();
      if (!pak->IsStashedInARAM()) {
        const rstl::vector< rstl::pair< rstl::string, SObjectTag > >& tagList =
            pak->GetStringToObjectList();
        ret.insert(ret.end(), tagList.begin(), tagList.end());
      }
    }
  }

  return ret;
}
int CResLoader::GetPakCount() const { return x0_aramList.size() + x18_pakLoadedList.size(); }
CPakFile* CResLoader::GetPakFile(const int idx) const {
  int numAramPaks = x0_aramList.size();
  if (idx < numAramPaks) {
    AUTO(it, x0_aramList.begin());
    for (int i = 0; i < idx; ++it, ++i) {
    }
    return it->get();
  }

  AUTO(it, x18_pakLoadedList.begin());
  for (int i = 0; i < idx - numAramPaks; ++it, ++i) {
  }
  return it->get();
}
