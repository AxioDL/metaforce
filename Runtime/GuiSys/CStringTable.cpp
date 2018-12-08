#include "CStringTable.hpp"
#include "CToken.hpp"

namespace urde {
const std::vector<FourCC> CStringTable::skLanguages = {FOURCC('ENGL'), FOURCC('FREN'), FOURCC('GERM'), FOURCC('SPAN'),
                                                       FOURCC('ITAL'), FOURCC('DUTC'), FOURCC('JAPN')};

FourCC CStringTable::mCurrentLanguage = CStringTable::skLanguages[0];

CStringTable::CStringTable(CInputStream& in) { LoadStringTable(in); }

void CStringTable::LoadStringTable(CInputStream& in) {
  in.readUint32Big();
  in.readUint32Big();
  u32 langCount = in.readUint32Big();
  x0_stringCount = in.readUint32Big();
  std::vector<std::pair<FourCC, u32>> langOffsets;
  for (u32 i = 0; i < langCount; ++i) {
    FourCC fcc(in.readUint32());
    u32 off = in.readUint32Big();
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
  if (offset == -1)
    offset = langOffsets[0].second;

  in.seek(offset);

  u32 dataLen = in.readUint32Big();
  m_bufLen = dataLen;
  x4_data.reset(new u8[dataLen]);
  in.readUBytesToBuf(x4_data.get(), dataLen);

  u32* off = reinterpret_cast<u32*>(x4_data.get());
  for (u32 i = 0; i < x0_stringCount; ++i, ++off)
    *off = hecl::SBig(*off);

  for (u32 i = x0_stringCount * 4; i < dataLen; i += 2) {
    u16* chr = reinterpret_cast<u16*>(x4_data.get() + i);
    *chr = hecl::SBig(*chr);
  }
}

const char16_t* CStringTable::GetString(s32 str) const {
  if (str < 0 || u32(str) >= x0_stringCount)
    return u"Invalid";

  u32 off = *reinterpret_cast<u32*>(x4_data.get() + str * 4);
  return reinterpret_cast<char16_t*>(x4_data.get() + off);
}

void CStringTable::SetLanguage(s32 lang) { mCurrentLanguage = skLanguages[lang]; }

CFactoryFnReturn FStringTableFactory(const SObjectTag&, CInputStream& in, const CVParamTransfer&,
                                     CObjectReference* selfRef) {
  return TToken<CStringTable>::GetIObjObjectFor(std::make_unique<CStringTable>(in));
}

} // namespace urde
