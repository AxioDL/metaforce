#ifndef _CALLFORMATSANIMSOURCE
#define _CALLFORMATSANIMSOURCE

#include <types.h>

#include "Kyoto/Animation/CAnimSource.hpp"
#include "Kyoto/Animation/CFBStreamedCompression.hpp"
#include "Kyoto/Animation/IAnimReader.hpp"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"

class CInputStream;
class IObjectStore;
class CAnimFormatUnion {
public:
  CAnimFormatUnion(CInputStream& in, IObjectStore& store);
  ~CAnimFormatUnion();

  int GetType() const { return mFormatType; }
  CAnimSource& AsCAnimSource() { return *reinterpret_cast< CAnimSource* >(mFormatData); }
  const CAnimSource& AsCAnimSource() const {
    return *reinterpret_cast< const CAnimSource* >(mFormatData);
  }
  CFBStreamedCompression& AsCFBStreamedCompression() {
    return *reinterpret_cast< CFBStreamedCompression* >(mFormatData);
  }
  const CFBStreamedCompression& AsCFBStreamedCompression() const {
    return *reinterpret_cast< const CFBStreamedCompression* >(mFormatData);
  }

  static void SubConstruct(uchar* ptr, const uint format, CInputStream& in, IObjectStore& store);

private:
  uint mFormatType;
  ALIGNAS(CAnimSource) ALIGNAS(CFBStreamedCompression)
  uchar mFormatData[sizeof(CAnimSource) > sizeof(CFBStreamedCompression)
                        ? sizeof(CAnimSource) : sizeof(CFBStreamedCompression)];
};

class CAllFormatsAnimSource {
public:
  CAllFormatsAnimSource(CInputStream& in, IObjectStore& store, const SObjectTag& tag);
  ~CAllFormatsAnimSource();

  int GetType() const { return mFormatUnion.GetType(); }
  CCharAnimTime GetAnimationDuration() const {
    switch (GetType()) {
    case 0:
      return AsCAnimSource().GetAnimationDuration();
    case 2:
      return AsCFBStreamedCompression().GetAnimationDuration();
    default:
      return AsCAnimSource().GetAnimationDuration();
    }
  }
  float GetAverageVelocity() const {
    switch (GetType()) {
    case 0:
      return AsCAnimSource().GetAverageVelocity();
    case 2:
      return AsCFBStreamedCompression().GetAverageVelocity();
    default:
      return AsCAnimSource().GetAverageVelocity();
    }
  }
  const CAnimSource& AsCAnimSource() const { return mFormatUnion.AsCAnimSource(); }
  const CFBStreamedCompression& AsCFBStreamedCompression() const {
    return mFormatUnion.AsCFBStreamedCompression();
  }
  void GetFormatPointer(const CAnimSource*& ptr) const { ptr = &AsCAnimSource(); }
  void GetFormatPointer(const CFBStreamedCompression*& ptr) const {
    ptr = &AsCFBStreamedCompression();
  }

  static rstl::ownership_transfer< IAnimReader >
  GetNewReader(const TLockedToken< CAllFormatsAnimSource >& tok, const CCharAnimTime& time);

private:
  CAnimFormatUnion mFormatUnion;
  CVector3f x68_;
  SObjectTag mTag;
};

CHECK_SIZEOF(CAnimFormatUnion, 0x68)
CHECK_SIZEOF(CAllFormatsAnimSource, 0x7c)

template < typename T >
class TSubAnimTypeToken {
public:
  TSubAnimTypeToken(const TLockedToken< CAllFormatsAnimSource >& tok) : mToken(tok) {
    mToken->GetFormatPointer(mSource);
  }
  const T* operator->() const { return mSource; }
  const T& operator*() const { return *mSource; }

private:
  TLockedToken< CAllFormatsAnimSource > mToken;
  const T* mSource;
};

const CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, CInputStream& in,
                                   const CVParamTransfer& param);
#endif // _CALLFORMATSANIMSOURCE
