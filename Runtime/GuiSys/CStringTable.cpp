#include "Runtime/GuiSys/CStringTable.hpp"

#include "Runtime/CBasics.hpp"
#include "Runtime/CInputStream.hpp"
#include "Runtime/CToken.hpp"

#include <array>

namespace metaforce {
namespace {
constexpr std::array languages{
    FOURCC('ENGL'), FOURCC('FREN'), FOURCC('GERM'), FOURCC('SPAN'), FOURCC('ITAL'), FOURCC('DUTC'), FOURCC('JAPN'),
};
} // Anonymous namespace

FourCC CStringTable::mCurrentLanguage = languages[0];

CStringTable::CStringTable(CInputStream& in) { LoadStringTable(in); }

void CStringTable::LoadStringTable(CInputStream& in) {
  in.ReadLong();
  in.ReadLong();
  u32 langCount = in.ReadLong();
  x0_stringCount = in.ReadLong();
  std::vector<std::pair<FourCC, u32>> langOffsets;
  for (u32 i = 0; i < langCount; ++i) {
    FourCC fcc;
    in.Get(reinterpret_cast<u8*>(&fcc), 4);
    u32 off = in.ReadLong();
    langOffsets.emplace_back(fcc, off);
  }

  u32 lang = 0;
  u32 offset = 0;
  while ((langCount--) > 0) {
    if (langOffsets[lang].first == mCurrentLanguage) {
      offset = langOffsets[lang].second;
      break;
    }

    lang++;
  }

  /*
   * If we fail to get a language, default to the first in the list
   * This way we always display _something_
   */
  if (offset == UINT32_MAX)
    offset = langOffsets[0].second;

  for (u32 i = 0; i < offset; ++i) {
    in.ReadChar();
  }

  u32 dataLen = in.ReadLong();
  m_bufLen = dataLen;
  x4_data.reset(new u8[dataLen]);
  in.Get(x4_data.get(), dataLen);

#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  u32* off = reinterpret_cast<u32*>(x4_data.get());
  for (u32 i = 0; i < x0_stringCount; ++i, ++off) {
    *off = CBasics::SwapBytes(*off);
  }

  for (u32 i = x0_stringCount * 4; i < dataLen; i += 2) {
    u16* chr = reinterpret_cast<u16*>(x4_data.get() + i);
    *chr = CBasics::SwapBytes(*chr);
  }
#endif
}

const char16_t* CStringTable::GetString(s32 str) const {
  if (str < 0 || u32(str) >= x0_stringCount)
    return u"Invalid";

  u32 off = *reinterpret_cast<u32*>(x4_data.get() + str * 4);
  return reinterpret_cast<char16_t*>(x4_data.get() + off);
}

void CStringTable::SetLanguage(s32 lang) { mCurrentLanguage = languages[lang]; }

CFactoryFnReturn FStringTableFactory(const SObjectTag&, CInputStream& in, const CVParamTransfer&,
                                     [[maybe_unused]] CObjectReference* selfRef) {
  return TToken<CStringTable>::GetIObjObjectFor(std::make_unique<CStringTable>(in));
}

} // namespace metaforce
