#ifndef SPECTER_MODAL_WINDOW_HPP
#define SPECTER_MODAL_WINDOW_HPP

#include <Specter/View.hpp>
#include <Specter/MultiLineTextView.hpp>

namespace Specter
{
class ModalWindow : public View
{
    unsigned m_frame = 0;
    unsigned m_contentStartFrame = 0;
    float m_lineTime = 0.0;

    enum class Phase
    {
        BuildIn,
        ResWait,
        Showing,
        BuildOut,
        Done
    } m_phase = Phase::BuildIn;

    int m_width = 0;
    int m_height = 0;
    int m_widthConstrain;
    int m_heightConstrain;

    Zeus::CColor m_windowBg;
    Zeus::CColor m_windowBgClear;
    Zeus::CColor m_line1;
    Zeus::CColor m_line2;

    ViewBlock m_viewBlock;
    boo::IGraphicsBufferD* m_viewBlockBuf;
    struct
    {
        SolidShaderVert lineVerts[22];
        SolidShaderVert fillVerts[16];
    } m_verts;

    void setLineVerts(int width, int height, float pf, float t);
    void setLineColors(float t);
    void setFillVerts(int width, int height, float pf);
    void setFillColors(float t);

    boo::IGraphicsBufferD* m_vertsBuf;
    boo::IVertexFormat* m_vertsVtxFmt; /* OpenGL only */
    boo::IShaderDataBinding* m_vertsShaderBinding;

    boo::GraphicsDataToken m_windowGfxData;

    std::unique_ptr<TextView> m_cornersOutline[4];
    std::unique_ptr<TextView> m_cornersFilled[4];

protected:
    virtual void updateContentOpacity(float opacity) {}

public:
    ModalWindow(ViewResources& res, View& parentView, int widthConstrain=-1, int heightConstrain=-1);
    void think();
    bool skipBuildInAnimation();

    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}


#endif // SPECTER_MODAL_WINDOW_HPP
