#ifndef __URDE_CWORLD_HPP__
#define __URDE_CWORLD_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CGameArea;

class CWorld
{
    std::vector<std::unique_ptr<CGameArea>> x18_areas;
public:
    std::vector<std::unique_ptr<CGameArea>>& GetGameAreas() {return x18_areas;}
};

}

#endif // __URDE_CWORLD_HPP__
