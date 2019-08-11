#pragma once

#include "specter/View.hpp"
#include "specter/MultiLineTextView.hpp"

namespace specter {

class Tooltip : public View {
  ViewBlock m_ttBlock;
  hecl::UniformBufferPool<ViewBlock>::Token m_ttBlockBuf;
  SolidShaderVert m_ttVerts[16];
  int m_nomWidth = 25;
  int m_nomHeight = 25;

  void setVerts(int width, int height, float pf);

  VertexBufferBindingSolid m_vertsBinding;

  std::string m_titleStr;
  std::unique_ptr<TextView> m_title;
  std::string m_messageStr;
  std::unique_ptr<MultiLineTextView> m_message;

  std::unique_ptr<TextView> m_cornersOutline[4];
  std::unique_ptr<TextView> m_cornersFilled[4];

public:
  Tooltip(ViewResources& res, View& parentView, std::string_view title, std::string_view message);
  void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub) override;
  void draw(boo::IGraphicsCommandQueue* gfxQ) override;

  int nominalWidth() const override { return m_nomWidth; }
  int nominalHeight() const override { return m_nomHeight; }
};

} // namespace specter
