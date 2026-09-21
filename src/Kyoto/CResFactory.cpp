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
  AUTO(pos, x84_loadList.end());
  if (data.x8_dvdReq->GetMediaType() == 0) {
    pos = x84_loadList.begin();
    while (pos != x84_loadList.end()) {
      if (pos->x8_dvdReq->GetMediaType() == 1) {
        break;
      }
      ++pos;
    }
  }
  AUTO(it, x84_loadList.insert(pos, data));
  x9c_loadMap.insert(rstl::pair< SObjectTag, LoadList::iterator >(data.x0_tag, it));
}

void CResFactory::EraseFromLoadList(const LoadList::iterator& it) {
  x9c_loadMap.erase(it->x0_tag);
  x84_loadList.erase(it);
}

CResFactory::LoadList::iterator CResFactory::FindInLoadList(const SObjectTag& tag) {
  rstl::map< SObjectTag, LoadList::iterator >::const_iterator it = x9c_loadMap.find(tag);
  if (it == x9c_loadMap.end()) {
    return x84_loadList.end();
  }
  return it->second;
}

rstl::auto_ptr< IObj > CResFactory::Build(const SObjectTag& tag, const CVParamTransfer& params) {
  AUTO(it, FindInLoadList(tag));
  if (it != x84_loadList.end()) {
    IObj** target = it->x10_target;
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
  if (x5c_factoryMgr.CanMakeMemory(tag)) {
    char* buffer;
    int size;
    x4_resLoader.LoadMemResourceSync(tag, &buffer, &size);
    return x5c_factoryMgr.MakeObjectFromMemory(tag, buffer, size,
                                               x4_resLoader.GetResourceCompression(tag) !=
                                                   CResLoader::kCompressionType_Uncompressed,
                                               params);
  }
  CInputStream* in = x4_resLoader.LoadNewResourceSync(tag, nullptr);
  rstl::auto_ptr< IObj > result = x5c_factoryMgr.MakeObject(tag, *in, params);
  delete in;
  return result;
}

void CResFactory::BuildAsync(const SObjectTag& tag, const CVParamTransfer& params, IObj** target) {
  *target = nullptr;
  const uint size = ResourceSize(tag);
  char* buffer = static_cast< char* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen));
  CDvdRequest* request = LoadResourceAsync(tag, buffer);
  SLoadingData data(tag, request, target, buffer, size, x4_resLoader.GetResourceCompression(tag),
                    params);
  AddToLoadList(data);
}

void CResFactory::CancelBuild(const SObjectTag& tag) {
  AUTO(it, FindInLoadList(tag));
  if (it != x84_loadList.end()) {
    SLoadingData& data = *it;
    data.x8_dvdReq->PostCancelRequest();
    xb0_cancelledList.push_back(data);
    EraseFromLoadList(it);
  }
}

void CResFactory::AsyncIdle(uint time) {
  bool work = true;
  CStopwatch timer;
  AUTO(it, xb0_cancelledList.begin());
  while (it != xb0_cancelledList.end()) {
    AUTO(current, it);
    ++it;
    if (current->x8_dvdReq->IsComplete()) {
      xb0_cancelledList.erase(current);
    }
  }

  while (work) {
    work = false;
    uint elapsed = timer.GetElapsedMicros();
    AUTO(it, x84_loadList.begin());
    while (it != x84_loadList.end() && elapsed < time) {
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
  if (it->x8_dvdReq->IsComplete()) {
    if (it->x30_compression == CResLoader::kCompressionType_Compressed &&
        !it->PumpDecompression(time)) {
      return false;
    }
    SLoadingData data(*it);
    EraseFromLoadList(it);
    *data.x10_target =
        x5c_factoryMgr
            .MakeObjectFromMemory(data.x0_tag, data.x14_buffer.release(), data.x2c_size,
                                  data.x30_compression != CResLoader::kCompressionType_Uncompressed,
                                  data.x34_params)
            .release();
    return true;
  }
  return false;
}

CResFactory::SLoadingData::SLoadingData(const SObjectTag& tag, CDvdRequest* request, IObj** target,
                                        void* buffer, int size,
                                        CResLoader::ECompressionType compression,
                                        const CVParamTransfer& params)
: x0_tag(tag)
, x8_dvdReq(request)
, x10_target(target)
, x14_buffer(static_cast< uchar* >(buffer))
, x2c_size(size)
, x30_compression(compression)
, x34_params(params) {}

CResFactory::SLoadingData::~SLoadingData() {
  if (x24_zip.owner()) {
    inflateEnd(x24_zip.get());
  }
}

bool CResFactory::SLoadingData::PumpDecompression(uint time) {
  CStopwatch timer;
  z_stream_s* zip = x24_zip.get();
  uint* buffer = reinterpret_cast< uint* >(x14_buffer.get());
#if TARGET_LITTLE_ENDIAN
  const uint length = CBasics::SwapBytes(*buffer);
#else
  const uint length = *buffer;
#endif
  if (zip == nullptr) {
    zip = rs_new z_stream_s;
    zip->zalloc = CZipSupport::Alloc;
    zip->zfree = CZipSupport::Free;
    zip->opaque = nullptr;
    inflateInit2(zip);
    zip->total_in = 0;
    zip->total_out = 0;
    zip->avail_in = 0;
    zip->avail_out = 0;
    zip->next_in = nullptr;
    zip->next_out = nullptr;
    x24_zip = rstl::auto_ptr< z_stream_s >(zip);
    x1c_decompBuffer = rstl::auto_ptr< uchar >(
        static_cast< uchar* >(CMemory::Alloc(length, IAllocator::kHI_RoundUpLen)));
  }

  const int size = x2c_size;
  uchar* compressed = reinterpret_cast< uchar* >(buffer + 1);
  while ((time == 0 || timer.GetElapsedMicros() < time) && length != zip->total_out) {
    if (zip->avail_in == 0) {
      const int amount = rstl::min_val< int >(kDecompChunkSize, size - 4 - zip->total_in);
      if (amount == 0) {
        break;
      }
      zip->next_in = compressed + zip->total_in;
      zip->avail_in = amount;
      zip->next_out = x1c_decompBuffer.get() + zip->total_out;
      zip->avail_out = length - zip->total_out;
    }
    inflate(zip, Z_NO_FLUSH);
  }

  if (length == zip->total_out) {
    inflateEnd(zip);
    x24_zip = rstl::auto_ptr< z_stream_s >();
    x14_buffer = rstl::auto_ptr< uchar >(x1c_decompBuffer.release());
    x1c_decompBuffer = rstl::auto_ptr< uchar >();
    x30_compression = CResLoader::kCompressionType_Uncompressed;
    x2c_size = length;
    return true;
  }
  return false;
}
