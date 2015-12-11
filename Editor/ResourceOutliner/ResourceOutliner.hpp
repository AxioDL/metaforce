#ifndef RUDE_RESOURCE_OUTLINER_HPP
#define RUDE_RESOURCE_OUTLINER_HPP

#include "Space.hpp"

namespace RUDE
{

struct ResourceOutlinerState : SpaceState
{
    DECL_YAML
};

class ResourceOutliner : public Space
{
    ResourceOutlinerState m_state;
    SpaceState& spaceState() {return m_state;}
};

}

#endif // RUDE_RESOURCE_OUTLINER_HPP
