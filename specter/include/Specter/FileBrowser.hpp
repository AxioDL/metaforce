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
#include "MessageWindow.hpp"
#include <HECL/HECL.hpp>

namespace Specter
{

class FileBrowser : public ModalWindow
{
public:
    enum class Type
    {
        SaveFile,
        SaveDirectory,
        OpenFile,
        OpenDirectory,
        OpenHECLProject
    };
private:
    Type m_type;
    HECL::SystemString m_path;
    std::vector<HECL::SystemString> m_comps;
    bool m_showingHidden = false;

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

    void okActivated(bool viaButton);
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
        void activated(const boo::SWindowCoord&) {m_fb.okActivated(true);}
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

    std::unique_ptr<MessageWindow> m_confirmWindow;

    struct FileListingDataBind : ITableDataBinding, ITableStateBinding
    {
        FileBrowser& m_fb;

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
        std::string m_projStr;
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

        float m_columnSplits[3] = {0.0f, 0.7f, 0.9f};

        bool columnSplitResizeAllowed() const {return true;}

        float getColumnSplit(size_t cIdx) const
        {
            return m_columnSplits[cIdx];
        }

        void setColumnSplit(size_t cIdx, float split)
        {
            m_columnSplits[cIdx] = split;
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
                {
                    if (HECL::SearchForProject(d.m_path))
                        ent.m_type = m_projStr;
                    else
                        ent.m_type = m_dirStr;
                }
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
            if (rIdx != -1)
                m_fb.m_fileField.m_view->setText(m_entries.at(rIdx).m_name);
            else
                m_fb.m_fileField.m_view->setText("");
            m_fb.m_fileField.m_view->clearErrorState();
        }

        void rowActivated(size_t rIdx)
        {
            m_fb.okActivated(false);
        }

        FileListingDataBind(FileBrowser& fb, const IViewManager& vm)
        : m_fb(fb)
        {
            m_nameCol = vm.translateOr("name", "Name");
            m_typeCol = vm.translateOr("type", "Type");
            m_sizeCol = vm.translateOr("size", "Size");
            m_dirStr = vm.translateOr("directory", "Directory");
            m_projStr = vm.translateOr("hecl_project", "HECL Project");
            m_fileStr = vm.translateOr("file", "File");
        }

    } m_fileListingBind;
    ViewChild<std::unique_ptr<Table>> m_fileListing;

    struct BookmarkDataBind : ITableDataBinding, ITableStateBinding
    {
        FileBrowser& m_fb;
        BookmarkDataBind(FileBrowser& fb) : m_fb(fb) {}

        struct Entry
        {
            HECL::SystemString m_path;
            std::string m_name;

            Entry(std::pair<HECL::SystemString, std::string>&& path)
            : m_path(std::move(path.first)), m_name(std::move(path.second)) {}

            Entry(const HECL::SystemString& path)
            : m_path(path)
            {
                HECL::SystemUTF8View utf8(path);
                if (utf8.str().size() == 1 && utf8.str()[0] == '/')
                {
                    m_name = "/";
                    return;
                }
                size_t lastSlash = utf8.str().rfind('/');
                if (lastSlash != std::string::npos)
                    m_name.assign(utf8.str().cbegin() + lastSlash + 1, utf8.str().cend());
                else
                    m_name = utf8.str();
            }
        };
        std::vector<Entry> m_entries;

        size_t columnCount() const {return 1;}
        size_t rowCount() const {return m_entries.size();}

        const std::string* cell(size_t, size_t rIdx) const
        {
            return &m_entries.at(rIdx).m_name;
        }

        void setSelectedRow(size_t rIdx)
        {
            if (rIdx != -1)
                m_fb.navigateToPath(m_entries.at(rIdx).m_path);
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

    std::function<void(bool, const HECL::SystemString&)> m_returnFunc;

public:
    FileBrowser(ViewResources& res, View& parentView, const std::string& title, Type type,
                std::function<void(bool, const HECL::SystemString&)> returnFunc)
    : FileBrowser(res, parentView, title, type, HECL::GetcwdStr(), returnFunc) {}
    FileBrowser(ViewResources& res, View& parentView, const std::string& title, Type type,
                const HECL::SystemString& initialPath,
                std::function<void(bool, const HECL::SystemString&)> returnFunc);

    static void SyncBookmarkSelections(Table& table, BookmarkDataBind& binding,
                                       const HECL::SystemString& sel);
    void navigateToPath(const HECL::SystemString& path);
    bool showingHidden() const {return m_showingHidden;}
    void setShowingHidden(bool showingHidden)
    {
        m_showingHidden = showingHidden;
        navigateToPath(m_path);
    }
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
    void charKeyDown(unsigned long, boo::EModifierKey, bool);
    void specialKeyDown(boo::ESpecialKey, boo::EModifierKey, bool);

    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void think();
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_FILEBROWSER_HPP
