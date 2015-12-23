#ifndef SPECTER_FILEBROWSER_HPP
#define SPECTER_FILEBROWSER_HPP

#include "View.hpp"
#include "ModalWindow.hpp"
#include "Button.hpp"
#include "TextField.hpp"
#include "ScrollView.hpp"
#include "Table.hpp"
#include "ViewResources.hpp"
#include <HECL/HECL.hpp>

namespace Specter
{

class FileBrowser : public ModalWindow
{
    std::vector<HECL::SystemString> m_comps;

    class LeftSide : public View
    {
        friend class FileBrowser;
        FileBrowser& m_fb;
        LeftSide(FileBrowser& fb, ViewResources& res) : View(res, fb), m_fb(fb) {}
        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
        void draw(boo::IGraphicsCommandQueue* gfxQ);
    } m_left;

    class RightSide : public View
    {
        friend class FileBrowser;
        FileBrowser& m_fb;
        RightSide(FileBrowser& fb, ViewResources& res) : View(res, fb), m_fb(fb) {}
        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
        void draw(boo::IGraphicsCommandQueue* gfxQ);
    } m_right;

    ViewChild<std::unique_ptr<SplitView>> m_split;

    void okActivated();
    struct OKButton : IButtonBinding
    {
        FileBrowser& m_fb;
        ViewChild<std::unique_ptr<Button>> m_button;
        std::string m_text;
        OKButton(FileBrowser& fb, ViewResources& res, const std::string& text)
        : m_fb(fb), m_text(text)
        {
            m_button.m_view.reset(new Button(res, fb, this, text, Button::Style::Block,
                RectangleConstraint(100 * res.pixelFactor(), -1, RectangleConstraint::Test::Minimum)));
        }
        const char* name() const {return m_text.c_str();}
        void activated(const boo::SWindowCoord&) {m_fb.okActivated();}
    } m_ok;

    void cancelActivated();
    struct CancelButton : IButtonBinding
    {
        FileBrowser& m_fb;
        ViewChild<std::unique_ptr<Button>> m_button;
        CancelButton(FileBrowser& fb, ViewResources& res)
        : m_fb(fb)
        {
            m_button.m_view.reset(new Button(res, fb, this, "Cancel", Button::Style::Block,
                RectangleConstraint(100 * res.pixelFactor(), -1, RectangleConstraint::Test::Minimum)));
        }
        const char* name() const {return "Cancel";}
        void activated(const boo::SWindowCoord&) {m_fb.cancelActivated();}
    } m_cancel;

    void pathButtonActivated(size_t idx);
    struct PathButton : IButtonBinding
    {
        FileBrowser& m_fb;
        size_t m_idx;
        ViewChild<std::unique_ptr<Button>> m_button;
        PathButton(FileBrowser& fb, ViewResources& res, size_t idx, const HECL::SystemString& str)
        : m_fb(fb), m_idx(idx)
        {
            HECL::SystemUTF8View utf8View(str);
            m_button.m_view.reset(new Button(res, fb, this, utf8View));
        }
        const char* name() const {return m_button.m_view->getText().c_str();}
        void activated(const boo::SWindowCoord&) {m_fb.pathButtonActivated(m_idx);}
    };
    friend struct PathButton;
    std::vector<PathButton> m_pathButtons;

    ViewChild<std::unique_ptr<TextField>> m_fileField;
    struct FileFieldBind : IStringBinding
    {
        FileBrowser& m_browser;
        FileFieldBind(FileBrowser& browser) : m_browser(browser) {}
        const char* name() const {return "File Name";}
        void changed(const std::string& val)
        {
        }
    } m_fileFieldBind;

    struct TableDataBind : ITableDataBinding
    {
        std::string m_nameCol = "Name";
        std::vector<std::string> m_names = {"One", "Two", "Three"};

        std::string m_typeCol = "Type";
        std::vector<std::string> m_types = {"t1", "t2", "t3"};

        std::string m_sizeCol = "Size";
        std::vector<std::string> m_sizes = {"s1", "s2", "s3"};

        size_t columnCount() const {return 3;}
        size_t rowCount() const {return 3;}

        const std::string* header(size_t cIdx) const
        {
            switch (cIdx)
            {
            case 0:
                return &m_nameCol;
            case 1:
                return &m_typeCol;
            case 2:
                return &m_sizeCol;
            default: break;
            }
            return nullptr;
        }

        const std::string* cell(size_t cIdx, size_t rIdx) const
        {
            switch (cIdx)
            {
            case 0:
                return &m_names.at(rIdx);
            case 1:
                return &m_types.at(rIdx);
            case 2:
                return &m_sizes.at(rIdx);
            default: break;
            }
            return nullptr;
        }
    } m_fileListingBind;
    ViewChild<std::unique_ptr<Table>> m_fileListing;

public:
    FileBrowser(ViewResources& res, View& parentView, const std::string& title)
    : FileBrowser(res, parentView, title, HECL::GetcwdStr()) {}
    FileBrowser(ViewResources& res, View& parentView, const std::string& title, const HECL::SystemString& initialPath);

    void updateContentOpacity(float opacity);

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void scroll(const boo::SWindowCoord&, const boo::SScrollDelta&);
    void touchDown(const boo::STouchCoord&, uintptr_t);
    void touchUp(const boo::STouchCoord&, uintptr_t);
    void touchMove(const boo::STouchCoord&, uintptr_t);

    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void think();
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_FILEBROWSER_HPP
