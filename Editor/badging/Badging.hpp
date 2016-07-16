#ifndef __URDE_BADGING_HPP__
#define __URDE_BADGING_HPP__

#include "specter/ViewResources.hpp"

namespace urde
{
boo::GraphicsDataToken InitializeBadging(specter::ViewResources& viewRes);

specter::Icon& GetBadge();
}

#ifndef BADGE_PHRASE
#define BADGE_PHRASE "Prototype"
#endif

#endif // __URDE_BADGING_HPP__
