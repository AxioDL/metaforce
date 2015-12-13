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

    std::unique_ptr<Specter::TextView> m_title;
    std::unique_ptr<Specter::MultiLineTextView> m_buildInfo;

    struct Child
    {
        std::unique_ptr<Specter::Button> m_button;
        bool m_mouseIn = false;
        bool m_mouseDown = false;

        void mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
        {
            if (!m_button)
                return;
            if (m_button->subRect().coordInRect(coord))
            {
                if (!m_mouseDown)
                {
                    m_button->mouseDown(coord, button, mod);
                    m_mouseDown = true;
                }
            }
        }

        void mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
        {
            if (!m_button)
                return;
            if (m_mouseDown)
            {
                m_button->mouseUp(coord, button, mod);
                m_mouseDown = false;
            }
        }

        void mouseMove(const boo::SWindowCoord& coord)
        {
            if (!m_button)
                return;
            if (m_button->subRect().coordInRect(coord))
            {
                if (!m_mouseIn)
                {
                    m_button->mouseEnter(coord);
                    m_mouseIn = true;
                }
                m_button->mouseMove(coord);
            }
            else
            {
                if (m_mouseIn)
                {
                    m_button->mouseLeave(coord);
                    m_mouseIn = false;
                }
            }
        }

        void mouseEnter(const boo::SWindowCoord& coord)
        {
            if (!m_button)
                return;
        }

        void mouseLeave(const boo::SWindowCoord& coord)
        {
            if (!m_button)
                return;
            if (m_mouseIn)
            {
                m_button->mouseLeave(coord);
                m_mouseIn = false;
            }
        }
    };
    Child m_newButt;
    Child m_openButt;
    Child m_extractButt;

    std::unique_ptr<Specter::TextView> m_cornersOutline[4];
    std::unique_ptr<Specter::TextView> m_cornersFilled[4];

    struct NewProjBinding : Specter::IButtonBinding
    {
        const char* name() const {return "New Project";}
        const char* help() const {return "Creates an empty project at selected path";}
        void pressed(const boo::SWindowCoord& coord)
        {
        }
    } m_newProjBind;

    struct OpenProjBinding : Specter::IButtonBinding
    {
        const char* name() const {return "Open Project";}
        const char* help() const {return "Opens an existing project at selected path";}
        void pressed(const boo::SWindowCoord& coord)
        {
        }
    } m_openProjBind;

    struct ExtractProjBinding : Specter::IButtonBinding
    {
        const char* name() const {return "Extract Game";}
        const char* help() const {return "Extracts game image as project at selected path";}
        void pressed(const boo::SWindowCoord& coord)
        {
        }
    } m_extractProjBind;

public:
    SplashScreen(ViewManager& vm, Specter::ViewResources& res);
    void think();

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}


#endif // RUDE_SPLASH_SCREEN_HPP
