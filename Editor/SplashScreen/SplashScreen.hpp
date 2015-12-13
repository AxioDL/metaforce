#ifndef RUDE_SPLASH_SCREEN_HPP
#define RUDE_SPLASH_SCREEN_HPP

#include <Specter/View.hpp>
#include <Specter/MultiLineTextView.hpp>
#include "ViewManager.hpp"

namespace RUDE
{
class SplashScreen : public Specter::View
{
    ViewManager& m_vm;
    unsigned m_frame = 0;
    unsigned m_textStartFrame = 0;

    Zeus::CColor m_splashBg;
    Zeus::CColor m_splashBgClear;
    Zeus::CColor m_splash1;
    Zeus::CColor m_splash2;
    Zeus::CColor m_textColor;
    Zeus::CColor m_textColorClear;

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

    std::string m_titleStr;
    std::unique_ptr<Specter::TextView> m_title;
    std::string m_messageStr;
    std::unique_ptr<Specter::MultiLineTextView> m_message;

    std::unique_ptr<Specter::TextView> m_cornersOutline[4];
    std::unique_ptr<Specter::TextView> m_cornersFilled[4];
public:
    SplashScreen(ViewManager& vm, Specter::ViewResources& res);
    void think();

    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}


#endif // RUDE_SPLASH_SCREEN_HPP
