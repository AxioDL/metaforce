#include "Kyoto/CResLoader.hpp"
#include "Kyoto/CPakFile.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Streams/CZipInputStream.hpp"
#include "rstl/StringExtras.hpp"

static inline int align_size(const int size) { return (size + 31) & ~31; }

CResLoader::CResLoader()
: mCurPak(mPakLoadedList.end())
, mCachedResId(kInvalidAssetId)
, mCachedResInfo(nullptr)
, mForwardSeek(false) {}

CResLoader::~CResLoader() {
  for (AUTO(it, mPakLoadingList.begin()); it != mPakLoadingList.end(); ++it) {
    CPakFile* pak = it->get();
    while (!pak->IsCompletelyLoaded()) {
      pak->AsyncIdle();
    }
  }
}

void CResLoader::MoveToCorrectLoadedList(const rstl::auto_ptr< CPakFile >& pak) {
  if (pak->IsARAMPak()) {
    mAramList.push_back(pak);
  } else {
    mPakLoadedList.push_back(pak);
  }
}

bool CResLoader::CacheFromPak(const CPakFile& pak, const CAssetId asset) const {
  const CPakFile::SResInfo* resInfo = pak.GetResInfo(asset);
  if (!resInfo) {
    return false;
  }

  mCachedResId = asset;
  mCachedResInfo = resInfo;

  return true;
}

bool CResLoader::CacheFromPakForLoad(CPakFile& pak, const CAssetId asset) {
  const CPakFile::SResInfo* resInfo = nullptr;
  if (mForwardSeek) {
    resInfo = pak.GetResInfoForLoadPreferForward(asset);
    mForwardSeek = false;
  } else {
    resInfo = pak.GetResInfoForLoadDirectionless(asset);
  }

  if (resInfo == nullptr) {
    return false;
  }

  mCachedResId = asset;
  mCachedResInfo = resInfo;

  return true;
}

CPakFile* CResLoader::FindResourceForLoad(const SObjectTag& tag) {
  return FindResourceForLoad(tag.GetId());
}

CPakFile* CResLoader::FindResourceForLoad(const CAssetId asset) {
  rstl::list< rstl::auto_ptr< CPakFile > >::iterator it;
  for (it = mAramList.begin(); it != mAramList.end(); ++it) {
    CPakFile* pak = it->get();
    if (CacheFromPak(*pak, asset)) {
      return pak;
    }
  }

  if (mCurPak != mPakLoadedList.end()) {
    CPakFile* pak = mCurPak->get();
    if (CacheFromPakForLoad(*pak, asset)) {
      return pak;
    }
  }

  for (it = mPakLoadedList.begin(); it != mPakLoadedList.end(); ++it) {
    CPakFile* pak = it->get();
    if (mCurPak != it && CacheFromPakForLoad(*pak, asset)) {
      mCurPak = it;
      return pak;
    }
  }

  return nullptr;
}

CPakFile* CResLoader::FindResource(const SObjectTag& tag) {
  mForwardSeek = false;
  CPakFile* ret = FindResourceForLoad(tag);
  mForwardSeek = true;
  return ret;
}

bool CResLoader::ResourceExists(CAssetId asset) {
  if (mCachedResId == asset) {
    return true;
  }

  for (AUTO(it, mAramList.begin()); it != mAramList.end(); ++it) {
    if (CacheFromPak(**it, asset)) {
      return true;
    }
  }

  if (mCurPak != mPakLoadedList.end()) {
    if (CacheFromPak(**mCurPak, asset)) {
      return true;
    }
  }

  for (AUTO(it, mPakLoadedList.begin()); it != mPakLoadedList.end(); ++it) {
    if (mCurPak != it && CacheFromPak(**it, asset)) {
      return true;
    }
  }

  return false;
}

void CResLoader::ClearCache() {
  mCurPak = mPakLoadedList.end();
  mCachedResId = kInvalidAssetId;
  mCachedResInfo = nullptr;
}

void CResLoader::AsyncIdlePakLoading() {
  bool skipIdle = false;
  for (AUTO(it, mPakLoadingList.begin()); it != mPakLoadingList.end();) {
    CPakFile* pak = it->get();
    const bool aramPak = pak->IsARAMPak();
    if (aramPak || !skipIdle) {
      pak->AsyncIdle();
    }

    if (pak->IsCompletelyLoaded()) {
      MoveToCorrectLoadedList((*it));
      it = mPakLoadingList.erase(it);
    } else {
      if (!aramPak) {
        skipIdle = true;
      }
      ++it;
    }
  }
}

bool CResLoader::AreAllPaksLoaded() const { return mPakLoadingList.empty(); }

const SObjectTag* CResLoader::GetResourceIdByName(const char* name) const {
  for (AUTO(it, mAramList.begin()); it != mAramList.end(); ++it) {
    const SObjectTag* id = (*it)->GetResIdByName(name);
    if (id != nullptr) {
      return id;
    }
  }

  for (AUTO(it, mPakLoadedList.begin()); it != mPakLoadedList.end(); ++it) {
    const SObjectTag* id = (*it)->GetResIdByName(name);
    if (id != nullptr) {
      return id;
    }
  }

  return nullptr;
}

FourCC CResLoader::GetResourceTypeById(const CAssetId asset) const {
  if (const_cast< CResLoader& >(*this).ResourceExists(asset)) {
    return mCachedResInfo->GetType();
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
    return mCachedResInfo->GetSize();
  }

  return 0;
}

CResLoader::ECompressionType CResLoader::GetResourceCompression(const SObjectTag& tag) const {
  if (const_cast< CResLoader& >(*this).ResourceExists(tag.GetId())) {
    return mCachedResInfo->IsCompressed() ? kCompressionType_Compressed
                                             : kCompressionType_Uncompressed;
  }

  return kCompressionType_Uncompressed;
}

CDvdRequest* CResLoader::LoadResourceAsync(const SObjectTag& tag, char* extBuf) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::SResInfo* info = mCachedResInfo;
  return curPak->DvdFile().AsyncSeekRead(extBuf, align_size(info->GetSize()), kSO_Begin, info->GetOffset());
}

CDvdRequest* CResLoader::LoadResourcePartAsync(const SObjectTag& tag, const int offset,
                                               const int length, char* extBuf) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::SResInfo* info = mCachedResInfo;
  return curPak->DvdFile().AsyncSeekRead(extBuf, length, kSO_Begin, info->GetOffset() + offset);
}
CInputStream* CResLoader::LoadNewResourceSync(const SObjectTag& tag, char* extBuf) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::SResInfo* info = mCachedResInfo;
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
  const CPakFile::SResInfo* info = mCachedResInfo;
  CInputStream* input = rs_new CMemoryInStream(extBuf, info->GetSize());

  if (info->IsCompressed()) {
    input->Get< uint >();
    return rs_new CZipInputStream(input);
  }
  return input;
}

void CResLoader::LoadMemResourceSync(const SObjectTag& tag, char** bufOut, int* lenOut) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::SResInfo* info = mCachedResInfo;
  uint len = align_size(info->GetSize());
  char* buf = static_cast< char* >(CMemory::Alloc(len, IAllocator::kHI_RoundUpLen));
  curPak->DvdFile().SyncSeekRead(buf, len, kSO_Begin, info->GetOffset());
  *bufOut = buf;
  *lenOut = info->GetSize();
}

CInputStream* CResLoader::LoadNewResourcePartSync(const SObjectTag& tag, int offset, int length,
                                                  char* extBuf) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const CPakFile::SResInfo* info = mCachedResInfo;

  void* dest = extBuf ? extBuf : CMemory::Alloc(length, IAllocator::kHI_RoundUpLen);
  curPak->DvdFile().SyncSeekRead(dest, length, kSO_Begin, info->GetOffset() + offset);

  CInputStream* input = rs_new CMemoryInStream(
      dest, length, extBuf == nullptr ? CMemoryInStream::kOS_Owned : CMemoryInStream::kOS_NotOwned);
  return input;
}

void CResLoader::AddPakFileAsync(const rstl::string& filePath, const bool a, const bool b) {
  const rstl::string pathWithExt(filePath + ".pak");

  if (CDvdFile::FileExists(pathWithExt.data())) {
    mPakLoadingList.push_back(rs_new CPakFile(pathWithExt, a, b));
  }
}

void CResLoader::RemovePakFile(const rstl::string& filePath) {
  rstl::string pathWithExt(filePath + ".pak");
  ClearCache();
  rstl::list< rstl::auto_ptr< CPakFile > >* lists[] = {&mAramList, &mPakLoadedList};

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

  for (AUTO(it, mPakLoadingList.begin()); it != mPakLoadingList.end(); ++it) {
    const CPakFile* pak = it->get();
    if (CStringExtras::CompareCaseInsensitive(pak->GetDvdFile().GetFilename(), pathWithExt) == 0) {
      while (!pak->IsCompletelyLoaded()) {
        AsyncIdlePakLoading();
      }
      mPakLoadingList.erase(it);
      return;
    }
  }
}

const rstl::vector< CAssetId >* CResLoader::GetTagListForFile(const rstl::string& filePath) const {
  rstl::string pathWithExt(filePath + ".pak");

  const rstl::list< rstl::auto_ptr< CPakFile > >* lists[] = {&mAramList, &mPakLoadedList};

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
  const rstl::list< rstl::auto_ptr< CPakFile > >* lists[] = {&mAramList, &mPakLoadedList};
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
int CResLoader::GetPakCount() const { return mAramList.size() + mPakLoadedList.size(); }
CPakFile* CResLoader::GetPakFile(const int idx) const {
  int numAramPaks = mAramList.size();
  if (idx < numAramPaks) {
    AUTO(it, mAramList.begin());
    for (int i = 0; i < idx; ++it, ++i) {
    }
    return it->get();
  }

  AUTO(it, mPakLoadedList.begin());
  for (int i = 0; i < idx - numAramPaks; ++it, ++i) {
  }
  return it->get();
}
