#ifndef URDE_ICONS_HPP
#define URDE_ICONS_HPP

#include "Specter/Icon.hpp"
#include "Specter/ViewResources.hpp"

namespace URDE
{
boo::GraphicsDataToken InitializeIcons(Specter::ViewResources& viewRes);

enum class SpaceIcon
{
    ResourceBrowser,
    ParticleEditor,
    WorldEditor,
    ModelViewer,
    InformationCenter
};
Specter::Icon& GetIcon(SpaceIcon icon);

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
Specter::Icon& GetIcon(MonoIcon icon);

}

#endif // URDE_ICONS_HPP
