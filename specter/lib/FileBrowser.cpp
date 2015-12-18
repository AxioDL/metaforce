#include "Specter/FileBrowser.hpp"
#include "Specter/RootView.hpp"

namespace Specter
{

#define BROWSER_MARGIN 30
#define BROWSER_MIN_WIDTH 100
#define BROWSER_MIN_HEIGHT 100

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
        ret.push_back("/");
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

FileBrowser::FileBrowser(ViewResources& res, View& parentView, const HECL::SystemString& initialPath)
: ModalWindow(res, parentView),
  m_comps(PathComponents(initialPath)),
  m_fileFieldBind(*this)
{
    commitResources(res);
    setBackground({0,0,0,0.5});

    m_pathButtons.reserve(m_comps.size());
    size_t idx = 0;
    for (const HECL::SystemString& c : m_comps)
        m_pathButtons.emplace_back(*this, res, idx++, c);

    m_fileField.m_view.reset(new TextField(res, *this, &m_fileFieldBind));
    updateContentOpacity(0.0);
}

void FileBrowser::updateContentOpacity(float opacity)
{
    Zeus::CColor color = Zeus::CColor::lerp({1,1,1,0}, {1,1,1,1}, opacity);
    for (PathButton& b : m_pathButtons)
        b.m_button.m_view->setMultiplyColor(color);
    m_fileField.m_view->setMultiplyColor(color);
}

void FileBrowser::pathButtonActivated(size_t idx)
{
}

void FileBrowser::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (skipBuildInAnimation())
        return;
    for (PathButton& b : m_pathButtons)
        b.m_button.mouseDown(coord, button, mod);
    m_fileField.mouseDown(coord, button, mod);
}

void FileBrowser::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    for (PathButton& b : m_pathButtons)
        b.m_button.mouseUp(coord, button, mod);
    m_fileField.mouseUp(coord, button, mod);
}

void FileBrowser::mouseMove(const boo::SWindowCoord& coord)
{
    for (PathButton& b : m_pathButtons)
        b.m_button.mouseMove(coord);
    m_fileField.mouseMove(coord);
}

void FileBrowser::mouseEnter(const boo::SWindowCoord& coord)
{
}

void FileBrowser::mouseLeave(const boo::SWindowCoord& coord)
{
    for (PathButton& b : m_pathButtons)
        b.m_button.mouseLeave(coord);
    m_fileField.mouseLeave(coord);
}

void FileBrowser::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    ModalWindow::resized(root, root);
    float pf = rootView().viewRes().pixelFactor();

    boo::SWindowRect centerRect = sub;
    centerRect.size[0] = std::max(root.size[0] - BROWSER_MARGIN * 2 * pf, BROWSER_MIN_WIDTH * pf);
    centerRect.size[1] = std::max(root.size[1] - BROWSER_MARGIN * 2 * pf, BROWSER_MIN_HEIGHT * pf);
    centerRect.location[0] = root.size[0] / 2 - (centerRect.size[0] / 2.0);
    centerRect.location[1] = root.size[1] / 2 - (centerRect.size[1] / 2.0);

    boo::SWindowRect pathRect = centerRect;
    pathRect.location[0] += 10 * pf;
    pathRect.location[1] += pathRect.size[1] - 20 * pf;
    for (PathButton& b : m_pathButtons)
    {
        pathRect.size[0] = b.m_button.m_view->nominalWidth();
        pathRect.size[1] = b.m_button.m_view->nominalHeight();
        b.m_button.m_view->resized(root, pathRect);
        pathRect.location[0] += pathRect.size[0] + 2;
    }

    pathRect.location[0] = centerRect.location[0] + 10 * pf;
    pathRect.location[1] -= 25 * pf;
    pathRect.size[0] = centerRect.size[0] - 20 * pf;
    pathRect.size[1] = m_fileField.m_view->nominalHeight();
    m_fileField.m_view->resized(root, pathRect);
}

void FileBrowser::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    ModalWindow::draw(gfxQ);
    for (PathButton& b : m_pathButtons)
        b.m_button.m_view->draw(gfxQ);
    m_fileField.m_view->draw(gfxQ);
}

}
