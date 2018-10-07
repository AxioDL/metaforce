#pragma once

#include "specter/Icon.hpp"
#include "specter/ViewResources.hpp"

namespace urde
{
void InitializeIcons(specter::ViewResources& viewRes);
void DestroyIcons();

enum class SpaceIcon
{
    ResourceBrowser,
    ParticleEditor,
    WorldEditor,
    ModelViewer,
    InformationCenter,
    GameMode
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

