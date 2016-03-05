#include "ResourceBrowser.hpp"

namespace urde
{
#define BROWSER_MARGIN 8

bool ResourceBrowser::navigateToPath(const hecl::ProjectPath& pathIn)
{
    if (pathIn.getPathType() == hecl::ProjectPath::Type::File)
        m_path = pathIn.getParentPath();
    else
        m_path = pathIn;

    m_comps = m_path.getPathComponents();

    hecl::DirectoryEnumerator dEnum(m_path.getAbsolutePath(),
                                    hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted,
                                    m_state.sortColumn==State::SortColumn::Size,
                                    m_state.sortDir==specter::SortDirection::Descending &&
                                    (m_state.sortColumn == State::SortColumn::Name || m_state.sortColumn == State::SortColumn::Size),
                                    true);
    m_resListingBind.updateListing(dEnum);
    if (m_pathButtons)
        m_pathButtons->setButtons(m_comps);

    if (m_view)
    {
        m_view->m_resListing.m_view->selectRow(-1);
        m_view->m_resListing.m_view->updateData();
        m_view->updateSize();
    }

    return true;
}

void ResourceBrowser::pathButtonActivated(size_t idx)
{
    if (idx >= m_comps.size())
        return;

    hecl::SystemString dir;
    bool needSlash = false;
    size_t i = 0;
    for (const hecl::SystemString& d : m_comps)
    {
        if (needSlash)
            dir += _S('/');
        if (d.compare(_S("/")))
            needSlash = true;
        dir += d;
        if (++i > idx)
            break;
    }
    navigateToPath(hecl::ProjectPath(*m_vm.project(), dir));
}

void ResourceBrowser::View::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    m_resListing.mouseDown(coord, button, mod);
}

void ResourceBrowser::View::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    m_resListing.mouseUp(coord, button, mod);
}

void ResourceBrowser::View::mouseMove(const boo::SWindowCoord& coord)
{
    m_resListing.mouseMove(coord);
}

void ResourceBrowser::View::mouseLeave(const boo::SWindowCoord& coord)
{
    m_resListing.mouseLeave(coord);
}

void ResourceBrowser::View::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    specter::View::resized(root, sub);
    m_resListing.m_view->resized(root, sub);
}
void ResourceBrowser::View::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    m_resListing.m_view->draw(gfxQ);
}

}
