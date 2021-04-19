#include "Runtime/Collision/CMaterialFilter.hpp"

namespace metaforce {
const CMaterialFilter CMaterialFilter::skPassEverything({0x00000000FFFFFFFF}, {0}, EFilterType::Always);
} // namespace metaforce
