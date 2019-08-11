#pragma once

#include "TextView.hpp"
#include "SplitView.hpp"
#include "Toolbar.hpp"
#include "Button.hpp"

#include <thread>

namespace specter {
class IThemeData {
public:
  virtual const zeus::CColor& uiText() const = 0;
  virtual const zeus::CColor& uiAltText() const = 0;
  virtual const zeus::CColor& fieldText() const = 0;
  virtual const zeus::CColor& fieldMarkedText() const = 0;
  virtual const zeus::CColor& selectedFieldText() const = 0;

  virtual const zeus::CColor& viewportBackground() const = 0;
  virtual const zeus::CColor& toolbarBackground() const = 0;
  virtual const zeus::CColor& tooltipBackground() const = 0;
  virtual const zeus::CColor& spaceBackground() const = 0;
  virtual const zeus::CColor& splashBackground() const = 0;
  virtual const zeus::CColor& splashErrorBackground() const = 0;

  virtual const zeus::CColor& splash1() const = 0;
  virtual const zeus::CColor& splash2() const = 0;

  virtual const zeus::CColor& button1Inactive() const = 0;
  virtual const zeus::CColor& button2Inactive() const = 0;
  virtual const zeus::CColor& button1Hover() const = 0;
  virtual const zeus::CColor& button2Hover() const = 0;
  virtual const zeus::CColor& button1Press() const = 0;
  virtual const zeus::CColor& button2Press() const = 0;
  virtual const zeus::CColor& button1Disabled() const = 0;
  virtual const zeus::CColor& button2Disabled() const = 0;

  virtual const zeus::CColor& textfield1Inactive() const = 0;
  virtual const zeus::CColor& textfield2Inactive() const = 0;
  virtual const zeus::CColor& textfield1Hover() const = 0;
  virtual const zeus::CColor& textfield2Hover() const = 0;
  virtual const zeus::CColor& textfield1Disabled() const = 0;
  virtual const zeus::CColor& textfield2Disabled() const = 0;
  virtual const zeus::CColor& textfieldSelection() const = 0;
  virtual const zeus::CColor& textfieldMarkSelection() const = 0;

  virtual const zeus::CColor& tableCellBg1() const = 0;
  virtual const zeus::CColor& tableCellBg2() const = 0;
  virtual const zeus::CColor& tableCellBgSelected() const = 0;

  virtual const zeus::CColor& scrollIndicator() const = 0;

  virtual const zeus::CColor& spaceTriangleShading1() const = 0;
  virtual const zeus::CColor& spaceTriangleShading2() const = 0;
};

class DefaultThemeData : public IThemeData {
  zeus::CColor m_uiText = zeus::skWhite;
  zeus::CColor m_uiAltText = zeus::skGrey;
  zeus::CColor m_fieldText = zeus::skBlack;
  zeus::CColor m_fieldMarkedText = {0.25, 0.25, 0.25, 1.0};
  zeus::CColor m_selectedFieldText = zeus::skWhite;

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
  const zeus::CColor& uiText() const override { return m_uiText; }
  const zeus::CColor& uiAltText() const override { return m_uiAltText; }
  const zeus::CColor& fieldText() const override { return m_fieldText; }
  const zeus::CColor& fieldMarkedText() const override { return m_fieldMarkedText; }
  const zeus::CColor& selectedFieldText() const override { return m_selectedFieldText; }

  const zeus::CColor& viewportBackground() const override { return m_vpBg; }
  const zeus::CColor& toolbarBackground() const override { return m_tbBg; }
  const zeus::CColor& tooltipBackground() const override { return m_tooltipBg; }
  const zeus::CColor& spaceBackground() const override { return m_spaceBg; }
  const zeus::CColor& splashBackground() const override { return m_splashBg; }
  const zeus::CColor& splashErrorBackground() const override { return m_splashErrorBg; }

  const zeus::CColor& splash1() const override { return m_splash1; }
  const zeus::CColor& splash2() const override { return m_splash2; }

  const zeus::CColor& button1Inactive() const override { return m_button1Inactive; }
  const zeus::CColor& button2Inactive() const override { return m_button2Inactive; }
  const zeus::CColor& button1Hover() const override { return m_button1Hover; }
  const zeus::CColor& button2Hover() const override { return m_button2Hover; }
  const zeus::CColor& button1Press() const override { return m_button1Press; }
  const zeus::CColor& button2Press() const override { return m_button2Press; }
  const zeus::CColor& button1Disabled() const override { return m_button1Disabled; }
  const zeus::CColor& button2Disabled() const override { return m_button2Disabled; }

  const zeus::CColor& textfield1Inactive() const override { return m_textfield1Inactive; }
  const zeus::CColor& textfield2Inactive() const override { return m_textfield2Inactive; }
  const zeus::CColor& textfield1Hover() const override { return m_textfield1Hover; }
  const zeus::CColor& textfield2Hover() const override { return m_textfield2Hover; }
  const zeus::CColor& textfield1Disabled() const override { return m_textfield1Disabled; }
  const zeus::CColor& textfield2Disabled() const override { return m_textfield2Disabled; }
  const zeus::CColor& textfieldSelection() const override { return m_textfieldSelection; }
  const zeus::CColor& textfieldMarkSelection() const override { return m_textfieldMarkSelection; }

  const zeus::CColor& tableCellBg1() const override { return m_tableCellBg1; }
  const zeus::CColor& tableCellBg2() const override { return m_tableCellBg2; }
  const zeus::CColor& tableCellBgSelected() const override { return m_tableCellBgSelected; }

  const zeus::CColor& scrollIndicator() const override { return m_scrollIndicator; }

  const zeus::CColor& spaceTriangleShading1() const override { return m_spaceTriangleShading1; }
  const zeus::CColor& spaceTriangleShading2() const override { return m_spaceTriangleShading2; }
};

class ViewResources {
  void init(boo::IGraphicsDataFactory::Context& factory, const IThemeData& theme, FontCache* fcache) {
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

  specter::FontTag m_mainFont;
  specter::FontTag m_monoFont10;
  specter::FontTag m_monoFont18;

  specter::FontTag m_heading14;
  specter::FontTag m_heading18;

  specter::FontTag m_titleFont;
  specter::FontTag m_curveFont;

  std::thread m_fcacheThread;
  std::atomic_bool m_fcacheInterrupt = {false};
  std::atomic_bool m_fcacheReady = {false};

  ViewResources() = default;
  ViewResources(const ViewResources& other) = delete;
  ViewResources& operator=(const ViewResources& other) = delete;

  void updateBuffers() {
    m_viewRes.updateBuffers();
    m_textRes.updateBuffers();
  }

  ~ViewResources() {
    if (m_fcacheThread.joinable()) {
      m_fcacheInterrupt.store(true);
      m_fcacheThread.join();
    }
  }

  void init(boo::IGraphicsDataFactory* factory, FontCache* fcache, const IThemeData* theme, float pixelFactor);
  void destroyResData();
  void prepFontCacheSync();
  void prepFontCacheAsync(boo::IWindow* window);
  bool fontCacheReady() const { return m_fcacheReady.load(); }
  void resetPixelFactor(float pixelFactor);
  void resetTheme(const IThemeData* theme);

  float m_pixelFactor = 0;
  float pixelFactor() const { return m_pixelFactor; }

  const IThemeData* m_theme;
  const IThemeData& themeData() const { return *m_theme; }
};
} // namespace specter
