#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/Streams/CZipSupport.hpp"
#include "rstl/math.hpp"

#include <zlib.h>

static const int kDecompChunkSize = 4096;

// Written by CMFGameLoader; its purpose is not yet identified.
int gResFactoryUnknown = 2;

CResFactory::CResFactory() {}

CResFactory::~CResFactory() {}

void CResFactory::AddToLoadList(const SLoadingData& data) {
  AUTO(pos, mLoadList.end());
  if (data.mDvdReq->GetMediaType() == 0) {
    pos = mLoadList.begin();
    while (pos != mLoadList.end()) {
      if (pos->mDvdReq->GetMediaType() == 1) {
        break;
      }
      ++pos;
    }
  }
  AUTO(it, mLoadList.insert(pos, data));
  mLoadMap.insert(rstl::pair< SObjectTag, LoadList::iterator >(data.mTag, it));
}

void CResFactory::EraseFromLoadList(const LoadList::iterator& it) {
  mLoadMap.erase(it->mTag);
  mLoadList.erase(it);
}

CResFactory::LoadList::iterator CResFactory::FindInLoadList(const SObjectTag& tag) {
  rstl::map< SObjectTag, LoadList::iterator >::const_iterator it = mLoadMap.find(tag);
  if (it == mLoadMap.end()) {
    return mLoadList.end();
  }
  return it->second;
}
#if !defined(TARGET_PC)
rstl::auto_ptr< IObj > CResFactory::Build(const SObjectTag& tag, const CVParamTransfer& params) {
  AUTO(it, FindInLoadList(tag));
  if (it != mLoadList.end()) {
    IObj** target = it->mTarget;
    while (*target == nullptr) {
      while (!PumpResource(it, 0)) {
      }
    }
    return rstl::auto_ptr< IObj >(*target);
  }
  return BuildSync(tag, params);
}

rstl::auto_ptr< IObj > CResFactory::BuildSync(const SObjectTag& tag,
                                              const CVParamTransfer& params) {
  if (mFactoryMgr.CanMakeMemory(tag)) {
    char* buffer;
    int size;
    mResLoader.LoadMemResourceSync(tag, &buffer, &size);
    return mFactoryMgr.MakeObjectFromMemory(tag, buffer, size,
                                               mResLoader.GetResourceCompression(tag) !=
                                                   CResLoader::kCompressionType_Uncompressed,
                                               params);
  }
  CInputStream* in = mResLoader.LoadNewResourceSync(tag, nullptr);
  rstl::auto_ptr< IObj > result = mFactoryMgr.MakeObject(tag, *in, params);
  delete in;
  return result;
}

void CResFactory::BuildAsync(const SObjectTag& tag, const CVParamTransfer& params, IObj** target) {
  *target = nullptr;
  const uint size = ResourceSize(tag);
  char* buffer = static_cast< char* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen));
  CDvdRequest* request = LoadResourceAsync(tag, buffer);
  SLoadingData data(tag, request, target, buffer, size, mResLoader.GetResourceCompression(tag),
                    params);
  AddToLoadList(data);
}
#endif

void CResFactory::CancelBuild(const SObjectTag& tag) {
  AUTO(it, FindInLoadList(tag));
  if (it != mLoadList.end()) {
    SLoadingData& data = *it;
    data.mDvdReq->PostCancelRequest();
    mCancelledList.push_back(data);
    EraseFromLoadList(it);
  }
}

void CResFactory::AsyncIdle(uint time) {
  bool work = true;
  CStopwatch timer;
  AUTO(it, mCancelledList.begin());
  while (it != mCancelledList.end()) {
    AUTO(current, it);
    ++it;
    if (current->mDvdReq->IsComplete()) {
      mCancelledList.erase(current);
    }
  }

  while (work) {
    work = false;
    uint elapsed = timer.GetElapsedMicros();
    AUTO(it, mLoadList.begin());
    while (it != mLoadList.end() && elapsed < time) {
      if (PumpResource(it, time - elapsed)) {
        work = true;
        break;
      }
      ++it;
      elapsed = timer.GetElapsedMicros();
    }
  }
}

bool CResFactory::PumpResource(const LoadList::iterator& it, uint time) {
  if (it->mDvdReq->IsComplete()) {
    if (it->mCompression == CResLoader::kCompressionType_Compressed &&
        !it->PumpDecompression(time)) {
      return false;
    }
    SLoadingData data(*it);
    EraseFromLoadList(it);
    *data.mTarget =
        mFactoryMgr
            .MakeObjectFromMemory(data.mTag, data.mBuffer.release(), data.mSize,
                                  data.mCompression != CResLoader::kCompressionType_Uncompressed,
                                  data.mParams)
            .release();
    return true;
  }
  return false;
}

CResFactory::SLoadingData::SLoadingData(const SObjectTag& tag, CDvdRequest* request, IObj** target,
                                        void* buffer, int size,
                                        CResLoader::ECompressionType compression,
                                        const CVParamTransfer& params)
: mTag(tag)
, mDvdReq(request)
, mTarget(target)
, mBuffer(static_cast< uchar* >(buffer))
, mSize(size)
, mCompression(compression)
, mParams(params) {}

CResFactory::SLoadingData::~SLoadingData() {
  if (mZip.owner()) {
    inflateEnd(mZip.get());
  }
}

bool CResFactory::SLoadingData::PumpDecompression(uint time) {
  CStopwatch timer;
  z_stream_s* zip = mZip.get();
  uint* buffer = reinterpret_cast< uint* >(mBuffer.get());
#if NONMATCHING
  const uint length = CBasics::SwapBytes(*buffer);
#else
  const uint length = *buffer;
#endif
  if (zip == nullptr) {
    zip = rs_new z_stream_s;
    zip->zalloc = CZipSupport::Alloc;
    zip->zfree = CZipSupport::Free;
    zip->opaque = nullptr;
#if NONMATCHING
    inflateInit(zip);
#else
    inflateInit2(zip);
#endif
    zip->total_in = 0;
    zip->total_out = 0;
    zip->avail_in = 0;
    zip->avail_out = 0;
    zip->next_in = nullptr;
    zip->next_out = nullptr;
    mZip = rstl::auto_ptr< z_stream_s >(zip);
    mDecompBuffer = rstl::auto_ptr< uchar >(
        static_cast< uchar* >(CMemory::Alloc(length, IAllocator::kHI_RoundUpLen)));
  }

  const int size = mSize;
  uchar* compressed = reinterpret_cast< uchar* >(buffer + 1);
  while ((time == 0 || timer.GetElapsedMicros() < time) && length != zip->total_out) {
    if (zip->avail_in == 0) {
      const int amount = rstl::min_val< int >(kDecompChunkSize, size - 4 - zip->total_in);
      if (amount == 0) {
        break;
      }
      zip->next_in = compressed + zip->total_in;
      zip->avail_in = amount;
      zip->next_out = mDecompBuffer.get() + zip->total_out;
      zip->avail_out = length - zip->total_out;
    }
    inflate(zip, Z_NO_FLUSH);
  }

  if (length == zip->total_out) {
    inflateEnd(zip);
    mZip = rstl::auto_ptr< z_stream_s >();
    mBuffer = rstl::auto_ptr< uchar >(mDecompBuffer.release());
    mDecompBuffer = rstl::auto_ptr< uchar >();
    mCompression = CResLoader::kCompressionType_Uncompressed;
    mSize = length;
    return true;
  }
  return false;
}
