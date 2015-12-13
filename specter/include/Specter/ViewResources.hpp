#ifndef SPECTER_VIEWRESOURCES_HPP
#define SPECTER_VIEWRESOURCES_HPP

#include "TextView.hpp"
#include "SplitView.hpp"
#include "Toolbar.hpp"
#include "Button.hpp"

#include <thread>

namespace Specter
{
class ThemeData
{
    Zeus::CColor m_vpBg = {0.2,0.2,0.2,1.0};
    Zeus::CColor m_tbBg = {0.4,0.4,0.4,1.0};
    Zeus::CColor m_uiText = Zeus::CColor::skWhite;
    Zeus::CColor m_button1Inactive = {0.2823, 0.2823, 0.2823, 1.0};
    Zeus::CColor m_button2Inactive = {0.1725, 0.1725, 0.1725, 1.0};
    Zeus::CColor m_button1Hover = {0.3523, 0.3523, 0.3523, 1.0};
    Zeus::CColor m_button2Hover = {0.2425, 0.2425, 0.2425, 1.0};
    Zeus::CColor m_button1Press = {0.1725, 0.1725, 0.1725, 1.0};
    Zeus::CColor m_button2Press = {0.2823, 0.2823, 0.2823, 1.0};
    Zeus::CColor m_button1Disabled = {0.2823, 0.2823, 0.2823, 0.5};
    Zeus::CColor m_button2Disabled = {0.1725, 0.1725, 0.1725, 0.5};
    Zeus::CColor m_tooltipBg = {0.0, 0.0, 0.0, 0.65};
    Zeus::CColor m_splashBg = {0.1, 0.1, 0.1, 0.65};
    Zeus::CColor m_splash1 = {1.0, 1.0, 1.0, 1.0};
    Zeus::CColor m_splash2 = {0.3, 0.3, 0.3, 1.0};
public:
    virtual const Zeus::CColor& viewportBackground() const {return m_vpBg;}
    virtual const Zeus::CColor& toolbarBackground() const {return m_tbBg;}
    virtual const Zeus::CColor& uiText() const {return m_uiText;}
    virtual const Zeus::CColor& button1Inactive() const {return m_button1Inactive;}
    virtual const Zeus::CColor& button2Inactive() const {return m_button2Inactive;}
    virtual const Zeus::CColor& button1Hover() const {return m_button1Hover;}
    virtual const Zeus::CColor& button2Hover() const {return m_button2Hover;}
    virtual const Zeus::CColor& button1Press() const {return m_button1Press;}
    virtual const Zeus::CColor& button2Press() const {return m_button2Press;}
    virtual const Zeus::CColor& button1Disabled() const {return m_button1Disabled;}
    virtual const Zeus::CColor& button2Disabled() const {return m_button2Disabled;}
    virtual const Zeus::CColor& tooltipBackground() const {return m_tooltipBg;}
    virtual const Zeus::CColor& splashBackground() const {return m_splashBg;}
    virtual const Zeus::CColor& splash1() const {return m_splash1;}
    virtual const Zeus::CColor& splash2() const {return m_splash2;}
};

class ViewResources
{
    template <class Factory>
    void init(Factory* factory, const ThemeData& theme, FontCache* fcache)
    {
        m_viewRes.init(factory, theme);
        m_textRes.init(factory, fcache);
        m_splitRes.init(factory, theme);
        m_toolbarRes.init(factory, theme);
        m_buttonRes.init(factory, theme);
    }

public:
    boo::IGraphicsDataFactory* m_factory = nullptr;
    FontCache* m_fcache = nullptr;
    View::Resources m_viewRes;
    TextView::Resources m_textRes;
    SplitView::Resources m_splitRes;
    Toolbar::Resources m_toolbarRes;
    Button::Resources m_buttonRes;
    boo::IGraphicsDataToken m_resData;

    Specter::FontTag m_mainFont;
    Specter::FontTag m_monoFont;

    Specter::FontTag m_heading14;
    Specter::FontTag m_heading18;

    Specter::FontTag m_titleFont;
    Specter::FontTag m_curveFont;

    std::thread m_fcacheThread;
    bool m_fcacheReady = false;

    ViewResources() = default;
    ViewResources(const ViewResources& other) = delete;
    ViewResources(ViewResources&& other) = default;
    ViewResources& operator=(const ViewResources& other) = delete;
    ViewResources& operator=(ViewResources&& other) = default;

    void init(boo::IGraphicsDataFactory* factory, FontCache* fcache, const ThemeData& theme, float pixelFactor);
    void prepFontCacheSync();
    void prepFontCacheAsync(boo::IWindow* window);
    bool fontCacheReady() const {return m_fcacheReady;}
    void resetPixelFactor(float pixelFactor);
    void resetTheme(const ThemeData& theme);
    void resetLanguage(const ThemeData& theme);

    float m_pixelFactor = 0;
    float pixelFactor() const {return m_pixelFactor;}

    ThemeData m_theme;
    const ThemeData& themeData() const {return m_theme;}
};
}

#endif // SPECTER_VIEWRESOURCES_HPP
