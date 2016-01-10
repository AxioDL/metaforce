#ifndef SPECTER_VIEWRESOURCES_HPP
#define SPECTER_VIEWRESOURCES_HPP

#include "TextView.hpp"
#include "SplitView.hpp"
#include "Toolbar.hpp"
#include "Button.hpp"

#include <thread>

namespace Specter
{
class IThemeData
{
public:
    virtual const Zeus::CColor& uiText() const=0;
    virtual const Zeus::CColor& fieldText() const=0;
    virtual const Zeus::CColor& fieldMarkedText() const=0;
    virtual const Zeus::CColor& selectedFieldText() const=0;

    virtual const Zeus::CColor& viewportBackground() const=0;
    virtual const Zeus::CColor& toolbarBackground() const=0;
    virtual const Zeus::CColor& tooltipBackground() const=0;
    virtual const Zeus::CColor& spaceBackground() const=0;
    virtual const Zeus::CColor& splashBackground() const=0;
    virtual const Zeus::CColor& splashErrorBackground() const=0;

    virtual const Zeus::CColor& splash1() const=0;
    virtual const Zeus::CColor& splash2() const=0;

    virtual const Zeus::CColor& button1Inactive() const=0;
    virtual const Zeus::CColor& button2Inactive() const=0;
    virtual const Zeus::CColor& button1Hover() const=0;
    virtual const Zeus::CColor& button2Hover() const=0;
    virtual const Zeus::CColor& button1Press() const=0;
    virtual const Zeus::CColor& button2Press() const=0;
    virtual const Zeus::CColor& button1Disabled() const=0;
    virtual const Zeus::CColor& button2Disabled() const=0;

    virtual const Zeus::CColor& textfield1Inactive() const=0;
    virtual const Zeus::CColor& textfield2Inactive() const=0;
    virtual const Zeus::CColor& textfield1Hover() const=0;
    virtual const Zeus::CColor& textfield2Hover() const=0;
    virtual const Zeus::CColor& textfield1Disabled() const=0;
    virtual const Zeus::CColor& textfield2Disabled() const=0;
    virtual const Zeus::CColor& textfieldSelection() const=0;
    virtual const Zeus::CColor& textfieldMarkSelection() const=0;

    virtual const Zeus::CColor& tableCellBg1() const=0;
    virtual const Zeus::CColor& tableCellBg2() const=0;
    virtual const Zeus::CColor& tableCellBgSelected() const=0;

    virtual const Zeus::CColor& scrollIndicator() const=0;

    virtual const Zeus::CColor& spaceTriangleShading1() const=0;
    virtual const Zeus::CColor& spaceTriangleShading2() const=0;
};

class DefaultThemeData : public IThemeData
{
    Zeus::CColor m_uiText = Zeus::CColor::skWhite;
    Zeus::CColor m_fieldText = Zeus::CColor::skBlack;
    Zeus::CColor m_fieldMarkedText = {0.25, 0.25, 0.25, 1.0};
    Zeus::CColor m_selectedFieldText = Zeus::CColor::skWhite;

    Zeus::CColor m_vpBg = {0.2, 0.2, 0.2, 1.0};
    Zeus::CColor m_tbBg = {0.2, 0.2, 0.2, 0.9};
    Zeus::CColor m_tooltipBg = {0.1, 0.1, 0.1, 0.85};
    Zeus::CColor m_spaceBg = {0.075, 0.075, 0.075, 0.85};
    Zeus::CColor m_splashBg = {0.075, 0.075, 0.075, 0.85};
    Zeus::CColor m_splashErrorBg = {0.1, 0.01, 0.01, 0.85};

    Zeus::CColor m_splash1 = {1.0, 1.0, 1.0, 1.0};
    Zeus::CColor m_splash2 = {0.3, 0.3, 0.3, 1.0};

    Zeus::CColor m_button1Inactive = {0.2823, 0.2823, 0.2823, 1.0};
    Zeus::CColor m_button2Inactive = {0.1725, 0.1725, 0.1725, 1.0};
    Zeus::CColor m_button1Hover = {0.3523, 0.3523, 0.3523, 1.0};
    Zeus::CColor m_button2Hover = {0.2425, 0.2425, 0.2425, 1.0};
    Zeus::CColor m_button1Press = {0.1725, 0.1725, 0.1725, 1.0};
    Zeus::CColor m_button2Press = {0.2823, 0.2823, 0.2823, 1.0};
    Zeus::CColor m_button1Disabled = {0.2823, 0.2823, 0.2823, 0.5};
    Zeus::CColor m_button2Disabled = {0.1725, 0.1725, 0.1725, 0.5};

    Zeus::CColor m_textfield2Inactive = {0.7823, 0.7823, 0.7823, 1.0};
    Zeus::CColor m_textfield1Inactive = {0.5725, 0.5725, 0.5725, 1.0};
    Zeus::CColor m_textfield2Hover = {0.8523, 0.8523, 0.8523, 1.0};
    Zeus::CColor m_textfield1Hover = {0.6425, 0.6425, 0.6425, 1.0};
    Zeus::CColor m_textfield2Disabled = {0.7823, 0.7823, 0.7823, 0.5};
    Zeus::CColor m_textfield1Disabled = {0.5725, 0.5725, 0.5725, 0.5};
    Zeus::CColor m_textfieldSelection = {0.2725, 0.2725, 0.2725, 1.0};
    Zeus::CColor m_textfieldMarkSelection = {1.0, 1.0, 0.2725, 1.0};

    Zeus::CColor m_tableCellBg1 = {0.1725, 0.1725, 0.1725, 0.75};
    Zeus::CColor m_tableCellBg2 = {0.2425, 0.2425, 0.2425, 0.75};
    Zeus::CColor m_tableCellBgSelected = {0.6425, 0.6425, 0.6425, 1.0};

    Zeus::CColor m_scrollIndicator = {0.2823, 0.2823, 0.2823, 1.0};

    Zeus::CColor m_spaceTriangleShading1 = {0.6425, 0.6425, 0.6425, 1.0};
    Zeus::CColor m_spaceTriangleShading2 = {0.5725, 0.5725, 0.5725, 1.0};

public:
    virtual const Zeus::CColor& uiText() const {return m_uiText;}
    virtual const Zeus::CColor& fieldText() const {return m_fieldText;}
    virtual const Zeus::CColor& fieldMarkedText() const {return m_fieldMarkedText;}
    virtual const Zeus::CColor& selectedFieldText() const {return m_selectedFieldText;}

    virtual const Zeus::CColor& viewportBackground() const {return m_vpBg;}
    virtual const Zeus::CColor& toolbarBackground() const {return m_tbBg;}
    virtual const Zeus::CColor& tooltipBackground() const {return m_tooltipBg;}
    virtual const Zeus::CColor& spaceBackground() const {return m_spaceBg;}
    virtual const Zeus::CColor& splashBackground() const {return m_splashBg;}
    virtual const Zeus::CColor& splashErrorBackground() const {return m_splashErrorBg;}

    virtual const Zeus::CColor& splash1() const {return m_splash1;}
    virtual const Zeus::CColor& splash2() const {return m_splash2;}

    virtual const Zeus::CColor& button1Inactive() const {return m_button1Inactive;}
    virtual const Zeus::CColor& button2Inactive() const {return m_button2Inactive;}
    virtual const Zeus::CColor& button1Hover() const {return m_button1Hover;}
    virtual const Zeus::CColor& button2Hover() const {return m_button2Hover;}
    virtual const Zeus::CColor& button1Press() const {return m_button1Press;}
    virtual const Zeus::CColor& button2Press() const {return m_button2Press;}
    virtual const Zeus::CColor& button1Disabled() const {return m_button1Disabled;}
    virtual const Zeus::CColor& button2Disabled() const {return m_button2Disabled;}

    virtual const Zeus::CColor& textfield1Inactive() const {return m_textfield1Inactive;}
    virtual const Zeus::CColor& textfield2Inactive() const {return m_textfield2Inactive;}
    virtual const Zeus::CColor& textfield1Hover() const {return m_textfield1Hover;}
    virtual const Zeus::CColor& textfield2Hover() const {return m_textfield2Hover;}
    virtual const Zeus::CColor& textfield1Disabled() const {return m_textfield1Disabled;}
    virtual const Zeus::CColor& textfield2Disabled() const {return m_textfield2Disabled;}
    virtual const Zeus::CColor& textfieldSelection() const {return m_textfieldSelection;}
    virtual const Zeus::CColor& textfieldMarkSelection() const {return m_textfieldMarkSelection;}

    virtual const Zeus::CColor& tableCellBg1() const {return m_tableCellBg1;}
    virtual const Zeus::CColor& tableCellBg2() const {return m_tableCellBg2;}
    virtual const Zeus::CColor& tableCellBgSelected() const {return m_tableCellBgSelected;}

    virtual const Zeus::CColor& scrollIndicator() const {return m_scrollIndicator;}

    virtual const Zeus::CColor& spaceTriangleShading1() const {return m_spaceTriangleShading1;}
    virtual const Zeus::CColor& spaceTriangleShading2() const {return m_spaceTriangleShading2;}
};

class ViewResources
{
    template <class Factory>
    void init(Factory* factory, const IThemeData& theme, FontCache* fcache)
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
    boo::GraphicsDataToken m_resData;

    Specter::FontTag m_mainFont;
    Specter::FontTag m_monoFont;

    Specter::FontTag m_heading14;
    Specter::FontTag m_heading18;

    Specter::FontTag m_titleFont;
    Specter::FontTag m_curveFont;

    std::thread m_fcacheThread;
    bool m_fcacheInterrupt = false;
    bool m_fcacheReady = false;

    ViewResources() = default;
    ViewResources(const ViewResources& other) = delete;
    ViewResources(ViewResources&& other) = default;
    ViewResources& operator=(const ViewResources& other) = delete;
    ViewResources& operator=(ViewResources&& other) = default;

    ~ViewResources()
    {
        if (m_fcacheThread.joinable())
        {
            m_fcacheInterrupt = true;
            m_fcacheThread.join();
        }
    }

    void init(boo::IGraphicsDataFactory* factory, FontCache* fcache, const IThemeData* theme, float pixelFactor);
    void prepFontCacheSync();
    void prepFontCacheAsync(boo::IWindow* window);
    bool fontCacheReady() const {return m_fcacheReady;}
    void resetPixelFactor(float pixelFactor);
    void resetTheme(const IThemeData* theme);

    float m_pixelFactor = 0;
    float pixelFactor() const {return m_pixelFactor;}

    const IThemeData* m_theme;
    const IThemeData& themeData() const {return *m_theme;}
};
}

#endif // SPECTER_VIEWRESOURCES_HPP
