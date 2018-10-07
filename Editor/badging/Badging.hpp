#pragma once

#include "specter/ViewResources.hpp"

namespace urde
{
void InitializeBadging(specter::ViewResources& viewRes);
void DestroyBadging();

specter::Icon& GetBadge();
}

#ifndef BADGE_PHRASE
#define BADGE_PHRASE "Prototype"
#endif

