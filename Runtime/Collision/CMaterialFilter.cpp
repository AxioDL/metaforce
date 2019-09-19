#include "Runtime/Collision/CMaterialFilter.hpp"

namespace urde {
const CMaterialFilter CMaterialFilter::skPassEverything({0x00000000FFFFFFFF}, {0},
                                                        CMaterialFilter::EFilterType::Always);
} // namespace urde
