#pragma once

#include <algorithm>
#include <memory>

#include "Runtime/Factory/CFactoryMgr.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CAnimSource.hpp"
#include "Runtime/Character/CFBStreamedCompression.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
class IAnimReader;
class IObjectStore;

enum class EAnimFormat { Uncompressed, Unknown, BitstreamCompressed, BitstreamCompressed24 };

class CAnimFormatUnion {
  friend class CAllFormatsAnimSource;
  union {
    EAnimFormat x0_format;
    u8 _align[16];
  };
  u8 x4_storage[std::max(sizeof(CAnimSource), sizeof(CFBStreamedCompression))];
  static void SubConstruct(u8* storage, EAnimFormat fmt, CInputStream& in, IObjectStore& store);

public:
  explicit CAnimFormatUnion(CInputStream& in, IObjectStore& store);
  ~CAnimFormatUnion();
  EAnimFormat GetFormat() const { return x0_format; }
  CAnimSource& GetAsCAnimSource() { return *reinterpret_cast<CAnimSource*>(x4_storage); }
  CFBStreamedCompression& GetAsCFBStreamedCompression() {
    return *reinterpret_cast<CFBStreamedCompression*>(x4_storage);
  }
};

class CAllFormatsAnimSource : public CAnimFormatUnion {
  zeus::CVector3f x68_;
  SObjectTag x74_tag;

public:
  explicit CAllFormatsAnimSource(CInputStream& in, IObjectStore& store, const SObjectTag& tag);
  static std::shared_ptr<IAnimReader> GetNewReader(const TLockedToken<CAllFormatsAnimSource>& tok,
                                                   const CCharAnimTime& startTime);
};

CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& params,
                                   CObjectReference* selfRef);

} // namespace metaforce
