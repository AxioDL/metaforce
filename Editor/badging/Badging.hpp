#pragma once

#include "specter/ViewResources.hpp"

namespace metaforce {
void InitializeBadging(specter::ViewResources& viewRes);
void DestroyBadging();

specter::Icon& GetBadge();
} // namespace metaforce

#ifndef BADGE_PHRASE
#define BADGE_PHRASE "Alpha"
#endif
