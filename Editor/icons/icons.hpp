#ifndef URDE_ICONS_HPP
#define URDE_ICONS_HPP

#include "specter/Icon.hpp"
#include "specter/ViewResources.hpp"

namespace urde
{
boo::GraphicsDataToken InitializeIcons(specter::ViewResources& viewRes);

enum class SpaceIcon
{
    ResourceBrowser,
    ParticleEditor,
    WorldEditor,
    ModelViewer,
    InformationCenter
};
specter::Icon& GetIcon(SpaceIcon icon);

enum class MonoIcon
{
    Sync,
    Edit,
    Caution,
    Save,
    Filter,
    Document,
    ZoomOut,
    ZoomIn,

    Exclaim,
    Clock,
    Gamepad,
    Unlink,
    Link,
    Folder,
    Info
};
specter::Icon& GetIcon(MonoIcon icon);

}

#endif // URDE_ICONS_HPP
