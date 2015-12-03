#ifndef SPECTER_VIEWRESOURCES_HPP
#define SPECTER_VIEWRESOURCES_HPP

#include "TextView.hpp"
#include "RootView.hpp"
#include "Toolbar.hpp"

namespace Specter
{
class ViewResources
{
    template <class Factory>
    void init(Factory* factory, FontCache* fcache)
    {
        m_viewRes.init(factory);
        m_textRes.init(factory, fcache);
        m_splitRes.init(factory);
    }

public:
    boo::IGraphicsDataFactory* m_factory = nullptr;
    View::Resources m_viewRes;
    TextView::Resources m_textRes;
    SplitView::Resources m_splitRes;
    Toolbar::Resources m_toolbarRes;
    std::unique_ptr<boo::IGraphicsData> m_resData;

    Specter::FontTag m_mainFont;
    Specter::FontTag m_monoFont;

    Specter::FontTag m_heading14;
    Specter::FontTag m_heading18;

    ViewResources() = default;
    ViewResources(const ViewResources& other) = delete;
    ViewResources(ViewResources&& other) = default;
    ViewResources& operator=(const ViewResources& other) = delete;
    ViewResources& operator=(ViewResources&& other) = default;

    void init(boo::IGraphicsDataFactory* factory, FontCache* fcache, unsigned dpi);

    float m_pixelFactor = 0;
    float pixelFactor() const {return m_pixelFactor;}
};
}

#endif // SPECTER_VIEWRESOURCES_HPP
