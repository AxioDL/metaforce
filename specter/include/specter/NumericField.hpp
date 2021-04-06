#pragma once

#include <memory>
#include <string>

#include "specter/View.hpp"

namespace boo {
struct IGraphicsBufferD;
struct IGraphicsDataFactory;
struct IShaderDataBinding;
} // namespace boo

namespace specter {
class TextView;
class ViewResources;

class NumericField : public View {
  std::string m_textStr;
  std::unique_ptr<TextView> m_text;
  SolidShaderVert m_verts[28];

  ViewBlock m_bBlock;
  boo::IGraphicsBufferD* m_bBlockBuf;

  boo::IGraphicsBufferD* m_bVertsBuf;
  boo::IShaderDataBinding* m_bShaderBinding;

  int m_nomWidth, m_nomHeight;
  bool m_pressed = false;
  bool m_hovered = false;

  void setInactive();
  void setHover();
  void setPressed();
  void setDisabled();

public:
  class Resources {
    friend class Button;
    friend class ViewResources;

    void init(boo::IGraphicsDataFactory* factory, const IThemeData& theme);
  };

  NumericField(ViewResources& res, View& parentView, std::string_view text);
  void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseEnter(const boo::SWindowCoord&) override;
  void mouseLeave(const boo::SWindowCoord&) override;
  void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub) override;
  void draw(boo::IGraphicsCommandQueue* gfxQ) override;

  void setText(std::string_view text);
  int nominalWidth() const override { return m_nomWidth; }
  int nominalHeight() const override { return m_nomHeight; }
};

} // namespace specter
