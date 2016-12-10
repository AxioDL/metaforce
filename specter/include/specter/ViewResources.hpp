#ifndef SPECTER_VIEWRESOURCES_HPP
#define SPECTER_VIEWRESOURCES_HPP

#include "TextView.hpp"
#include "SplitView.hpp"
#include "Toolbar.hpp"
#include "Button.hpp"

#include <thread>

namespace specter
{
class IThemeData
{
public:
    virtual const zeus::CColor& uiText() const=0;
    virtual const zeus::CColor& uiAltText() const=0;
    virtual const zeus::CColor& fieldText() const=0;
    virtual const zeus::CColor& fieldMarkedText() const=0;
    virtual const zeus::CColor& selectedFieldText() const=0;

    virtual const zeus::CColor& viewportBackground() const=0;
    virtual const zeus::CColor& toolbarBackground() const=0;
    virtual const zeus::CColor& tooltipBackground() const=0;
    virtual const zeus::CColor& spaceBackground() const=0;
    virtual const zeus::CColor& splashBackground() const=0;
    virtual const zeus::CColor& splashErrorBackground() const=0;

    virtual const zeus::CColor& splash1() const=0;
    virtual const zeus::CColor& splash2() const=0;

    virtual const zeus::CColor& button1Inactive() const=0;
    virtual const zeus::CColor& button2Inactive() const=0;
    virtual const zeus::CColor& button1Hover() const=0;
    virtual const zeus::CColor& button2Hover() const=0;
    virtual const zeus::CColor& button1Press() const=0;
    virtual const zeus::CColor& button2Press() const=0;
    virtual const zeus::CColor& button1Disabled() const=0;
    virtual const zeus::CColor& button2Disabled() const=0;

    virtual const zeus::CColor& textfield1Inactive() const=0;
    virtual const zeus::CColor& textfield2Inactive() const=0;
    virtual const zeus::CColor& textfield1Hover() const=0;
    virtual const zeus::CColor& textfield2Hover() const=0;
    virtual const zeus::CColor& textfield1Disabled() const=0;
    virtual const zeus::CColor& textfield2Disabled() const=0;
    virtual const zeus::CColor& textfieldSelection() const=0;
    virtual const zeus::CColor& textfieldMarkSelection() const=0;

    virtual const zeus::CColor& tableCellBg1() const=0;
    virtual const zeus::CColor& tableCellBg2() const=0;
    virtual const zeus::CColor& tableCellBgSelected() const=0;

    virtual const zeus::CColor& scrollIndicator() const=0;

    virtual const zeus::CColor& spaceTriangleShading1() const=0;
    virtual const zeus::CColor& spaceTriangleShading2() const=0;
};

class DefaultThemeData : public IThemeData
{
    zeus::CColor m_uiText = zeus::CColor::skWhite;
    zeus::CColor m_uiAltText = zeus::CColor::skGrey;
    zeus::CColor m_fieldText = zeus::CColor::skBlack;
    zeus::CColor m_fieldMarkedText = {0.25, 0.25, 0.25, 1.0};
    zeus::CColor m_selectedFieldText = zeus::CColor::skWhite;

    zeus::CColor m_vpBg = {0.2, 0.2, 0.2, 1.0};
    zeus::CColor m_tbBg = {0.2, 0.2, 0.2, 0.9};
    zeus::CColor m_tooltipBg = {0.1, 0.1, 0.1, 0.85};
    zeus::CColor m_spaceBg = {0.075, 0.075, 0.075, 0.85};
    zeus::CColor m_splashBg = {0.075, 0.075, 0.075, 0.85};
    zeus::CColor m_splashErrorBg = {0.1, 0.01, 0.01, 0.85};

    zeus::CColor m_splash1 = {1.0, 1.0, 1.0, 1.0};
    zeus::CColor m_splash2 = {0.3, 0.3, 0.3, 1.0};

    zeus::CColor m_button1Inactive = {0.2823, 0.2823, 0.2823, 1.0};
    zeus::CColor m_button2Inactive = {0.1725, 0.1725, 0.1725, 1.0};
    zeus::CColor m_button1Hover = {0.3523, 0.3523, 0.3523, 1.0};
    zeus::CColor m_button2Hover = {0.2425, 0.2425, 0.2425, 1.0};
    zeus::CColor m_button1Press = {0.1725, 0.1725, 0.1725, 1.0};
    zeus::CColor m_button2Press = {0.2823, 0.2823, 0.2823, 1.0};
    zeus::CColor m_button1Disabled = {0.2823, 0.2823, 0.2823, 0.5};
    zeus::CColor m_button2Disabled = {0.1725, 0.1725, 0.1725, 0.5};

    zeus::CColor m_textfield2Inactive = {0.7823, 0.7823, 0.7823, 1.0};
    zeus::CColor m_textfield1Inactive = {0.5725, 0.5725, 0.5725, 1.0};
    zeus::CColor m_textfield2Hover = {0.8523, 0.8523, 0.8523, 1.0};
    zeus::CColor m_textfield1Hover = {0.6425, 0.6425, 0.6425, 1.0};
    zeus::CColor m_textfield2Disabled = {0.7823, 0.7823, 0.7823, 0.5};
    zeus::CColor m_textfield1Disabled = {0.5725, 0.5725, 0.5725, 0.5};
    zeus::CColor m_textfieldSelection = {0.2725, 0.2725, 0.2725, 1.0};
    zeus::CColor m_textfieldMarkSelection = {1.0, 1.0, 0.2725, 1.0};

    zeus::CColor m_tableCellBg1 = {0.1725, 0.1725, 0.1725, 0.75};
    zeus::CColor m_tableCellBg2 = {0.2425, 0.2425, 0.2425, 0.75};
    zeus::CColor m_tableCellBgSelected = {0.6425, 0.6425, 0.6425, 1.0};

    zeus::CColor m_scrollIndicator = {0.2823, 0.2823, 0.2823, 1.0};

    zeus::CColor m_spaceTriangleShading1 = {0.6425, 0.6425, 0.6425, 1.0};
    zeus::CColor m_spaceTriangleShading2 = {0.5725, 0.5725, 0.5725, 1.0};

public:
    virtual const zeus::CColor& uiText() const {return m_uiText;}
    virtual const zeus::CColor& uiAltText() const {return m_uiAltText;}
    virtual const zeus::CColor& fieldText() const {return m_fieldText;}
    virtual const zeus::CColor& fieldMarkedText() const {return m_fieldMarkedText;}
    virtual const zeus::CColor& selectedFieldText() const {return m_selectedFieldText;}

    virtual const zeus::CColor& viewportBackground() const {return m_vpBg;}
    virtual const zeus::CColor& toolbarBackground() const {return m_tbBg;}
    virtual const zeus::CColor& tooltipBackground() const {return m_tooltipBg;}
    virtual const zeus::CColor& spaceBackground() const {return m_spaceBg;}
    virtual const zeus::CColor& splashBackground() const {return m_splashBg;}
    virtual const zeus::CColor& splashErrorBackground() const {return m_splashErrorBg;}

    virtual const zeus::CColor& splash1() const {return m_splash1;}
    virtual const zeus::CColor& splash2() const {return m_splash2;}

    virtual const zeus::CColor& button1Inactive() const {return m_button1Inactive;}
    virtual const zeus::CColor& button2Inactive() const {return m_button2Inactive;}
    virtual const zeus::CColor& button1Hover() const {return m_button1Hover;}
    virtual const zeus::CColor& button2Hover() const {return m_button2Hover;}
    virtual const zeus::CColor& button1Press() const {return m_button1Press;}
    virtual const zeus::CColor& button2Press() const {return m_button2Press;}
    virtual const zeus::CColor& button1Disabled() const {return m_button1Disabled;}
    virtual const zeus::CColor& button2Disabled() const {return m_button2Disabled;}

    virtual const zeus::CColor& textfield1Inactive() const {return m_textfield1Inactive;}
    virtual const zeus::CColor& textfield2Inactive() const {return m_textfield2Inactive;}
    virtual const zeus::CColor& textfield1Hover() const {return m_textfield1Hover;}
    virtual const zeus::CColor& textfield2Hover() const {return m_textfield2Hover;}
    virtual const zeus::CColor& textfield1Disabled() const {return m_textfield1Disabled;}
    virtual const zeus::CColor& textfield2Disabled() const {return m_textfield2Disabled;}
    virtual const zeus::CColor& textfieldSelection() const {return m_textfieldSelection;}
    virtual const zeus::CColor& textfieldMarkSelection() const {return m_textfieldMarkSelection;}

    virtual const zeus::CColor& tableCellBg1() const {return m_tableCellBg1;}
    virtual const zeus::CColor& tableCellBg2() const {return m_tableCellBg2;}
    virtual const zeus::CColor& tableCellBgSelected() const {return m_tableCellBgSelected;}

    virtual const zeus::CColor& scrollIndicator() const {return m_scrollIndicator;}

    virtual const zeus::CColor& spaceTriangleShading1() const {return m_spaceTriangleShading1;}
    virtual const zeus::CColor& spaceTriangleShading2() const {return m_spaceTriangleShading2;}
};

class ViewResources
{
    template <class FactoryCtx>
    void init(FactoryCtx& factory, const IThemeData& theme, FontCache* fcache)
    {
        m_viewRes.init(factory, theme);
        m_textRes.init(factory, fcache);
        m_splitRes.init(factory, theme);
        m_toolbarRes.init(factory, theme);
        m_buttonRes.init(factory, theme);
    }

public:
    boo::IGraphicsDataFactory* m_factory;
    FontCache* m_fcache = nullptr;
    View::Resources m_viewRes;
    TextView::Resources m_textRes;
    SplitView::Resources m_splitRes;
    Toolbar::Resources m_toolbarRes;
    Button::Resources m_buttonRes;
    boo::GraphicsDataToken m_resData;

    specter::FontTag m_mainFont;
    specter::FontTag m_monoFont;

    specter::FontTag m_heading14;
    specter::FontTag m_heading18;

    specter::FontTag m_titleFont;
    specter::FontTag m_curveFont;

    std::thread m_fcacheThread;
    bool m_fcacheInterrupt = false;
    bool m_fcacheReady = false;

    ViewResources() = default;
    ViewResources(const ViewResources& other) = delete;
    ViewResources(ViewResources&& other) = default;
    ViewResources& operator=(const ViewResources& other) = delete;
    ViewResources& operator=(ViewResources&& other) = default;

    void destroyResData()
    {
        m_resData.doDestroy();
    }

    void updateBuffers()
    {
        m_viewRes.updateBuffers();
        m_textRes.updateBuffers();
    }

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
