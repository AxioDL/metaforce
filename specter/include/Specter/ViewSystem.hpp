#ifndef SPECTER_VIEWSYSTEM_HPP
#define SPECTER_VIEWSYSTEM_HPP

#include "TextView.hpp"

namespace Specter
{
class ViewSystem
{
    template <class Factory>
    void init(Factory* factory, FontCache* fcache)
    {
        m_viewSystem.init(factory);
        m_textSystem.init(factory, fcache);
    }

public:
    boo::IGraphicsDataFactory* m_factory = nullptr;
    View::System m_viewSystem;
    TextView::System m_textSystem;

    ViewSystem() = default;
    ViewSystem(const ViewSystem& other) = delete;
    ViewSystem(ViewSystem&& other) = default;
    ViewSystem& operator=(const ViewSystem& other) = delete;
    ViewSystem& operator=(ViewSystem&& other) = default;

    void init(boo::IGraphicsDataFactory* factory, FontCache* fcache);
};
}

#endif // SPECTER_VIEWSYSTEM_HPP
