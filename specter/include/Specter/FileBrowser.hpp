#ifndef SPECTER_FILEBROWSER_HPP
#define SPECTER_FILEBROWSER_HPP

#include "View.hpp"
#include "ModalWindow.hpp"
#include "Button.hpp"
#include "TextField.hpp"
#include "ScrollView.hpp"
#include "Table.hpp"
#include <HECL/HECL.hpp>

namespace Specter
{

class FileBrowser : public ModalWindow
{
    std::vector<HECL::SystemString> m_comps;

    void pathButtonActivated(size_t idx);
    struct PathButton : Specter::IButtonBinding
    {
        FileBrowser& m_fb;
        size_t m_idx;
        Specter::ViewChild<Specter::Button> m_button;
        PathButton(FileBrowser& fb, ViewResources& res, size_t idx, const HECL::SystemString& str)
        : m_fb(fb), m_idx(idx)
        {
            HECL::SystemUTF8View utf8View(str);
            m_button.m_view.reset(new Specter::Button(res, fb, this, utf8View));
        }
        const char* name() const {return m_button.m_view->getText().c_str();}
        void activated(const boo::SWindowCoord&) {m_fb.pathButtonActivated(m_idx);}
    };
    friend struct PathButton;
    std::vector<PathButton> m_pathButtons;

    Specter::ViewChild<Specter::TextField> m_fileField;
    struct FileFieldBind : Specter::IStringBinding
    {
        FileBrowser& m_browser;
        FileFieldBind(FileBrowser& browser) : m_browser(browser) {}
        const char* name() const {return "File Name";}
        void changed(const std::string& val)
        {
        }
    } m_fileFieldBind;

    Specter::ViewChild<Specter::ScrollView> m_fileScroll;
    Specter::ViewChild<Specter::Table> m_fileListing;

public:
    FileBrowser(ViewResources& res, View& parentView)
    : FileBrowser(res, parentView, HECL::GetcwdStr()) {}
    FileBrowser(ViewResources& res, View& parentView, const HECL::SystemString& initialPath);

    void updateContentOpacity(float opacity);

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_FILEBROWSER_HPP
