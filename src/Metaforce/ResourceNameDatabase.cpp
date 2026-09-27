#include "Metaforce/ResourceNameDatabase.hpp"

#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "borealis/io.hpp"
#include "borealis/log.hpp"
#include "dolphin/os.h"
#include "rstl/auto_ptr.hpp"

#include <map>

namespace metaforce::ResourceNameDatabase {
namespace {
std::map< CAssetId, rstl::string > mResourceNames;
borealis::Log Log{"ResourceNameDatabase"};
} // namespace

const rstl::string* GetNameForResource(const CAssetId uid) {
  if (HaveNameForResource(uid)) {
    return &mResourceNames[uid];
  }
  return nullptr;
}

bool HaveNameForResource(const CAssetId uid) { return mResourceNames.contains(uid); }
bool Initialize(const std::string_view databasePath) {
  auto file = borealis::io::open(databasePath, borealis::io::File::Mode::Read);
  if (file.status != borealis::io::Status::Ok) {
    Log.warn("{}", file.message);
    return false;
  }
  uint64_t size = file.file.size();
  rstl::auto_ptr< u8 > data = rs_new u8[size];

  uint64_t off = 0;
  while (size > 0) {
    const auto readBytes = file.file.read(data.get() + off, size);
    off += readBytes;
    size -= readBytes;
  }

  if (off != file.file.size()) {
    Log.warn("{}", file.message);
    return false;
  }

  CMemoryInStream inStream(data.get(), file.file.size(), CMemoryInStream::kOS_NotOwned);
  uint resourceCount = inStream.Get< uint >();

  for (int i = 0; i < resourceCount; i++) {
    const auto aid = inStream.Get< CAssetId >();
    const auto path = inStream.Get< rstl::string >();
    mResourceNames[aid] = path;
  }
  return true;
}
} // namespace metaforce::ResourceNameDatabase