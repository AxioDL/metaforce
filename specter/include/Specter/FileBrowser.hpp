#ifndef SPECTER_FILEBROWSER_HPP
#define SPECTER_FILEBROWSER_HPP

#include "View.hpp"
#include "ModalWindow.hpp"
#include "Button.hpp"
#include "TextField.hpp"
#include "ScrollView.hpp"
#include "Table.hpp"
#include "ViewResources.hpp"
#include "IViewManager.hpp"
#include <HECL/HECL.hpp>

namespace Specter
{

class FileBrowser : public ModalWindow
{
public:
    enum class Type
    {
        SaveFile,
        OpenFile,
        OpenDirectory,
        OpenHECLProject
    };
private:
    Type m_type;
    HECL::SystemString m_path;
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
        std::string m_text;
        CancelButton(FileBrowser& fb, ViewResources& res, const std::string& text)
        : m_fb(fb), m_text(text)
        {
            m_button.m_view.reset(new Button(res, fb, this, text, Button::Style::Block,
                RectangleConstraint(m_fb.m_ok.m_button.m_view->nominalWidth(), -1, RectangleConstraint::Test::Minimum)));
        }
        const char* name() const {return m_text.c_str();}
        void activated(const boo::SWindowCoord&) {m_fb.cancelActivated();}
    } m_cancel;

    int m_pathButtonPending = -1;
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
        void activated(const boo::SWindowCoord&) {m_fb.m_pathButtonPending = m_idx;}
    };
    friend struct PathButton;
    std::vector<PathButton> m_pathButtons;

    ViewChild<std::unique_ptr<TextField>> m_fileField;
    struct FileFieldBind : IStringBinding
    {
        FileBrowser& m_browser;
        std::string m_name;
        FileFieldBind(FileBrowser& browser, const IViewManager& vm)
        : m_browser(browser), m_name(vm.translateOr("file_name", "File Name")) {}
        const char* name() const {return m_name.c_str();}
        void changed(const std::string& val)
        {
        }
    } m_fileFieldBind;

    struct FileListingDataBind : ITableDataBinding, ITableStateBinding
    {
        struct Entry
        {
            HECL::SystemString m_path;
            std::string m_name;
            std::string m_type;
            std::string m_size;
        };
        std::vector<Entry> m_entries;

        std::string m_nameCol;
        std::string m_typeCol;
        std::string m_sizeCol;

        std::string m_dirStr;
        std::string m_fileStr;

        size_t columnCount() const {return 3;}
        size_t rowCount() const {return m_entries.size();}

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
                return &m_entries.at(rIdx).m_name;
            case 1:
                return &m_entries.at(rIdx).m_type;
            case 2:
                return &m_entries.at(rIdx).m_size;
            default: break;
            }
            return nullptr;
        }

        void updateListing(const HECL::DirectoryEnumerator& dEnum)
        {
            m_entries.clear();
            m_entries.reserve(dEnum.size());

            for (const HECL::DirectoryEnumerator::Entry& d : dEnum)
            {
                m_entries.emplace_back();
                Entry& ent = m_entries.back();
                ent.m_path = d.m_path;
                HECL::SystemUTF8View nameUtf8(d.m_name);
                ent.m_name = nameUtf8.str();
                if (d.m_isDir)
                    ent.m_type = m_dirStr;
                else
                {
                    ent.m_type = m_fileStr;
                    ent.m_size = HECL::HumanizeNumber(d.m_fileSz, 7, nullptr, int(HECL::HNScale::AutoScale),
                                                      HECL::HNFlags::B | HECL::HNFlags::Decimal);
                }
            }

            m_needsUpdate = false;
        }

        bool m_sizeSort = false;
        SortDirection m_sortDir = SortDirection::Ascending;
        bool m_needsUpdate = false;

        SortDirection getSort(size_t& cIdx) const
        {
            cIdx = m_sizeSort ? 2 : 0;
            return m_sortDir;
        }

        void setSort(size_t cIdx, SortDirection dir)
        {
            if (cIdx == 1)
                return;
            m_sizeSort = cIdx == 2;
            m_sortDir = dir;
            m_needsUpdate = true;
        }

        void setSelectedRow(size_t rIdx)
        {

        }

        FileListingDataBind(const IViewManager& vm)
        {
            m_nameCol = vm.translateOr("name", "Name");
            m_typeCol = vm.translateOr("type", "Type");
            m_sizeCol = vm.translateOr("size", "Size");
            m_dirStr = vm.translateOr("directory", "Directory");
            m_fileStr = vm.translateOr("file", "File");
        }

    } m_fileListingBind;
    ViewChild<std::unique_ptr<Table>> m_fileListing;

    struct BookmarkDataBind : ITableDataBinding
    {
        struct Entry
        {
            HECL::SystemString m_path;
            std::string m_name;
        };
        std::vector<Entry> m_entries;

        size_t columnCount() const {return 1;}
        size_t rowCount() const {return m_entries.size();}

        const std::string* cell(size_t, size_t rIdx) const
        {
            return &m_entries.at(rIdx).m_name;
        }
    };

    BookmarkDataBind m_systemBookmarkBind;
    std::unique_ptr<TextView> m_systemBookmarksLabel;
    ViewChild<std::unique_ptr<Table>> m_systemBookmarks;

    BookmarkDataBind m_projectBookmarkBind;
    std::unique_ptr<TextView> m_projectBookmarksLabel;
    ViewChild<std::unique_ptr<Table>> m_projectBookmarks;

    BookmarkDataBind m_recentBookmarkBind;
    std::unique_ptr<TextView> m_recentBookmarksLabel;
    ViewChild<std::unique_ptr<Table>> m_recentBookmarks;

public:
    FileBrowser(ViewResources& res, View& parentView, const std::string& title, Type type)
    : FileBrowser(res, parentView, title, type, HECL::GetcwdStr()) {}
    FileBrowser(ViewResources& res, View& parentView, const std::string& title, Type type, const HECL::SystemString& initialPath);

    void navigateToPath(const HECL::SystemString& path);
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
