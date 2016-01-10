#include "Specter/FileBrowser.hpp"
#include "Specter/RootView.hpp"
#include "Specter/MessageWindow.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::FileBrowser");

#define BROWSER_MARGIN 8
#define BROWSER_MIN_WIDTH 600
#define BROWSER_MIN_HEIGHT 300

static std::vector<HECL::SystemString> PathComponents(const HECL::SystemString& path)
{
    std::vector<HECL::SystemString> ret;
    HECL::SystemString sPath = path;
    HECL::SanitizePath(sPath);
    if (sPath.empty())
        return ret;
    auto it = sPath.cbegin();
    if (*it == _S('/'))
    {
        ret.push_back(_S("/"));
        ++it;
    }
    HECL::SystemString comp;
    for (; it != sPath.cend() ; ++it)
    {
        if (*it == _S('/'))
        {
            if (comp.empty())
                continue;
            ret.push_back(std::move(comp));
            comp.clear();
            continue;
        }
        comp += *it;
    }
    if (comp.size())
        ret.push_back(std::move(comp));
    return ret;
}

FileBrowser::FileBrowser(ViewResources& res, View& parentView, const std::string& title,
                         Type type, const HECL::SystemString& initialPath,
                         std::function<void(bool, const HECL::SystemString&)> returnFunc)
: ModalWindow(res, parentView, RectangleConstraint(BROWSER_MIN_WIDTH * res.pixelFactor(),
                                                   BROWSER_MIN_HEIGHT * res.pixelFactor(),
                                                   RectangleConstraint::Test::Minimum,
                                                   RectangleConstraint::Test::Minimum)),
  m_type(type),
  m_left(*this, res),
  m_right(*this, res),
  m_ok(*this, res, title),
  m_cancel(*this, res, rootView().viewManager().translateOr("cancel", "Cancel")),
  m_fileFieldBind(*this, rootView().viewManager()),
  m_fileListingBind(*this, rootView().viewManager()),
  m_systemBookmarkBind(*this),
  m_projectBookmarkBind(*this),
  m_recentBookmarkBind(*this),
  m_returnFunc(returnFunc)
{
    commitResources(res);
    setBackground({0,0,0,0.5});

    IViewManager& vm = rootView().viewManager();
    m_pathButtons.m_view.reset(new PathButtons(res, *this, *this));
    m_fileField.m_view.reset(new TextField(res, *this, &m_fileFieldBind));
    m_fileListing.m_view.reset(new Table(res, *this, &m_fileListingBind, &m_fileListingBind, 3));
    m_systemBookmarks.m_view.reset(new Table(res, *this, &m_systemBookmarkBind, &m_systemBookmarkBind, 1));
    m_systemBookmarksLabel.reset(new TextView(res, *this, res.m_mainFont));
    m_systemBookmarksLabel->typesetGlyphs(vm.translateOr("system_locations", "System Locations"), res.themeData().uiText());
    m_projectBookmarks.m_view.reset(new Table(res, *this, &m_projectBookmarkBind, &m_projectBookmarkBind, 1));
    m_projectBookmarksLabel.reset(new TextView(res, *this, res.m_mainFont));
    m_projectBookmarksLabel->typesetGlyphs(vm.translateOr("recent_projects", "Recent Projects"), res.themeData().uiText());
    m_recentBookmarks.m_view.reset(new Table(res, *this, &m_recentBookmarkBind, &m_recentBookmarkBind, 1));
    m_recentBookmarksLabel.reset(new TextView(res, *this, res.m_mainFont));
    m_recentBookmarksLabel->typesetGlyphs(vm.translateOr("recent_files", "Recent Files"), res.themeData().uiText());

    /* Populate system bookmarks */
    std::vector<std::pair<HECL::SystemString, std::string>> systemLocs = HECL::GetSystemLocations();
    for (auto& loc : systemLocs)
        m_systemBookmarkBind.m_entries.emplace_back(std::move(loc));
    m_systemBookmarks.m_view->updateData();

    navigateToPath(initialPath);

    m_split.m_view.reset(new SplitView(res, *this, SplitView::Axis::Vertical,
                                       200 * res.pixelFactor(), 400 * res.pixelFactor()));
    m_split.m_view->setContentView(0, &m_left);
    m_split.m_view->setContentView(1, &m_right);
    m_split.m_view->setSplit(0.2);

    updateContentOpacity(0.0);
}

void FileBrowser::SyncBookmarkSelections(Table& table, BookmarkDataBind& binding,
                                         const HECL::SystemString& sel)
{
    size_t idx = 0;
    for (const BookmarkDataBind::Entry& e : binding.m_entries)
    {
        if (e.m_path == sel)
        {
            table.selectRow(idx);
            break;
        }
        ++idx;
    }
    if (idx == binding.m_entries.size())
        table.selectRow(-1);
}

void FileBrowser::navigateToPath(const HECL::SystemString& path)
{
    HECL::Sstat theStat;
    if (HECL::Stat(path.c_str(), &theStat))
        return;

    m_path = path;
    m_comps = PathComponents(m_path);
    if (S_ISREG(theStat.st_mode))
    {
        HECL::SystemUTF8View utf8(m_comps.back());
        m_fileField.m_view->setText(utf8);
        m_fileField.m_view->clearErrorState();
        m_comps.pop_back();
    }

    HECL::SystemString dir;
    bool needSlash = false;
    for (const HECL::SystemString& d : m_comps)
    {
        if (needSlash)
            dir += _S('/');
        if (d.compare(_S("/")))
            needSlash = true;
        dir += d;
    }

    SyncBookmarkSelections(*m_systemBookmarks.m_view, m_systemBookmarkBind, dir);
    SyncBookmarkSelections(*m_projectBookmarks.m_view, m_projectBookmarkBind, dir);
    SyncBookmarkSelections(*m_recentBookmarks.m_view, m_recentBookmarkBind, dir);

    HECL::DirectoryEnumerator dEnum(dir, HECL::DirectoryEnumerator::Mode::DirsThenFilesSorted,
                                    m_fileListingBind.m_sizeSort,
                                    m_fileListingBind.m_sortDir==SortDirection::Descending,
                                    !m_showingHidden);
    m_fileListingBind.updateListing(dEnum);
    m_fileListing.m_view->selectRow(-1);
    m_fileListing.m_view->updateData();

    m_pathButtons.m_view->setButtons(m_comps);

    updateSize();
}

void FileBrowser::updateContentOpacity(float opacity)
{
    Zeus::CColor color = Zeus::CColor::lerp({1,1,1,0}, {1,1,1,1}, opacity);
    m_split.m_view->setMultiplyColor(color);
    m_pathButtons.m_view->setMultiplyColor(color);
    m_fileField.m_view->setMultiplyColor(color);
    m_fileListing.m_view->setMultiplyColor(color);
    m_ok.m_button.m_view->setMultiplyColor(color);
    m_cancel.m_button.m_view->setMultiplyColor(color);
    m_systemBookmarks.m_view->setMultiplyColor(color);
    m_systemBookmarksLabel->setMultiplyColor(color);
    m_projectBookmarks.m_view->setMultiplyColor(color);
    m_projectBookmarksLabel->setMultiplyColor(color);
    m_recentBookmarks.m_view->setMultiplyColor(color);
    m_recentBookmarksLabel->setMultiplyColor(color);
}

void FileBrowser::okActivated(bool viaButton)
{
    IViewManager& vm = rootView().viewManager();

    HECL::SystemString path;
    bool needSlash = false;
    for (const HECL::SystemString& d : m_comps)
    {
        if (needSlash)
            path += _S('/');
        if (d.compare(_S("/")))
            needSlash = true;
        path += d;
    }

    HECL::Sstat theStat;
    if (HECL::Stat(path.c_str(), &theStat) || !S_ISDIR(theStat.st_mode))
    {
        HECL::SystemUTF8View utf8(path);
        m_fileField.m_view->setErrorState(
            HECL::Format(vm.translateOr("no_access_as_dir", "Unable to access '%s' as directory").c_str(),
                         utf8.c_str()));
        return;
    }

    path += _S('/');
    path += HECL::SystemStringView(m_fileField.m_view->getText()).sys_str();

    int err = HECL::Stat(path.c_str(), &theStat);
    if (m_type == Type::SaveFile)
    {
        if (m_fileField.m_view->getText().empty())
        {
            m_fileField.m_view->setErrorState(
                vm.translateOr("file_field_empty", "Unable to save empty file").c_str());
            return;
        }
        if (!err && !S_ISDIR(theStat.st_mode))
        {
            m_confirmWindow.reset(new MessageWindow(rootView().viewRes(), *this,
                                                    MessageWindow::Type::ConfirmOkCancel,
                                                    HECL::Format(vm.translateOr("overwrite_confirm", "Overwrite '%s'?").c_str(), path.c_str()),
                                                    [&,path](bool ok)
            {
                if (ok)
                {
                    m_returnFunc(true, path);
                    m_confirmWindow->close();
                    close();
                }
                else
                    m_confirmWindow->close();
            }));
            updateSize();
            return;
        }
        if (!err && S_ISDIR(theStat.st_mode))
        {
            navigateToPath(path);
            return;
        }
        m_returnFunc(true, path);
        close();
        return;
    }
    else if (m_type == Type::SaveDirectory || m_type == Type::NewHECLProject)
    {
        if (m_fileField.m_view->getText().empty())
        {
            m_fileField.m_view->setErrorState(
                vm.translateOr("directory_field_empty", "Unable to make empty-named directory").c_str());
            return;
        }
        if (!err && !S_ISDIR(theStat.st_mode))
        {
            m_fileField.m_view->setErrorState(
                vm.translateOr("no_overwrite_file", "Unable to make directory over file").c_str());
            return;
        }
        if (!err && S_ISDIR(theStat.st_mode))
        {
            if (m_type == Type::NewHECLProject)
            {
                HECL::ProjectRootPath projRoot = HECL::SearchForProject(path);
                if (projRoot)
                {
                    m_fileField.m_view->setErrorState(
                        vm.translateOr("no_overwrite_project", "Unable to make project within existing project").c_str());
                    return;
                }
            }
            navigateToPath(path);
            return;
        }
        m_returnFunc(true, path);
        close();
        return;
    }

    if (m_type == Type::OpenFile)
    {
        if (!err && S_ISDIR(theStat.st_mode))
        {
            navigateToPath(path);
            return;
        }
        else if (err || !S_ISREG(theStat.st_mode))
        {
            HECL::SystemUTF8View utf8(path);
            m_fileField.m_view->setErrorState(
                HECL::Format(vm.translateOr("no_access_as_file", "Unable to access '%s' as file").c_str(),
                             utf8.c_str()));
            return;
        }
        m_returnFunc(true, path);
        close();
        return;
    }
    else if (m_type == Type::OpenDirectory || m_type == Type::OpenHECLProject)
    {
        if (!viaButton && !err && S_ISDIR(theStat.st_mode))
        {
            if (m_type == Type::OpenHECLProject)
            {
                HECL::ProjectRootPath projRoot = HECL::SearchForProject(path);
                if (projRoot)
                {
                    m_returnFunc(true, projRoot.getAbsolutePath());
                    close();
                    return;
                }
            }
            navigateToPath(path);
            return;
        }
        if (err || !S_ISDIR(theStat.st_mode))
        {
            HECL::SystemUTF8View utf8(path);
            m_fileField.m_view->setErrorState(
                HECL::Format(vm.translateOr("no_access_as_dir", "Unable to access '%s' as directory").c_str(),
                             utf8.c_str()));
            return;
        }
        m_returnFunc(true, path);
        close();
        return;
    }
}

void FileBrowser::cancelActivated()
{
    HECL::SystemString path;
    bool needSlash = false;
    for (const HECL::SystemString& d : m_comps)
    {
        if (needSlash)
            path += _S('/');
        if (d.compare(_S("/")))
            needSlash = true;
        path += d;
    }

    path += _S('/');
    path += HECL::SystemStringView(m_fileField.m_view->getText()).sys_str();

    m_returnFunc(false, path);
    close();
}

void FileBrowser::pathButtonActivated(size_t idx)
{
    if (idx >= m_comps.size())
        return;

    HECL::SystemString dir;
    bool needSlash = false;
    size_t i = 0;
    for (const HECL::SystemString& d : m_comps)
    {
        if (needSlash)
            dir += _S('/');
        if (d.compare(_S("/")))
            needSlash = true;
        dir += d;
        if (++i > idx)
            break;
    }
    navigateToPath(dir);
}

void FileBrowser::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (skipBuildInAnimation() || closed())
        return;

    if (m_confirmWindow)
        m_confirmWindow->mouseDown(coord, button, mod);

    m_split.mouseDown(coord, button, mod);
    m_pathButtons.mouseDown(coord, button, mod);
    m_fileField.mouseDown(coord, button, mod);
    m_fileListing.mouseDown(coord, button, mod);
    m_systemBookmarks.m_view->mouseDown(coord, button, mod);
    m_projectBookmarks.m_view->mouseDown(coord, button, mod);
    m_recentBookmarks.m_view->mouseDown(coord, button, mod);
    m_ok.m_button.mouseDown(coord, button, mod);
    m_cancel.m_button.mouseDown(coord, button, mod);
}

void FileBrowser::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (closed())
        return;

    m_split.mouseUp(coord, button, mod);

    m_pathButtons.mouseUp(coord, button, mod);

    m_fileField.mouseUp(coord, button, mod);
    m_fileListing.mouseUp(coord, button, mod);
    m_systemBookmarks.m_view->mouseUp(coord, button, mod);
    m_projectBookmarks.m_view->mouseUp(coord, button, mod);
    m_recentBookmarks.m_view->mouseUp(coord, button, mod);
    m_ok.m_button.mouseUp(coord, button, mod);
    m_cancel.m_button.mouseUp(coord, button, mod);

    if (m_confirmWindow)
        m_confirmWindow->mouseUp(coord, button, mod);
}

void FileBrowser::mouseMove(const boo::SWindowCoord& coord)
{
    if (closed())
        return;
    m_split.mouseMove(coord);
    m_pathButtons.mouseMove(coord);
    m_fileField.mouseMove(coord);
    m_fileListing.mouseMove(coord);
    m_systemBookmarks.m_view->mouseMove(coord);
    m_projectBookmarks.m_view->mouseMove(coord);
    m_recentBookmarks.m_view->mouseMove(coord);
    m_ok.m_button.mouseMove(coord);
    m_cancel.m_button.mouseMove(coord);

    if (m_confirmWindow)
        m_confirmWindow->mouseMove(coord);
}

void FileBrowser::mouseLeave(const boo::SWindowCoord& coord)
{
    if (closed())
        return;
    m_split.mouseLeave(coord);
    m_pathButtons.mouseLeave(coord);
    m_fileField.mouseLeave(coord);
    m_fileListing.mouseLeave(coord);
    m_ok.m_button.mouseLeave(coord);
    m_cancel.m_button.mouseLeave(coord);

    if (m_confirmWindow)
        m_confirmWindow->mouseLeave(coord);
}

void FileBrowser::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
{
    m_pathButtons.scroll(coord, scroll);
    m_fileListing.scroll(coord, scroll);
}

void FileBrowser::touchDown(const boo::STouchCoord& coord, uintptr_t tid)
{
}

void FileBrowser::touchUp(const boo::STouchCoord& coord, uintptr_t tid)
{
}

void FileBrowser::touchMove(const boo::STouchCoord& coord, uintptr_t tid)
{
}

void FileBrowser::charKeyDown(unsigned long charcode, boo::EModifierKey mod, bool isRepeat)
{
    if (skipBuildInAnimation() || closed())
        return;
    if ((mod & boo::EModifierKey::CtrlCommand) != boo::EModifierKey::None && !isRepeat)
    {
        if (charcode == 'h' || charcode == 'H')
            setShowingHidden(!showingHidden());
        else if (charcode == 'r' || charcode == 'R')
            navigateToPath(m_path);
    }
}

void FileBrowser::specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mod, bool isRepeat)
{
    if (skipBuildInAnimation() || closed())
        return;
    if (key == boo::ESpecialKey::Enter && !isRepeat)
        okActivated(true);
}

void FileBrowser::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    ModalWindow::resized(root, root);
    float pf = rootView().viewRes().pixelFactor();

    boo::SWindowRect centerRect = subRect();
    centerRect.location[0] = root.size[0] / 2 - (centerRect.size[0] / 2.0) + 2 * pf;
    centerRect.location[1] = root.size[1] / 2 - (centerRect.size[1] / 2.0) + 2 * pf;
    centerRect.size[0] -= 4 * pf;
    centerRect.size[1] -= 4 * pf;

    if (m_split.m_view)
        m_split.m_view->resized(root, centerRect);

    if (m_confirmWindow)
        m_confirmWindow->resized(root, sub);
}

void FileBrowser::LeftSide::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    float pf = rootView().viewRes().pixelFactor();

    int div = (sub.size[1] - BROWSER_MARGIN * pf) / 3;

    boo::SWindowRect bookmarkRect = sub;
    bookmarkRect.size[0] -= BROWSER_MARGIN * 2 * pf;
    bookmarkRect.size[1] = div;
    bookmarkRect.location[0] += BROWSER_MARGIN * pf;
    bookmarkRect.location[1] = sub.location[1] + BROWSER_MARGIN * pf + div * 2;

    boo::SWindowRect labelRect = bookmarkRect;
    labelRect.size[1] = 20;
    labelRect.location[1] += div - 16 * pf;

    m_fb.m_systemBookmarks.m_view->resized(root, bookmarkRect);
    m_fb.m_systemBookmarksLabel->resized(root, labelRect);
    bookmarkRect.location[1] -= div;
    labelRect.location[1] -= div;

    m_fb.m_projectBookmarks.m_view->resized(root, bookmarkRect);
    m_fb.m_projectBookmarksLabel->resized(root, labelRect);
    bookmarkRect.location[1] -= div;
    labelRect.location[1] -= div;

    m_fb.m_recentBookmarks.m_view->resized(root, bookmarkRect);
    m_fb.m_recentBookmarksLabel->resized(root, labelRect);
}

void FileBrowser::RightSide::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    float pf = rootView().viewRes().pixelFactor();

    boo::SWindowRect pathRect = sub;
    pathRect.location[0] += BROWSER_MARGIN * pf;
    pathRect.location[1] += pathRect.size[1] - (BROWSER_MARGIN + 20) * pf;
    pathRect.size[0] = sub.size[0] - m_fb.m_ok.m_button.m_view->nominalWidth() - 20 * pf;
    pathRect.size[1] = m_fb.m_fileField.m_view->nominalHeight();
    m_fb.m_pathButtons.m_view->resized(root, pathRect);

    pathRect.location[0] = sub.location[0] + BROWSER_MARGIN * pf;
    pathRect.location[1] -= 25 * pf;
    m_fb.m_fileField.m_view->resized(root, pathRect);

    pathRect.location[1] = sub.location[1] + BROWSER_MARGIN * pf;
    pathRect.size[0] = sub.size[0] - BROWSER_MARGIN * 2 * pf;
    pathRect.size[1] = sub.size[1] - (BROWSER_MARGIN + 56) * pf;
    m_fb.m_fileListing.m_view->resized(root, pathRect);

    boo::SWindowRect buttonRect = sub;
    buttonRect.size[0] = m_fb.m_ok.m_button.m_view->nominalWidth();
    buttonRect.size[1] = m_fb.m_ok.m_button.m_view->nominalHeight();
    buttonRect.location[0] += sub.size[0] - BROWSER_MARGIN * pf - buttonRect.size[0];
    buttonRect.location[1] += sub.size[1] - (BROWSER_MARGIN + 20) * pf;
    m_fb.m_ok.m_button.m_view->resized(root, buttonRect);
    buttonRect.location[1] -= 25 * pf;
    m_fb.m_cancel.m_button.m_view->resized(root, buttonRect);
}

void FileBrowser::think()
{
    ModalWindow::think();
    if (m_fileListingBind.m_needsUpdate)
        navigateToPath(m_path);
    m_pathButtons.m_view->think();
    m_fileField.m_view->think();
    m_fileListing.m_view->think();
    m_systemBookmarks.m_view->think();
    m_projectBookmarks.m_view->think();
    m_recentBookmarks.m_view->think();

    if (m_confirmWindow)
        m_confirmWindow->think();
}

void FileBrowser::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    ModalWindow::draw(gfxQ);
    m_split.m_view->draw(gfxQ);

    if (m_confirmWindow)
        m_confirmWindow->draw(gfxQ);
}

void FileBrowser::LeftSide::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    m_fb.m_systemBookmarks.m_view->draw(gfxQ);
    m_fb.m_systemBookmarksLabel->draw(gfxQ);
    m_fb.m_projectBookmarks.m_view->draw(gfxQ);
    m_fb.m_projectBookmarksLabel->draw(gfxQ);
    m_fb.m_recentBookmarks.m_view->draw(gfxQ);
    m_fb.m_recentBookmarksLabel->draw(gfxQ);
}

void FileBrowser::RightSide::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    m_fb.m_pathButtons.m_view->draw(gfxQ);
    m_fb.m_fileListing.m_view->draw(gfxQ);
    m_fb.m_ok.m_button.m_view->draw(gfxQ);
    m_fb.m_cancel.m_button.m_view->draw(gfxQ);
    m_fb.m_fileField.m_view->draw(gfxQ);
}

}
