#ifndef SPECTER_TOOLTIP_HPP
#define SPECTER_TOOLTIP_HPP

#include "Specter/View.hpp"
#include "Specter/MultiLineTextView.hpp"

namespace Specter
{

class Tooltip : public View
{
    ViewBlock m_ttBlock;
    boo::IGraphicsBufferD* m_ttBlockBuf;
    SolidShaderVert m_ttVerts[16];
    int m_nomWidth = 25;
    int m_nomHeight = 25;

    void setVerts(int width, int height, float pf);

    boo::IGraphicsBufferD* m_ttVertsBuf;
    boo::IVertexFormat* m_ttVtxFmt; /* OpenGL only */
    boo::IShaderDataBinding* m_ttShaderBinding;

    std::string m_titleStr;
    std::unique_ptr<TextView> m_title;
    std::string m_messageStr;
    std::unique_ptr<MultiLineTextView> m_message;

    std::unique_ptr<TextView> m_cornersOutline[4];
    std::unique_ptr<TextView> m_cornersFilled[4];
public:
    Tooltip(ViewResources& res, View& parentView, const std::string& title,
            const std::string& message);
    void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub);
    void resetResources(ViewResources& res);
    void draw(boo::IGraphicsCommandQueue* gfxQ);

    int nominalWidth() const {return m_nomWidth;}
    int nominalHeight() const {return m_nomHeight;}
};

}

#endif // SPECTER_TOOLTIP_HPP
