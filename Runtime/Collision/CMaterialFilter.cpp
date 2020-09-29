#include "Runtime/Collision/CMaterialFilter.hpp"

namespace urde {
const CMaterialFilter CMaterialFilter::skPassEverything({0x00000000FFFFFFFF}, {0}, EFilterType::Always);
} // namespace urde
