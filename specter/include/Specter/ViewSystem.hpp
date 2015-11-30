#ifndef SPECTER_VIEWSYSTEM_HPP
#define SPECTER_VIEWSYSTEM_HPP

#include "TextView.hpp"
#include "RootView.hpp"

namespace Specter
{
class ViewSystem
{
    template <class Factory>
    void init(Factory* factory, FontCache* fcache)
    {
        m_viewSystem.init(factory);
        m_textSystem.init(factory, fcache);
        m_splitViewSystem.init(factory);
    }

public:
    boo::IGraphicsDataFactory* m_factory = nullptr;
    View::System m_viewSystem;
    TextView::System m_textSystem;
    RootView::SplitView::System m_splitViewSystem;
    std::unique_ptr<boo::IGraphicsData> m_sysData;

    Specter::FontTag m_mainFont;
    Specter::FontTag m_monoFont;

    ViewSystem() = default;
    ViewSystem(const ViewSystem& other) = delete;
    ViewSystem(ViewSystem&& other) = default;
    ViewSystem& operator=(const ViewSystem& other) = delete;
    ViewSystem& operator=(ViewSystem&& other) = default;

    void init(boo::IGraphicsDataFactory* factory, FontCache* fcache);
};
}

#endif // SPECTER_VIEWSYSTEM_HPP
