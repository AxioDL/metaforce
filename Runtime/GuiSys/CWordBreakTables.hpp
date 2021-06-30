#pragma once

namespace metaforce {

class CWordBreakTables {
public:
  static int GetBeginRank(char16_t ch);
  static int GetEndRank(char16_t ch);
};

} // namespace metaforce
