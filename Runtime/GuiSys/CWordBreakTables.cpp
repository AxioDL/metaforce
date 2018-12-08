#include "CWordBreakTables.hpp"

namespace urde {

struct CCharacterIdentifier {
  wchar_t chr;
  u32 rank;
};

static const CCharacterIdentifier gCantBeginChars[] = {
    {L'!', 1},   {L')', 1},   {L',', 1},   {L'-', 1},   {L'.', 1},   {L':', 1},   {L';', 1},   {L'?', 1},
    {L']', 1},   {L'}', 1},   {0x92, 1},   {0x94, 1},   {0xBB, 1},   {0x3001, 1}, {0x3002, 1}, {0x3005, 1},
    {0x300D, 1}, {0x300F, 1}, {0x3011, 1}, {0x3015, 1}, {0x3017, 1}, {0x3019, 1}, {0x301B, 1}, {0x301C, 3},
    {0x301E, 1}, {0x302B, 3}, {0x3041, 2}, {0x3043, 2}, {0x3045, 2}, {0x3047, 2}, {0x3049, 2}, {0x3063, 2},
    {0x3083, 2}, {0x3085, 2}, {0x3087, 2}, {0x308E, 2}, {0x309D, 3}, {0x309E, 3}, {0x30A1, 2}, {0x30A3, 2},
    {0x30A5, 2}, {0x30A7, 2}, {0x30A9, 2}, {0x30C3, 2}, {0x30E3, 2}, {0x30E5, 2}, {0x30E7, 2}, {0x30EE, 2},
    {0x30F5, 2}, {0x30F6, 2}, {0x30FC, 2}, {0x30FD, 3}, {0x30FE, 3}, {0xFF01, 1}, {0xFF05, 3}, {0xFF09, 1},
    {0xFF0D, 1}, {0xFF3D, 1}, {0xFF5D, 1}, {0xFF61, 1}, {0xFF63, 1}, {0xFF64, 1}, {0xFF1F, 1}};

static const CCharacterIdentifier gCantEndChars[] = {
    {L'#', 2},   {L'$', 2},   {L'(', 1},   {L'@', 2},   {L'B', 4},   {L'C', 4},   {L'D', 4},   {L'E', 4},   {L'F', 4},
    {L'G', 4},   {L'J', 4},   {L'K', 4},   {L'L', 4},   {L'M', 4},   {L'N', 4},   {L'P', 4},   {L'Q', 4},   {L'R', 4},
    {L'S', 4},   {L'T', 4},   {L'V', 4},   {L'W', 4},   {L'X', 4},   {L'Y', 4},   {L'Z', 4},   {L'b', 4},   {L'c', 4},
    {L'd', 4},   {L'f', 4},   {L'g', 4},   {L'h', 4},   {L'j', 4},   {L'k', 4},   {L'l', 4},   {L'm', 4},   {L'n', 4},
    {L'p', 4},   {L'q', 4},   {L'r', 4},   {L's', 4},   {L't', 4},   {L'v', 4},   {L'w', 4},   {L'x', 4},   {L'y', 4},
    {L'z', 4},   {0xD1, 4},   {0xF1, 4},   {L'[', 1},   {L'{', 1},   {0x91, 1},   {0x93, 1},   {0xA2, 2},   {0xA3, 2},
    {0xA5, 2},   {0xA7, 2},   {0xA9, 2},   {0xAB, 1},   {0x20A0, 2}, {0x20A1, 2}, {0x20A2, 2}, {0x20A3, 2}, {0x20A4, 2},
    {0x20A5, 2}, {0x20A6, 2}, {0x20A7, 2}, {0x20A8, 2}, {0x20A9, 2}, {0x20AA, 2}, {0x20AB, 2}, {0x20AC, 2}, {0x300C, 1},
    {0x300E, 1}, {0x3010, 1}, {0x3012, 2}, {0x3014, 1}, {0x3016, 1}, {0x3018, 1}, {0x301A, 1}, {0xFF03, 2}, {0xFF04, 2},
    {0xFF20, 2}, {0xFF3C, 1}, {0xFF5C, 1}, {0xFFE0, 2}, {0xFFE1, 2}, {0xFFEF, 2},
};

int CWordBreakTables::GetBeginRank(wchar_t ch) {
  auto search = rstl::binary_find(std::cbegin(gCantBeginChars), std::cend(gCantBeginChars), ch,
                                  [](const CCharacterIdentifier& item) { return item.chr; });
  if (search == std::cend(gCantBeginChars))
    return 5;
  return search->rank;
}

int CWordBreakTables::GetEndRank(wchar_t ch) {
  auto search = rstl::binary_find(std::cbegin(gCantEndChars), std::cend(gCantEndChars), ch,
                                  [](const CCharacterIdentifier& item) { return item.chr; });
  if (search == std::cend(gCantEndChars))
    return 5;
  return search->rank;
}

} // namespace urde
