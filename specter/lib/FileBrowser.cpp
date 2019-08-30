#include "specter/FileBrowser.hpp"

#include "specter/Button.hpp"
#include "specter/IViewManager.hpp"
#include "specter/MessageWindow.hpp"
#include "specter/RootView.hpp"
#include "specter/TextField.hpp"

#include <hecl/hecl.hpp>
#include <logvisor/logvisor.hpp>

namespace specter {
static logvisor::Module Log("specter::FileBrowser");

#define BROWSER_MARGIN 8
#define BROWSER_MIN_WIDTH 600
#define BROWSER_MIN_HEIGHT 300

std::vector<hecl::SystemString> FileBrowser::PathComponents(hecl::SystemStringView path) {
  std::vector<hecl::SystemString> ret;
  hecl::SystemString sPath(path);
  hecl::SanitizePath(sPath);
  if (sPath.empty())
    return ret;
  auto it = sPath.cbegin();
  if (*it == _SYS_STR('/')) {
    ret.push_back(_SYS_STR("/"));
    ++it;
  }
  hecl::SystemString comp;
  for (; it != sPath.cend(); ++it) {
    if (*it == _SYS_STR('/')) {
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

FileBrowser::FileBrowser(ViewResources& res, View& parentView, std::string_view title, Type type,
                         hecl::SystemStringView initialPath,
                         std::function<void(bool, hecl::SystemStringView)> returnFunc)
: ModalWindow(res, parentView,
              RectangleConstraint(BROWSER_MIN_WIDTH * res.pixelFactor(), BROWSER_MIN_HEIGHT * res.pixelFactor(),
                                  RectangleConstraint::Test::Minimum, RectangleConstraint::Test::Minimum))
, m_type(type)
, m_left(*this, res)
, m_right(*this, res)
, m_ok(*this, res, title)
, m_cancel(*this, res, rootView().viewManager().translate<locale::cancel>())
, m_fileFieldBind(*this, rootView().viewManager())
, m_fileListingBind(*this, rootView().viewManager())
, m_systemBookmarkBind(*this)
, m_projectBookmarkBind(*this)
, m_recentBookmarkBind(*this)
, m_returnFunc(returnFunc) {
  setBackground({0, 0, 0, 0.5});

  IViewManager& vm = rootView().viewManager();
  m_pathButtons.m_view.reset(new PathButtons(res, *this, *this));
  m_fileField.m_view.reset(new TextField(res, *this, &m_fileFieldBind));
  m_fileListing.m_view.reset(new Table(res, *this, &m_fileListingBind, &m_fileListingBind, 3));
  m_systemBookmarks.m_view.reset(new Table(res, *this, &m_systemBookmarkBind, &m_systemBookmarkBind, 1));
  m_systemBookmarksLabel.reset(new TextView(res, *this, res.m_mainFont));
  m_systemBookmarksLabel->typesetGlyphs(vm.translate<locale::system_locations>(),
                                        res.themeData().uiText());
  m_projectBookmarks.m_view.reset(new Table(res, *this, &m_projectBookmarkBind, &m_projectBookmarkBind, 1));
  m_projectBookmarksLabel.reset(new TextView(res, *this, res.m_mainFont));
  m_projectBookmarksLabel->typesetGlyphs(vm.translate<locale::recent_projects>(),
                                         res.themeData().uiText());
  m_recentBookmarks.m_view.reset(new Table(res, *this, &m_recentBookmarkBind, &m_recentBookmarkBind, 1));
  m_recentBookmarksLabel.reset(new TextView(res, *this, res.m_mainFont));
  m_recentBookmarksLabel->typesetGlyphs(vm.translate<locale::recent_files>(), res.themeData().uiText());

  /* Populate system bookmarks */
  std::vector<std::pair<hecl::SystemString, std::string>> systemLocs = hecl::GetSystemLocations();
  for (auto& loc : systemLocs)
    m_systemBookmarkBind.m_entries.emplace_back(std::move(loc));
  m_systemBookmarks.m_view->updateData();

  const std::vector<hecl::SystemString>* recentProjects = vm.recentProjects();

  for (auto& proj : *recentProjects)
    m_projectBookmarkBind.m_entries.emplace_back(proj);
  m_projectBookmarks.m_view->updateData();

  const std::vector<hecl::SystemString>* recentFiles = vm.recentFiles();

  for (auto& file : *recentFiles)
    m_recentBookmarkBind.m_entries.emplace_back(file);
  m_recentBookmarks.m_view->updateData();

  navigateToPath(initialPath);

  m_split.m_view.reset(new SplitView(res, *this, nullptr, SplitView::Axis::Vertical, 0.2, 200 * res.pixelFactor(),
                                     400 * res.pixelFactor()));
  m_split.m_view->setContentView(0, &m_left);
  m_split.m_view->setContentView(1, &m_right);

  updateContentOpacity(0.0);
}

FileBrowser::~FileBrowser() = default;

void FileBrowser::SyncBookmarkSelections(Table& table, BookmarkDataBind& binding, const hecl::SystemString& sel) {
  size_t idx = 0;
  for (const BookmarkDataBind::Entry& e : binding.m_entries) {
    if (e.m_path == sel) {
      table.selectRow(idx);
      break;
    }
    ++idx;
  }
  if (idx == binding.m_entries.size())
    table.selectRow(-1);
}

void FileBrowser::navigateToPath(hecl::SystemStringView path) {
  hecl::Sstat theStat;
  if (hecl::Stat(path.data(), &theStat))
    return;

  m_path = hecl::SystemString(path);
  m_comps = PathComponents(m_path);
  if (S_ISREG(theStat.st_mode)) {
    hecl::SystemUTF8Conv utf8(m_comps.back());
    m_fileField.m_view->setText(utf8.str());
    m_fileField.m_view->clearErrorState();
    m_comps.pop_back();
  }

  hecl::SystemString dir;
  bool needSlash = false;
  for (const hecl::SystemString& d : m_comps) {
    if (needSlash)
      dir += _SYS_STR('/');
    if (d.compare(_SYS_STR("/")))
      needSlash = true;
    dir += d;
  }

  SyncBookmarkSelections(*m_systemBookmarks.m_view, m_systemBookmarkBind, dir);
  SyncBookmarkSelections(*m_projectBookmarks.m_view, m_projectBookmarkBind, dir);
  SyncBookmarkSelections(*m_recentBookmarks.m_view, m_recentBookmarkBind, dir);

  hecl::DirectoryEnumerator dEnum(dir, hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted,
                                  m_fileListingBind.m_sizeSort,
                                  m_fileListingBind.m_sortDir == SortDirection::Descending, !m_showingHidden);
  m_fileListingBind.updateListing(dEnum);
  m_fileListing.m_view->selectRow(-1);
  m_fileListing.m_view->updateData();

  m_pathButtons.m_view->setButtons(m_comps);

  updateSize();
}

void FileBrowser::updateContentOpacity(float opacity) {
  zeus::CColor color = zeus::CColor::lerp({1, 1, 1, 0}, {1, 1, 1, 1}, opacity);
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

void FileBrowser::okActivated(bool viaButton) {
  IViewManager& vm = rootView().viewManager();

  hecl::SystemString path;
  bool needSlash = false;
  for (const hecl::SystemString& d : m_comps) {
    if (needSlash)
      path += _SYS_STR('/');
    if (d.compare(_SYS_STR("/")))
      needSlash = true;
    path += d;
  }

  hecl::Sstat theStat;
  if (hecl::Stat(path.c_str(), &theStat) || !S_ISDIR(theStat.st_mode)) {
    hecl::SystemUTF8Conv utf8(path);
    m_fileField.m_view->setErrorState(vm.translate<locale::no_access_as_dir>(utf8));
    return;
  }

  path += _SYS_STR('/');
  path += hecl::SystemStringConv(m_fileField.m_view->getText()).sys_str();

  int err = hecl::Stat(path.c_str(), &theStat);
  if (m_type == Type::SaveFile) {
    if (m_fileField.m_view->getText().empty()) {
      m_fileField.m_view->setErrorState(vm.translate<locale::file_field_empty>());
      return;
    }
    if (!err && !S_ISDIR(theStat.st_mode)) {
      m_confirmWindow.reset(
          new MessageWindow(rootView().viewRes(), *this, MessageWindow::Type::ConfirmOkCancel,
                            vm.translate<locale::overwrite_confirm>(hecl::SystemUTF8Conv(path)),
                            [&, path](bool ok) {
                              if (ok) {
                                m_returnFunc(true, path);
                                m_confirmWindow->close();
                                close();
                              } else
                                m_confirmWindow->close();
                            }));
      updateSize();
      return;
    }
    if (!err && S_ISDIR(theStat.st_mode)) {
      navigateToPath(path);
      return;
    }
    m_returnFunc(true, path);
    close();
    return;
  } else if (m_type == Type::SaveDirectory || m_type == Type::NewHECLProject) {
    if (m_fileField.m_view->getText().empty()) {
      m_fileField.m_view->setErrorState(vm.translate<locale::directory_field_empty>());
      return;
    }
    if (!err && !S_ISDIR(theStat.st_mode)) {
      m_fileField.m_view->setErrorState(vm.translate<locale::no_overwrite_file>());
      return;
    }
    if (!err && S_ISDIR(theStat.st_mode)) {
      if (m_type == Type::NewHECLProject) {
        hecl::ProjectRootPath projRoot = hecl::SearchForProject(path);
        if (projRoot) {
          m_fileField.m_view->setErrorState(vm.translate<locale::no_overwrite_project>());
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

  if (m_type == Type::OpenFile) {
    if (!err && S_ISDIR(theStat.st_mode)) {
      navigateToPath(path);
      return;
    } else if (err || !S_ISREG(theStat.st_mode)) {
      hecl::SystemUTF8Conv utf8(path);
      m_fileField.m_view->setErrorState(vm.translate<locale::no_access_as_file>(utf8));
      return;
    }
    m_returnFunc(true, path);
    close();
    return;
  } else if (m_type == Type::OpenDirectory || m_type == Type::OpenHECLProject) {
    if (!viaButton && !err && S_ISDIR(theStat.st_mode)) {
      if (m_type == Type::OpenHECLProject) {
        hecl::ProjectRootPath projRoot = hecl::SearchForProject(path);
        if (projRoot) {
          m_returnFunc(true, projRoot.getAbsolutePath());
          close();
          return;
        }
      }
      navigateToPath(path);
      return;
    }
    if (err || !S_ISDIR(theStat.st_mode)) {
      hecl::SystemUTF8Conv utf8(path);
      m_fileField.m_view->setErrorState(vm.translate<locale::no_access_as_dir>(utf8));
      return;
    }
    m_returnFunc(true, path);
    close();
    return;
  }
}

void FileBrowser::cancelActivated() {
  hecl::SystemString path;
  bool needSlash = false;
  for (const hecl::SystemString& d : m_comps) {
    if (needSlash)
      path += _SYS_STR('/');
    if (d.compare(_SYS_STR("/")))
      needSlash = true;
    path += d;
  }

  path += _SYS_STR('/');
  path += hecl::SystemStringConv(m_fileField.m_view->getText()).sys_str();

  m_returnFunc(false, path);
  close();
}

FileBrowser::FileFieldBind::FileFieldBind(FileBrowser& browser, const IViewManager& vm)
: m_browser(browser), m_name(vm.translate<locale::file_name>()) {}

void FileBrowser::pathButtonActivated(size_t idx) {
  if (idx >= m_comps.size())
    return;

  hecl::SystemString dir;
  bool needSlash = false;
  size_t i = 0;
  for (const hecl::SystemString& d : m_comps) {
    if (needSlash)
      dir += _SYS_STR('/');
    if (d.compare(_SYS_STR("/")))
      needSlash = true;
    dir += d;
    if (++i > idx)
      break;
  }
  navigateToPath(dir);
}

void FileBrowser::FileListingDataBind::updateListing(const hecl::DirectoryEnumerator& dEnum) {
  m_entries.clear();
  m_entries.reserve(dEnum.size());

  for (const hecl::DirectoryEnumerator::Entry& d : dEnum) {
    m_entries.emplace_back();
    Entry& ent = m_entries.back();
    ent.m_path = d.m_path;
    hecl::SystemUTF8Conv nameUtf8(d.m_name);
    ent.m_name = nameUtf8.str();
    if (d.m_isDir) {
      if (hecl::SearchForProject(d.m_path))
        ent.m_type = m_projStr;
      else
        ent.m_type = m_dirStr;
    } else {
      ent.m_type = m_fileStr;
      ent.m_size = hecl::HumanizeNumber(d.m_fileSz, 7, nullptr, int(hecl::HNScale::AutoScale),
                                        hecl::HNFlags::B | hecl::HNFlags::Decimal);
    }
  }

  m_needsUpdate = false;
}

void FileBrowser::FileListingDataBind::setSelectedRow(size_t rIdx) {
  if (rIdx != SIZE_MAX) {
    m_fb.m_fileField.m_view->setText(m_entries.at(rIdx).m_name);
  } else {
    m_fb.m_fileField.m_view->setText("");
  }

  m_fb.m_fileField.m_view->clearErrorState();
}

FileBrowser::FileListingDataBind::FileListingDataBind(FileBrowser& fb, const IViewManager& vm) : m_fb(fb) {
  m_nameCol = vm.translate<locale::name>();
  m_typeCol = vm.translate<locale::type>();
  m_sizeCol = vm.translate<locale::size>();
  m_dirStr = vm.translate<locale::directory>();
  m_projStr = vm.translate<locale::hecl_project>();
  m_fileStr = vm.translate<locale::file>();
}

void FileBrowser::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod) {
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

void FileBrowser::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod) {
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

void FileBrowser::mouseMove(const boo::SWindowCoord& coord) {
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

void FileBrowser::mouseLeave(const boo::SWindowCoord& coord) {
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

void FileBrowser::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll) {
  m_pathButtons.scroll(coord, scroll);
  m_fileListing.scroll(coord, scroll);
}

void FileBrowser::touchDown(const boo::STouchCoord& coord, uintptr_t tid) {}

void FileBrowser::touchUp(const boo::STouchCoord& coord, uintptr_t tid) {}

void FileBrowser::touchMove(const boo::STouchCoord& coord, uintptr_t tid) {}

void FileBrowser::charKeyDown(unsigned long charcode, boo::EModifierKey mod, bool isRepeat) {
  if (skipBuildInAnimation() || closed())
    return;
  if (True(mod & boo::EModifierKey::CtrlCommand) && !isRepeat) {
    if (charcode == 'h' || charcode == 'H')
      setShowingHidden(!showingHidden());
    else if (charcode == 'r' || charcode == 'R')
      navigateToPath(m_path);
  }
}

void FileBrowser::specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mod, bool isRepeat) {
  if (skipBuildInAnimation() || closed())
    return;
  if (key == boo::ESpecialKey::Enter && !isRepeat)
    okActivated(true);
}

void FileBrowser::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) {
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

void FileBrowser::LeftSide::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) {
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

void FileBrowser::RightSide::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) {
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

void FileBrowser::think() {
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

void FileBrowser::draw(boo::IGraphicsCommandQueue* gfxQ) {
  ModalWindow::draw(gfxQ);
  m_split.m_view->draw(gfxQ);

  if (m_confirmWindow)
    m_confirmWindow->draw(gfxQ);
}

void FileBrowser::LeftSide::draw(boo::IGraphicsCommandQueue* gfxQ) {
  m_fb.m_systemBookmarks.m_view->draw(gfxQ);
  m_fb.m_systemBookmarksLabel->draw(gfxQ);
  m_fb.m_projectBookmarks.m_view->draw(gfxQ);
  m_fb.m_projectBookmarksLabel->draw(gfxQ);
  m_fb.m_recentBookmarks.m_view->draw(gfxQ);
  m_fb.m_recentBookmarksLabel->draw(gfxQ);
}

void FileBrowser::RightSide::draw(boo::IGraphicsCommandQueue* gfxQ) {
  m_fb.m_pathButtons.m_view->draw(gfxQ);
  m_fb.m_fileListing.m_view->draw(gfxQ);
  m_fb.m_ok.m_button.m_view->draw(gfxQ);
  m_fb.m_cancel.m_button.m_view->draw(gfxQ);
  m_fb.m_fileField.m_view->draw(gfxQ);
}

FileBrowser::OKButton::OKButton(FileBrowser& fb, ViewResources& res, std::string_view text) : m_fb(fb), m_text(text) {
  m_button.m_view.reset(
      new Button(res, fb, this, text, nullptr, Button::Style::Block, zeus::skWhite,
                 RectangleConstraint(100 * res.pixelFactor(), -1, RectangleConstraint::Test::Minimum)));
}

FileBrowser::CancelButton::CancelButton(FileBrowser& fb, ViewResources& res, std::string_view text)
: m_fb(fb), m_text(text) {
  m_button.m_view.reset(new Button(
      res, fb, this, text, nullptr, Button::Style::Block, zeus::skWhite,
      RectangleConstraint(m_fb.m_ok.m_button.m_view->nominalWidth(), -1, RectangleConstraint::Test::Minimum)));
}

} // namespace specter
