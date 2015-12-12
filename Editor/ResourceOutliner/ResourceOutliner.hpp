#ifndef RUDE_RESOURCE_OUTLINER_HPP
#define RUDE_RESOURCE_OUTLINER_HPP

#include "Space.hpp"

namespace RUDE
{

class ResourceOutliner : public Space
{
    struct State : SpaceState
    {
        DECL_YAML
    } m_state;
    SpaceState* spaceState() {return &m_state;}
};

}

#endif // RUDE_RESOURCE_OUTLINER_HPP
