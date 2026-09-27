#include "Kyoto/Text/CStringTable.hpp"

#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#if TARGET_LITTLE_ENDIAN || WCHAR_MAX > 0xffff
#include "Kyoto/Basics/CBasics.hpp"
#include <string.h>
#endif

#include <rstl/pair.hpp>
#include <rstl/vector.hpp>

static const wchar_t skInvalidString[] = L"Invalid";
#if VERSION >= VERSION_GM8P_00
static const FourCC skLanguages[] = {'ENGL', 'GERM', 'FREN', 'SPAN', 'ITAL', 'DUTC', 'JAPN'};
static FourCC mCurrentLanguage = skLanguages[0];

void CStringTable::SetLanguage(int language) { mCurrentLanguage = skLanguages[language]; }
#else
static FourCC mCurrentLanguage = 'ENGL';
#endif

CStringTable::CStringTable(CInputStream& in)
: mStringCount(0)
, mData(NULL)
#if VERSION >= VERSION_GM8P_00
, mReloadData(nullptr)
{
  Load(in);
}

CStringTable::~CStringTable() {}

void CStringTable::Load(CInputStream& in)
#endif
{
  in.ReadLong();
  in.ReadLong();
  int langCount = in.Get(TType< int >());
  mStringCount = in.Get(TType< uint >());
  rstl::vector< rstl::pair< FourCC, uint > > langOffsets(langCount);
  for (int i = 0; i < langCount; ++i) {
    langOffsets.push_back(in.Get(TType< rstl::pair< FourCC, uint > >()));
  }

  int offset = langOffsets.front().second;
  for (int i = 0; i < langCount; ++i) {
    if (langOffsets[i].first == mCurrentLanguage) {
      offset = langOffsets[i].second;
      break;
    }
  }
  for (uint i = 0; i < offset; ++i) {
    in.ReadChar();
  }

  uint dataLen = in.Get(TType< uint >());
#if TARGET_LITTLE_ENDIAN || WCHAR_MAX > 0xffff
  mNativeStrings.clear();
  rstl::vector< uchar > data(dataLen, uchar(0));
  in.ReadBytes(data.data(), dataLen);
  if (mStringCount < 0 || static_cast< uint >(mStringCount) > dataLen / sizeof(uint)) {
    mStringCount = 0;
    return;
  }

  CMemoryInStream offsets(data.data(), mStringCount * sizeof(uint));
  mNativeStrings.reserve(mStringCount);
  for (int i = 0; i < mStringCount; ++i) {
    uint pos = offsets.ReadLong();
    rstl::vector< wchar_t > text;
    bool terminated = false;
    while (pos <= dataLen && dataLen - pos >= sizeof(ushort)) {
      ushort unit;
      memcpy(&unit, data.data() + pos, sizeof(unit));
      uint codepoint = CBasics::SwapBytes(unit);
      pos += sizeof(unit);
      if (codepoint == 0) {
        terminated = true;
        break;
      }

      // Preserve UTF-16 on 16-bit wchar_t hosts; combine surrogate pairs on
      // hosts whose wide characters can hold a complete Unicode code point.
      if (sizeof(wchar_t) > sizeof(ushort) && codepoint >= 0xd800 && codepoint <= 0xdbff &&
          dataLen - pos >= sizeof(ushort)) {
        memcpy(&unit, data.data() + pos, sizeof(unit));
        const ushort low = CBasics::SwapBytes(unit);
        if (low >= 0xdc00 && low <= 0xdfff) {
          codepoint = 0x10000 + ((codepoint - 0xd800) << 10) + low - 0xdc00;
          pos += sizeof(unit);
        }
      }
      text.push_back(static_cast< wchar_t >(codepoint));
    }
    if (!terminated) {
      text.clear();
      for (const wchar_t* c = skInvalidString; *c; ++c) {
        text.push_back(*c);
      }
    }
    text.push_back(0);
    mNativeStrings.push_back(text);
  }
#else
  mData = rs_new uchar[dataLen];
  in.ReadBytes(mData.get(), dataLen);
#endif
}

const wchar_t* CStringTable::GetString(int idx) const {
  if (idx < 0 || idx >= mStringCount) {
    return skInvalidString;
  }
#if TARGET_LITTLE_ENDIAN || WCHAR_MAX > 0xffff
  return mNativeStrings[idx].data();
#else
  int offset = *(reinterpret_cast< const int* >(mData.get()) + idx);
  return reinterpret_cast< const wchar_t* >(mData.get() + offset);
#endif
}

#if VERSION >= VERSION_GM8P_00
void CStringTable::Reload(CAssetId id, CResFactory& factory) {
  mReloadData = nullptr;
  mReloadData = rs_new SReloadData(id, factory);
}

void CStringTable::TryFinishReload() {
  SReloadData* data = mReloadData.get();
  if (data != nullptr && data->mRequest->IsComplete()) {
    CMemoryInStream in(data->mBuffer.get(), data->mSize);
    Load(in);
    mReloadData = nullptr;
  }
}

bool CStringTable::IsReloading() const { return !mReloadData.null(); }

CStringTable::SReloadData::SReloadData(CAssetId id, CResFactory& factory)
: mRequest(nullptr) {
  const SObjectTag tag('STRG', id);
  mSize = factory.ResourceSize(tag);
  mBuffer = static_cast< uchar* >(CMemory::Alloc(mSize, IAllocator::kHI_RoundUpLen));
  mRequest = factory.LoadResourceAsync(tag, mBuffer.get());
}

CStringTable::SReloadData::~SReloadData() {}
#endif

const CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, CInputStream& in,
                                     const CVParamTransfer& xfer) {
  return rs_new CStringTable(in);
}
