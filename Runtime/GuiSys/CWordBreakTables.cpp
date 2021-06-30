#include "Runtime/GuiSys/CWordBreakTables.hpp"

#include <array>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/rstl.hpp"

namespace metaforce {
namespace {
struct CCharacterIdentifier {
  char16_t chr;
  u32 rank;
};

constexpr std::array<CCharacterIdentifier, 63> sCantBeginChars{{
    {u'!', 1},   {u')', 1},   {u',', 1},   {u'-', 1},   {u'.', 1},   {u':', 1},   {u';', 1},   {u'?', 1},   {u']', 1},
    {u'}', 1},   {0x92, 1},   {0x94, 1},   {0xBB, 1},   {0x3001, 1}, {0x3002, 1}, {0x3005, 1}, {0x300D, 1}, {0x300F, 1},
    {0x3011, 1}, {0x3015, 1}, {0x3017, 1}, {0x3019, 1}, {0x301B, 1}, {0x301C, 3}, {0x301E, 1}, {0x302B, 3}, {0x3041, 2},
    {0x3043, 2}, {0x3045, 2}, {0x3047, 2}, {0x3049, 2}, {0x3063, 2}, {0x3083, 2}, {0x3085, 2}, {0x3087, 2}, {0x308E, 2},
    {0x309D, 3}, {0x309E, 3}, {0x30A1, 2}, {0x30A3, 2}, {0x30A5, 2}, {0x30A7, 2}, {0x30A9, 2}, {0x30C3, 2}, {0x30E3, 2},
    {0x30E5, 2}, {0x30E7, 2}, {0x30EE, 2}, {0x30F5, 2}, {0x30F6, 2}, {0x30FC, 2}, {0x30FD, 3}, {0x30FE, 3}, {0xFF01, 1},
    {0xFF05, 3}, {0xFF09, 1}, {0xFF0D, 1}, {0xFF3D, 1}, {0xFF5D, 1}, {0xFF61, 1}, {0xFF63, 1}, {0xFF64, 1}, {0xFF1F, 1},
}};

constexpr std::array<CCharacterIdentifier, 87> sCantEndChars{{
    {u'#', 2},   {u'$', 2},   {u'(', 1},   {u'@', 2},   {u'B', 4},   {u'C', 4},   {u'D', 4},   {u'E', 4},   {u'F', 4},
    {u'G', 4},   {u'J', 4},   {u'K', 4},   {u'L', 4},   {u'M', 4},   {u'N', 4},   {u'P', 4},   {u'Q', 4},   {u'R', 4},
    {u'S', 4},   {u'T', 4},   {u'V', 4},   {u'W', 4},   {u'X', 4},   {u'Y', 4},   {u'Z', 4},   {u'b', 4},   {u'c', 4},
    {u'd', 4},   {u'f', 4},   {u'g', 4},   {u'h', 4},   {u'j', 4},   {u'k', 4},   {u'l', 4},   {u'm', 4},   {u'n', 4},
    {u'p', 4},   {u'q', 4},   {u'r', 4},   {u's', 4},   {u't', 4},   {u'v', 4},   {u'w', 4},   {u'x', 4},   {u'y', 4},
    {u'z', 4},   {0xD1, 4},   {0xF1, 4},   {u'[', 1},   {u'{', 1},   {0x91, 1},   {0x93, 1},   {0xA2, 2},   {0xA3, 2},
    {0xA5, 2},   {0xA7, 2},   {0xA9, 2},   {0xAB, 1},   {0x20A0, 2}, {0x20A1, 2}, {0x20A2, 2}, {0x20A3, 2}, {0x20A4, 2},
    {0x20A5, 2}, {0x20A6, 2}, {0x20A7, 2}, {0x20A8, 2}, {0x20A9, 2}, {0x20AA, 2}, {0x20AB, 2}, {0x20AC, 2}, {0x300C, 1},
    {0x300E, 1}, {0x3010, 1}, {0x3012, 2}, {0x3014, 1}, {0x3016, 1}, {0x3018, 1}, {0x301A, 1}, {0xFF03, 2}, {0xFF04, 2},
    {0xFF20, 2}, {0xFF3C, 1}, {0xFF5C, 1}, {0xFFE0, 2}, {0xFFE1, 2}, {0xFFEF, 2},
}};
} // Anonymous namespace

int CWordBreakTables::GetBeginRank(char16_t ch) {
  const auto search = rstl::binary_find(sCantBeginChars.cbegin(), sCantBeginChars.cend(), ch,
                                        [](const CCharacterIdentifier& item) { return item.chr; });
  if (search == sCantBeginChars.cend()) {
    return 5;
  }
  return search->rank;
}

int CWordBreakTables::GetEndRank(char16_t ch) {
  const auto search = rstl::binary_find(sCantEndChars.cbegin(), sCantEndChars.cend(), ch,
                                        [](const CCharacterIdentifier& item) { return item.chr; });
  if (search == sCantEndChars.cend()) {
    return 5;
  }
  return search->rank;
}

} // namespace metaforce
