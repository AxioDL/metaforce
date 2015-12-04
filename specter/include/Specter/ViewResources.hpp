#ifndef SPECTER_VIEWRESOURCES_HPP
#define SPECTER_VIEWRESOURCES_HPP

#include "TextView.hpp"
#include "RootView.hpp"
#include "Toolbar.hpp"

namespace Specter
{
class ThemeData
{
    Zeus::CColor m_vpBg = {0.2,0.2,0.2,1.0};
    Zeus::CColor m_tbBg = {0.4,0.4,0.4,1.0};
    Zeus::CColor m_uiText = Zeus::CColor::skWhite;
public:
    virtual const Zeus::CColor& viewportBackground() const {return m_vpBg;}
    virtual const Zeus::CColor& toolbarBackground() const {return m_tbBg;}
    virtual const Zeus::CColor& uiText() const {return m_uiText;}
};

class ViewResources
{
    template <class Factory>
    void init(Factory* factory, FontCache* fcache, const ThemeData& theme)
    {
        m_viewRes.init(factory, theme);
        m_textRes.init(factory, fcache);
        m_splitRes.init(factory, theme);
        m_toolbarRes.init(factory, theme);
    }

public:
    boo::IGraphicsDataFactory* m_factory = nullptr;
    FontCache* m_fcache = nullptr;
    View::Resources m_viewRes;
    TextView::Resources m_textRes;
    SplitView::Resources m_splitRes;
    Toolbar::Resources m_toolbarRes;
    std::unique_ptr<boo::IGraphicsData> m_fontData;
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

    void init(boo::IGraphicsDataFactory* factory, FontCache* fcache, const ThemeData& theme, unsigned dpi);
    void resetDPI(unsigned dpi);
    void resetTheme(const ThemeData& theme);

    float m_pixelFactor = 0;
    float pixelFactor() const {return m_pixelFactor;}

    ThemeData m_theme;
    const ThemeData& themeData() const {return m_theme;}
};
}

#endif // SPECTER_VIEWRESOURCES_HPP
