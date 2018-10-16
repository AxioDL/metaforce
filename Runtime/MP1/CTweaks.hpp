#pragma once

#include "RetroTypes.hpp"
namespace hecl
{
class CVarManager;
}

namespace urde
{

namespace MP1
{

class CTweaks
{
public:
    void RegisterTweaks(hecl::CVarManager* cvarMgr);
    void RegisterResourceTweaks(hecl::CVarManager* cvarMgr);
};

}
}

