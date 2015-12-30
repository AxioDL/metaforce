#include "Specter/FileBrowser.hpp"
#include "Specter/RootView.hpp"

namespace Specter
{

#define BROWSER_MARGIN 30
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
                         const HECL::SystemString& initialPath)
: ModalWindow(res, parentView, RectangleConstraint(BROWSER_MIN_WIDTH * res.pixelFactor(),
                                                   BROWSER_MIN_HEIGHT * res.pixelFactor(),
                                                   RectangleConstraint::Test::Minimum,
                                                   RectangleConstraint::Test::Minimum)),
  m_comps(PathComponents(initialPath)),
  m_left(*this, res),
  m_right(*this, res),
  m_ok(*this, res, title),
  m_cancel(*this, res),
  m_fileFieldBind(*this)
{
    commitResources(res);
    setBackground({0,0,0,0.5});

    m_pathButtons.reserve(m_comps.size());
    size_t idx = 0;
    for (const HECL::SystemString& c : m_comps)
        m_pathButtons.emplace_back(*this, res, idx++, c);

    m_fileField.m_view.reset(new TextField(res, *this, &m_fileFieldBind));
    m_fileListing.m_view.reset(new Table(res, *this, &m_fileListingBind));
    m_fileListing.m_view->setBackground(Zeus::CColor::skBlue);

    m_split.m_view.reset(new SplitView(res, *this, SplitView::Axis::Vertical,
                                       200 * res.pixelFactor(), 400 * res.pixelFactor()));
    m_split.m_view->setContentView(0, &m_left);
    m_split.m_view->setContentView(1, &m_right);
    m_split.m_view->setSlide(0.2);

    updateContentOpacity(0.0);
}

void FileBrowser::updateContentOpacity(float opacity)
{
    Zeus::CColor color = Zeus::CColor::lerp({1,1,1,0}, {1,1,1,1}, opacity);
    m_split.m_view->setMultiplyColor(color);
    for (PathButton& b : m_pathButtons)
        b.m_button.m_view->setMultiplyColor(color);
    m_fileField.m_view->setMultiplyColor(color);
    m_fileListing.m_view->setMultiplyColor(color);
    m_ok.m_button.m_view->setMultiplyColor(color);
    m_cancel.m_button.m_view->setMultiplyColor(color);
}

void FileBrowser::okActivated()
{
}

void FileBrowser::cancelActivated()
{
    close();
}

void FileBrowser::pathButtonActivated(size_t idx)
{
}

void FileBrowser::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (skipBuildInAnimation() || closed())
        return;
    m_split.mouseDown(coord, button, mod);
    for (PathButton& b : m_pathButtons)
        b.m_button.mouseDown(coord, button, mod);
    m_fileField.mouseDown(coord, button, mod);
    m_fileListing.mouseDown(coord, button, mod);
    m_ok.m_button.mouseDown(coord, button, mod);
    m_cancel.m_button.mouseDown(coord, button, mod);
}

void FileBrowser::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (closed())
        return;
    m_split.mouseUp(coord, button, mod);
    for (PathButton& b : m_pathButtons)
        b.m_button.mouseUp(coord, button, mod);
    m_fileField.mouseUp(coord, button, mod);
    m_fileListing.mouseUp(coord, button, mod);
    m_ok.m_button.mouseUp(coord, button, mod);
    m_cancel.m_button.mouseUp(coord, button, mod);
}

void FileBrowser::mouseMove(const boo::SWindowCoord& coord)
{
    if (closed())
        return;
    m_split.mouseMove(coord);
    for (PathButton& b : m_pathButtons)
        b.m_button.mouseMove(coord);
    m_fileField.mouseMove(coord);
    m_fileListing.mouseMove(coord);
    m_ok.m_button.mouseMove(coord);
    m_cancel.m_button.mouseMove(coord);
}

void FileBrowser::mouseEnter(const boo::SWindowCoord& coord)
{
}

void FileBrowser::mouseLeave(const boo::SWindowCoord& coord)
{
    if (closed())
        return;
    m_split.mouseLeave(coord);
    for (PathButton& b : m_pathButtons)
        b.m_button.mouseLeave(coord);
    m_fileField.mouseLeave(coord);
    m_fileListing.mouseLeave(coord);
    m_ok.m_button.mouseLeave(coord);
    m_cancel.m_button.mouseLeave(coord);
}

void FileBrowser::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
{
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

void FileBrowser::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    ModalWindow::resized(root, root);
    float pf = rootView().viewRes().pixelFactor();

    boo::SWindowRect centerRect = subRect();
    centerRect.location[0] = root.size[0] / 2 - (centerRect.size[0] / 2.0) + 2 * pf;
    centerRect.location[1] = root.size[1] / 2 - (centerRect.size[1] / 2.0) + 2 * pf;
    centerRect.size[0] -= 4 * pf;
    centerRect.size[1] -= 4 * pf;

    m_split.m_view->resized(root, centerRect);
}

void FileBrowser::LeftSide::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
}

void FileBrowser::RightSide::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    float pf = rootView().viewRes().pixelFactor();

    boo::SWindowRect pathRect = sub;
    pathRect.location[0] += 25 * pf;
    pathRect.location[1] += pathRect.size[1] - 50 * pf;
    for (PathButton& b : m_fb.m_pathButtons)
    {
        pathRect.size[0] = b.m_button.m_view->nominalWidth();
        pathRect.size[1] = b.m_button.m_view->nominalHeight();
        b.m_button.m_view->resized(root, pathRect);
        pathRect.location[0] += pathRect.size[0] + 2;
    }

    pathRect.location[0] = sub.location[0] + 25 * pf;
    pathRect.location[1] -= 25 * pf;
    pathRect.size[0] = sub.size[0] - 155 * pf;
    pathRect.size[1] = m_fb.m_fileField.m_view->nominalHeight();
    m_fb.m_fileField.m_view->resized(root, pathRect);

    pathRect.location[1] = sub.location[1] + 25 * pf;
    pathRect.size[0] = sub.size[0] - 50 * pf;
    pathRect.size[1] = sub.size[1] - 105 * pf;
    m_fb.m_fileListing.m_view->resized(root, pathRect);

    boo::SWindowRect buttonRect = sub;
    buttonRect.size[0] = m_fb.m_ok.m_button.m_view->nominalWidth();
    buttonRect.size[1] = m_fb.m_ok.m_button.m_view->nominalHeight();
    buttonRect.location[0] += sub.size[0] - 25 * pf - buttonRect.size[0];
    buttonRect.location[1] += sub.size[1] - 50 * pf;
    m_fb.m_ok.m_button.m_view->resized(root, buttonRect);
    buttonRect.location[1] -= 25 * pf;
    m_fb.m_cancel.m_button.m_view->resized(root, buttonRect);
}

void FileBrowser::think()
{
    ModalWindow::think();
    m_fileField.m_view->think();
    m_fileListing.m_view->think();
}

void FileBrowser::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    ModalWindow::draw(gfxQ);
    m_split.m_view->draw(gfxQ);
}

void FileBrowser::LeftSide::draw(boo::IGraphicsCommandQueue* gfxQ)
{
}

void FileBrowser::RightSide::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    for (PathButton& b : m_fb.m_pathButtons)
        b.m_button.m_view->draw(gfxQ);
    m_fb.m_fileField.m_view->draw(gfxQ);
    m_fb.m_fileListing.m_view->draw(gfxQ);
    m_fb.m_ok.m_button.m_view->draw(gfxQ);
    m_fb.m_cancel.m_button.m_view->draw(gfxQ);
}

}
