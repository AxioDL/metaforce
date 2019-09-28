#pragma once

#include <algorithm>
#include <memory>

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CAnimSource.hpp"
#include "Runtime/Character/CFBStreamedCompression.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
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
  CAnimFormatUnion(CInputStream& in, IObjectStore& store);
  ~CAnimFormatUnion();
  EAnimFormat GetFormat() const { return x0_format; }
  const CAnimSource& GetAsCAnimSource() const { return *reinterpret_cast<const CAnimSource*>(x4_storage); }
  const CFBStreamedCompression& GetAsCFBStreamedCompression() const {
    return *reinterpret_cast<const CFBStreamedCompression*>(x4_storage);
  }
};

class CAllFormatsAnimSource : public CAnimFormatUnion {
  zeus::CVector3f x68_;
  SObjectTag x74_tag;

public:
  CAllFormatsAnimSource(CInputStream& in, IObjectStore& store, const SObjectTag& tag);
  static std::shared_ptr<IAnimReader> GetNewReader(const TLockedToken<CAllFormatsAnimSource>& tok,
                                                   const CCharAnimTime& startTime);
};

CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& params,
                                   CObjectReference* selfRef);

} // namespace urde
