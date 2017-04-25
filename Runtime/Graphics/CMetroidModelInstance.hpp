#ifndef __URDE_CMETROIDMODELINSTANCE_HPP__
#define __URDE_CMETROIDMODELINSTANCE_HPP__

#include <vector>
#include "RetroTypes.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
class CBooModel;
struct CBooSurface;

class CMetroidModelInstance
{
    friend class CBooRenderer;
    friend class CGameArea;

    int x0_visorFlags;
    zeus::CTransform x4_xf;
    zeus::CAABox x34_aabb;
    std::vector<CBooSurface> m_surfaces;
    std::unique_ptr<CBooModel> m_instance;
public:
    CMetroidModelInstance() = default;
    CMetroidModelInstance(CMetroidModelInstance&&) = default;
};

}

#endif // __URDE_CMETROIDMODELINSTANCE_HPP__
